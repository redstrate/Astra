#include "account.h"

#include <QEventLoop>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <qt5keychain/keychain.h>

#include "cotp.h"
#include "launchercore.h"

Account::Account(LauncherCore &launcher, const QString &key, QObject *parent)
    : QObject(parent)
    , m_config(key)
    , m_key(key)
    , m_launcher(launcher)
{
    fetchAvatar();
}

QString Account::uuid() const
{
    return m_key;
}

QString Account::name() const
{
    return m_config.name();
}

void Account::setName(const QString &name)
{
    if (m_config.name() != name) {
        m_config.setName(name);
        m_config.save();
        Q_EMIT nameChanged();
    }
}

QString Account::lodestoneId() const
{
    return m_config.lodestoneId();
}

void Account::setLodestoneId(const QString &id)
{
    if (m_config.lodestoneId() != id) {
        m_config.setLodestoneId(id);
        m_config.save();
        fetchAvatar();
        Q_EMIT lodestoneIdChanged();
    }
}

QString Account::avatarUrl() const
{
    return m_url.toString();
}

bool Account::isSapphire() const
{
    return m_config.isSapphire();
}

void Account::setIsSapphire(bool value)
{
    if (m_config.isSapphire() != value) {
        m_config.setIsSapphire(value);
        m_config.save();
        Q_EMIT isSapphireChanged();
    }
}

QString Account::lobbyUrl() const
{
    return m_config.lobbyUrl();
}

void Account::setLobbyUrl(const QString &value)
{
    if (m_config.lobbyUrl() != value) {
        m_config.setLobbyUrl(value);
        m_config.save();
        Q_EMIT lobbyUrlChanged();
    }
}

bool Account::rememberPassword() const
{
    return m_config.rememberPassword();
}

void Account::setRememberPassword(const bool value)
{
    if (m_config.rememberPassword() != value) {
        m_config.setRememberPassword(value);
        m_config.save();
        Q_EMIT rememberPasswordChanged();
    }
}

bool Account::rememberOTP() const
{
    return m_config.rememberOTP();
}

void Account::setRememberOTP(const bool value)
{
    if (m_config.rememberOTP() != value) {
        m_config.setRememberOTP(value);
        m_config.save();
        Q_EMIT rememberOTPChanged();
    }
}

bool Account::useOTP() const
{
    return m_config.useOTP();
}

void Account::setUseOTP(const bool value)
{
    if (m_config.useOTP() != value) {
        m_config.setUseOTP(value);
        m_config.save();
        Q_EMIT useOTPChanged();
    }
}

Account::GameLicense Account::license() const
{
    return static_cast<GameLicense>(m_config.license());
}

void Account::setLicense(const GameLicense license)
{
    if (static_cast<GameLicense>(m_config.license()) != license) {
        m_config.setLicense(static_cast<int>(license));
        m_config.save();
        Q_EMIT licenseChanged();
    }
}

bool Account::isFreeTrial() const
{
    return m_config.isFreeTrial();
}

void Account::setIsFreeTrial(const bool value)
{
    if (m_config.isFreeTrial() != value) {
        m_config.setIsFreeTrial(value);
        m_config.save();
        Q_EMIT isFreeTrialChanged();
    }
}

QString Account::getPassword() const
{
    return getKeychainValue("password");
}

void Account::setPassword(const QString &password)
{
    setKeychainValue("password", password);
}

QString Account::getOTP() const
{
    auto otpSecret = getKeychainValue("otp-secret");

    char *totp = get_totp(otpSecret.toStdString().c_str(), 6, 30, SHA1, nullptr);
    QString totpStr(totp);
    free(totp);

    return totpStr;
}

void Account::fetchAvatar()
{
    if (lodestoneId().isEmpty()) {
        return;
    }

    QNetworkRequest request(QStringLiteral("https://xivapi.com/character/%1").arg(lodestoneId()));
    auto reply = m_launcher.mgr->get(request);
    connect(reply, &QNetworkReply::finished, [this, reply] {
        auto document = QJsonDocument::fromJson(reply->readAll());
        if (document.isObject()) {
            m_url = document.object()["Character"].toObject()["Avatar"].toString();
            Q_EMIT avatarUrlChanged();
        }
    });
}

void Account::setKeychainValue(const QString &key, const QString &value) const
{
    auto job = new QKeychain::WritePasswordJob("Astra");
    job->setTextData(value);
    job->setKey(m_key + "-" + key);
    job->start();
}

QString Account::getKeychainValue(const QString &key) const
{
    auto loop = new QEventLoop();

    auto job = new QKeychain::ReadPasswordJob("Astra");
    job->setKey(m_key + "-" + key);
    job->start();

    QString value;

    QObject::connect(job, &QKeychain::ReadPasswordJob::finished, [loop, job, &value](QKeychain::Job *j) {
        Q_UNUSED(j)
        value = job->textData();
        loop->quit();
    });

    loop->exec();

    return value;
}
