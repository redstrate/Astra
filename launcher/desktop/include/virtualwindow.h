#pragma once

#include <QMdiSubWindow>
#include <QWidget>
#include <QMainWindow>

class DesktopInterface;

class VirtualWindow : public QObject {
    Q_OBJECT
public:
    VirtualWindow(DesktopInterface& interface, QWidget* parent = nullptr);

    void setWindowTitle(QString title);
    void setCentralWidget(QWidget* widget);
    void show();
    void showMaximized();
    void hide();

    QMenuBar* menuBar();

    QWidget* getRootWidget();

    QMdiSubWindow* mdi_window = nullptr;
    QMainWindow* normal_window = nullptr;

private:
    DesktopInterface& interface;
};