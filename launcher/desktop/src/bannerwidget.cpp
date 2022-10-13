#include "bannerwidget.h"

#include <QDebug>
#include <QDesktopServices>
#include <utility>

BannerWidget::BannerWidget() : QLabel() {
    setCursor(Qt::CursorShape::PointingHandCursor);
}

void BannerWidget::mousePressEvent(QMouseEvent* event) {
    qDebug() << "Clicked!";
    QDesktopServices::openUrl(url);
}

void BannerWidget::setUrl(QUrl newUrl) {
    this->url = std::move(newUrl);
}
