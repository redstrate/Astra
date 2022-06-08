#pragma once

#include <QMainWindow>
#include <QComboBox>
#include <QCheckBox>
#include <QPushButton>
#include <QTreeWidget>
#include <QGridLayout>
#include <QFormLayout>
#include <QScrollArea>

#include "launchercore.h"
#include "headline.h"

class LauncherWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit LauncherWindow(LauncherCore& core, QWidget* parent = nullptr);

    ProfileSettings currentProfile() const;
    ProfileSettings& currentProfile();

    void openPath(const QString path);

public slots:
    void reloadControls();

private:
    void reloadNews();

    LauncherCore& core;

    Headline headline;

    bool currentlyReloadingControls = false;

    QGridLayout* layout;
    QFormLayout* loginLayout;

    QScrollArea* bannerScrollArea;
    QWidget* bannerParentWidget;
    QHBoxLayout* bannerLayout;
    QTreeWidget* newsListView;
    QTimer* bannerTimer = nullptr;
    int currentBanner = 0;

    std::vector<QLabel*> bannerWidgets;

    QAction* launchOfficial;
    QAction* launchSysInfo;
    QAction* launchCfgBackup;
    QAction* openGameDir;

    QComboBox* profileSelect;
    QLineEdit* usernameEdit, *passwordEdit;
    QLineEdit* otpEdit;
    QCheckBox* rememberUsernameBox, *rememberPasswordBox;
    QPushButton* loginButton, *registerButton;

#if defined(Q_OS_MAC) || defined(Q_OS_LINUX)
    QAction* wineCfg;
#endif
};