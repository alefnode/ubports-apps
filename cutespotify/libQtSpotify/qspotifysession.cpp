/****************************************************************************
**
** Copyright (c) 2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Yoann Lopes (yoann.lopes@nokia.com)
**
** This file is part of the MeeSpot project.
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions
** are met:
**
** Redistributions of source code must retain the above copyright notice,
** this list of conditions and the following disclaimer.
**
** Redistributions in binary form must reproduce the above copyright
** notice, this list of conditions and the following disclaimer in the
** documentation and/or other materials provided with the distribution.
**
** Neither the name of Nokia Corporation and its Subsidiary(-ies) nor the names of its
** contributors may be used to endorse or promote products derived from
** this software without specific prior written permission.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
** FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
** TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
** PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
** LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
** NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
** SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**
** If you have questions regarding the use of this file, please contact
** Nokia at qt-info@nokia.com.
**
****************************************************************************/


#include "qspotifysession.h"

#include <QtCore/QBuffer>
#include <QtCore/QCoreApplication>
#include <QtCore/QDebug>
#include <QtCore/QEvent>
#include <QtCore/QIODevice>
#include <QtCore/QMutexLocker>
#include <QtCore/QSettings>
#include <QtCore/QStandardPaths>
#include <QtCore/QThread>
#include <QtCore/QTimer>
#include <QtCore/QProcess>
#include <QtGui/QDesktopServices>
#include <QtMultimedia/QAudioOutput>
#include <QtNetwork/QNetworkConfigurationManager>
#include <QKeyEvent>
#include <QDir>

#include <QtDBus/QDBusConnection>

#include <assert.h>

#include "qspotifyalbum.h"
#include "qspotifyartist.h"
#include "qspotifyplayqueue.h"
#include "qspotifytracklist.h"
#include "qspotifyevents.h"
#include "qspotifyuser.h"
#include "spotify_key.h"
#include "qspotifyplaylist.h"
#include "qspotifycachemanager.h"

#include "qspotifyaudiothreadworker.h"

#include "mpris/mprismediaplayer.h"
#include "mpris/mprismediaplayerplayer.h"

static QSpotifyAudioThreadWorker *g_audioWorker;

QSpotifySession *QSpotifySession::m_instance = nullptr;

static void SP_CALLCONV callback_logged_in(sp_session *, sp_error error)
{
    qDebug() << "Logged in";
    QCoreApplication::postEvent(QSpotifySession::instance(), new QSpotifyConnectionErrorEvent(error));
    if (error == SP_ERROR_OK)
        QCoreApplication::postEvent(QSpotifySession::instance(), new QEvent(QEvent::Type(LoggedInEventType)));
}

static void SP_CALLCONV callback_logged_out(sp_session *)
{
    qDebug() << "Logged out";
    QCoreApplication::postEvent(QSpotifySession::instance(), new QEvent(QEvent::Type(LoggedOutEventType)));
}

static void SP_CALLCONV callback_connection_error(sp_session *, sp_error error)
{
    qDebug() << "Connection error ";
    QCoreApplication::postEvent(QSpotifySession::instance(), new QSpotifyConnectionErrorEvent(error));
}

static void SP_CALLCONV callback_notify_main_thread(sp_session *)
{
    qDebug() << "Notify main thread";
    QCoreApplication::postEvent(QSpotifySession::instance(), new QEvent(QEvent::Type(NotifyMainThreadEventType)));
}

static void SP_CALLCONV callback_metadata_updated(sp_session *)
{
    qDebug() << "Metadata updated";
    QCoreApplication::postEvent(QSpotifySession::instance(), new QEvent(QEvent::Type(MetaDataEventType)));
}

static void SP_CALLCONV callback_userinfo_updated(sp_session* )
{
    qDebug() << "User info updated";
    QCoreApplication::postEvent(QSpotifySession::instance(), new QEvent(QEvent::Type(MetaDataEventType)));
}

static int SP_CALLCONV callback_music_delivery(sp_session *, const sp_audioformat *format, const void *frames, int num_frames)
{
//    qDebug() << "Music delivery";
    if (num_frames == 0)
        return 0;

    // We don't want to block here so just trylock
    if(!g_mutex.tryLock())
        return 0;

    if (!g_buffer.isOpen()) {
        g_buffer.open();
        QCoreApplication::postEvent(g_audioWorker,
                                    new QSpotifyStreamingStartedEvent(format->channels, format->sample_rate));
    }

    int availableFrames = (g_buffer.freeBytes()) / (sizeof(int16_t) * format->channels);
    int writtenFrames = qMin(num_frames, availableFrames);

    if (writtenFrames == 0) {
        g_mutex.unlock();
        return 0;
    }

    g_buffer.write((const char *) frames, writtenFrames * sizeof(int16_t) * format->channels);

    g_mutex.unlock();
    return writtenFrames;
}

