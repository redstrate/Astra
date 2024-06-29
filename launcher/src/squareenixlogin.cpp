// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "squareenixlogin.h"

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
#include "astra_log.h"
#include "launchercore.h"
#include "utility.h"

using namespace Qt::StringLiterals;

SquareEnixLogin::SquareEnixLogin(LauncherCore &window, QObject *parent)
    : QObject(parent)
    , m_launcher(window)
{
}

QCoro::Task<std::optional<LoginAuth>> SquareEnixLogin::login(LoginInformation *info)
{
    Q_ASSERT(info != nullptr);
    m_info = info;

    // First, let's check for boot updates. While not technically required for us, it's needed for later hash checking.
    // It's also a really good idea anyway, in case the official launcher is needed.
    while (m_lastRunHasPatched) {
        // There seems to be a limitation in their boot patching system.
        // Their server can only give one patch a time, so the boot process must keep trying to patch until
        // there is no patches left.
        if (!co_await checkBootUpdates()) {
            co_return std::nullopt;
        }
    }

    // Then check if we can even login.
    if (!co_await checkLoginStatus()) {
        co_return std::nullopt;
    }

    // Login with through the oauth API. This gives us some information like a temporary SID, region and expansion information
    if (!co_await loginOAuth()) {
        co_return std::nullopt;
    }

    // Register the session with the server. This method also updates the game as necessary.
    if (!co_await registerSession()) {
        co_return std::nullopt;
    }

    // Finally, double check the *world* status to make sure we don't try to log in during maintenance.
    // Doing it late here ensures we handle cases where the patch is available during maintenance (like during expansion launches)
    // but stops before trying to log in when you're not supposed to.
    if (!co_await checkGateStatus()) {
        co_return std::nullopt;
    }

    co_return m_auth;
}

QCoro::Task<bool> SquareEnixLogin::checkGateStatus()
{
    Q_EMIT m_launcher.stageChanged(i18n("Checking gate..."));
    qInfo(ASTRA_LOG) << "Checking if the gate is open...";

    QUrl url;
    url.setScheme(m_launcher.settings()->preferredProtocol());
    url.setHost(QStringLiteral("frontier.%1").arg(m_launcher.settings()->squareEnixServer()));
    url.setPath(QStringLiteral("/worldStatus/gate_status.json"));
    url.setQuery(QString::number(QDateTime::currentMSecsSinceEpoch()));

    QNetworkRequest request(url);

    // TODO: really?
    m_launcher.buildRequest(*m_info->profile, request);

    Utility::printRequest(QStringLiteral("GET"), request);

    const auto reply = m_launcher.mgr()->get(request);
    m_launcher.setupIgnoreSSL(reply);
    co_await reply;

    const QJsonDocument document = QJsonDocument::fromJson(reply->readAll());
    if (document.isEmpty()) {
        Q_EMIT m_launcher.loginError(i18n("An error occured when checking login gate status:\n\n%1", reply->errorString()));
        co_return false;
    }

    const bool isGateOpen = !document.isEmpty() && document.object()["status"_L1].toInt() != 0;

    if (isGateOpen) {
        qInfo(ASTRA_LOG) << "Gate is open!";
        co_return true;
    }

    qInfo(ASTRA_LOG) << "Gate is closed!";
    Q_EMIT m_launcher.loginError(i18n("The login gate is closed, the game may be under maintenance."));

    co_return false;
}

QCoro::Task<bool> SquareEnixLogin::checkLoginStatus()
{
    Q_EMIT m_launcher.stageChanged(i18n("Checking login..."));
    qInfo(ASTRA_LOG) << "Checking if login is open...";

    QUrl url;
    url.setScheme(m_launcher.settings()->preferredProtocol());
    url.setHost(QStringLiteral("frontier.%1").arg(m_launcher.settings()->squareEnixServer()));
    url.setPath(QStringLiteral("/worldStatus/login_status.json"));
    url.setQuery(QString::number(QDateTime::currentMSecsSinceEpoch()));

    QNetworkRequest request(url);

    // TODO: really?
    m_launcher.buildRequest(*m_info->profile, request);

    Utility::printRequest(QStringLiteral("GET"), request);

    const auto reply = m_launcher.mgr()->get(request);
    m_launcher.setupIgnoreSSL(reply);
    co_await reply;

    const QJsonDocument document = QJsonDocument::fromJson(reply->readAll());
    const bool isGateOpen = !document.isEmpty() && document.object()["status"_L1].toInt() != 0;

    if (isGateOpen) {
        qInfo(ASTRA_LOG) << "Login is open!";
        co_return true;
    } else {
        qInfo(ASTRA_LOG) << "Lgoin is closed!";
        Q_EMIT m_launcher.loginError(i18n("The login gate is closed, the game may be under maintenance.\n\n%1", reply->errorString()));
        co_return false;
    }
}

