#include "gameparser.h"

#include <QBuffer>
#include <QDebug>
#include <QRegularExpression>

GameParser::GameParser() {
    api = new tesseract::TessBaseAPI();

    if (api->Init(nullptr, "eng")) {
        qDebug() << "Could not initialize tesseract!";
        return;
    }

    api->SetPageSegMode(tesseract::PageSegMode::PSM_SINGLE_BLOCK);
}

GameParser::~GameParser() {
    api->End();
    delete api;
}

GameParseResult GameParser::parseImage(QImage img) {
    QBuffer buf;
    img = img.convertToFormat(QImage::Format_Grayscale8);
    img.save(&buf, "PNG", 100);

    Pix* image = pixReadMem((const l_uint8*)buf.data().data(), buf.size());
    api->SetImage(image);
    api->SetSourceResolution(300);

    const QString text = api->GetUTF8Text();

    // TODO: clean up these names
    const bool hasWorldFullText = text.contains("This World is currently full.") || text.contains("Players in queue");
    const bool hasLobbyErrorText = text.contains("The lobby server connection has encountered an error.");
    const bool hasCONFIGURATIONText = text.contains("CONFIGURATION") || text.contains("ONLINE");
    const bool hasConnectingToData = text.contains("Connecting to data center");
    const bool worldTotallyFull = text.contains("3001");

    if (hasLobbyErrorText) {
        qDebug() << "LOBBY ERROR";

        return {ScreenState::LobbyError, -1};
    } else {
        if (worldTotallyFull) {
            qDebug() << "TOTALLY FULL WORLD (CLOSED BY SQENIX)";

            return {ScreenState::WorldFull, -1};
        } else {
            if (hasConnectingToData) {
                qDebug() << "CONNECTING TO DATA CENTER";

                return {ScreenState::ConnectingToDataCenter, -1};
            } else {
                if (hasWorldFullText) {
                    qDebug() << "FULL WORLD";

                    // attempt to extract number of players in queue
                    QRegularExpression exp("(?:Players in queue: )([\\d|,]*)");

                    auto match = exp.match(text);
                    if (match.isValid()) {
                        return {ScreenState::InLoginQueue, match.captured(1).remove(',').toInt()};
                    }

                    return {ScreenState::InLoginQueue, -1};
                } else {
                    if (hasCONFIGURATIONText) {
                        qDebug() << "TITLE SCREEN";
                        return {ScreenState::EnteredTitleScreen, -1};
                    }
                }
            }
        }
    }

    // TODO: figure out how to properly clear tesseract data
    api->Clear();
    api->ClearAdaptiveClassifier();

    return {ScreenState::Splash, -1};
}