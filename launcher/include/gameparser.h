// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QImage>
#include <leptonica/allheaders.h>
#include <tesseract/baseapi.h>

enum class ScreenState { Splash, LobbyError, WorldFull, ConnectingToDataCenter, EnteredTitleScreen, InLoginQueue };

struct GameParseResult {
    ScreenState state;

    int playersInQueue = -1;
};

inline bool operator==(const GameParseResult a, const GameParseResult b)
{
    return a.state == b.state && a.playersInQueue == b.playersInQueue;
}

inline bool operator!=(const GameParseResult a, const GameParseResult b)
{
    return !(a == b);
}

class GameParser
{
public:
    GameParser();
    ~GameParser();

    GameParseResult parseImage(QImage image);

private:
    tesseract::TessBaseAPI *api;
};