QCoro::Task<bool> SquareEnixLogin::checkBootUpdates()
{
    m_lastRunHasPatched = false;

    Q_EMIT m_launcher.stageChanged(i18n("Checking for launcher updates..."));
    qInfo(ASTRA_LOG) << "Checking for updates to boot components...";

    QString formattedDate = QDateTime::currentDateTimeUtc().toString(QStringLiteral("yyyy-MM-dd-HH-mm"));
    formattedDate[15] = '0'_L1;

    const QUrlQuery query{{QStringLiteral("time"), formattedDate}};

    QUrl url;
    url.setScheme(QStringLiteral("http"));
    url.setHost(QStringLiteral("patch-bootver.%1").arg(m_launcher.settings()->squareEnixServer()));
    url.setPath(QStringLiteral("/http/win32/ffxivneo_release_boot/%1/").arg(m_info->profile->bootVersion()));
    url.setQuery(query);

    auto request = QNetworkRequest(url);
    if (m_info->profile->account()->license() == Account::GameLicense::macOS) {
        request.setRawHeader(QByteArrayLiteral("User-Agent"), QByteArrayLiteral("FFXIV-MAC PATCH CLIENT"));
    } else {
        request.setRawHeader(QByteArrayLiteral("User-Agent"), QByteArrayLiteral("FFXIV PATCH CLIENT"));
    }

    request.setRawHeader(QByteArrayLiteral("Host"), QStringLiteral("patch-bootver.%1").arg(m_launcher.settings()->squareEnixServer()).toUtf8());
    Utility::printRequest(QStringLiteral("GET"), request);

    const auto reply = m_launcher.mgr()->get(request);
    co_await reply;

    if (reply->error() == QNetworkReply::NoError) {
        const QString patchList = QString::fromUtf8(reply->readAll());
        if (!patchList.isEmpty()) {
            m_patcher = new Patcher(m_launcher, m_info->profile->gamePath() + QStringLiteral("/boot"), *m_info->profile->bootData(), this);
            const bool hasPatched = co_await m_patcher->patch(PatchList(patchList));
            if (hasPatched) {
                // update game version information
                m_info->profile->readGameVersion();
            }
            m_patcher->deleteLater();
            m_lastRunHasPatched = true;
        }
    } else {
        qWarning(ASTRA_LOG) << "Unknown error when verifying boot files:" << reply->errorString();
        Q_EMIT m_launcher.loginError(i18n("Unknown error when verifying boot files.\n\n%1", reply->errorString()));
        co_return false;
    }

    co_return true;
}

QCoro::Task<std::optional<SquareEnixLogin::StoredInfo>> SquareEnixLogin::getStoredValue()
{
    qInfo(ASTRA_LOG) << "Getting the STORED value...";

    Q_EMIT m_launcher.stageChanged(i18n("Logging in..."));

    QUrlQuery query;
    // en is always used to the top url
    query.addQueryItem(QStringLiteral("lng"), QStringLiteral("en"));
    // for some reason, we always use region 3. the actual region is acquired later
    query.addQueryItem(QStringLiteral("rgn"), QString::number(3));
    query.addQueryItem(QStringLiteral("isft"), QString::number(m_info->profile->account()->isFreeTrial() ? 1 : 0));
    query.addQueryItem(QStringLiteral("cssmode"), QString::number(1));
    query.addQueryItem(QStringLiteral("isnew"), QString::number(1));
    query.addQueryItem(QStringLiteral("launchver"), QString::number(3));

    if (m_info->profile->account()->license() == Account::GameLicense::WindowsSteam) {
        query.addQueryItem(QStringLiteral("issteam"), QString::number(1));

        // TODO: get steam ticket information from steam api
        query.addQueryItem(QStringLiteral("session_ticket"), QString::number(1));
        query.addQueryItem(QStringLiteral("ticket_size"), QString::number(1));
    }

    QUrl url;
    url.setScheme(m_launcher.settings()->preferredProtocol());
    url.setHost(QStringLiteral("ffxiv-login.%1").arg(m_launcher.settings()->squareEnixLoginServer()));
    url.setPath(QStringLiteral("/oauth/ffxivarr/login/top"));
    url.setQuery(query);

    auto request = QNetworkRequest(url);
    m_launcher.buildRequest(*m_info->profile, request);

    Utility::printRequest(QStringLiteral("GET"), request);

    const auto reply = m_launcher.mgr()->get(request);
    co_await reply;

    const QString str = QString::fromUtf8(reply->readAll());

    // fetches Steam username
    if (m_info->profile->account()->license() == Account::GameLicense::WindowsSteam) {
        const QRegularExpression re(QStringLiteral(R"lit(<input name=""sqexid"" type=""hidden"" value=""(?<sqexid>.*)""\/>)lit"));
        const QRegularExpressionMatch match = re.match(str);

        if (match.hasMatch()) {
            m_username = match.captured(1);
        } else {
            Q_EMIT m_launcher.loginError(i18n("Could not get Steam username, have you attached your account?"));
        }
    } else {
        m_username = m_info->username;
    }

    const QRegularExpression re(QStringLiteral(R"lit(\t<\s*input .* name="_STORED_" value="(?<stored>.*)">)lit"));
    const QRegularExpressionMatch match = re.match(str);
    if (match.hasMatch()) {
        co_return StoredInfo{match.captured(1), url};
    } else {
        Q_EMIT m_launcher.loginError(
            i18n("Square Enix servers refused to confirm session information. The game may be under maintenance, try the official launcher."));
        co_return {};
    }
}

