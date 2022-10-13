#include "launcherwindow.h"

#include <QApplication>
#include <QDesktopServices>
#include <QDirIterator>
#include <QFormLayout>
#include <QHeaderView>
#include <QMenuBar>
#include <QNetworkReply>
#include <QScrollBar>
#include <QTimer>
#include <QTreeWidgetItem>
#include <utility>

#include "aboutwindow.h"
#include "assetupdater.h"
#include "bannerwidget.h"
#include "encryptedarg.h"
#include "gameinstaller.h"
#include "headline.h"
#include "sapphirelauncher.h"
#include "settingswindow.h"
#include "squarelauncher.h"

LauncherWindow::LauncherWindow(LauncherCore& core, QWidget* parent) : QMainWindow(parent), core(core) {
    setWindowTitle("Astra");

    connect(&core, &LauncherCore::settingsChanged, this, &LauncherWindow::reloadControls);

    QMenu* toolsMenu = menuBar()->addMenu("Tools");

    launchOfficial = toolsMenu->addAction("Open Official Launcher...");
    launchOfficial->setIcon(QIcon::fromTheme("application-x-executable"));
    connect(launchOfficial, &QAction::triggered, [=] {
        struct Argument {
            QString key, value;
        };

        QString executeArg("%1%2%3%4");
        QDateTime dateTime = QDateTime::currentDateTime();
        executeArg = executeArg.arg(dateTime.date().month() + 1, 2, 10, QLatin1Char('0'));
        executeArg = executeArg.arg(dateTime.date().day(), 2, 10, QLatin1Char('0'));
        executeArg = executeArg.arg(dateTime.time().hour(), 2, 10, QLatin1Char('0'));
        executeArg = executeArg.arg(dateTime.time().minute(), 2, 10, QLatin1Char('0'));

        QList<Argument> arguments;
        arguments.push_back({"ExecuteArg", executeArg});

        // find user path
        QString userPath;

        // TODO: don't put this here
        QString searchDir;
#if defined(Q_OS_LINUX) || defined(Q_OS_MAC)
        searchDir = currentProfile().winePrefixPath + "/drive_c/users";
#else
        searchDir = "C:/Users";
#endif

        QDirIterator it(searchDir);
        while (it.hasNext()) {
            QString dir = it.next();
            QFileInfo fi(dir);
            QString fileName = fi.fileName();

            // FIXME: is there no easier way to filter out these in Qt?
            if (fi.fileName() != "Public" && fi.fileName() != "." && fi.fileName() != "..") {
                userPath = fileName;
            }
        }

        arguments.push_back(
            {"UserPath",
             QString(R"(C:\Users\%1\Documents\My Games\FINAL FANTASY XIV - A Realm Reborn)").arg(userPath)});

        const QString argFormat = " /%1 =%2";

        QString argJoined;
        for (auto& arg : arguments) {
            argJoined += argFormat.arg(arg.key, arg.value.replace(" ", "  "));
        }

        QString finalArg = encryptGameArg(argJoined);

        auto launcherProcess = new QProcess();
        this->core.launchExecutable(currentProfile(),
                                   launcherProcess,
                                   {currentProfile().gamePath + "/boot/ffxivlauncher64.exe", finalArg},
                                   false,
                                   true);
    });

    launchSysInfo = toolsMenu->addAction("Open System Info...");
    launchSysInfo->setIcon(QIcon::fromTheme("application-x-executable"));
    connect(launchSysInfo, &QAction::triggered, [=] {
        auto sysinfoProcess = new QProcess();
        this->core.launchExecutable(currentProfile(),
                                    sysinfoProcess,
                                    {currentProfile().gamePath + "/boot/ffxivsysinfo64.exe"},
                                    false,
                                    false);
    });

    launchCfgBackup = toolsMenu->addAction("Open Config Backup...");
    launchCfgBackup->setIcon(QIcon::fromTheme("application-x-executable"));
    connect(launchCfgBackup, &QAction::triggered, [=] {
        auto configProcess = new QProcess();
        this->core.launchExecutable(currentProfile(),
                                    configProcess,
                                    {currentProfile().gamePath + "/boot/ffxivconfig64.exe"},
                                    false,
                                    false);
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
        detailedText.append(
            "\n\nIf you do not wish to install it to this location, please change your profile settings.");

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
        auto configProcess = new QProcess();
        this->core.launchExecutable(currentProfile(),
                                    configProcess,
                                    {"winecfg.exe"},
                                    false,
                                    false);
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

    bannerScrollArea = new QScrollArea();
    bannerLayout = new QHBoxLayout();
    bannerLayout->setContentsMargins(0, 0, 0, 0);
    bannerLayout->setSpacing(0);
    bannerLayout->setSizeConstraint(QLayout::SizeConstraint::SetMinAndMaxSize);
    bannerParentWidget = new QWidget();
    bannerParentWidget->setFixedHeight(250);
    bannerScrollArea->setFixedWidth(640);
    bannerScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    bannerScrollArea->verticalScrollBar()->setEnabled(false);
    bannerScrollArea->horizontalScrollBar()->setEnabled(false);

    bannerScrollArea->setWidget(bannerParentWidget);
    bannerParentWidget->setLayout(bannerLayout);

    newsListView = new QTreeWidget();
    newsListView->setColumnCount(2);
    newsListView->setHeaderLabels({"Title", "Date"});
    connect(newsListView, &QTreeWidget::itemClicked, [](QTreeWidgetItem* item, int column) {
        auto url = item->data(0, Qt::UserRole).toUrl();
        qInfo() << "clicked" << url;
        QDesktopServices::openUrl(url);
    });

    loginLayout = new QFormLayout();
    layout->addLayout(loginLayout, 0, 1, 1, 1);

    profileSelect = new QComboBox();
    connect(profileSelect, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), [=](int index) {
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

    connect(otpEdit, &QLineEdit::returnPressed, [this] {
        if (loginButton->isEnabled())
            this->core.assetUpdater->update(currentProfile());
    });

    connect(passwordEdit, &QLineEdit::returnPressed, [this] {
        if (loginButton->isEnabled())
            this->core.assetUpdater->update(currentProfile());
    });

    auto emptyWidget = new QWidget();
    emptyWidget->setLayout(layout);
    setCentralWidget(emptyWidget);

    connect(core.assetUpdater, &AssetUpdater::finishedUpdating, [=] {
        auto& profile = currentProfile();

        auto info = new LoginInformation();
        info->settings = &profile;
        info->username = usernameEdit->text();
        info->password = passwordEdit->text();
        info->oneTimePassword = otpEdit->text();

        if (currentProfile().rememberUsername) {
            profile.setKeychainValue("username", usernameEdit->text());
        }

        if (currentProfile().rememberPassword) {
            profile.setKeychainValue("password", passwordEdit->text());
        }

        this->core.login(info);
    });

    connect(loginButton, &QPushButton::released, [=] {
        // update the assets first if needed, then it calls the slot above :-)
        this->core.assetUpdater->update(currentProfile());
    });

    connect(registerButton, &QPushButton::released, [=] {
        if (currentProfile().isSapphire) {
            auto& profile = currentProfile();

            LoginInformation info;
            info.settings = &profile;
            info.username = usernameEdit->text();
            info.password = passwordEdit->text();
            info.oneTimePassword = otpEdit->text();

            this->core.sapphireLauncher->registerAccount(currentProfile().lobbyURL, info);
        }
    });

    connect(&core, &LauncherCore::successfulLaunch, [&] {
        if (core.appSettings.closeWhenLaunched)
            hide();
    });

    connect(&core, &LauncherCore::gameClosed, [&] {
        if (core.appSettings.closeWhenLaunched)
            QCoreApplication::quit();
    });

    getHeadline(core, [&](Headline new_headline) {
        this->headline = std::move(new_headline);
        reloadNews();
    });

    reloadControls();
}

ProfileSettings& LauncherWindow::currentProfile() {
    return core.getProfile(profileSelect->currentIndex());
}

void LauncherWindow::reloadControls() {
    if (currentlyReloadingControls)
        return;

    currentlyReloadingControls = true;

    const int oldIndex = profileSelect->currentIndex();

    profileSelect->clear();

    for (const auto& profile : core.profileList()) {
        profileSelect->addItem(profile);
    }

    profileSelect->setCurrentIndex(oldIndex);

    if (profileSelect->currentIndex() == -1) {
        profileSelect->setCurrentIndex(core.defaultProfileIndex);
    }

    rememberUsernameBox->setChecked(currentProfile().rememberUsername);
    if (currentProfile().rememberUsername) {
        usernameEdit->setText(currentProfile().getKeychainValue("username"));
    }

    rememberPasswordBox->setChecked(currentProfile().rememberPassword);
    if (currentProfile().rememberPassword) {
        passwordEdit->setText(currentProfile().getKeychainValue("password"));
    }

    bool canLogin = true;
    if (currentProfile().isSapphire) {
        if (currentProfile().lobbyURL.isEmpty()) {
            loginButton->setText("Login (Lobby URL is invalid)");
            canLogin = false;
        }
    }

#if defined(Q_OS_LINUX) || defined(Q_OS_MAC)
    if (!currentProfile().isWineInstalled() && !core.isSteam) {
        loginButton->setText("Login (Wine is not installed)");
        canLogin = false;
    }
#endif
    if (!currentProfile().isGameInstalled()) {
        loginButton->setText("Login (Game is not installed)");
        canLogin = false;
    }

    if (canLogin)
        loginButton->setText("Login");

    launchOfficial->setEnabled(currentProfile().isGameInstalled());
    launchSysInfo->setEnabled(currentProfile().isGameInstalled());
    launchCfgBackup->setEnabled(currentProfile().isGameInstalled());
    openGameDir->setEnabled(currentProfile().isGameInstalled());

#if defined(Q_OS_MAC) || defined(Q_OS_LINUX)
    wineCfg->setEnabled(currentProfile().isWineInstalled());
#endif

    layout->removeWidget(bannerScrollArea);
    bannerScrollArea->hide();
    layout->removeWidget(newsListView);
    newsListView->hide();

    auto field = loginLayout->labelForField(otpEdit);
    if (field != nullptr)
        field->deleteLater();

    loginLayout->takeRow(otpEdit);
    otpEdit->hide();

    if (currentProfile().useOneTimePassword && !currentProfile().isSapphire) {
        loginLayout->addRow("One-Time Password", otpEdit);
        otpEdit->show();
    }

    loginLayout->takeRow(loginButton);
    loginButton->setEnabled(canLogin);
    registerButton->setEnabled(canLogin);
    loginLayout->addRow(loginButton);

    loginLayout->takeRow(registerButton);
    registerButton->hide();

    if (currentProfile().isSapphire) {
        loginLayout->addRow(registerButton);
        registerButton->show();
    }

    reloadNews();

    currentlyReloadingControls = false;
}

void LauncherWindow::reloadNews() {
    if (core.appSettings.showBanners || core.appSettings.showNewsList) {
        for (auto widget : bannerWidgets) {
            bannerLayout->removeWidget(widget);
        }

        bannerWidgets.clear();

        int totalRow = 0;
        if (core.appSettings.showBanners) {
            bannerScrollArea->show();
            layout->addWidget(bannerScrollArea, totalRow++, 0);
        }

        if (core.appSettings.showNewsList) {
            newsListView->show();
            layout->addWidget(newsListView, totalRow++, 0);
        }

        newsListView->clear();

        if (!headline.banner.empty()) {
            if (core.appSettings.showBanners) {
                for (const auto& banner : headline.banner) {
                    auto request = QNetworkRequest(banner.bannerImage);
                    core.buildRequest(currentProfile(), request);

                    auto reply = core.mgr->get(request);
                    connect(reply, &QNetworkReply::finished, [=] {
                        auto bannerImageView = new BannerWidget();
                        bannerImageView->setUrl(banner.link);

                        QPixmap pixmap;
                        pixmap.loadFromData(reply->readAll());
                        bannerImageView->setPixmap(pixmap);

                        bannerLayout->addWidget(bannerImageView);
                        bannerWidgets.push_back(bannerImageView);
                    });
                }

                if (bannerTimer == nullptr) {
                    bannerTimer = new QTimer();
                    connect(bannerTimer, &QTimer::timeout, this, [=] {
                        if (currentBanner >= headline.banner.size())
                            currentBanner = 0;

                        bannerScrollArea->ensureVisible(640 * (currentBanner + 1), 0, 0, 0);

                        currentBanner++;
                    });
                    bannerTimer->start(5000);
                }
            } else {
                if (bannerTimer != nullptr) {
                    bannerTimer->stop();
                    bannerTimer->deleteLater();
                    bannerTimer = nullptr;
                }
            }

            if (core.appSettings.showNewsList) {
                auto newsItem = new QTreeWidgetItem((QTreeWidgetItem*)nullptr, QStringList("News"));
                for (const auto& news : headline.news) {
                    auto item = new QTreeWidgetItem();
                    item->setText(0, news.title);
                    item->setText(1, QLocale().toString(news.date, QLocale::ShortFormat));
                    item->setData(0, Qt::UserRole, news.url);

                    newsItem->addChild(item);
                }

                auto pinnedItem = new QTreeWidgetItem((QTreeWidgetItem*)nullptr, QStringList("Pinned"));
                for (const auto& pinned : headline.pinned) {
                    auto item = new QTreeWidgetItem();
                    item->setText(0, pinned.title);
                    item->setText(1, QLocale().toString(pinned.date, QLocale::ShortFormat));
                    item->setData(0, Qt::UserRole, pinned.url);

                    pinnedItem->addChild(item);
                }

                auto topicsItem = new QTreeWidgetItem((QTreeWidgetItem*)nullptr, QStringList("Topics"));
                for (const auto& news : headline.topics) {
                    auto item = new QTreeWidgetItem();
                    item->setText(0, news.title);
                    item->setText(1, QLocale().toString(news.date, QLocale::ShortFormat));
                    item->setData(0, Qt::UserRole, news.url);

                    topicsItem->addChild(item);
                }

                newsListView->insertTopLevelItems(0, QList<QTreeWidgetItem*>({newsItem, pinnedItem, topicsItem}));

                for (int i = 0; i < 3; i++) {
                    newsListView->expandItem(newsListView->topLevelItem(i));
                    newsListView->resizeColumnToContents(i);
                }
            }
        }
    }
}

void LauncherWindow::openPath(const QString& path) {
#if defined(Q_OS_WIN)
    // for some reason, windows requires special treatment (what else is new?)
    const QFileInfo fileInfo(path);

    QProcess::startDetached("explorer.exe", QStringList(QDir::toNativeSeparators(fileInfo.canonicalFilePath())));
#else
    QDesktopServices::openUrl("file://" + path);
#endif
}
