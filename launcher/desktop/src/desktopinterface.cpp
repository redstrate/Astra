#include "desktopinterface.h"
#include "autologinwindow.h"
#include "gameinstaller.h"

DesktopInterface::DesktopInterface(LauncherCore& core) {
    window = new LauncherWindow(core);

    auto& defaultProfile = core.getProfile(core.defaultProfileIndex);

    if (!defaultProfile.isGameInstalled()) {
        auto messageBox = new QMessageBox(window);
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
    if (!defaultProfile.isWineInstalled()) {
        auto messageBox = new QMessageBox(window);
        messageBox->setIcon(QMessageBox::Icon::Critical);
        messageBox->setText("No Wine Found");
        messageBox->setInformativeText("Wine is not installed but is required to FFXIV on this operating system.");
        messageBox->setWindowModality(Qt::WindowModal);

        messageBox->addButton(QMessageBox::StandardButton::Ok);
        messageBox->setDefaultButton(QMessageBox::StandardButton::Ok);

        messageBox->exec();
    }
#endif

    if(defaultProfile.autoLogin) {
        autoLoginWindow = new AutoLoginWindow(defaultProfile, core);
        QObject::connect(autoLoginWindow, &AutoLoginWindow::loginCanceled, [this] {
            autoLoginWindow->hide();
            window->show();
        });
        autoLoginWindow->show();
    } else {
        window->show();
    }
}