QCoro::Task<bool> SquareEnixLogin::loginOAuth()
{
    const auto storedResult = co_await getStoredValue();
    if (storedResult == std::nullopt) {
        co_return false;
    }

    const auto [stored, referer] = *storedResult;

    qInfo(ASTRA_LOG) << "Logging in...";

    QUrlQuery postData;
    postData.addQueryItem(QStringLiteral("_STORED_"), stored);
    postData.addQueryItem(QStringLiteral("sqexid"), m_info->username);
    postData.addQueryItem(QStringLiteral("password"), m_info->password);
    postData.addQueryItem(QStringLiteral("otppw"), m_info->oneTimePassword);

    QUrl url;
    url.setScheme(m_launcher.settings()->preferredProtocol());
    url.setHost(QStringLiteral("ffxiv-login.%1").arg(m_launcher.settings()->squareEnixLoginServer()));
    url.setPath(QStringLiteral("/oauth/ffxivarr/login/login.send"));

    QNetworkRequest request(url);
    m_launcher.buildRequest(*m_info->profile, request);
    request.setHeader(QNetworkRequest::ContentTypeHeader, QByteArrayLiteral("application/x-www-form-urlencoded"));
    request.setRawHeader(QByteArrayLiteral("Referer"), referer.toEncoded());
    request.setRawHeader(QByteArrayLiteral("Cache-Control"), QByteArrayLiteral("no-cache"));

    Utility::printRequest(QStringLiteral("POST"), request);

    const auto reply = m_launcher.mgr()->post(request, postData.toString(QUrl::FullyEncoded).toUtf8());
    m_launcher.setupIgnoreSSL(reply);
    co_await reply;

    const QString str = QString::fromUtf8(reply->readAll());

    const QRegularExpression re(QStringLiteral(R"lit(window.external.user\("login=auth,ok,(?<launchParams>.*)\);)lit"));
    const QRegularExpressionMatch match = re.match(str);
    if (match.hasMatch()) {
        const auto parts = match.captured(1).split(','_L1);

        const bool terms = parts[3] == "1"_L1;
        const bool playable = parts[9] == "1"_L1;

        if (!playable) {
            Q_EMIT m_launcher.loginError(i18n("Your account is unplayable. Check that you have the correct license, and a valid subscription."));
            co_return false;
        }

        if (!terms) {
            Q_EMIT m_launcher.loginError(i18n("Your account is unplayable. You need to accept the terms of service from the official launcher first."));
            co_return false;
        }

        m_SID = parts[1];
        m_auth.region = parts[5].toInt();
        m_auth.maxExpansion = parts[13].toInt();

        co_return true;
    } else {
        const QRegularExpression errorRe(QStringLiteral(R"lit(window.external.user\("login=auth,ng,err,(?<launchParams>.*)\);)lit"));
        const QRegularExpressionMatch errorMatch = errorRe.match(str);

        if (errorMatch.hasCaptured(1)) {
            // there's a stray quote at the end of the error string, so let's remove that
            Q_EMIT m_launcher.loginError(errorMatch.captured(1).chopped(1));
        } else {
            Q_EMIT m_launcher.loginError(i18n("Unknown error"));
        }

        co_return false;
    }
}

