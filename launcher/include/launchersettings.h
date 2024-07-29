// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QtQml>

#include "config.h"
#include "profile.h"

class LauncherSettings : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Use LauncherCore.settings")

    Q_PROPERTY(bool closeWhenLaunched READ closeWhenLaunched WRITE setCloseWhenLaunched NOTIFY closeWhenLaunchedChanged)
    Q_PROPERTY(bool showNews READ showNews WRITE setShowNews NOTIFY showNewsChanged)
    Q_PROPERTY(bool showDevTools READ showDevTools WRITE setShowDevTools NOTIFY showDevToolsChanged)
    Q_PROPERTY(bool keepPatches READ keepPatches WRITE setKeepPatches NOTIFY keepPatchesChanged)
    Q_PROPERTY(QString dalamudDistribServer READ dalamudDistribServer WRITE setDalamudDistribServer NOTIFY dalamudDistribServerChanged)
    Q_PROPERTY(QString squareEnixServer READ squareEnixServer WRITE setSquareEnixServer NOTIFY squareEnixServerChanged)
    Q_PROPERTY(QString squareEnixLoginServer READ squareEnixLoginServer WRITE setSquareEnixLoginServer NOTIFY squareEnixLoginServerChanged)
    Q_PROPERTY(QString mainServer READ mainServer WRITE setMainServer NOTIFY mainServerChanged)
    Q_PROPERTY(QString preferredProtocol READ preferredProtocol WRITE setPreferredProtocol NOTIFY preferredProtocolChanged)
    Q_PROPERTY(QString screenshotDir READ screenshotDir WRITE setScreenshotDir NOTIFY screenshotDirChanged)
    Q_PROPERTY(bool argumentsEncrypted READ argumentsEncrypted WRITE setArgumentsEncrypted NOTIFY encryptedArgumentsChanged)
    Q_PROPERTY(bool enableRenderDocCapture READ enableRenderDocCapture WRITE setEnableRenderDocCapture NOTIFY enableRenderDocCaptureChanged)

public:
    explicit LauncherSettings(QObject *parent = nullptr);

    [[nodiscard]] bool closeWhenLaunched() const;
    void setCloseWhenLaunched(bool value);

    [[nodiscard]] bool showNews() const;
    void setShowNews(bool value);

    [[nodiscard]] bool showDevTools() const;
    void setShowDevTools(bool value);

    [[nodiscard]] bool keepPatches() const;
    void setKeepPatches(bool value);

    [[nodiscard]] QString dalamudDistribServer() const;
    void setDalamudDistribServer(const QString &value);
    Q_INVOKABLE QString defaultDalamudDistribServer() const;

    [[nodiscard]] QString squareEnixServer() const;
    void setSquareEnixServer(const QString &value);
    Q_INVOKABLE QString defaultSquareEnixServer() const;

    [[nodiscard]] QString squareEnixLoginServer() const;
    void setSquareEnixLoginServer(const QString &value);
    Q_INVOKABLE QString defaultSquareEnixLoginServer() const;

    [[nodiscard]] QString mainServer() const;
    void setMainServer(const QString &value);
    Q_INVOKABLE QString defaultMainServer() const;

    [[nodiscard]] QString preferredProtocol() const;
    void setPreferredProtocol(const QString &value);
    Q_INVOKABLE QString defaultPreferredProtocol() const;

    [[nodiscard]] QString screenshotDir() const;
    void setScreenshotDir(const QString &value);

    [[nodiscard]] bool argumentsEncrypted() const;
    void setArgumentsEncrypted(bool value);

    [[nodiscard]] bool enableRenderDocCapture() const;
    void setEnableRenderDocCapture(bool value);

    [[nodiscard]] QString currentProfile() const;
    void setCurrentProfile(const QString &value);

    Config *config();

Q_SIGNALS:
    void closeWhenLaunchedChanged();
    void showNewsChanged();
    void showDevToolsChanged();
    void keepPatchesChanged();
    void dalamudDistribServerChanged();
    void squareEnixServerChanged();
    void squareEnixLoginServerChanged();
    void mainServerChanged();
    void preferredProtocolChanged();
    void screenshotDirChanged();
    void encryptedArgumentsChanged();
    void enableRenderDocCaptureChanged();

private:
    Config *m_config = nullptr;
};