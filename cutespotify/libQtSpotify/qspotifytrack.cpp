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


#include "qspotifytrack.h"

#include <QtCore/QDebug>

#include "qspotifyalbum.h"
#include "qspotifyartist.h"
#include "qspotifyplaylist.h"
#include "qspotifyplayqueue.h"
#include "qspotifysession.h"
#include "qspotifytracklist.h"
#include "qspotifyuser.h"
#include "qspotifycachemanager.h"

uint qHash(const std::shared_ptr<QSpotifyTrack> &v) {
    std::hash<std::shared_ptr<QSpotifyTrack> > hash;
    return static_cast<uint>(hash(v));
}

QSpotifyTrack::QSpotifyTrack(sp_track *track, QSpotifyPlaylist *playlist)
    : QSpotifyObject(true)
    , m_playlist(playlist)
    , m_album(nullptr)
    , m_discNumber(0)
    , m_duration(0)
    , m_discIndex(0)
    , m_isAvailable(false)
    , m_numArtists(0)
    , m_popularity(0)
    , m_seen(true)
    , m_offlineStatus(No)
    , m_isCurrentPlayingTrack(false)
{
    Q_ASSERT(track);
    sp_track_add_ref(track);
    m_sp_track = track;
    m_error = TrackError(sp_track_error(m_sp_track));

    connect(QSpotifySession::instance(), SIGNAL(currentTrackChanged()), this, SLOT(onSessionCurrentTrackChanged()));
    connect(this, SIGNAL(dataChanged()), this, SIGNAL(trackDataChanged()));
    connect(QSpotifySession::instance(), SIGNAL(offlineModeChanged()), this, SLOT(onSessionOfflineModeChanged()));
}

QSpotifyTrack::~QSpotifyTrack()
{
    stop();
    if(m_sp_track)
        sp_track_release(m_sp_track);
}

bool QSpotifyTrack::isLoaded()
{
    return sp_track_is_loaded(m_sp_track);
}

bool QSpotifyTrack::updateData()
{
    bool updated = false;

    TrackError error = TrackError(sp_track_error(m_sp_track));
    if (m_error != error) {
        m_error = error;
        updated = true;
    }

    if (m_error == Ok) {
        if (auto rawName = sp_track_name(m_sp_track)) {
            QString name = QString::fromUtf8(rawName);
            if (m_name != name) {
                m_name = name;
                updated = true;
            }
        }
        int discNumber = sp_track_disc(m_sp_track);
        int duration = sp_track_duration(m_sp_track);
        int discIndex = sp_track_index(m_sp_track);
        bool isAvailable = sp_track_get_availability(QSpotifySession::instance()->m_sp_session, m_sp_track) == SP_TRACK_AVAILABILITY_AVAILABLE;
        int numArtists = sp_track_num_artists(m_sp_track);
        int popularity = sp_track_popularity(m_sp_track);
        OfflineStatus offlineSt = OfflineStatus(sp_track_offline_get_status(m_sp_track));
        if (m_playlist && m_playlist->type() == QSpotifyPlaylist::Inbox) {
            int tindex = m_playlist->m_trackList->indexOf(shared_from_this());

            if (tindex >= 0 && tindex < sp_playlist_num_tracks(m_playlist->m_sp_playlist)) {
                bool seen = sp_playlist_track_seen(m_playlist->m_sp_playlist, tindex);
                if (m_seen != seen)
                    updateSeen(seen);

                /*QString crea = QString::fromUtf8(sp_user_canonical_name(sp_playlist_track_creator(m_playlist->m_sp_playlist, tindex)));
                if (m_creator != crea) {
                    m_creator = crea;
                    updated = true;
                }*/

                int cd = sp_playlist_track_create_time(m_playlist->m_sp_playlist, tindex);
                QDateTime dt = QDateTime::fromTime_t(cd);
                if (m_creationDate != dt) {
                    m_creationDate = dt;
                    updated = true;
                }
            }
        }


        if (m_discNumber != discNumber) {
            m_discNumber = discNumber;
            updated = true;
        }
        if (m_duration != duration) {
            m_duration = duration;
            m_durationString = QSpotifySession::instance()->formatDuration(m_duration);
            updated = true;
        }
        if (m_discIndex != discIndex) {
            m_discIndex = discIndex;
            updated = true;
        }
        if (m_isAvailable != isAvailable) {
            m_isAvailable = isAvailable;
            emit isAvailableChanged();
            updated = true;
        }
        if (m_numArtists != numArtists) {
            m_numArtists = numArtists;
            updated = true;
        }
        if (m_popularity != popularity) {
            m_popularity = popularity;
            updated = true;
        }
        if (m_offlineStatus != offlineSt) {
            m_offlineStatus = offlineSt;
            emit offlineStatusChanged();
            updated = true;
        }

        if (!m_artist) {
            int count = sp_track_num_artists(m_sp_track);
            for (int i = 0; i < count; ++i) {
                if (auto sartist = sp_track_artist(m_sp_track, i)) {
                    if (auto artist = QSpotifyCacheManager::instance().getArtist(sartist)) {
                        if(0 == i)
                            m_artist = artist;
                        m_artistsString += artist->name();
                        if (i != count - 1)
                            m_artistsString += QLatin1String(", ");
                    }
                }
            }
            updated = true;
        }
        if (!m_album) {
            if (auto salb = sp_track_album(m_sp_track)) {
                if (auto alb = QSpotifyCacheManager::instance().getAlbum(salb)) {
                    m_album = alb;
                    updated = true;
                    m_albumString = m_album->name();
                }
            }
        }
    }

    return updated;
}

