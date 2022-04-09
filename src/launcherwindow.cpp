#include "launcherwindow.h"

#include <QMenuBar>
#include <keychain.h>
#include <QFormLayout>
#include <QApplication>
#include <QDesktopServices>
#include <QNetworkReply>
#include <QTreeWidgetItem>
#include <QHeaderView>

#include "settingswindow.h"
#include "squareboot.h"
#include "squarelauncher.h"
#include "sapphirelauncher.h"
#include "assetupdater.h"
#include "headline.h"
#include "config.h"

LauncherWindow::LauncherWindow(LauncherCore& core, QWidget* parent) : QMainWindow(parent), core(core) {
    setWindowTitle("Astra");

    connect(&core, &LauncherCore::settingsChanged, this, &LauncherWindow::reloadControls);

    QMenu* toolsMenu = menuBar()->addMenu("Tools");

    launchOfficial = toolsMenu->addAction("Open Official Client...");
    launchOfficial->setIcon(QIcon::fromTheme("application-x-executable"));
    connect(launchOfficial, &QAction::triggered, [=] {
        this->core.launchExecutable(currentProfile(), {currentProfile().gamePath + "/boot/ffxivboot.exe"});
    });

    launchSysInfo = toolsMenu->addAction("Open System Info...");
    launchSysInfo->setIcon(QIcon::fromTheme("application-x-executable"));
    connect(launchSysInfo, &QAction::triggered, [=] {
        this->core.launchExecutable(currentProfile(), {currentProfile().gamePath + "/boot/ffxivsysinfo64.exe"});
    });

    launchCfgBackup = toolsMenu->addAction("Open Config Backup...");
    launchCfgBackup->setIcon(QIcon::fromTheme("application-x-executable"));
    connect(launchCfgBackup, &QAction::triggered, [=] {
        this->core.launchExecutable(currentProfile(), {currentProfile().gamePath + "/boot/ffxivconfig64.exe"});
    });

    toolsMenu->addSeparator();

    openGameDir = toolsMenu->addAction("Open Game Directory...");
    openGameDir->setIcon(QIcon::fromTheme("document-open"));
    connect(openGameDir, &QAction::triggered, [=] {
        openPath(currentProfile().gamePath);
    });

    QMenu* fileMenu = menuBar()->addMenu("Settings");

    QAction* settingsAction = fileMenu->addAction("Configure Astra...");
    settingsAction->setIcon(QIcon::fromTheme("settings"));
    connect(settingsAction, &QAction::triggered, [=] {
        auto window = new SettingsWindow(0, *this, this->core, this);
        connect(&this->core, &LauncherCore::settingsChanged, window, &SettingsWindow::reloadControls);
        window->show();
    });

    QAction* profilesAction = fileMenu->addAction("Configure Profiles...");
    profilesAction->setIcon(QIcon::fromTheme("settings"));
    connect(profilesAction, &QAction::triggered, [=] {
        auto window = new SettingsWindow(1, *this, this->core, this);
        connect(&this->core, &LauncherCore::settingsChanged, window, &SettingsWindow::reloadControls);
        window->show();
    });

#if defined(Q_OS_MAC) || defined(Q_OS_LINUX)
    fileMenu->addSeparator();

    wineCfg = fileMenu->addAction("Configure Wine...");
    wineCfg->setIcon(QIcon::fromTheme("settings"));
    connect(wineCfg, &QAction::triggered, [=] {
        this->core.launchExternalTool(currentProfile(), {"winecfg.exe"});
    });
#endif

    QMenu* helpMenu = menuBar()->addMenu("Help");
    QAction* showAbout = helpMenu->addAction("About Astra");
    showAbout->setIcon(QIcon::fromTheme("help-about"));
    connect(showAbout, &QAction::triggered, [=] {
        QString aboutText;
        aboutText.append(QString("Version: %1\n").arg(version));
        aboutText.append("The source code is available at https://sr.ht/~redstrate/astra.");

        QMessageBox::about(this, "About Astra", aboutText);
    });

    QAction* showAboutQt = helpMenu->addAction("About Qt");
    showAboutQt->setIcon(QIcon::fromTheme("help-about"));
    connect(showAboutQt, &QAction::triggered, [=] {
        QMessageBox::aboutQt(this);
    });

    auto layout = new QGridLayout();

    bannerImageView = new QLabel();
    layout->addWidget(bannerImageView, 0, 0);

    newsListView = new QTreeWidget();
    newsListView->setColumnCount(2);
    newsListView->setHeaderLabels({"Title", "Date"});
    newsListView->header()->setStretchLastSection(true);
    newsListView->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    connect(newsListView, &QTreeWidget::itemClicked, [](QTreeWidgetItem* item, int column) {
        auto url = item->data(0, Qt::UserRole).toUrl();
        qInfo() << "clicked" << url;
        QDesktopServices::openUrl(url);
    });
    layout->addWidget(newsListView, 1, 0);

    auto loginLayout = new QFormLayout();
    layout->addLayout(loginLayout, 0, 1, 1, 1);

    profileSelect = new QComboBox();
    connect(profileSelect, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), [=](int index) {
        reloadControls();
    });

    loginLayout->addRow("Profile", profileSelect);

    usernameEdit = new QLineEdit();
    loginLayout->addRow("Username", usernameEdit);

    rememberUsernameBox = new QCheckBox();
    connect(rememberUsernameBox, &QCheckBox::stateChanged, [=](int) {
        currentProfile().rememberUsername = rememberUsernameBox->isChecked();
        this->core.saveSettings();
    });
    loginLayout->addRow("Remember Username?", rememberUsernameBox);

    passwordEdit = new QLineEdit();
    passwordEdit->setEchoMode(QLineEdit::EchoMode::Password);
    loginLayout->addRow("Password", passwordEdit);

    rememberPasswordBox = new QCheckBox();
    connect(rememberPasswordBox, &QCheckBox::stateChanged, [=](int) {
        currentProfile().rememberPassword = rememberPasswordBox->isChecked();
        this->core.saveSettings();
    });
    loginLayout->addRow("Remember Password?", rememberPasswordBox);

    otpEdit = new QLineEdit();
    loginLayout->addRow("One-Time Password", otpEdit);

    loginButton = new QPushButton("Login");
    loginLayout->addRow(loginButton);

    registerButton = new QPushButton("Register");
    loginLayout->addRow(registerButton);

    auto emptyWidget = new QWidget();
    emptyWidget->setLayout(layout);
    setCentralWidget(emptyWidget);

    connect(core.assetUpdater, &AssetUpdater::finishedUpdating, [=] {
        auto info = LoginInformation{&currentProfile(), usernameEdit->text(), passwordEdit->text(), otpEdit->text()};

#ifndef QT_DEBUG
        if(currentProfile().rememberUsername) {
            auto job = new QKeychain::WritePasswordJob("LauncherWindow");
            job->setTextData(usernameEdit->text());
            job->setKey(currentProfile().name + "-username");
            job->start();
        }
#endif

#ifndef QT_DEBUG
        if(currentProfile().rememberPassword) {
            auto job = new QKeychain::WritePasswordJob("LauncherWindow");
            job->setTextData(passwordEdit->text());
            job->setKey(currentProfile().name + "-password");
            job->start();
        }
#endif

        if(currentProfile().isSapphire) {
            //this->core.sapphireLauncher->login(currentProfile().lobbyURL, info);
        } else {
            this->core.squareBoot->bootCheck(info);
        }
    });

    connect(loginButton, &QPushButton::released, [=] {
        // update the assets first if needed, then it calls the slot above :-)
        this->core.assetUpdater->update(currentProfile());
    });

    connect(registerButton, &QPushButton::released, [=] {
        if(currentProfile().isSapphire) {
            auto info = LoginInformation{&currentProfile(), usernameEdit->text(), passwordEdit->text(), otpEdit->text()};
            this->core.sapphireLauncher->registerAccount(currentProfile().lobbyURL, info);
        }
    });

    connect(&core, &LauncherCore::successfulLaunch, [&] {
        hide();
    });

    connect(&core, &LauncherCore::gameClosed, [&] {
        if(core.appSettings.closeWhenLaunched)
            QCoreApplication::quit();
    });

    reloadControls();

    getHeadline(core, [&](Headline headline) {
        this->headline = headline;

        if(!headline.banner.empty()) {
            auto request = QNetworkRequest(headline.banner[0].bannerImage);
            core.buildRequest(currentProfile(), request);

            auto reply = core.mgr->get(request);
            connect(reply, &QNetworkReply::finished, [=] {
                QPixmap pixmap;
                pixmap.loadFromData(reply->readAll());
                bannerImageView->setPixmap(pixmap);
            });

            QTreeWidgetItem* newsItem = new QTreeWidgetItem((QTreeWidgetItem*)nullptr, QStringList("News"));
            for(auto news : headline.news) {
                QTreeWidgetItem* item = new QTreeWidgetItem();
                item->setText(0, news.title);
                item->setText(1, QLocale().toString(news.date, QLocale::ShortFormat));
                item->setData(0, Qt::UserRole, news.url);

                newsItem->addChild(item);
            }

            QTreeWidgetItem* pinnedItem = new QTreeWidgetItem((QTreeWidgetItem*)nullptr, QStringList("Pinned"));
            for(auto pinned : headline.pinned) {
                QTreeWidgetItem* item = new QTreeWidgetItem();
                item->setText(0, pinned.title);
                item->setText(1, QLocale().toString(pinned.date, QLocale::ShortFormat));
                item->setData(0, Qt::UserRole, pinned.url);

                pinnedItem->addChild(item);
            }

            QTreeWidgetItem* topicsItem = new QTreeWidgetItem((QTreeWidgetItem*)nullptr, QStringList("Topics"));
            for(auto news : headline.topics) {
                QTreeWidgetItem* item = new QTreeWidgetItem();
                item->setText(0, news.title);
                item->setText(1, QLocale().toString(news.date, QLocale::ShortFormat));
                item->setData(0, Qt::UserRole, news.url);

                qInfo() << news.url;

                topicsItem->addChild(item);
            }

            newsListView->insertTopLevelItems(0, QList<QTreeWidgetItem*>({newsItem, pinnedItem, topicsItem}));
        }
    });
}

