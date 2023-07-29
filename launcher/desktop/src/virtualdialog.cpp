#include "virtualdialog.h"

#include <QLayout>

#include "desktopinterface.h"

VirtualDialog::VirtualDialog(DesktopInterface& interface, QWidget* widget) : interface(interface), QObject(widget) {
    if (interface.oneWindow) {
        mdi_window = new QMdiSubWindow();
        mdi_window->setAttribute(Qt::WA_DeleteOnClose);
    } else {
        normal_dialog = new QDialog();
    }

    interface.addDialog(this);
}

void VirtualDialog::setWindowTitle(const QString& title) {
    if (interface.oneWindow) {
        mdi_window->setWindowTitle(title);
    } else {
        normal_dialog->setWindowTitle(title);
    }
}

void VirtualDialog::show() {
    if (interface.oneWindow) {
        mdi_window->show();
    } else {
        normal_dialog->show();
    }
}

void VirtualDialog::hide() {
    if(interface.oneWindow) {
        mdi_window->hide();
    } else {
        normal_dialog->hide();
    }
}

void VirtualDialog::close() {
    if(interface.oneWindow) {
        mdi_window->close();
    } else {
        normal_dialog->close();
    }
}

void VirtualDialog::setWindowModality(Qt::WindowModality modality) {
    if(interface.oneWindow) {
        mdi_window->setWindowModality(modality);
    } else {
        normal_dialog->setWindowModality(modality);
    }
}

void VirtualDialog::setLayout(QLayout* layout) {
    if(interface.oneWindow) {
        auto emptyWidget = new QWidget();
        emptyWidget->setLayout(layout);

        mdi_window->layout()->addWidget(emptyWidget);
    } else {
        normal_dialog->setLayout(layout);
    }
}

QWidget* VirtualDialog::getRootWidget() {
    if(interface.oneWindow) {
        return mdi_window;
    } else {
        return normal_dialog;
    }
}
