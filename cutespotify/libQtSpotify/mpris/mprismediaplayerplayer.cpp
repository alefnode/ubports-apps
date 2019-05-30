#include "mprismediaplayerplayer.h"

#include <QtCore/QDebug>

#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusMessage>

#include "qspotifysession.h"
#include "qspotifytrack.h"
#include "qspotifyalbum.h"
#include "qspotifyartist.h"

MPRISMediaPlayerPlayer::MPRISMediaPlayerPlayer(QObject *parent)
    : QDBusAbstractAdaptor(parent)
{
    connect(QSpotifySession::instance(), &QSpotifySession::isPlayingChanged, this, &MPRISMediaPlayerPlayer::playbackStatusChanged);
    connect(QSpotifySession::instance(), &QSpotifySession::currentTrackChanged, this, &MPRISMediaPlayerPlayer::metaDataChanged);
}

QString MPRISMediaPlayerPlayer::PlaybackStatus()
{
    auto inst = QSpotifySession::instance();
    if (inst->isPlaying())
        return "Playing";
    else if (inst->hasCurrentTrack())
        return "Paused";
    else
        return "Stopped";
}

QString MPRISMediaPlayerPlayer::LoopStatus()
{
    auto inst = QSpotifySession::instance();
    if (inst->repeatOne()) return "Track";
    else if (inst->repeat()) return "Playlist";
    else return "None";
}

double MPRISMediaPlayerPlayer::Rate()
{
    return 1.0;
}

qint64 MPRISMediaPlayerPlayer::Position()
{
    return QSpotifySession::instance()->currentTrackPosition();
}

double MPRISMediaPlayerPlayer::MinimumRate()
{
    return 1.0;
}

double MPRISMediaPlayerPlayer::MaximumRate()
{
    return 1.0;
}

QVariantMap MPRISMediaPlayerPlayer::Metadata()
{
    auto inst = QSpotifySession::instance();
    QVariantMap metadata;
    if (auto track = inst->currentTrack()) {
        metadata.insert("mpris:trackid", QString("/org/mpris/MediaPlayer2/Track/%1").arg(1));//TODO use correct value?
        metadata.insert("mpris:length", track->durationString());
        metadata.insert("xesam:albumArtist", track->albumObject()->artist());
        metadata.insert("xesam:artist", track->artistObject()->name());
        metadata.insert("xesam:title", track->name());
    }
    return metadata;
}

double MPRISMediaPlayerPlayer::Volume()
{
    return 1.0;
}

bool MPRISMediaPlayerPlayer::CanGoNext()
{
    return true;
}

bool MPRISMediaPlayerPlayer::CanGoPrevious()
{
    return true;
}

bool MPRISMediaPlayerPlayer::CanPlay()
{
    return true;
}

bool MPRISMediaPlayerPlayer::CanPause()
{
    return true;
}

bool MPRISMediaPlayerPlayer::CanSeek()
{
    return true;
}

bool MPRISMediaPlayerPlayer::CanControl()
{
    return true;
}

void MPRISMediaPlayerPlayer::Play()
{
    QSpotifySession::instance()->resume();
}

void MPRISMediaPlayerPlayer::Pause()
{
    QSpotifySession::instance()->pause();
}

void MPRISMediaPlayerPlayer::PlayPause()
{
    auto inst = QSpotifySession::instance();
    inst->isPlaying() ? inst->pause() : inst->resume();
}

void MPRISMediaPlayerPlayer::Stop()
{
    QSpotifySession::instance()->stop();
}

void MPRISMediaPlayerPlayer::Previous()
{
    QSpotifySession::instance()->playPrevious();
}

void MPRISMediaPlayerPlayer::Next()
{
    QSpotifySession::instance()->playNext();
}

void MPRISMediaPlayerPlayer::OpenUri(QString)
{
    qDebug() << "MPRIS: OpenUri not supported";
}

void MPRISMediaPlayerPlayer::Seek(qint64 offset)
{
    QSpotifySession::instance()->seek(offset);
}

void MPRISMediaPlayerPlayer::SetPosition(QString, qint64)
{
    qDebug() << "MPRIS: SetPosition not supported";
}

void MPRISMediaPlayerPlayer::playbackStatusChanged()
{
    QDBusMessage signal = QDBusMessage::createSignal("/org/mpris/MediaPlayer2","org.freedesktop.DBus.Properties","PropertiesChanged" );
    signal << "org.mpris.MediaPlayer2.Player";
    QVariantMap changedProps;
    changedProps.insert("PlaybackStatus", PlaybackStatus());
    signal << changedProps;
    signal << QStringList();
    QDBusConnection::sessionBus().send(signal);
}

void MPRISMediaPlayerPlayer::metaDataChanged()
{
    QDBusMessage signal = QDBusMessage::createSignal("/org/mpris/MediaPlayer2","org.freedesktop.DBus.Properties","PropertiesChanged" );
    signal << "org.mpris.MediaPlayer2.Player";
    QVariantMap changedProps;
    changedProps.insert("Metadata", Metadata());
    signal << changedProps;
    signal << QStringList();
    QDBusConnection::sessionBus().send(signal);
}
