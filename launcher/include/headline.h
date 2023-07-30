#pragma once

#include <QDateTime>
#include <QObject>
#include <QUrl>

class News
{
    Q_GADGET

    Q_PROPERTY(QDateTime date MEMBER date CONSTANT)
    Q_PROPERTY(QString id MEMBER id CONSTANT)
    Q_PROPERTY(QString tag MEMBER tag CONSTANT)
    Q_PROPERTY(QString title MEMBER title CONSTANT)
    Q_PROPERTY(QUrl url MEMBER url CONSTANT)

public:
    QDateTime date;
    QString id;
    QString tag;
    QString title;
    QUrl url;
};

class Banner
{
    Q_GADGET

    Q_PROPERTY(QUrl link MEMBER link CONSTANT)
    Q_PROPERTY(QUrl bannerImage MEMBER bannerImage CONSTANT)

public:
    QUrl link;
    QUrl bannerImage;
};

class Headline : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QList<Banner> banners MEMBER banners CONSTANT)
    Q_PROPERTY(QList<News> news MEMBER news CONSTANT)
    Q_PROPERTY(QList<News> pinned MEMBER pinned CONSTANT)
    Q_PROPERTY(QList<News> topics MEMBER topics CONSTANT)

public:
    explicit Headline(QObject *parent = nullptr)
        : QObject(parent)
    {
    }

    QList<Banner> banners;
    QList<News> news;
    QList<News> pinned;
    QList<News> topics;
};