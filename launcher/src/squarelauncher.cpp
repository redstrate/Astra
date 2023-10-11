// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "squarelauncher.h"

#include <KLocalizedString>
#include <QDesktopServices>
#include <QFile>
#include <QNetworkReply>
#include <QRegularExpressionMatch>
#include <QUrlQuery>
#include <QtConcurrentMap>
#include <qcorofuture.h>
#include <qcoronetworkreply.h>

#include "account.h"
#include "launchercore.h"
#include "utility.h"

SquareLauncher::SquareLauncher(LauncherCore &window, QObject *parent)
    : QObject(parent)
    , window(window)
{
}

QString getFileHash(const QString &file)
{
    auto f = QFile(file);
    if (!f.open(QIODevice::ReadOnly))
        return {};

    QCryptographicHash hash(QCryptographicHash::Sha1);
    hash.addData(&f);

    return QStringLiteral("%1/%2").arg(QString::number(f.size()), hash.result().toHex());
}

QCoro::Task<std::optional<SquareLauncher::StoredInfo>> SquareLauncher::getStored(const LoginInformation &info)
{
    Q_EMIT window.stageChanged(i18n("Logging in..."));

    QUrlQuery query;
    // en is always used to the top url
    query.addQueryItem(QStringLiteral("lng"), QStringLiteral("en"));
    // for some reason, we always use region 3. the actual region is acquired later
    query.addQueryItem(QStringLiteral("rgn"), QStringLiteral("3"));
    query.addQueryItem(QStringLiteral("isft"), info.profile->account()->isFreeTrial() ? QStringLiteral("1") : QStringLiteral("0"));
    query.addQueryItem(QStringLiteral("cssmode"), QStringLiteral("1"));
    query.addQueryItem(QStringLiteral("isnew"), QStringLiteral("1"));
    query.addQueryItem(QStringLiteral("launchver"), QStringLiteral("3"));

    if (info.profile->account()->license() == Account::GameLicense::WindowsSteam) {
        query.addQueryItem(QStringLiteral("issteam"), QStringLiteral("1"));

        // TODO: get steam ticket information from steam api
        query.addQueryItem(QStringLiteral("session_ticket"), QStringLiteral("1"));
        query.addQueryItem(QStringLiteral("ticket_size"), QStringLiteral("1"));
    }

    QUrl url;
    url.setScheme(window.settings()->preferredProtocol());
    url.setHost(QStringLiteral("ffxiv-login.%1").arg(window.settings()->squareEnixLoginServer()));
    url.setPath(QStringLiteral("/oauth/ffxivarr/login/top"));
    url.setQuery(query);

    auto request = QNetworkRequest(url);
    window.buildRequest(*info.profile, request);

    Utility::printRequest(QStringLiteral("GET"), request);

    const auto reply = window.mgr()->get(request);
    co_await reply;

    const QString str = reply->readAll();

    // fetches Steam username
    if (info.profile->account()->license() == Account::GameLicense::WindowsSteam) {
        const QRegularExpression re(QStringLiteral(R"lit(<input name=""sqexid"" type=""hidden"" value=""(?<sqexid>.*)""\/>)lit"));
        const QRegularExpressionMatch match = re.match(str);

        if (match.hasMatch()) {
            username = match.captured(1);
        } else {
            Q_EMIT window.loginError(i18n("Could not get Steam username, have you attached your account?"));
        }
    } else {
        username = info.username;
    }

    const QRegularExpression re(QStringLiteral(R"lit(\t<\s*input .* name="_STORED_" value="(?<stored>.*)">)lit"));
    const QRegularExpressionMatch match = re.match(str);
    if (match.hasMatch()) {
        co_return StoredInfo{match.captured(1), url};
    } else {
        Q_EMIT window.loginError(
            i18n("Square Enix servers refused to confirm session information. The game may be under maintenance, try the official launcher."));
        co_return {};
    }
}

QCoro::Task<> SquareLauncher::login(const LoginInformation &info)
{
    const auto storedResult = co_await getStored(info);
    if (storedResult == std::nullopt) {
        co_return;
    }

    const auto [stored, referer] = *storedResult;

    qInfo() << "Performing oauth...";

    QUrlQuery postData;
    postData.addQueryItem(QStringLiteral("_STORED_"), stored);
    postData.addQueryItem(QStringLiteral("sqexid"), info.username);
    postData.addQueryItem(QStringLiteral("password"), info.password);
    postData.addQueryItem(QStringLiteral("otppw"), info.oneTimePassword);

    QUrl url;
    url.setScheme(QStringLiteral("https"));
    url.setHost(QStringLiteral("ffxiv-login.%1").arg(window.settings()->squareEnixLoginServer()));
    url.setPath(QStringLiteral("/oauth/ffxivarr/login/login.send"));

    QNetworkRequest request(url);
    window.buildRequest(*info.profile, request);
    request.setHeader(QNetworkRequest::ContentTypeHeader, QByteArrayLiteral("application/x-www-form-urlencoded"));
    request.setRawHeader(QByteArrayLiteral("Referer"), referer.toEncoded());
    request.setRawHeader(QByteArrayLiteral("Cache-Control"), QByteArrayLiteral("no-cache"));

    Utility::printRequest(QStringLiteral("POST"), request);

    const auto reply = window.mgr()->post(request, postData.toString(QUrl::FullyEncoded).toUtf8());
    window.setupIgnoreSSL(reply);
    co_await reply;

    const QString str = reply->readAll();

    const QRegularExpression re(QStringLiteral(R"lit(window.external.user\("login=auth,ok,(?<launchParams>.*)\);)lit"));
    const QRegularExpressionMatch match = re.match(str);
    if (match.hasMatch()) {
        const auto parts = match.captured(1).split(QLatin1Char(','));

        const bool terms = parts[3] == QLatin1String("1");
        const bool playable = parts[9] == QLatin1String("1");

        if (!playable) {
            Q_EMIT window.loginError(i18n("Your account is unplayable. Check that you have the correct license, and a valid subscription."));
            co_return;
        }

        if (!terms) {
            Q_EMIT window.loginError(i18n("Your account is unplayable. You need to accept the terms of service from the official launcher first."));
            co_return;
        }

        SID = parts[1];
        auth.region = parts[5].toInt();
        auth.maxExpansion = parts[13].toInt();

        registerSession(info);
    } else {
        const QRegularExpression re(QStringLiteral(R"lit(window.external.user\("login=auth,ng,err,(?<launchParams>.*)\);)lit"));
        const QRegularExpressionMatch match = re.match(str);

        // there's a stray quote at the end of the error string, so let's remove that
        Q_EMIT window.loginError(match.captured(1).chopped(1));
    }
}