QString QSpotifyTrack::artists() const
{
    return m_artistsString;
}

QString QSpotifyTrack::album() const
{
    return m_albumString;
}

QString QSpotifyTrack::albumCoverId() const
{
    if (!m_album)
        return QString();

    return m_album->coverId();
}

bool QSpotifyTrack::isStarred() const
{
    return sp_track_is_starred(QSpotifySession::instance()->m_sp_session, m_sp_track);
}

void QSpotifyTrack::setIsStarred(bool v)
{
    sp_track_set_starred(QSpotifySession::instance()->m_sp_session, const_cast<sp_track* const*>(&m_sp_track), 1, v);
}

void QSpotifyTrack::pause()
{
    if (isCurrentPlayingTrack())
        QSpotifySession::instance()->pause();
}

void QSpotifyTrack::resume()
{
    if (isCurrentPlayingTrack())
        QSpotifySession::instance()->resume();
}

void QSpotifyTrack::stop()
{
    if (isCurrentPlayingTrack())
        QSpotifySession::instance()->stop();
}

void QSpotifyTrack::seek(int offset)
{
    if (!isCurrentPlayingTrack())
        return; // This should not happen.
    QSpotifySession::instance()->seek(offset);
}

void QSpotifyTrack::enqueue()
{
    QSpotifySession::instance()->enqueue(shared_from_this());
}

void QSpotifyTrack::removeFromPlaylist()
{
    if (m_playlist)
        m_playlist->remove(this);
}

void QSpotifyTrack::onSessionCurrentTrackChanged()
{
    bool newValue = QSpotifySession::instance()->currentTrackShared() == shared_from_this();
    if (m_isCurrentPlayingTrack != newValue) {
        m_isCurrentPlayingTrack = newValue;
        emit isCurrentPlayingTrackChanged();
        emit dataChanged();
    }
}

void QSpotifyTrack::onStarredListTracksAdded(QVector<sp_track *> v)
{
    if (v.contains(m_sp_track)) {
        emit isStarredChanged();
        emit dataChanged();
    }
}

void QSpotifyTrack::onStarredListTracksRemoved(QVector<sp_track *> v)
{
    if (v.contains(m_sp_track)) {
        emit isStarredChanged();
        emit dataChanged();
    }
}

void QSpotifyTrack::updateSeen(bool s)
{
    m_seen = s;
    emit seenChanged();
    emit dataChanged();
}

void QSpotifyTrack::destroy()
{
    // Even if we still are playing we are destroyed so we shouldn't stop the new music
    m_isCurrentPlayingTrack = false;
    // We don't care about signals if we are scheduled to be destroyed
    disconnect(QSpotifySession::instance(), SIGNAL(currentTrackChanged()), this, SLOT(onSessionCurrentTrackChanged()));
    disconnect(this, SIGNAL(dataChanged()), this, SIGNAL(trackDataChanged()));
    disconnect(QSpotifySession::instance(), SIGNAL(offlineModeChanged()), this, SLOT(onSessionOfflineModeChanged()));
    deleteLater();
}

void QSpotifyTrack::setSeen(bool s)
{
    if (!m_playlist)
        return;

    sp_playlist_track_set_seen(m_playlist->m_sp_playlist, m_playlist->m_trackList->indexOf(shared_from_this()), s);
}

bool QSpotifyTrack::isAvailableOffline() const
{
    return m_offlineStatus == Yes || m_offlineStatus == DoneResync;
}

bool QSpotifyTrack::isAvailable() const
{
    return m_isAvailable && (!QSpotifySession::instance()->offlineMode() || isAvailableOffline());
}

void QSpotifyTrack::onSessionOfflineModeChanged()
{
    if (!isAvailableOffline()) {
        emit isAvailableChanged();
        emit dataChanged();
    }
}
