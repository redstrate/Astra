#include "headline.h"

#include <QUrlQuery>
#include <QNetworkRequest>
#include <QDateTime>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include "launchercore.h"

void getHeadline(LauncherCore& core, std::function<void(Headline)> return_func) {
    QUrlQuery query;
    query.addQueryItem("lang", "en-us");
    query.addQueryItem("media", "pcapp");

    QUrl url;
    url.setScheme("https");
    url.setHost("frontier.ffxiv.com");
    url.setPath("/news/headline.json");
    url.setQuery(query);

    auto request = QNetworkRequest(QString("%1&%2").arg(url.toString(), QString::number(QDateTime::currentMSecsSinceEpoch())));

    core.buildRequest(request);

    qInfo() << request.url();
    request.setRawHeader("Accept", "application/json, text/plain, */*");
    request.setRawHeader("Origin", "https://launcher.finalfantasyxiv.com");
    request.setRawHeader("Referer", QString("https://launcher.finalfantasyxiv.com/v600/index.html?rc_lang=%1&time=%2").arg("en-us", QDateTime::currentDateTimeUtc().toString("yyyy-MM-dd-HH")).toUtf8());

    auto reply = core.mgr->get(request);
    core.connect(reply, &QNetworkReply::finished, [=] {
        auto document = QJsonDocument::fromJson(reply->readAll());

        Headline headline;

        const auto parseNews = [](QJsonObject object) -> News {
            News news;
            news.date = QDateTime::fromString(object["date"].toString());
            news.id = object["id"].toString();
            news.tag = object["tag"].toString();
            news.title = object["title"].toString();
            news.url = QUrl(object["url"].toString());

            return news;
        };

        for(auto bannerObject : document.object()["banner"].toArray()) {
            Banner banner;
            banner.link = QUrl(bannerObject.toObject()["link"].toString());
            banner.bannerImage = QUrl(bannerObject.toObject()["lsb_banner"].toString());

            headline.banner.push_back(banner);
        }

        for(auto newsObject : document.object()["news"].toArray()) {
            auto news = parseNews(newsObject.toObject());
            headline.news.push_back(news);
        }

        for(auto pinnedObject : document.object()["news"].toArray()) {
            auto pinned = parseNews(pinnedObject.toObject());
            headline.pinned.push_back(pinned);
        }

        return_func(headline);
    });
}