static void SP_CALLCONV callback_end_of_track(sp_session *)
{
    qDebug() << "End of track";
    QCoreApplication::postEvent(QSpotifySession::instance(), new QEvent(QEvent::Type(EndOfTrackEventType)));
}

static void SP_CALLCONV callback_play_token_lost(sp_session *)
{
    qDebug() << "Play token lost";
    QCoreApplication::postEvent(QSpotifySession::instance(), new QEvent(QEvent::Type(PlayTokenLostEventType)));
}

static void SP_CALLCONV callback_log_message(sp_session *, const char *data)
{
    // Scrobble error doesn't actually work for authentication failures (only reports first failure)
    // So we have to parse the log for errors instead
    QString qsdata = QString(data);
    if(qsdata.contains("Scrobbling failure: 5001")) {
        QCoreApplication::postEvent(QSpotifySession::instance(), new QEvent(QEvent::Type(ScrobbleLoginErrorEventType)));
    }
    fprintf(stderr, "%s\n", data);
}

static void SP_CALLCONV callback_offline_error(sp_session *, sp_error error)
{
    qDebug() << "Offline error " << int(error);
    if (error != SP_ERROR_OK)
        QCoreApplication::postEvent(QSpotifySession::instance(), new QSpotifyOfflineErrorEvent(error));
}

static void SP_CALLCONV callback_scrobble_error(sp_session *, sp_error error)
{
    qDebug() << "Scrobble error " << int(error);
}

static void SP_CALLCONV callback_connectionstate_updated(sp_session *)
{
    qDebug() << "Connection state updated";
    QCoreApplication::postEvent(QSpotifySession::instance(), new QEvent(ConnectionStateUpdateEventType));
}

QSpotifySession::QSpotifySession()
    : QObject(0)
    , m_timerID(0)
    , m_sp_session(nullptr)
    , m_connectionStatus(LoggedOut)
    , m_connectionError(Ok)
    , m_connectionRules(AllowSyncOverWifi | AllowNetworkIfRoaming)
    , m_streamingQuality(Unknown)
    , m_syncQuality(Unknown)
    , m_syncOverMobile(false)
    , m_user(nullptr)
    , m_pending_connectionRequest(false)
    , m_isLoggedIn(false)
    , m_explicitLogout(false)
    , m_aboutToQuit(false)
    , m_offlineMode(false)
    , m_forcedOfflineMode(false)
    , m_ignoreNextConnectionError(false)
    , m_playQueue(new QSpotifyPlayQueue(this))
    , m_currentTrack(nullptr)
    , m_isPlaying(false)
    , m_currentTrackPosition(0)
    , m_currentTrackPlayedDuration(0)
    , m_previousTrackRemaining(0)
    , m_shuffle(false)
    , m_repeat(false)
    , m_repeatOne(false)
    , m_volumeNormalize(true)
    , m_trackChangedAutomatically(false)
{
    m_networkingStatus = new ubuntu::connectivity::NetworkingStatus(this);
    connect(m_networkingStatus, SIGNAL(statusChanged(Status)), this, SLOT(onOnlineChanged()));
    connect(m_networkingStatus, SIGNAL(statusChanged(Status)), this, SIGNAL(isOnlineChanged()));
    connect(m_networkingStatus, SIGNAL(limitationsChanged()), this, SLOT(configurationChanged()));
    
    QCoreApplication::setOrganizationName("CuteSpotify");
    QCoreApplication::setOrganizationDomain("com.mikeasoft.cutespotify");
    QCoreApplication::setApplicationName("CuteSpotify");

    connect(qApp, SIGNAL(aboutToQuit()), this, SLOT(initiateQuit()));

    m_audioThread = new QThread();
    g_audioWorker = new QSpotifyAudioThreadWorker();
    g_audioWorker->moveToThread(m_audioThread);
    connect(m_audioThread, SIGNAL(finished()), g_audioWorker, SLOT(deleteLater()));
    connect(m_audioThread, SIGNAL(finished()), m_audioThread, SLOT(deleteLater()));
    m_audioThread->start(QThread::HighestPriority);

//    QCoreApplication::instance()->installEventFilter(this);
}

