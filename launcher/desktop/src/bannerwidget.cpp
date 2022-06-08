#include "bannerwidget.h"

#include <QDebug>
#include <QDesktopServices>

BannerWidget::BannerWidget() : QLabel() {
    setCursor(Qt::CursorShape::PointingHandCursor);
}

void BannerWidget::mousePressEvent(QMouseEvent* event) {
    qDebug() << "Clicked!";
    QDesktopServices::openUrl(url);
}

void BannerWidget::setUrl(QUrl url) {
    this->url = url;
}
