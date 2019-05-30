#ifndef QSPOTIFYEVENTS_H
#define QSPOTIFYEVENTS_H

#include <QtCore/QEvent>
#include <QtCore/QString>

#include <libspotify/api.h>

extern const QEvent::Type NotifyMainThreadEventType;
extern const QEvent::Type ConnectionErrorEventType;
extern const QEvent::Type MetaDataEventType;
extern const QEvent::Type StreamingStartedEventType;
extern const QEvent::Type EndOfTrackEventType;
extern const QEvent::Type StopEventType;
extern const QEvent::Type ResumeEventType;
extern const QEvent::Type SuspendEventType;
extern const QEvent::Type AudioStopEventType;
extern const QEvent::Type ResetBufferEventType;
extern const QEvent::Type TrackProgressEventType;
extern const QEvent::Type SendImageRequestEventType;
extern const QEvent::Type ReceiveImageRequestEventType;
extern const QEvent::Type PlayTokenLostEventType;
extern const QEvent::Type LoggedInEventType;
extern const QEvent::Type LoggedOutEventType;
extern const QEvent::Type OfflineErrorEventType;
extern const QEvent::Type ScrobbleLoginErrorEventType;
extern const QEvent::Type ConnectionStateUpdateEventType;

class QSpotifyConnectionErrorEvent : public QEvent
{
public:
    QSpotifyConnectionErrorEvent(sp_error error)
        : QEvent(Type(ConnectionErrorEventType))
        , m_error(error)
    { }

    sp_error error() const { return m_error; }

private:
    sp_error m_error;
    QString m_message;
};


class QSpotifyStreamingStartedEvent : public QEvent
{
public:
    QSpotifyStreamingStartedEvent(int channels, int sampleRate)
        : QEvent(Type(StreamingStartedEventType))
        , m_channels(channels)
        , m_sampleRate(sampleRate)
    { }

    int channels() const { return m_channels; }
    int sampleRate() const { return m_sampleRate; }

private:
    int m_channels;
    int m_sampleRate;
};


class QSpotifyTrackProgressEvent : public QEvent
{
public:
    QSpotifyTrackProgressEvent(int delta)
        : QEvent(Type(TrackProgressEventType))
        , m_delta(delta)
    { }

    int delta() const { return m_delta; }

private:
    int m_delta;
};

class QSpotifyRequestImageEvent : public QEvent
{
public:
    QSpotifyRequestImageEvent(const QString &id)
        : QEvent(Type(SendImageRequestEventType))
        , m_id(id)
    { }

    QString imageId() const { return m_id; }

private:
    QString m_id;
};

class QSpotifyReceiveImageEvent : public QEvent
{
public:
    QSpotifyReceiveImageEvent(sp_image *image)
        : QEvent(Type(ReceiveImageRequestEventType))
        , m_image(image)
    { Q_ASSERT(image); }

    sp_image *image() const { return m_image; }

private:
    sp_image *m_image;
};

class QSpotifyOfflineErrorEvent : public QEvent
{
public:
    QSpotifyOfflineErrorEvent(sp_error error)
        : QEvent(Type(OfflineErrorEventType))
        , m_error(error)
    { }

    sp_error error() const { return m_error; }

private:
    sp_error m_error;
    QString m_message;
};

#endif // QSPOTIFYEVENTS_H
