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
#include "aboutwindow.h"
#include "gameinstaller.h"

LauncherWindow::LauncherWindow(LauncherCore& core, QWidget* parent) : QMainWindow(parent), core(core) {
    setWindowTitle("Astra");

    connect(&core, &LauncherCore::settingsChanged, this, &LauncherWindow::reloadControls);

    QMenu* toolsMenu = menuBar()->addMenu("Tools");

    launchOfficial = toolsMenu->addAction("Open Official Client...");
    launchOfficial->setIcon(QIcon::fromTheme("application-x-executable"));
    connect(launchOfficial, &QAction::triggered, [=] {
        this->core.launchExecutable(currentProfile(), {currentProfile().gamePath + "/boot/ffxivboot64.exe"});
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

    QMenu* gameMenu = menuBar()->addMenu("Game");

    auto installGameAction = gameMenu->addAction("Install game...");
    connect(installGameAction, &QAction::triggered, [this] {
        // TODO: lol duplication
        auto messageBox = new QMessageBox(this);
        messageBox->setIcon(QMessageBox::Icon::Question);
        messageBox->setText("Warning");
        messageBox->setInformativeText("FFXIV will be installed to your selected game directory.");

        QString detailedText = QString("Astra will install FFXIV for you at '%1'").arg(this->currentProfile().gamePath);
        detailedText.append("\n\nIf you do not wish to install it to this location, please change your profile settings.");

        messageBox->setDetailedText(detailedText);
        messageBox->setWindowModality(Qt::WindowModal);

        auto installButton = messageBox->addButton("Install Game", QMessageBox::YesRole);
        connect(installButton, &QPushButton::clicked, [this, messageBox] {
            installGame(this->core, this->currentProfile(), [this, messageBox] {
                this->core.readGameVersion();

                messageBox->close();

            });
        });

        messageBox->addButton(QMessageBox::StandardButton::No);
        messageBox->setDefaultButton(installButton);

        messageBox->exec();
    });

    QMenu* fileMenu = menuBar()->addMenu("Settings");

    QAction* settingsAction = fileMenu->addAction("Configure Astra...");
    settingsAction->setIcon(QIcon::fromTheme("configure"));
    settingsAction->setMenuRole(QAction::MenuRole::PreferencesRole);
    connect(settingsAction, &QAction::triggered, [=] {
        auto window = new SettingsWindow(0, *this, this->core, this);
        connect(&this->core, &LauncherCore::settingsChanged, window, &SettingsWindow::reloadControls);
        window->show();
    });

    QAction* profilesAction = fileMenu->addAction("Configure Profiles...");
    profilesAction->setIcon(QIcon::fromTheme("configure"));
    profilesAction->setMenuRole(QAction::MenuRole::NoRole);
    connect(profilesAction, &QAction::triggered, [=] {
        auto window = new SettingsWindow(1, *this, this->core, this);
        connect(&this->core, &LauncherCore::settingsChanged, window, &SettingsWindow::reloadControls);
        window->show();
    });

#if defined(Q_OS_MAC) || defined(Q_OS_LINUX)
    fileMenu->addSeparator();

    wineCfg = fileMenu->addAction("Configure Wine...");
    wineCfg->setMenuRole(QAction::MenuRole::NoRole);
    wineCfg->setIcon(QIcon::fromTheme("configure"));
    connect(wineCfg, &QAction::triggered, [=] {
        this->core.launchExternalTool(currentProfile(), {"regedit.exe"});
    });
#endif

    QMenu* helpMenu = menuBar()->addMenu("Help");
    QAction* showAbout = helpMenu->addAction("About Astra");
    showAbout->setIcon(QIcon::fromTheme("help-about"));
    connect(showAbout, &QAction::triggered, [=] {
        auto window = new AboutWindow(this);
        window->show();
    });

    QAction* showAboutQt = helpMenu->addAction("About Qt");
    showAboutQt->setIcon(QIcon::fromTheme("help-about"));
    connect(showAboutQt, &QAction::triggered, [=] {
        QMessageBox::aboutQt(this);
    });

    layout = new QGridLayout();

    bannerImageView = new QLabel();

    newsListView = new QTreeWidget();
    newsListView->setColumnCount(2);
    newsListView->setHeaderLabels({"Title", "Date"});
    //newsListView->header()->setStretchLastSection(true);
    //newsListView->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    connect(newsListView, &QTreeWidget::itemClicked, [](QTreeWidgetItem* item, int column) {
        auto url = item->data(0, Qt::UserRole).toUrl();
        qInfo() << "clicked" << url;
        QDesktopServices::openUrl(url);
    });

    loginLayout = new QFormLayout();
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
    loginButton = new QPushButton("Login");
    registerButton = new QPushButton("Register");

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

    bool canLogin = true;
    if(currentProfile().isSapphire) {
        if(currentProfile().lobbyURL.isEmpty()) {
            loginButton->setText("Login (Lobby URL is invalid)");
            canLogin = false;
        }
    } else {
        if(!core.squareLauncher->isGateOpen) {
            loginButton->setText("Login (Maintenance is in progress)");
            canLogin = false;
        }
    }

#if defined(Q_OS_LINUX) || defined(Q_OS_MAC)
    if(!currentProfile().isWineInstalled()) {
        loginButton->setText("Login (Wine is not installed)");
        canLogin = false;
    }
#endif
    if(!currentProfile().isGameInstalled()) {
        loginButton->setText("Login (Game is not installed)");
        canLogin = false;
    }

    if(canLogin)
        loginButton->setText("Login");

    launchOfficial->setEnabled(currentProfile().isGameInstalled());
    launchSysInfo->setEnabled(currentProfile().isGameInstalled());
    launchCfgBackup->setEnabled(currentProfile().isGameInstalled());
    openGameDir->setEnabled(currentProfile().isGameInstalled());

#if defined(Q_OS_MAC) || defined(Q_OS_LINUX)
    wineCfg->setEnabled(currentProfile().isWineInstalled());
#endif

    layout->removeWidget(bannerImageView);
    bannerImageView->hide();
    layout->removeWidget(newsListView);
    newsListView->hide();

    auto field = loginLayout->labelForField(otpEdit);
    if(field != nullptr)
        field->deleteLater();

    loginLayout->takeRow(otpEdit);
    otpEdit->hide();

    if(currentProfile().useOneTimePassword && !currentProfile().isSapphire) {
        loginLayout->addRow("One-Time Password", otpEdit);
        otpEdit->show();
    }

    loginLayout->takeRow(loginButton);
    loginButton->setEnabled(canLogin);
    registerButton->setEnabled(canLogin);
    loginLayout->addRow(loginButton);

    loginLayout->takeRow(registerButton);
    registerButton->hide();

    if(currentProfile().isSapphire) {
        loginLayout->addRow(registerButton);
        registerButton->show();
    }

    if(core.appSettings.showBanners || core.appSettings.showNewsList) {
        int totalRow = 0;
        if(core.appSettings.showBanners) {
            bannerImageView->show();
            layout->addWidget(bannerImageView, totalRow++, 0);
        }

        if(core.appSettings.showNewsList) {
            newsListView->show();
            layout->addWidget(newsListView, totalRow++, 0);
        }

        newsListView->clear();

        getHeadline(core, [&](Headline headline) {
            this->headline = headline;

            if (!headline.banner.empty()) {
                if(core.appSettings.showBanners) {
                    auto request =
                        QNetworkRequest(headline.banner[0].bannerImage);
                    core.buildRequest(currentProfile(), request);

                    auto reply = core.mgr->get(request);
                    connect(reply, &QNetworkReply::finished, [=] {
                        QPixmap pixmap;
                        pixmap.loadFromData(reply->readAll());
                        bannerImageView->setPixmap(pixmap);
                    });
                }

                if(core.appSettings.showNewsList) {
                    QTreeWidgetItem* newsItem = new QTreeWidgetItem(
                        (QTreeWidgetItem*)nullptr, QStringList("News"));
                    for (auto news : headline.news) {
                        QTreeWidgetItem* item = new QTreeWidgetItem();
                        item->setText(0, news.title);
                        item->setText(1, QLocale().toString(
                                             news.date, QLocale::ShortFormat));
                        item->setData(0, Qt::UserRole, news.url);

                        newsItem->addChild(item);
                    }

                    QTreeWidgetItem* pinnedItem = new QTreeWidgetItem(
                        (QTreeWidgetItem*)nullptr, QStringList("Pinned"));
                    for (auto pinned : headline.pinned) {
                        QTreeWidgetItem* item = new QTreeWidgetItem();
                        item->setText(0, pinned.title);
                        item->setText(1,
                                      QLocale().toString(pinned.date,
                                                         QLocale::ShortFormat));
                        item->setData(0, Qt::UserRole, pinned.url);

                        pinnedItem->addChild(item);
                    }

                    QTreeWidgetItem* topicsItem = new QTreeWidgetItem(
                        (QTreeWidgetItem*)nullptr, QStringList("Topics"));
                    for (auto news : headline.topics) {
                        QTreeWidgetItem* item = new QTreeWidgetItem();
                        item->setText(0, news.title);
                        item->setText(1, QLocale().toString(
                                             news.date, QLocale::ShortFormat));
                        item->setData(0, Qt::UserRole, news.url);

                        qInfo() << news.url;

                        topicsItem->addChild(item);
                    }

                    newsListView->insertTopLevelItems(
                        0, QList<QTreeWidgetItem*>(
                               {newsItem, pinnedItem, topicsItem}));

                    for (int i = 0; i < 3; i++) {
                        newsListView->expandItem(newsListView->topLevelItem(i));
                        newsListView->resizeColumnToContents(i);
                    }
                }
            }
        });
    }

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
