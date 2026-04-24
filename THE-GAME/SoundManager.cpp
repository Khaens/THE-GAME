#include "SoundManager.h"
#include <Windows.h>
#include <mmsystem.h>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QDebug>

#pragma comment(lib, "Winmm.lib")

SoundManager* SoundManager::m_instance = nullptr;

SoundManager* SoundManager::instance(QObject* parent) {
    if (!m_instance) {
        m_instance = new SoundManager(parent);
    }
    return m_instance;
}

SoundManager::SoundManager(QObject* parent) : QObject(parent) {
    loadSound(SoundType::BigButtonClick, "Big_Button_Click.mp3", "BigButtonClick");
    loadSound(SoundType::CardDraw, "Card_Draw.mp3", "CardDraw");
    loadSound(SoundType::FancyShuffle, "Fancy_Shuffle.mp3", "FancyShuffle");
    loadSound(SoundType::RoundButtonClick, "Round_Button_Click.mp3", "RoundButtonClick");
    loadSound(SoundType::RoundShuffle, "Round_Shuffle.mp3", "RoundShuffle");
}

SoundManager::~SoundManager() {
    for (auto const& [type, alias] : m_soundAliases) {
        QString cmd = QString("close %1").arg(alias);
        mciSendString(cmd.toStdWString().c_str(), NULL, 0, NULL);
    }
}

void SoundManager::loadSound(SoundType type, const QString& fileName, const QString& alias) {
    QString path = "Resources/Sounds/" + fileName;
    if (!QFile::exists(path)) {
        // Try app dir path
        path = QCoreApplication::applicationDirPath() + "/Resources/Sounds/" + fileName;
    }

    if (QFile::exists(path)) {
        QString absPath = QDir::current().absoluteFilePath(path);
        absPath.replace("/", "\\"); // MCI prefers backslashes on Windows

        // Open command
        QString command = QString("open \"%1\" type mpegvideo alias %2").arg(absPath).arg(alias);
        MCIERROR err = mciSendString(command.toStdWString().c_str(), NULL, 0, NULL);
        
        if (err == 0) {
            m_soundAliases[type] = alias;
            qDebug() << "Loaded sound (MCI):" << fileName << "as" << alias;
        } else {
            wchar_t errText[256];
            mciGetErrorString(err, errText, 256);
            qWarning() << "MCI Error loading" << fileName << ":" << QString::fromWCharArray(errText);
        }
    } else {
        qWarning() << "Sound file not found (MCI):" << fileName;
    }
}

void SoundManager::play(SoundType type) {
    auto it = m_soundAliases.find(type);
    if (it != m_soundAliases.end()) {
        QString alias = it->second;
        
        // Stop and Seek to start (to allow re-playing before finishing)
        QString stopCmd = QString("stop %1").arg(alias);
        mciSendString(stopCmd.toStdWString().c_str(), NULL, 0, NULL);
        
        QString seekCmd = QString("seek %1 to start").arg(alias);
        mciSendString(seekCmd.toStdWString().c_str(), NULL, 0, NULL);
        
        QString playCmd = QString("play %1").arg(alias);
        mciSendString(playCmd.toStdWString().c_str(), NULL, 0, NULL);
    }
}
