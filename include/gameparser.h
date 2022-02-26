#pragma once

#include <QImage>
#include <tesseract/baseapi.h>
#include <leptonica/allheaders.h>

enum class ScreenState {
    Splash,
    LobbyError,
    WorldFull,
    ConnectingToDataCenter,
    EnteredTitleScreen,
    InLoginQueue
};

struct GameParseResult {
    ScreenState state;

    int playersInQueue = -1;
};

inline bool operator==(const GameParseResult a, const GameParseResult b) {
    return a.state == b.state && a.playersInQueue == b.playersInQueue;
}

inline bool operator!=(const GameParseResult a, const GameParseResult b) {
    return !(a == b);
}

class GameParser {
public:
    GameParser();
    ~GameParser();

    GameParseResult parseImage(QImage image);

private:
    tesseract::TessBaseAPI* api;
};