void QSpotifySession::init()
{
    memset(&m_sp_callbacks, 0, sizeof(m_sp_callbacks));
    m_sp_callbacks.logged_in = callback_logged_in;
    m_sp_callbacks.logged_out = callback_logged_out;
    m_sp_callbacks.metadata_updated = callback_metadata_updated;
    m_sp_callbacks.connection_error = callback_connection_error;
    m_sp_callbacks.notify_main_thread = callback_notify_main_thread;
    m_sp_callbacks.music_delivery = callback_music_delivery;
    m_sp_callbacks.play_token_lost = callback_play_token_lost;
    m_sp_callbacks.log_message = callback_log_message;
    m_sp_callbacks.end_of_track = callback_end_of_track;
    m_sp_callbacks.userinfo_updated = callback_userinfo_updated;
    m_sp_callbacks.offline_error = callback_offline_error;
    m_sp_callbacks.connectionstate_updated = callback_connectionstate_updated;
    m_sp_callbacks.scrobble_error = callback_scrobble_error;

    QString dpString = settings.value("dataPath").toString();
    dataPath = (char *) calloc(strlen(dpString.toLatin1()) + 1, sizeof(char));
    strcpy(dataPath, dpString.toLatin1());

    memset(&m_sp_config, 0, sizeof(m_sp_config));
    m_sp_config.api_version = SPOTIFY_API_VERSION;
    m_sp_config.cache_location = dataPath;
    m_sp_config.settings_location = dataPath;
    m_sp_config.application_key = g_appkey;
    m_sp_config.application_key_size = g_appkey_size;
    m_sp_config.user_agent = "CuteSpot";
    m_sp_config.callbacks = &m_sp_callbacks;
    sp_error error = sp_session_create(&m_sp_config, &m_sp_session);

    if (error != SP_ERROR_OK)
    {
        fprintf(stderr, "failed to create session: %s\n",
                sp_error_message(error));

        m_sp_session = nullptr;
        return;
    }
    Q_ASSERT(m_sp_session);

    sp_session_set_cache_size(m_sp_session, 0);

    // Remove stored login information from older version of MeeSpot
    if (settings.contains("username")) {
        settings.remove("username");
        settings.remove("password");
    }
    // Remove old volume information
    if (settings.contains("volume")) {
        settings.remove("volume");
    }

    m_offlineMode = settings.value("offlineMode", false).toBool();

    checkNetworkAccess();

    StreamingQuality quality = StreamingQuality(settings.value("streamingQuality", int(LowQuality)).toInt());
    setStreamingQuality(quality);

    StreamingQuality syncQuality = StreamingQuality(settings.value("syncQuality", int(HighQuality)).toInt());
    setSyncQuality(syncQuality);

    bool syncMobile = settings.value("syncOverMobile", false).toBool();
    setSyncOverMobile(syncMobile);

    QString storedLogin = getStoredLoginInformation();
    if (!storedLogin.isEmpty()) {
        login(storedLogin);
    }

    bool shuffle = settings.value("shuffle", false).toBool();
    setShuffle(shuffle);

    bool repeat = settings.value("repeat", false).toBool();
    setRepeat(repeat);

    bool repeatOne = settings.value("repeatOne", false).toBool();
    setRepeatOne(repeatOne);

    bool volumeNormalizeSet = settings.value("volumeNormalize", true).toBool();
    setVolumeNormalize(volumeNormalizeSet);

    m_lfmLoggedIn = false;

    // Flush cache regularly as we might get killed by life cycle manager / OOM killer
    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(flush()));
    timer->start(60000);

    connect(this, SIGNAL(offlineModeChanged()), m_playQueue, SLOT(onOfflineModeChanged()));

    new MPRISMediaPlayer(this);
    new MPRISMediaPlayerPlayer(this);

    QDBusConnection::sessionBus().registerObject(QString("/org/mpris/MediaPlayer2"), this, QDBusConnection::ExportAdaptors);
    QDBusConnection::sessionBus().registerService("org.mpris.MediaPlayer2.CuteSpot");
}

QSpotifySession::~QSpotifySession()
{
    qDebug() << "QSpotifySession::cleanUp";
    if (m_sp_session)
        sp_session_release(m_sp_session);
    free(dataPath);
}

QSpotifySession *QSpotifySession::instance()
{
//    qDebug() << "QSpotifySession::instance";
    if (!m_instance) {
        m_instance = new QSpotifySession;
        m_instance->init();
    }
    return m_instance;
}

//bool QSpotifySession::eventFilter(QObject *obj, QEvent *e)
//{
//    if(e->type() == QEvent::KeyPress) {
//        QKeyEvent *ek = dynamic_cast<QKeyEvent *>(e);
//        int key = ek->key();
//        if (key == Qt::Key_VolumeUp || key == Qt::Key_VolumeDown) {
//            if (key == Qt::Key_VolumeUp && m_volume <= 90) {
//                m_volume += 10;
//            } else if (key == Qt::Key_VolumeDown && m_volume >= 10) {
//                m_volume -= 10;
//            }
//            setVolume(m_volume);
//            e->accept();
//            return true;
//        }
        // TODO this would be the headset key event, currently
        // it is only received when window is open.
        // also check #sailfishos log 11.7.2014 around 12:30:
        // lukedirtwalker: well, there is an hacky way... setting the
        // GRABBED_KEYS native property of your window to the keys you want to grab
//        if (key == Qt::Key_ToggleCallHangup) {
//            return true;
//        }
//    }

//    return QObject::eventFilter(obj, e);
//}

