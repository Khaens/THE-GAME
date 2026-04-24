#pragma once
#include <QObject>
#include <QString>
#include <map>

enum class SoundType {
    BigButtonClick,
    CardDraw,
    FancyShuffle,
    RoundButtonClick,
    RoundShuffle
};

class SoundManager : public QObject {
    Q_OBJECT
public:
    static SoundManager* instance(QObject* parent = nullptr);
    void play(SoundType type);

private:
    explicit SoundManager(QObject* parent = nullptr);
    ~SoundManager();

    static SoundManager* m_instance;
    
    std::map<SoundType, QString> m_soundAliases;
    void loadSound(SoundType type, const QString& fileName, const QString& alias);
};