ProfileSettings LauncherWindow::currentProfile() const {
    return core.getProfile(profileSelect->currentIndex());
}

ProfileSettings& LauncherWindow::currentProfile() {
    return core.getProfile(profileSelect->currentIndex());
}

void LauncherWindow::reloadControls() {
    if(currentlyReloadingControls)
        return;

    currentlyReloadingControls = true;

    const int oldIndex = profileSelect->currentIndex();

    profileSelect->clear();

    for(const auto& profile : core.profileList()) {
        profileSelect->addItem(profile);
    }

    profileSelect->setCurrentIndex(oldIndex);

    if(profileSelect->currentIndex() == -1) {
        profileSelect->setCurrentIndex(core.defaultProfileIndex);
    }

    rememberUsernameBox->setChecked(currentProfile().rememberUsername);
#ifndef QT_DEBUG
    if(currentProfile().rememberUsername) {
        auto job = new QKeychain::ReadPasswordJob("LauncherWindow");
        job->setKey(currentProfile().name + "-username");
        job->start();

        connect(job, &QKeychain::ReadPasswordJob::finished, [=](QKeychain::Job* j) {
            usernameEdit->setText(job->textData());
        });
    }
#endif

    rememberPasswordBox->setChecked(currentProfile().rememberPassword);
#ifndef QT_DEBUG
    if(currentProfile().rememberPassword) {
        auto job = new QKeychain::ReadPasswordJob("LauncherWindow");
        job->setKey(currentProfile().name + "-password");
        job->start();

        connect(job, &QKeychain::ReadPasswordJob::finished, [=](QKeychain::Job* j) {
            passwordEdit->setText(job->textData());
        });
    }
#endif

    const bool canLogin = currentProfile().isSapphire || (!currentProfile().isSapphire && core.squareLauncher->isGateOpen) && currentProfile().isGameInstalled();

    if(canLogin) {
        loginButton->setText("Login");
    } else if(!core.squareLauncher->isGateOpen) {
        loginButton->setText("Login (Maintenance is in progress)");
#if defined(Q_OS_LINUX) || defined(Q_OS_MAC)
    } else if(!currentProfile().isWineInstalled()) {
        loginButton->setText("Login (Wine is not installed)");
#endif
    } else {
        loginButton->setText("Login (Game is not installed)");
    }

    loginButton->setEnabled(canLogin);
    registerButton->setEnabled(currentProfile().isSapphire);
    otpEdit->setEnabled(!currentProfile().isSapphire);

    launchOfficial->setEnabled(currentProfile().isGameInstalled());
    launchSysInfo->setEnabled(currentProfile().isGameInstalled());
    launchCfgBackup->setEnabled(currentProfile().isGameInstalled());
    openGameDir->setEnabled(currentProfile().isGameInstalled());

#if defined(Q_OS_MAC) || defined(Q_OS_LINUX)
    wineCfg->setEnabled(currentProfile().isWineInstalled());
#endif

    currentlyReloadingControls = false;
}

void LauncherWindow::openPath(const QString path) {
#if defined(Q_OS_WIN)
    // for some reason, windows requires special treatment (what else is new?)
    const QFileInfo fileInfo(path);

    QProcess::startDetached("explorer.exe", QStringList(QDir::toNativeSeparators(fileInfo.canonicalFilePath())));
#else
    QDesktopServices::openUrl("file://" + path);
#endif
}
