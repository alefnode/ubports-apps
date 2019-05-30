#ifndef MPRISMEDIAPLAYER_H
#define MPRISMEDIAPLAYER_H

#include <QtCore/QObject>
#include <QtDBus/QDBusAbstractAdaptor>

class MPRISMediaPlayer : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO ("D-Bus Interface", "org.mpris.MediaPlayer2")
    Q_PROPERTY(bool CanQuit READ CanQuit)
    Q_PROPERTY(bool CanRaise READ CanRaise)
    Q_PROPERTY(bool HasTrackList READ HasTrackList)
    Q_PROPERTY(QString Identity READ Identity)

public:
    MPRISMediaPlayer(QObject* parent) : QDBusAbstractAdaptor(parent) {}
    bool CanQuit() const { return false; }
    bool CanRaise() const { return false; }
    bool HasTrackList() const { return false; }
    QString Identity() const { return "CuteSpot"; }

public slots:
    void Raise() {}
    void Quit() {}
};

#endif // MPRISMEDIAPLAYER_H
