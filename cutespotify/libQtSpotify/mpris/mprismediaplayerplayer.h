#ifndef MPRISMEDIAPLAYERPLAYER_H
#define MPRISMEDIAPLAYERPLAYER_H

#include <QtCore/QObject>
#include <QtDBus/QDBusAbstractAdaptor>

class MPRISMediaPlayerPlayer : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO ("D-Bus Interface", "org.mpris.MediaPlayer2.Player")

    Q_PROPERTY(QString PlaybackStatus READ PlaybackStatus)
    Q_PROPERTY(QString LoopStatus READ LoopStatus)
    Q_PROPERTY(double Rate READ Rate)
    Q_PROPERTY(qint64 Position READ Position)
    Q_PROPERTY(double MinimumRate READ MinimumRate)
    Q_PROPERTY(double MaximumRate READ MaximumRate)
    Q_PROPERTY(double Volume READ Volume)
    Q_PROPERTY(bool CanGoNext READ CanGoNext)
    Q_PROPERTY(bool CanGoPrevious READ CanGoPrevious)
    Q_PROPERTY(bool CanPlay READ CanPlay)
    Q_PROPERTY(bool CanPause READ CanPause)
    Q_PROPERTY(bool CanSeek READ CanSeek)
    Q_PROPERTY(bool CanControl READ CanControl)
    Q_PROPERTY(QVariantMap Metadata READ Metadata)
public:
    MPRISMediaPlayerPlayer(QObject* parent);
    QString PlaybackStatus();
    QString LoopStatus();
    double Rate();
    qint64 Position();
    double MinimumRate();
    double MaximumRate();
    QVariantMap Metadata();
    double Volume();
    bool CanGoNext();
    bool CanGoPrevious();
    bool CanPlay();
    bool CanPause();
    bool CanSeek();
    bool CanControl();

public slots:
    void Play();
    void Pause();
    void PlayPause();
    void Stop();
    void Previous();
    void Next();
    void OpenUri(QString uri);
    void Seek(qint64 offset);
    void SetPosition(QString trackId, qint64 position);

private slots:
    void playbackStatusChanged();
    void metaDataChanged();
};

#endif // MPRISMEDIAPLAYERPLAYER_H
