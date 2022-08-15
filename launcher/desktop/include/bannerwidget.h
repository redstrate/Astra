#pragma once

#include <QLabel>
#include <QUrl>

class BannerWidget : public QLabel {
public:
    BannerWidget();

    void setUrl(QUrl url);

protected:
    void mousePressEvent(QMouseEvent* event) override;

private:
    QUrl url;
};