QCoro::Task<> SquareLauncher::registerSession(const LoginInformation &info)
{
    QUrl url;
    url.setScheme(QStringLiteral("https"));
    url.setHost(QStringLiteral("patch-gamever.%1").arg(window.settings()->squareEnixServer()));
    url.setPath(QStringLiteral("/http/win32/ffxivneo_release_game/%1/%2").arg(info.profile->baseGameVersion(), SID));

    auto request = QNetworkRequest(url);
    window.setSSL(request);
    request.setRawHeader(QByteArrayLiteral("X-Hash-Check"), QByteArrayLiteral("enabled"));
    request.setRawHeader(QByteArrayLiteral("User-Agent"), QByteArrayLiteral("FFXIV PATCH CLIENT"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, QByteArrayLiteral("application/x-www-form-urlencoded"));

    QString report = QStringLiteral("%1=%2").arg(info.profile->bootVersion(), co_await getBootHash(info));
    for (int i = 0; i < auth.maxExpansion; i++) {
        if (i < static_cast<int>(info.profile->numInstalledExpansions())) {
            report += QStringLiteral("\nex%1\t%2").arg(QString::number(i + 1), info.profile->expansionVersion(i));
        } else {
            report += QStringLiteral("\nex%1\t2012.01.01.0000.0000").arg(QString::number(i + 1));
        }
    }

    Utility::printRequest(QStringLiteral("POST"), request);

    const auto reply = window.mgr()->post(request, report.toUtf8());
    co_await reply;

    if (reply->error() == QNetworkReply::NoError) {
        QString patchUniqueId;
        if (reply->rawHeaderList().contains(QByteArrayLiteral("X-Patch-Unique-Id"))) {
            patchUniqueId = reply->rawHeader(QByteArrayLiteral("X-Patch-Unique-Id"));
        } else if (reply->rawHeaderList().contains(QByteArrayLiteral("x-patch-unique-id"))) {
            patchUniqueId = reply->rawHeader(QByteArrayLiteral("x-patch-unique-id"));
        }

        if (!patchUniqueId.isEmpty()) {
            const QString body = reply->readAll();

            if (!body.isEmpty()) {
                patcher = new Patcher(window, info.profile->gamePath() + QStringLiteral("/game"), *info.profile->gameData(), this);
                const bool hasPatched = co_await patcher->patch(PatchList(body));
                if (hasPatched) {
                    // re-read game version if it has updated
                    info.profile->readGameVersion();
                }
                patcher->deleteLater();
            }

            auth.SID = patchUniqueId;

            window.launchGame(*info.profile, auth);
        } else {
            Q_EMIT window.loginError(i18n("Fatal error, request was successful but X-Patch-Unique-Id was not recieved."));
        }
    } else {
        if (reply->error() == QNetworkReply::SslHandshakeFailedError) {
            Q_EMIT window.loginError(
                i18n("SSL handshake error detected. If you are using OpenSUSE or Fedora, try running `update-crypto-policies --set LEGACY`."));
        } else if (reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() == 405) {
            Q_EMIT window.loginError(i18n("The game failed the anti-tamper check. Restore the game to the original state and try updating again."));
        } else if (reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() == 410) {
            Q_EMIT window.loginError(i18n("This game version is no longer supported."));
        } else {
            Q_EMIT window.loginError(i18n("Unknown error when registering the session."));
        }
    }
}

QCoro::Task<QString> SquareLauncher::getBootHash(const LoginInformation &info)
{
    const QList<QString> fileList = {QStringLiteral("ffxivboot.exe"),
                                     QStringLiteral("ffxivboot64.exe"),
                                     QStringLiteral("ffxivlauncher.exe"),
                                     QStringLiteral("ffxivlauncher64.exe"),
                                     QStringLiteral("ffxivupdater.exe"),
                                     QStringLiteral("ffxivupdater64.exe")};

    const auto hashFuture = QtConcurrent::mapped(fileList, [&info](const auto &filename) -> QString {
        return getFileHash(info.profile->gamePath() + QStringLiteral("/boot/") + filename);
    });

    co_await hashFuture;
    const QList<QString> hashes = hashFuture.results();

    QString result;
    for (int i = 0; i < fileList.count(); i++) {
        result += fileList[i] + QStringLiteral("/") + hashes[i];

        if (i != fileList.length() - 1)
            result += QStringLiteral(",");
    }

    co_return result;
}