#pragma once

#include <QPixmap>
#include <QMap>
#include <QByteArray>
#include "NetworkManager.h"

class UiUtils
{
public:
    // Fetches avatar from cache or network, processes it into a circle with border.
    // Size is fixed to 120px internal + border, scaled down by UI if needed.
    static QPixmap GetAvatar(int userId, NetworkManager* networkManager);
    
    // Clear cache if needed (e.g. on logout)
    static void ClearAvatarCache();

private:
    static QMap<int, QPixmap> s_avatarCache;
};
