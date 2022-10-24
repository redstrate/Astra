#include "desktopinterface.h"
#include "autologinwindow.h"
#include "gameinstaller.h"

DesktopInterface::DesktopInterface(LauncherCore& core) {
    if(oneWindow) {
        mdiArea = new QMdiArea();
        mdiWindow = new QMainWindow();

        if(isSteamDeck) {
            mdiWindow->setWindowFlag(Qt::FramelessWindowHint);
            mdiWindow->setFixedSize(1280, 800);
        }

        mdiWindow->setWindowTitle("Combined Interface");
        mdiWindow->setCentralWidget(mdiArea);
        mdiWindow->show();
    }

    window = new LauncherWindow(*this, core);

    auto& defaultProfile = core.getProfile(core.defaultProfileIndex);

    if (!defaultProfile.isGameInstalled()) {
        auto messageBox = new QMessageBox();
        messageBox->setIcon(QMessageBox::Icon::Question);
        messageBox->setText("No Game Found");
        messageBox->setInformativeText("FFXIV is not installed. Would you like to install it now?");

        QString detailedText =
            QString("Astra will install FFXIV for you at '%1'").arg(core.getProfile(core.defaultProfileIndex).gamePath);
        detailedText.append(
            "\n\nIf you do not wish to install it to this location, please set it in your default profile first.");

        messageBox->setDetailedText(detailedText);
        messageBox->setWindowModality(Qt::WindowModal);

        auto installButton = messageBox->addButton("Install Game", QMessageBox::YesRole);
        QObject::connect(installButton, &QPushButton::clicked, [&core, messageBox] {
            installGame(core, core.getProfile(core.defaultProfileIndex), [messageBox, &core] {
                core.readGameVersion();

                messageBox->close();
            });
        });

        messageBox->addButton(QMessageBox::StandardButton::No);
        messageBox->setDefaultButton(installButton);

        messageBox->exec();
    }

#if defined(Q_OS_LINUX) || defined(Q_OS_MAC)
    if (!core.isSteam && !defaultProfile.isWineInstalled()) {
        auto messageBox = new QMessageBox();
        messageBox->setIcon(QMessageBox::Icon::Critical);
        messageBox->setAttribute(Qt::WA_DeleteOnClose);
        messageBox->setText("No Wine Found");
        messageBox->setInformativeText("Wine is not installed but is required to FFXIV on this operating system.");
        //messageBox->setWindowModality(Qt::WindowModal);

        messageBox->addButton(QMessageBox::StandardButton::Ok);
        messageBox->setDefaultButton(QMessageBox::StandardButton::Ok);

        messageBox->show();
    }
#endif

    if(defaultProfile.autoLogin) {
        autoLoginWindow = new AutoLoginWindow(*this, defaultProfile, core);
        autoLoginWindow->show();

        QObject::connect(autoLoginWindow, &AutoLoginWindow::loginCanceled,[=] {
            autoLoginWindow->hide();
            window->show();
        });
    } else {
        if(oneWindow) {
            window->showMaximized();
        } else {
            window->show();
        }
    }
}

void DesktopInterface::addWindow(VirtualWindow* window) {
    if(oneWindow) {
        mdiArea->addSubWindow(window->mdi_window);
    }
}

void DesktopInterface::addDialog(VirtualDialog* dialog) {
    if(oneWindow) {
        mdiArea->addSubWindow(dialog->mdi_window);
    }
}