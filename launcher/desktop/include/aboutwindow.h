#pragma once

#include "virtualdialog.h"

class AboutWindow : public VirtualDialog {
public:
    explicit AboutWindow(DesktopInterface& interface, QWidget* widget = nullptr);
};