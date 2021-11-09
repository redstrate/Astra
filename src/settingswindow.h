#pragma once

#include <QWidget>
#include <QListWidget>
#include <QComboBox>

class LauncherWindow;

class SettingsWindow : public QWidget {
public:
    SettingsWindow(LauncherWindow& window, QWidget* parent = nullptr);

public slots:
    void reloadControls();

private:
    void openPath(const QString path);

    QListWidget* profileWidget = nullptr;
    QComboBox* directXCombo = nullptr;

    bool currentlyReloadingControls = false;

    LauncherWindow& window;
};