#include "qspotifyaudiothreadworker.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QDebug>
#include <QtCore/QEvent>
#include <QtCore/QMutexLocker>
#include <QtMultimedia/QAudioOutput>

#include "qspotifysession.h"
#include "qspotifyevents.h"

QSpotifyRingbuffer g_buffer;
QMutex g_mutex;

QMutex g_imageRequestMutex;
QHash<QString, QWaitCondition *> g_imageRequestConditions;
QHash<QString, QImage> g_imageRequestImages;
QHash<sp_image *, QString> g_imageRequestObject;

QSpotifyAudioThreadWorker::QSpotifyAudioThreadWorker(QObject *parent)
    : QObject(parent)
    , m_audioOutput(nullptr)
    , m_iodevice(nullptr)
    , m_audioTimerID(0)
    , m_timeCounter(0)
    , m_previousElapsedTime(0)
{
    m_powerInterface = new QDBusInterface("com.canonical.powerd",
                                          "/com/canonical/powerd",
                                          "com.canonical.powerd", QDBusConnection::systemBus());
}

bool QSpotifyAudioThreadWorker::event(QEvent *e)
{
    // Ignore timer events to have less log trashing
    if(e->type() != QEvent::Timer)
        qDebug() << "QSpotifyAudioThreadWorker::event" << e->type();
    if (e->type() == StreamingStartedEventType) {
        QMutexLocker lock(&g_mutex);
        m_lockCookie = m_powerInterface->call("requestSysState", "cutespotify", 1).arguments().at(0).toString();
        QSpotifyStreamingStartedEvent *ev = static_cast<QSpotifyStreamingStartedEvent *>(e);
        startStreaming(ev->channels(), ev->sampleRate());
        e->accept();
        return true;
    } else if (e->type() == ResumeEventType) {
        QMutexLocker lock(&g_mutex);
        if (m_audioOutput) {
            m_lockCookie = m_powerInterface->call("requestSysState", "cutespotify", 1).arguments().at(0).toString();
            m_audioOutput->resume();
            m_audioTimerID = startTimer(AUDIOSTREAM_UPDATE_INTERVAL);
        }
        e->accept();
        return true;
    } else if (e->type() == SuspendEventType) {
        QMutexLocker lock(&g_mutex);
        if (m_audioOutput) {
            killTimer(m_audioTimerID);
            m_audioOutput->suspend();
            m_powerInterface->call("clearSysState", m_lockCookie);
        }
        e->accept();
        return true;
    } else if (e->type() == AudioStopEventType) {
        QMutexLocker lock(&g_mutex);
        killTimer(m_audioTimerID);
        g_buffer.close();
        if (m_audioOutput) {
            m_audioOutput->suspend();
            m_audioOutput->stop();
            m_audioOutput->deleteLater();
            m_audioOutput = nullptr;
            m_iodevice = nullptr;
            m_powerInterface->call("clearSysState", m_lockCookie);
        }
        e->accept();
        return true;
    } else if (e->type() == ResetBufferEventType) {
        QMutexLocker lock(&g_mutex);
        if (m_audioOutput) {
            killTimer(m_audioTimerID);
            m_audioOutput->suspend();
            m_audioOutput->stop();
            g_buffer.close();
            g_buffer.open();
            m_audioOutput->reset();
            m_iodevice = m_audioOutput->start();
            m_audioOutput->suspend();
            m_audioTimerID = startTimer(AUDIOSTREAM_UPDATE_INTERVAL);
            m_timeCounter = 0;
            m_previousElapsedTime = 0;
            m_audioOutput->resume();
            m_powerInterface->call("clearSysState", m_lockCookie);
        }
        e->accept();
        return true;
    } else if (e->type() == QEvent::Timer) {
        QTimerEvent *te = static_cast<QTimerEvent *>(e);
        if (te->timerId() == m_audioTimerID) {
            updateAudioBuffer();
            e->accept();
            return true;
        }
    }
    return QObject::event(e);
}

void QSpotifyAudioThreadWorker::startStreaming(int channels, int sampleRate)
{
    qDebug() << "QSpotifyAudioThreadWorker::startStreaming";
    if (!m_audioOutput) {
        QAudioFormat af;
        af.setChannelCount(channels);
        af.setCodec("audio/pcm");
        af.setSampleRate(sampleRate);
        af.setSampleSize(16);
        af.setSampleType(QAudioFormat::SignedInt);

        QAudioDeviceInfo info(QAudioDeviceInfo::defaultOutputDevice());
        if (!info.isFormatSupported(af)) {
            QList<QAudioDeviceInfo> devices = QAudioDeviceInfo::availableDevices(QAudio::AudioOutput);
            for (int i = 0; i < devices.size(); i++) {
                QAudioDeviceInfo dev = devices[i];
                qWarning() << dev.deviceName();
            }
            QCoreApplication::postEvent(QSpotifySession::instance(), new QEvent(QEvent::Type(StopEventType)));
            return;
        }

        m_audioOutput = new QAudioOutput(af);
        connect(m_audioOutput, SIGNAL(stateChanged(QAudio::State)), QSpotifySession::instance(), SLOT(audioStateChange(QAudio::State)));
        m_audioOutput->setBufferSize(BUF_SIZE);

        m_iodevice = m_audioOutput->start();
        m_audioOutput->suspend();
        m_audioTimerID = startTimer(AUDIOSTREAM_UPDATE_INTERVAL);
        m_timeCounter = 0;
        m_previousElapsedTime = 0;
        m_audioOutput->resume();
    }
}

void QSpotifyAudioThreadWorker::updateAudioBuffer()
{
//    qDebug() << "QSpotifyAudioThreadWorker::updateAudioBuffer";
    if (!m_audioOutput)
        return;

    g_mutex.lock();
    int toRead = qMin(g_buffer.filledBytes(), m_audioOutput->bytesFree());
    char data[toRead];
    int read =  g_buffer.read(&data[0], toRead);
    g_mutex.unlock();

    m_iodevice->write(&data[0], read);

    m_timeCounter += AUDIOSTREAM_UPDATE_INTERVAL;
    if (m_timeCounter >= 1000) {
        m_timeCounter = 0;
        int elapsedTime = int(m_audioOutput->processedUSecs() / 1000);
        QCoreApplication::postEvent(QSpotifySession::instance(), new QSpotifyTrackProgressEvent(elapsedTime - m_previousElapsedTime));
        m_previousElapsedTime = elapsedTime;
    }
}
