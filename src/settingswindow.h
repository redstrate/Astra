#pragma once

#include <QWidget>
#include <QListWidget>

class LauncherWindow;

class SettingsWindow : public QWidget {
public:
    SettingsWindow(LauncherWindow& window, QWidget* parent = nullptr);

public slots:
    void reloadControls();

private:
    void openPath(const QString path);

    QListWidget* profileWidget;

    LauncherWindow& window;
};