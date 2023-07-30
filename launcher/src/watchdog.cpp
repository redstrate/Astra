#include "watchdog.h"

#include <QGuiApplication>
#include <QMenu>
#include <QScreen>
#include <QTimer>

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/extensions/Xcomposite.h>
#include <X11/extensions/Xrender.h>

// from https://github.com/adobe/webkit/blob/master/Source/WebCore/plugins/qt/QtX11ImageConversion.cpp
// code is licensed under GPLv2
// Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies)
QImage qimageFromXImage(XImage *xi)
{
    QImage::Format format = QImage::Format_ARGB32_Premultiplied;
    if (xi->depth == 24)
        format = QImage::Format_RGB32;
    else if (xi->depth == 16)
        format = QImage::Format_RGB16;

    QImage image = QImage(reinterpret_cast<uchar *>(xi->data), xi->width, xi->height, xi->bytes_per_line, format).copy();

    // we may have to swap the byte order
    if ((QSysInfo::ByteOrder == QSysInfo::LittleEndian && xi->byte_order == MSBFirst)
        || (QSysInfo::ByteOrder == QSysInfo::BigEndian && xi->byte_order == LSBFirst)) {
        for (int i = 0; i < image.height(); i++) {
            if (xi->depth == 16) {
                ushort *p = reinterpret_cast<ushort *>(image.scanLine(i));
                ushort *end = p + image.width();
                while (p < end) {
                    *p = ((*p << 8) & 0xff00) | ((*p >> 8) & 0x00ff);
                    p++;
                }
            } else {
                uint *p = reinterpret_cast<uint *>(image.scanLine(i));
                uint *end = p + image.width();
                while (p < end) {
                    *p = ((*p << 24) & 0xff000000) | ((*p << 8) & 0x00ff0000) | ((*p >> 8) & 0x0000ff00) | ((*p >> 24) & 0x000000ff);
                    p++;
                }
            }
        }
    }

    // fix-up alpha channel
    if (format == QImage::Format_RGB32) {
        QRgb *p = reinterpret_cast<QRgb *>(image.bits());
        for (int y = 0; y < xi->height; ++y) {
            for (int x = 0; x < xi->width; ++x)
                p[x] |= 0xff000000;
            p += xi->bytes_per_line / 4;
        }
    }

    return image;
}

void Watchdog::launchGame(const ProfileSettings &settings, const LoginAuth &auth)
{
    if (icon == nullptr) {
        icon = new QSystemTrayIcon();
    }

    icon->setToolTip("Queue Status");
    icon->show();
    icon->showMessage("Watchdog", "Watchdog service has started. Waiting for you to connect to data center...");

    auto timer = new QTimer(this);

    auto menu = new QMenu();

    auto stopAction = menu->addAction("Stop");
    connect(stopAction, &QAction::triggered, [=] {
        timer->stop();
        processWindowId = -1;
        icon->hide();
    });

    icon->setContextMenu(menu);

    core.launchGame(settings, auth);

    if (parser == nullptr) {
        parser = std::make_unique<GameParser>();
    }

    connect(timer, &QTimer::timeout, [=] {
        if (processWindowId == -1) {
            auto xdoProcess = new QProcess();

            connect(xdoProcess, static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), [=](int, QProcess::ExitStatus) {
                QString output = xdoProcess->readAllStandardOutput();
                qDebug() << "Found XIV Window: " << output.toInt();

                processWindowId = output.toInt();
            });

            // TODO: don't use xdotool for this, find a better way to
            xdoProcess->start("bash", {"-c", "xdotool search --name \"FINAL FANTASY XIV\""});
        } else {
            Display *display = XOpenDisplay(nullptr);

            XSynchronize(display, True);

            XWindowAttributes attr;
            Status status = XGetWindowAttributes(display, processWindowId, &attr);
            if (status == 0) {
                qDebug() << "Failed to get window attributes! The window is possibly closed now.";
                processWindowId = -1;
                timer->stop();
                icon->hide();
            } else {
                XCompositeRedirectWindow(display, processWindowId, CompositeRedirectAutomatic);
                XCompositeNameWindowPixmap(display, processWindowId);

                XRenderPictFormat *format = XRenderFindVisualFormat(display, attr.visual);

                XRenderPictureAttributes pa;
                pa.subwindow_mode = IncludeInferiors;

                Picture picture = XRenderCreatePicture(display, processWindowId, format, CPSubwindowMode, &pa);
                XFlush(display); // TODO: does this actually make a difference?

                XImage *image = XGetImage(display, processWindowId, 0, 0, attr.width, attr.height, AllPlanes, ZPixmap);
                if (!image) {
                    qDebug() << "Unable to get image...";
                } else {
                    auto result = parser->parseImage(qimageFromXImage(image));
                    if (result != lastResult) {
                        // skip OCR errors (TODO: should be handled by GameParser itself)
                        if (result.state == ScreenState::InLoginQueue && result.playersInQueue == 0)
                            return;

                        switch (result.state) {
                        case ScreenState::InLoginQueue: {
                            icon->showMessage("Watchdog",
                                              QString("You are now at position %1 (moved %2 spots)")
                                                  .arg(result.playersInQueue)
                                                  .arg(lastResult.playersInQueue - result.playersInQueue));

                            icon->setToolTip(QString("Queue Status (%1)").arg(result.playersInQueue));
                        } break;
                        case ScreenState::LobbyError: {
                            // TODO: kill game?
                            icon->showMessage("Watchdog", "You have been disconnected due to a lobby error.");
                        } break;
                        case ScreenState::ConnectingToDataCenter: {
                            icon->showMessage("Watchdog", "You are in the process of being connected to the data center.");
                        } break;
                        case ScreenState::WorldFull: {
                            icon->showMessage("Watchdog", "You have been disconnected due to a lobby error.");
                        } break;
                        }

                        lastResult = result;
                    }

                    XFreePixmap(display, picture);
                }
            }

            XCompositeUnredirectWindow(display, processWindowId, CompositeRedirectAutomatic);
        }
    });

    timer->start(5000);
}
