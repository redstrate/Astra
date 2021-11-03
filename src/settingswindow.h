#pragma once

#include <QWidget>

class LauncherWindow;

class SettingsWindow : public QWidget {
public:
    SettingsWindow(LauncherWindow& window, QWidget* parent = nullptr);

private:
    void openPath(const QString path);

    LauncherWindow& window;
};