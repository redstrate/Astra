#pragma once

#include <QMainWindow>
#include <QComboBox>
#include <QCheckBox>
#include <QPushButton>

#include "launchercore.h"

class LauncherWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit LauncherWindow(LauncherCore& core, QWidget* parent = nullptr);

    ProfileSettings currentProfile() const;
    ProfileSettings& currentProfile();

public slots:
    void reloadControls();

private:
    LauncherCore& core;

    bool currentlyReloadingControls = false;

    QComboBox* profileSelect;
    QLineEdit* usernameEdit, *passwordEdit;
    QLineEdit* otpEdit;
    QCheckBox* rememberUsernameBox, *rememberPasswordBox;
    QPushButton* loginButton, *registerButton;
};