void QSpotifySession::lfmLogin(const QString &lfmUser, const QString &lfmPass)
{
    settings.setValue("lfmUser", lfmUser);
    settings.setValue("lfmPass", lfmPass);
    sp_session_set_social_credentials(m_sp_session, SP_SOCIAL_PROVIDER_LASTFM, lfmUser.toUtf8().constData(), lfmPass.toUtf8().constData());
    if(lfmUser == "") {
        m_lfmLoggedIn = false;
    } else {
        m_lfmLoggedIn = true;
    }
    emit lfmLoggedInChanged();
    setScrobble(m_scrobble);
}

void QSpotifySession::setScrobble(bool scrobble)
{
    m_scrobble = scrobble;
    settings.setValue("scrobble", m_scrobble);
    emit scrobbleChanged();
    sp_session_set_scrobbling(m_sp_session, SP_SOCIAL_PROVIDER_LASTFM, m_scrobble ? SP_SCROBBLING_STATE_LOCAL_ENABLED : SP_SCROBBLING_STATE_LOCAL_DISABLED);
}

bool QSpotifySession::event(QEvent *e)
{
    if (e->type() == NotifyMainThreadEventType) {
        qDebug() << "Process spotify event";
        processSpotifyEvents();
        e->accept();
        return true;
    } else if (e->type() == QEvent::Timer) {
        qDebug() << "Timer, start spotify events";
        QTimerEvent *te = static_cast<QTimerEvent *>(e);
        if (te->timerId() == m_timerID) {
            processSpotifyEvents();
            e->accept();
            return true;
        }
    } else if (e->type() == ConnectionErrorEventType) {
        qDebug() << "Connection error";
        QSpotifyConnectionErrorEvent *ev = static_cast<QSpotifyConnectionErrorEvent *>(e);
        setConnectionError(ConnectionError(ev->error()), QString::fromUtf8(sp_error_message(ev->error())));
        e->accept();
        return true;
    } else if (e->type() == MetaDataEventType) {
        qDebug() << "Meta data";
        emit metadataUpdated();
        e->accept();
        return true;
    } else if (e->type() == EndOfTrackEventType) {
        qDebug() << "End track";
        m_trackChangedAutomatically = true;
        playNext();
        e->accept();
        return true;
    } else if (e->type() == StopEventType) {
        qDebug() << "Stop";
        stop();
        e->accept();
        return true;
    } else if (e->type() == TrackProgressEventType) {
        qDebug() << "Track progress";
        if(!m_isPlaying) {
            e->accept();
            return true;
        }
        // Track progressed
        QSpotifyTrackProgressEvent *ev = static_cast<QSpotifyTrackProgressEvent *>(e);
        int currentTrackPositionDelta = ev->delta();
        if (m_previousTrackRemaining > 0) {
            // We're still playing the previous back from our buffer
            int fromPreviousTrack = qMin(currentTrackPositionDelta, m_previousTrackRemaining);
            currentTrackPositionDelta -= fromPreviousTrack;
            m_previousTrackRemaining -= fromPreviousTrack;
        }

        m_currentTrackPosition += currentTrackPositionDelta;
        m_currentTrackPlayedDuration += currentTrackPositionDelta;
        emit currentTrackPositionChanged();
        e->accept();
        return true;
    } else if (e->type() == SendImageRequestEventType) {
        qDebug() << "Send image request";
        QSpotifyRequestImageEvent *ev = static_cast<QSpotifyRequestImageEvent *>(e);
        sendImageRequest(ev->imageId());
        e->accept();
        return true;
    } else if (e->type() == ReceiveImageRequestEventType) {
        qDebug() << "Receive image request";
        QSpotifyReceiveImageEvent *ev = static_cast<QSpotifyReceiveImageEvent *>(e);
        receiveImageResponse(ev->image());
        e->accept();
        return true;
    } else if (e->type() == PlayTokenLostEventType) {
        qDebug() << "Play token lost";
        emit playTokenLost();
        pause();
        e->accept();
        return true;
    } else if (e->type() == LoggedInEventType) {
        qDebug() << "Logged in 1";
        onLoggedIn();
        e->accept();
        return true;
    } else if (e->type() == LoggedOutEventType) {
        qDebug() << "Logged out";
        onLoggedOut();
        e->accept();
        return true;
    } else if (e->type() == OfflineErrorEventType) {
        qDebug() << "Offline error";
        QSpotifyOfflineErrorEvent *ev = static_cast<QSpotifyOfflineErrorEvent *>(e);
        m_offlineErrorMessage = QString::fromUtf8(sp_error_message(ev->error()));
        emit offlineErrorMessageChanged();
        e->accept();
        return true;
    } else if (e->type() == ScrobbleLoginErrorEventType) {
        qDebug() << "Scrobble login error";
        m_lfmLoggedIn = false;
        emit lfmLoggedInChanged();
        emit lfmLoginError();
        e->accept();
        return true;
    } else if (e->type() == ConnectionStateUpdateEventType) {
        qDebug() << "Connectionstate update event";
        setConnectionStatus(ConnectionStatus(sp_session_connectionstate(m_sp_session)));
        if (m_offlineMode && m_connectionStatus == LoggedIn) {
            setConnectionRules(m_connectionRules | AllowNetwork);
            setConnectionRules(m_connectionRules & ~AllowNetwork);
        }
        e->accept();
        return true;
    }
    return QObject::event(e);
}

