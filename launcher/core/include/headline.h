#pragma once

#include <QDateTime>
#include <QUrl>

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

    QList<News> topics;
};

class LauncherCore;

void getHeadline(LauncherCore& core, const std::function<void(Headline)>& return_func);