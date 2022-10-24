#include "virtualwindow.h"

#include <QLayout>
#include <QMenuBar>

#include "desktopinterface.h"

VirtualWindow::VirtualWindow(DesktopInterface& interface, QWidget* widget) : interface(interface) {
    if(interface.oneWindow) {
        mdi_window = new QMdiSubWindow();
        mdi_window->setAttribute(Qt::WA_DeleteOnClose);
    } else {
        normal_window = new QMainWindow();
    }

    interface.addWindow(this);
}

void VirtualWindow::setWindowTitle(QString title) {
    if(interface.oneWindow) {
        mdi_window->setWindowTitle(title);
    } else {
        normal_window->setWindowTitle(title);
    }
}

void VirtualWindow::show() {
    if(interface.oneWindow) {
        mdi_window->show();
    } else {
        normal_window->show();
    }
}

void VirtualWindow::setCentralWidget(QWidget* widget) {
    if(interface.oneWindow) {
        mdi_window->layout()->addWidget(widget);
    } else {
        normal_window->setCentralWidget(widget);
    }
}

void VirtualWindow::hide() {
    if(interface.oneWindow) {
        mdi_window->hide();
    } else {
        normal_window->hide();
    }
}

QMenuBar* VirtualWindow::menuBar() {
    if(interface.oneWindow) {
        if(mdi_window->layout()->menuBar() == nullptr) {
            auto bar = new QMenuBar();
            bar->setObjectName("test");
            qDebug() << "new obj name: " << bar->objectName();
            mdi_window->layout()->setMenuBar(bar);
        }

        qDebug() << mdi_window->layout()->menuBar()->objectName();

        return dynamic_cast<QMenuBar*>(mdi_window->layout()->menuBar());
    } else {
        return normal_window->menuBar();
    }
}

void VirtualWindow::showMaximized() {
    if(interface.oneWindow) {
        mdi_window->showMaximized();
    } else {
        normal_window->showMaximized();
    }
}

QWidget* VirtualWindow::getRootWidget() {
    if(interface.oneWindow) {
        return mdi_window;
    } else {
        return normal_window;
    }
}
