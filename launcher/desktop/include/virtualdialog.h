#pragma once

#include <QMdiSubWindow>
#include <QWidget>
#include <QDialog>

class DesktopInterface;

class VirtualDialog : public QObject {
    Q_OBJECT
public:
    VirtualDialog(DesktopInterface& interface, QWidget* parent = nullptr);

    void setWindowTitle(QString title);
    void show();
    void hide();
    void close();
    void setWindowModality(Qt::WindowModality modality);
    void setLayout(QLayout* layout);

    QWidget* getRootWidget();

    QMdiSubWindow* mdi_window = nullptr;
    QDialog* normal_dialog = nullptr;

private:
    DesktopInterface& interface;
};