void QSpotifySession::initiateQuit()
{
    qDebug() << "QSpotifySession::initiateQuit";
    stop();
    m_audioThread->quit();
    m_audioThread->wait();
    if(!m_isLoggedIn) {
        this->deleteLater();
        return;
    }
    QEventLoop evLoop;
    evLoop.connect(this, SIGNAL(readyToQuit()), SLOT(quit()));
    m_aboutToQuit = true;
    QSpotifyCacheManager::instance().clearTables();
    logout(true);
    evLoop.exec();
    this->deleteLater();
}

void QSpotifySession::processSpotifyEvents()
{
    qDebug() << "QSpotifySession::processSpotifyEvents";
    if (m_timerID)
        killTimer(m_timerID);

    int nextTimeout = 0;

    if (!m_aboutToQuit)
        QSpotifyCacheManager::instance().clean();

    do {
        assert(isValid());

        qDebug() << "Processing events...";
        sp_session_process_events(m_sp_session, &nextTimeout);
    } while (nextTimeout == 0);
    m_timerID = startTimer(nextTimeout);
}

void QSpotifySession::setStreamingQuality(StreamingQuality q)
{
    qDebug() << "QSpotifySession::setStreamingQuality";
    if (m_streamingQuality == q)
        return;

    m_streamingQuality = q;
    settings.setValue("streamingQuality", int(q));
    sp_session_preferred_bitrate(m_sp_session, sp_bitrate(q));

    emit streamingQualityChanged();
}

void QSpotifySession::setSyncQuality(StreamingQuality q)
{
    qDebug() << "QSpotifySession::setSyncQuality" << q;
    if (m_syncQuality == q)
        return;

    m_syncQuality = q;
    settings.setValue("syncQuality", int(q));
    sp_session_preferred_offline_bitrate(m_sp_session, sp_bitrate(q), true);

    emit syncQualityChanged();
}

void QSpotifySession::onLoggedIn()
{
    qDebug() << "Logged in";

    if (m_user)
        return;

    m_isLoggedIn = true;
    m_user = new QSpotifyUser(sp_session_user(m_sp_session));
    m_user->init();

    setScrobble(settings.value("scrobble", false).toBool());
    lfmLogin(settings.value("lfmUser", "").toString(), settings.value("lfmPass", "").toString());

    m_pending_connectionRequest = false;
    emit pendingConnectionRequestChanged();
    emit isLoggedInChanged();
    emit userChanged();

    if(!m_uriToOpen.isEmpty()) {
        handleUri(m_uriToOpen);
        m_uriToOpen = "";
    }

    sp_session_flush_caches(m_sp_session);
    checkNetworkAccess();
    qDebug() << "Done";
}

void QSpotifySession::onLoggedOut()
{
    qDebug() << "QSpotifySession::onLoggedOut";
    if (!m_explicitLogout)
        return;

    if(m_aboutToQuit) {
        m_aboutToQuit = false;
        emit readyToQuit();
        return;
    }

    m_explicitLogout = false;
    m_isLoggedIn = false;

    m_pending_connectionRequest = false;
    emit pendingConnectionRequestChanged();
    emit isLoggedInChanged();
}

void QSpotifySession::setConnectionStatus(ConnectionStatus status)
{
    qDebug() << "QSpotifySession::setConnectionStatus" << status;
    if (m_connectionStatus == status)
        return;

    // If we are in offline mode we don't care that we are disconnected.
    if(m_offlineMode && status == Disconnected)
        return;

    m_connectionStatus = status;
    emit connectionStatusChanged();
}

void QSpotifySession::setConnectionError(ConnectionError error, const QString &message)
{
    qDebug() << "QSpotifySession::setConnectionError" << error << message;
    if (error == Ok || m_offlineMode)
        return;

    if (m_pending_connectionRequest) {
        m_pending_connectionRequest = false;
        emit pendingConnectionRequestChanged();
    }

    if (error == UnableToContactServer && m_ignoreNextConnectionError) {
        m_ignoreNextConnectionError = false;
        return;
    }

    if (error == UnableToContactServer) {
        setOfflineMode(true, true);
    }

    m_connectionError = error;
    m_connectionErrorMessage = message;
    if (error != Ok && m_currentTrack && !m_currentTrack->isAvailableOffline())
        pause();
    emit connectionErrorChanged();
}

