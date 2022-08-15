#pragma once

#include <QCheckBox>
#include <QComboBox>
#include <QFormLayout>
#include <QGridLayout>
#include <QMainWindow>
#include <QPushButton>
#include <QScrollArea>
#include <QTreeWidget>

#include "headline.h"
#include "launchercore.h"

class LauncherWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit LauncherWindow(LauncherCore& core, QWidget* parent = nullptr);

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
    QLineEdit *usernameEdit, *passwordEdit;
    QLineEdit* otpEdit;
    QCheckBox *rememberUsernameBox, *rememberPasswordBox;
    QPushButton *loginButton, *registerButton;

#if defined(Q_OS_MAC) || defined(Q_OS_LINUX)
    QAction* wineCfg;
#endif
};