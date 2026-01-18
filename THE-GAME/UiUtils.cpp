#include "UiUtils.h"
#include <QPainter>
#include <QPainterPath>
#include <QBuffer>

QMap<int, QPixmap> UiUtils::s_avatarCache;

QPixmap UiUtils::GetAvatar(int userId, NetworkManager* networkManager)
{
    if (userId < 0) return QPixmap();

    if (s_avatarCache.contains(userId)) {
        return s_avatarCache[userId];
    }

    if (!networkManager) return QPixmap();

    QByteArray data = networkManager->getProfilePicture(userId);
    
    if (data.isEmpty()) {
        QPixmap empty(1, 1);
        empty.fill(Qt::transparent);
        s_avatarCache.insert(userId, empty);
        return QPixmap();
    }

    QPixmap p;
    p.loadFromData(data);
    if (p.isNull()) return QPixmap();

    int size = 120; 
    int border = 4;
    int totalSize = size + border;
    
    QImage outImage(totalSize, totalSize, QImage::Format_ARGB32);
    outImage.fill(Qt::transparent);
    
    QPainter painter(&outImage);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
    
    QPainterPath path;
    path.addEllipse(border/2, border/2, size, size);
    painter.setClipPath(path);
    
    QImage image = p.toImage();
    QImage scaled = image.scaled(size, size, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
    
    int x = (scaled.width() - size) / 2;
    int y = (scaled.height() - size) / 2;
    
    painter.drawImage(border/2, border/2, scaled, x, y, size, size);
    
    painter.setClipping(false);
    QPen pen(QColor("#f3d05a"));
    pen.setWidth(border);
    painter.setPen(pen);
    painter.setBrush(Qt::NoBrush);
    painter.drawEllipse(border/2, border/2, size, size);

    QPixmap finalPix = QPixmap::fromImage(outImage);
    s_avatarCache.insert(userId, finalPix);
    return finalPix;
}

void UiUtils::ClearAvatarCache()
{
    s_avatarCache.clear();
}