void QSpotifySession::login(const QString &username, const QString &password)
{
    qDebug() << "QSpotifySession::login";
    if (!isValid() || m_isLoggedIn || m_pending_connectionRequest)
        return;

    m_pending_connectionRequest = true;
    emit pendingConnectionRequestChanged();
    emit loggingIn();

    if (password.isEmpty()) {
        qDebug() << "Relogin";
        sp_session_relogin(m_sp_session);
    } else {
        qDebug() << "Fresh login";
        sp_session_login(m_sp_session, username.toUtf8().constData(), password.toUtf8().constData(), true, NULL);
    }
}

void QSpotifySession::logout(bool keepLoginInfo)
{
    qDebug() << "QSpotifySession::logout";
    if (!m_isLoggedIn || m_pending_connectionRequest)
        return;

    stop();
    m_playQueue->clear();

    if (!keepLoginInfo) {
        setOfflineMode(false);
        sp_session_forget_me(m_sp_session);
    }

    m_explicitLogout = true;

    m_pending_connectionRequest = true;
    emit pendingConnectionRequestChanged();
    emit loggingOut();

    if(m_user) {
        m_user->deleteLater();
        m_user = nullptr;
    }
    sp_session_logout(m_sp_session);
}

void QSpotifySession::setShuffle(bool s)
{
    qDebug() << "QSpotifySession::setShuffle";
    if (m_shuffle == s)
        return;

    settings.setValue("shuffle", s);
    m_playQueue->setShuffle(s);
    m_shuffle = s;
    emit shuffleChanged();
}

void QSpotifySession::setRepeat(bool r)
{
    qDebug() << "QSpotifySession::setRepeat";
    if (m_repeat == r)
        return;

    settings.setValue("repeat", r);
    m_playQueue->setRepeat(r);
    m_repeat = r;
    emit repeatChanged();
}

void QSpotifySession::setRepeatOne(bool r)
{
    qDebug() << "QSpotifySession::setRepeatOne";
    if (m_repeatOne == r)
        return;

    settings.setValue("repeatOne", r);
    m_repeatOne = r;
    emit repeatOneChanged();
}

void QSpotifySession::setVolumeNormalize(bool normalize)
{
    qDebug() << "QSpotifySession::setVolumeNormalize" << normalize;
    if(m_volumeNormalize == normalize)
        return;

    QSettings s;
    s.setValue("volumeNormalize", normalize);
    m_volumeNormalize = normalize;

    if(sp_session_set_volume_normalization(m_sp_session, normalize) != SP_ERROR_OK)
        qDebug() << "Failed to set volume normalization";

    emit volumeNormalizeChanged();
}

void QSpotifySession::play(std::shared_ptr<QSpotifyTrack> track, bool restart)
{
    qDebug() << "QSpotifySession::play";
    if (track->error() != QSpotifyTrack::Ok || !track->isAvailable() || (m_currentTrack == track && !restart))
        return;

    if (m_currentTrack) {
        if (m_trackChangedAutomatically) {
            // We're done decoding the track, but we might not be done playing it
            m_previousTrackRemaining = m_currentTrack->duration() - m_currentTrackPosition;
        } else {
            m_previousTrackRemaining = 0;
        }
        sp_session_player_unload(m_sp_session);
        m_isPlaying = false;
        m_currentTrack.reset();
        m_currentTrackPosition = 0;
        m_currentTrackPlayedDuration = 0;
        // Only discard buffers if the track change was initialized manually
        // since we will otherwise potentially discard the end of the just played track
        if (!m_trackChangedAutomatically) {
            QCoreApplication::postEvent(g_audioWorker, new QEvent(QEvent::Type(ResetBufferEventType)));
        }
    } else {
        m_previousTrackRemaining = 0;
    }

    m_trackChangedAutomatically = false;

    if (!track->seen())
        track->setSeen(true);

    sp_error error = sp_session_player_load(m_sp_session, track->m_sp_track);
    if (error != SP_ERROR_OK) {
        fprintf(stderr, "failed to load track: %s\n",
                sp_error_message(error));
        return;
    }
    m_currentTrack = track;
    m_currentTrackPosition = 0;
    m_currentTrackPlayedDuration = 0;
    emit currentTrackChanged();
    emit currentTrackPositionChanged();

    beginPlayBack();
}

void QSpotifySession::beginPlayBack(bool notifyThread)
{
    qDebug() << "QSpotifySession::beginPlayBack";
    sp_session_player_play(m_sp_session, true);
    m_isPlaying = true;
    emit isPlayingChanged();

    if(notifyThread)
        QCoreApplication::postEvent(g_audioWorker, new QEvent(QEvent::Type(ResumeEventType)));
}

void QSpotifySession::pause(bool notifyThread)
{
    qDebug() << "QSpotifySession::pause";
    if (!m_isPlaying)
        return;

    sp_session_player_play(m_sp_session, false);
    m_isPlaying = false;
    emit isPlayingChanged();

    if(notifyThread)
        QCoreApplication::postEvent(g_audioWorker, new QEvent(QEvent::Type(SuspendEventType)));
}