QCoro::Task<bool> SquareEnixLogin::registerSession()
{
    qInfo(ASTRA_LOG) << "Registering the session...";

    QUrl url;
    url.setScheme(m_launcher.settings()->preferredProtocol());
    url.setHost(QStringLiteral("patch-gamever.%1").arg(m_launcher.settings()->squareEnixServer()));
    url.setPath(QStringLiteral("/http/win32/ffxivneo_release_game/%1/%2").arg(m_info->profile->baseGameVersion(), m_SID));

    auto request = QNetworkRequest(url);
    Utility::setSSL(request);
    request.setRawHeader(QByteArrayLiteral("X-Hash-Check"), QByteArrayLiteral("enabled"));
    request.setRawHeader(QByteArrayLiteral("User-Agent"), QByteArrayLiteral("FFXIV PATCH CLIENT"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, QByteArrayLiteral("application/x-www-form-urlencoded"));

    QString report = QStringLiteral("%1=%2\n").arg(m_info->profile->bootVersion(), co_await getBootHash());
    for (int i = 0; i < m_auth.maxExpansion; i++) {
        if (i < static_cast<int>(m_info->profile->numInstalledExpansions())) {
            report += QStringLiteral("ex%1\t%2\n").arg(QString::number(i + 1), m_info->profile->expansionVersion(i));
        } else {
            report += QStringLiteral("ex%1\t2012.01.01.0000.0000\n").arg(QString::number(i + 1));
        }
    }

    Utility::printRequest(QStringLiteral("POST"), request);

    const auto reply = m_launcher.mgr()->post(request, report.toUtf8());
    co_await reply;

    if (reply->error() == QNetworkReply::NoError) {
        QString patchUniqueId;
        if (reply->rawHeaderList().contains(QByteArrayLiteral("X-Patch-Unique-Id"))) {
            patchUniqueId = QString::fromUtf8(reply->rawHeader(QByteArrayLiteral("X-Patch-Unique-Id")));
        } else if (reply->rawHeaderList().contains(QByteArrayLiteral("x-patch-unique-id"))) {
            patchUniqueId = QString::fromUtf8(reply->rawHeader(QByteArrayLiteral("x-patch-unique-id")));
        }

        if (!patchUniqueId.isEmpty()) {
            const QString body = QString::fromUtf8(reply->readAll());

            if (!body.isEmpty()) {
                m_patcher = new Patcher(m_launcher, m_info->profile->gamePath() + QStringLiteral("/game"), *m_info->profile->gameData(), this);
                const bool hasPatched = co_await m_patcher->patch(PatchList(body));
                if (hasPatched) {
                    // re-read game version if it has updated
                    m_info->profile->readGameVersion();
                }
                m_patcher->deleteLater();
            }

            m_auth.SID = patchUniqueId;

            co_return true;
        } else {
            Q_EMIT m_launcher.loginError(i18n("Fatal error, request was successful but X-Patch-Unique-Id was not recieved."));
        }
    } else {
        if (reply->error() == QNetworkReply::SslHandshakeFailedError) {
            Q_EMIT m_launcher.loginError(
                i18n("SSL handshake error detected. If you are using OpenSUSE or Fedora, try running `update-crypto-policies --set LEGACY`."));
        } else if (reply->error() == QNetworkReply::ContentConflictError) {
            Q_EMIT m_launcher.loginError(i18n("The boot files are outdated, please login in again to update them."));
        } else if (reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() == 405) {
            Q_EMIT m_launcher.loginError(i18n("The game failed the anti-tamper check. Restore the game to the original state and try updating again."));
        } else if (reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() == 410) {
            Q_EMIT m_launcher.loginError(i18n("This game version is no longer supported."));
        } else {
            Q_EMIT m_launcher.loginError(i18n("Unknown error when registering the session."));
        }
    }

    co_return false;
}

QCoro::Task<QString> SquareEnixLogin::getBootHash()
{
    const QList<QString> fileList = {QStringLiteral("ffxivboot.exe"),
                                     QStringLiteral("ffxivboot64.exe"),
                                     QStringLiteral("ffxivlauncher.exe"),
                                     QStringLiteral("ffxivlauncher64.exe"),
                                     QStringLiteral("ffxivupdater.exe"),
                                     QStringLiteral("ffxivupdater64.exe")};

    const auto hashFuture = QtConcurrent::mapped(fileList, [this](const auto &filename) -> QString {
        return getFileHash(m_info->profile->gamePath() + QStringLiteral("/boot/") + filename);
    });

    co_await hashFuture;
    const QList<QString> hashes = hashFuture.results();

    QString result;
    for (int i = 0; i < fileList.count(); i++) {
        if (!hashes[i].isEmpty()) {
            result += fileList[i] + QStringLiteral("/") + hashes[i];

            if (i != fileList.length() - 1)
                result += QStringLiteral(",");
        }
    }

    co_return result;
}

QString SquareEnixLogin::getFileHash(const QString &file)
{
    auto f = QFile(file);
    if (!f.open(QIODevice::ReadOnly))
        return {};

    QCryptographicHash hash(QCryptographicHash::Sha1);
    hash.addData(&f);

    return QStringLiteral("%1/%2").arg(QString::number(f.size()), QString::fromUtf8(hash.result().toHex()));
}

#include "moc_squareenixlogin.cpp"