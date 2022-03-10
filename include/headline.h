#pragma once

#include <QUrl>
#include <QDateTime>

struct News {
    QDateTime date;
    QString id;
    QString tag;
    QString title;
    QUrl url;
};

struct Banner {
    QUrl link;
    QUrl bannerImage;
};

struct Headline {
    QList<Banner> banner;

    QList<News> news;

    QList<News> pinned;
};

class LauncherCore;

void getHeadline(LauncherCore& core, std::function<void(Headline)> return_func);