void QSpotifySession::resume()
{
    qDebug () << "QSpotifySession::resume";
    if (m_isPlaying || !m_currentTrack)
        return;

    beginPlayBack();
}

void QSpotifySession::stop(bool dontEmitSignals)
{
    qDebug() << "QSpotifySession::stop";
    if (!m_isPlaying && !m_currentTrack)
        return;

    sp_session_player_unload(m_sp_session);
    m_isPlaying = false;
    m_currentTrack.reset();
    m_currentTrackPosition = 0;
    m_currentTrackPlayedDuration = 0;

    if (!dontEmitSignals) {
        emit isPlayingChanged();
        emit currentTrackChanged();
        emit currentTrackPositionChanged();
    }

    QCoreApplication::postEvent(g_audioWorker, new QEvent(QEvent::Type(AudioStopEventType)));
}

void QSpotifySession::seek(int offset)
{
    qDebug () << "QSpotifySession::seek";
    if (!m_currentTrack)
        return;

    sp_session_player_seek(m_sp_session, offset);

    m_currentTrackPosition = offset;
    m_previousTrackRemaining = 0;
    emit currentTrackPositionChanged();

    QCoreApplication::postEvent(g_audioWorker, new QEvent(QEvent::Type(ResetBufferEventType)));
}

void QSpotifySession::playNext()
{
    qDebug() << "QSpotifySession::playNext";
    m_playQueue->playNext(m_repeatOne);
}

void QSpotifySession::playPrevious()
{
    qDebug() << "QSpotifySession::playPrevious";
    m_playQueue->playPrevious();
}

void QSpotifySession::enqueue(std::shared_ptr<QSpotifyTrack> track)
{
    qDebug() << "QSpotifySession::enqeue";
    m_playQueue->enqueueTrack(track);
}

void QSpotifySession::audioStateChange(QAudio::State state)
{
    qDebug() << "audio state change";
    switch(state) {
    case QAudio::ActiveState:
        qDebug() << "Audio is now in active state";
        beginPlayBack(false);
        break;
    case QAudio::SuspendedState:
        qDebug() << "Audio is now in supended state";
        pause(false);
        break;
    default:
        qDebug() << "unhandled audioStateChange" << state;
        break;
    }
}

QString QSpotifySession::formatDuration(qint64 d) const
{
//    qDebug() << "QSpotifySession::formatDuration";
    d /= 1000;
    int s = d % 60;
    d /= 60;
    int m = d % 60;
    int h = d / 60;

    QString r;
    if (h > 0)
        r += QString::number(h) + QLatin1String(":");
    r += QLatin1String(m > 9 || h == 0 ? "" : "0") + QString::number(m) + QLatin1String(":");
    r += QLatin1String(s > 9 ? "" : "0") + QString::number(s);

    return r;
}

QString QSpotifySession::getStoredLoginInformation() const
{
    qDebug() << "QSpotifySession::getStoredLoginInformation";
    QString username;
    char buffer[200];
    int size = sp_session_remembered_user(m_sp_session, &buffer[0], 200);
    if (size > 0) {
        username = QString::fromUtf8(&buffer[0], size);
    }
    return username;
}

QImage QSpotifySession::requestSpotifyImage(const QString &id)
{
    qDebug() << "QSpotifySession::requestSpotifyImage";
    g_imageRequestMutex.lock();
    g_imageRequestConditions.insert(id, new QWaitCondition);
    QCoreApplication::postEvent(this, new QSpotifyRequestImageEvent(id));
    g_imageRequestConditions[id]->wait(&g_imageRequestMutex);
    delete g_imageRequestConditions.take(id);

    QImage im = g_imageRequestImages.take(id);

    g_imageRequestMutex.unlock();

    return im;
}

static void SP_CALLCONV callback_image_loaded(sp_image *image, void *)
{
    qDebug() << "callback_image_loaded";
    QCoreApplication::postEvent(QSpotifySession::instance(), new QSpotifyReceiveImageEvent(image));
}

void QSpotifySession::sendImageRequest(const QString &id)
{
    qDebug() << "QSpotifySession::sendImageRequest" << id;
    sp_image *image = nullptr;
    byte *idPtr = QSpotifyPlaylist::getImageIdPtr(id);
    if(idPtr)
        image = sp_image_create(m_sp_session, idPtr);
    else {
        sp_link *link = sp_link_create_from_string(id.toUtf8().constData());
        if(link) {
            image = sp_image_create_from_link(m_sp_session, link);
            sp_link_release(link);
        }
    }

    if (image) {
        g_imageRequestObject.insert(image, id);
        sp_image_add_load_callback(image, callback_image_loaded, nullptr);
    }
}

