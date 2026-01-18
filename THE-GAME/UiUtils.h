#pragma once

#include <QPixmap>
#include <QMap>
#include <QByteArray>
#include "NetworkManager.h"

class UiUtils
{
public:
    static QPixmap GetAvatar(int userId, NetworkManager* networkManager);
    static void ClearAvatarCache();

private:
    static QMap<int, QPixmap> s_avatarCache;
};