void QSpotifySession::receiveImageResponse(sp_image *image)
{
    Q_ASSERT(image);
    qDebug() << "QSpotifySession::receiveImageResponse";
    sp_image_remove_load_callback(image, callback_image_loaded, 0);

    QString id = g_imageRequestObject.take(image);
    QImage im;
    if (sp_image_error(image) == SP_ERROR_OK) {
        size_t dataSize;
        const void *data = sp_image_data(image, &dataSize);
        im = QImage::fromData(reinterpret_cast<const uchar *>(data), dataSize, "JPG");
    }

    sp_image_release(image);

    g_imageRequestMutex.lock();
    g_imageRequestImages.insert(id, im);
    g_imageRequestConditions[id]->wakeAll();
    g_imageRequestMutex.unlock();
}

bool QSpotifySession::isOnline() const
{
    qDebug() << "QSpotifySession::isOnline";
    return m_networkingStatus->status() == ubuntu::connectivity::NetworkingStatus::Status::Online;
}

void QSpotifySession::onOnlineChanged()
{
    qDebug() << "QSpotifySession::onOnlineChanged";
    checkNetworkAccess();
}

void QSpotifySession::configurationChanged()
{
    qDebug() << "QSpotifySession::configurationChanged";
    checkNetworkAccess();
}

void QSpotifySession::checkNetworkAccess()
{
    qDebug() << "QSpotifySession::checkNetworkAccess";
    if (!isOnline()) {
        sp_session_set_connection_type(m_sp_session, SP_CONNECTION_TYPE_NONE);
        setOfflineMode(true, true);
    } else {
        bool limited = m_networkingStatus->limitations().contains(ubuntu::connectivity::NetworkingStatus::Limitations::Bandwith);
        sp_connection_type type;
        if (!limited)
            type = SP_CONNECTION_TYPE_WIFI;
        else
            type = SP_CONNECTION_TYPE_MOBILE;

        sp_session_set_connection_type(m_sp_session, type);

        if (m_forcedOfflineMode)
            setOfflineMode(false, true);
        else
            setConnectionRules(m_offlineMode ? m_connectionRules & ~AllowNetwork :
                                               m_connectionRules | AllowNetwork);
    }
}

void QSpotifySession::setConnectionRules(ConnectionRules r)
{
    qDebug() << "QSpotifySession::setConnectionRules";
    if (m_connectionRules == r)
        return;

    m_connectionRules = r;
    sp_session_set_connection_rules(m_sp_session, sp_connection_rules(int(m_connectionRules)));
}

void QSpotifySession::setOfflineMode(bool on, bool forced)
{
    qDebug() << "QSpotifySession::setOfflineMode" << on << forced;
    if (m_offlineMode == on)
        return;

    m_offlineMode = on;

    if (m_offlineMode && m_currentTrack && !m_currentTrack->isAvailableOffline())
        stop();

    if (!forced) {
        settings.setValue("offlineMode", m_offlineMode);

        if (m_offlineMode)
            m_ignoreNextConnectionError = true;
    }

    m_forcedOfflineMode = forced && on;

    setConnectionRules(on ? m_connectionRules & ~AllowNetwork :
                           m_connectionRules | AllowNetwork);

    emit offlineModeChanged();
}

bool QSpotifySession::privateSession() const
{
    if(!m_isLoggedIn)
        return false;

    return sp_session_is_private_session(m_sp_session);
}

void QSpotifySession::setPrivateSession(bool on)
{
    qDebug() << "QSpotifySession::setPrivateSession " << on;

    if(!m_isLoggedIn)
        return;

    sp_session_set_private_session(m_sp_session, on);
}

void QSpotifySession::setSyncOverMobile(bool s)
{
    qDebug() << "QSpotifySession::setSyncOverMobile";
    if (m_syncOverMobile == s)
        return;

    m_syncOverMobile = s;

    settings.setValue("syncOverMobile", m_syncOverMobile);

    setConnectionRules(s ? m_connectionRules | AllowSyncOverMobile :
                          m_connectionRules & ~AllowSyncOverMobile);
    emit syncOverMobileChanged();
}

void QSpotifySession::clearCache() {
    qDebug() << "QSpotifySession::clearCache";
    QString dataPath = settings.value("dataPath").toString();
    if(dataPath.contains(".local/share") || dataPath.contains("/mnt/sdcard/")) {
        QDir *dataDir = new QDir(dataPath);
        dataDir->removeRecursively();
    }
}

void QSpotifySession::handleUri(QString uri) {
    qDebug() << "QSpotifySession::handleUri";
    if (!m_isLoggedIn) {
        m_uriToOpen = uri;
        return;
    }
    sp_link *link = sp_link_create_from_string(uri.toLatin1().data());
    sp_linktype link_type = sp_link_type(link);
    if (link_type == SP_LINKTYPE_TRACK) {
        sp_track *track = sp_link_as_track(link);
        std::shared_ptr<QSpotifyTrack> q_track(new QSpotifyTrack(track, nullptr));
        enqueue(q_track);
        play(q_track, true);
    }
}

void QSpotifySession::flush() {
    sp_session_flush_caches(m_sp_session);
}
