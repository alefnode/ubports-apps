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


#include "qspotifyplaylist.h"

#include <algorithm>

#include <QtCore/QCoreApplication>
#include <QtCore/QDebug>
#include <QtCore/QHash>

#include "qspotifyalbumbrowse.h"
#include "qspotifyplayqueue.h"
#include "qspotifysession.h"
#include "qspotifytrack.h"
#include "qspotifyuser.h"
#include "qspotifycachemanager.h"

static QHash<QString, byte*> m_imagePointers;

class QSpotifyTracksAddedEvent : public QEvent
{
public:
    QSpotifyTracksAddedEvent(QVector<sp_track*> tracks, int pos)
        : QEvent(Type(User + 3))
        , m_tracks(tracks)
        , m_position(pos)
    { }

    QVector<sp_track*> tracks() const { return m_tracks; }
    int position() const { return m_position; }

private:
    QVector<sp_track*> m_tracks;
    int m_position;
};

class QSpotifyTracksRemovedEvent : public QEvent
{
public:
    QSpotifyTracksRemovedEvent(QVector<int> positions)
        : QEvent(Type(User + 4))
        , m_positions(positions)
    { }

    QVector<int> positions() const { return m_positions; }

private:
    QVector<int> m_positions;
};

class QSpotifyTracksMovedEvent : public QEvent
{
public:
    QSpotifyTracksMovedEvent(QVector<int> positions, int newpos)
        : QEvent(Type(User + 5))
        , m_positions(positions)
        , m_newposition(newpos)
    { }

    QVector<int> positions() const { return m_positions; }
    int newPosition() const { return m_newposition; }

private:
    QVector<int> m_positions;
    int m_newposition;
};

class QSpotifyTrackSeenEvent : public QEvent
{
public:
    QSpotifyTrackSeenEvent(int pos, bool seen)
        : QEvent(Type(User + 6))
        , m_position(pos)
        , m_seen(seen)
    { }

    int position() const { return m_position; }
    bool seen() const { return m_seen; }

private:
    int m_position;
    bool m_seen;
};

static void callback_playlist_state_changed(sp_playlist *, void *objectPtr)
{
    QCoreApplication::postEvent(static_cast<QSpotifyPlaylist*>(objectPtr), new QEvent(QEvent::User));
}

static void callback_playlist_metadata_updated(sp_playlist *, void *objectPtr)
{
    QCoreApplication::postEvent(static_cast<QSpotifyPlaylist*>(objectPtr), new QEvent(QEvent::Type(QEvent::User + 1)));
}

static void callback_playlist_renamed(sp_playlist *, void *objectPtr)
{
    QCoreApplication::postEvent(static_cast<QSpotifyPlaylist*>(objectPtr), new QEvent(QEvent::Type(QEvent::User + 2)));
}

static void callback_tracks_added(sp_playlist *, sp_track *const *tracks, int num_tracks, int position, void *objectPtr)
{
    QVector<sp_track*> vec;
    for (int i = 0; i < num_tracks; ++i)
        if (tracks[i] != nullptr)
            vec.append(tracks[i]);
    QCoreApplication::postEvent(static_cast<QSpotifyPlaylist*>(objectPtr), new QSpotifyTracksAddedEvent(vec, position));
}

static void callback_tracks_removed(sp_playlist *, const int *tracks, int num_tracks, void *objectPtr)
{
    QVector<int> vec;
    for (int i = 0; i < num_tracks; ++i)
        vec.append(tracks[i]);
    QCoreApplication::postEvent(static_cast<QSpotifyPlaylist*>(objectPtr), new QSpotifyTracksRemovedEvent(vec));
}

static void callback_tracks_moved(sp_playlist *, const int *tracks, int num_tracks, int new_position, void *objectPtr)
{
    QVector<int> vec;
    for (int i = 0; i < num_tracks; ++i)
        vec.append(tracks[i]);
    QCoreApplication::postEvent(static_cast<QSpotifyPlaylist*>(objectPtr), new QSpotifyTracksMovedEvent(vec, new_position));
}

static void callback_track_seen_changed(sp_playlist *, int position, bool seen, void *objectPtr)
{
    QCoreApplication::postEvent(static_cast<QSpotifyPlaylist*>(objectPtr), new QSpotifyTrackSeenEvent(position, seen));
}


QSpotifyPlaylist::QSpotifyPlaylist(Type type, sp_playlist *playlist, bool incrRefCount)
    : QSpotifyObject(true)
    , m_callbacks(nullptr)
    , m_type(type)
    , m_offlineStatus(No)
    , m_collaborative(false)
    , m_offlineDownloadProgress(0)
    , m_availableOffline(false)
    , m_hasImage(false)
    , m_skipUpdateTracks(false)
    , m_updateEventPosted(false)
{
    Q_ASSERT(playlist);
    m_trackList = nullptr;
    if (type != Folder && type != None)
        m_trackList = new QSpotifyTrackList(this, type == Starred || type == Inbox);

    if (incrRefCount)
        sp_playlist_add_ref(playlist);
    m_sp_playlist = playlist;
    connect(this, SIGNAL(dataChanged()), this, SIGNAL(playlistDataChanged()));
    connect(this, SIGNAL(isLoadedChanged()), this, SIGNAL(thisIsLoadedChanged()));
    connect(this, SIGNAL(playlistDataChanged()), this , SIGNAL(seenCountChanged()));
    connect(this, SIGNAL(playlistDataChanged()), this, SIGNAL(tracksChanged()));
}

QSpotifyPlaylist::~QSpotifyPlaylist()
{
    emit playlistDestroyed();
    auto ptr = m_imagePointers.take(m_hashKey);
    if(ptr) delete[] ptr;
    if (m_sp_playlist) {
        sp_playlist_remove_callbacks(m_sp_playlist, m_callbacks, nullptr);
        sp_playlist_release(m_sp_playlist);
    }
    delete m_callbacks;
}

bool QSpotifyPlaylist::isLoaded()
{
    return sp_playlist_is_loaded(m_sp_playlist);
}

bool QSpotifyPlaylist::updateData()
{
    bool updated = false;

    if (m_type != Folder) {
        if (auto rawName = sp_playlist_name(m_sp_playlist)) {
            QString name = QString::fromUtf8(rawName);
            if (m_name != name) {
                m_name = name;
                updated = true;
            }
        }
    }

    if (m_hashKey.isEmpty()) {
        auto link = sp_link_create_from_playlist(m_sp_playlist);
        if(link) {
            char buffer[200];
            int uriSize = sp_link_as_string(link, &buffer[0], 200);
            if(uriSize >= 200) {
                qWarning() << "Link is larger than buffer !!";
            }
            m_hashKey = QString::fromUtf8(&buffer[0], uriSize);
            sp_link_release(link);
            // this is not really an update as its used only internal.
        }
    }

    // Image
    if (m_ImageId.isEmpty()) {
        const int image_id_size = 20;
        byte *image_id_buffer = new byte[image_id_size];
        if(sp_playlist_get_image(m_sp_playlist, image_id_buffer)) {
            m_imagePointers.insert(m_hashKey, image_id_buffer);
            m_ImageId = m_hashKey;
            m_hasImage = true;
            updated = true;
        } else {
            delete[] image_id_buffer;
        }
    }

    if (auto rawOwner = sp_playlist_owner(m_sp_playlist)) {
        QString owner = QString::fromUtf8(sp_user_canonical_name(rawOwner));
        if (m_owner != owner) {
            m_owner = owner;
            updated = true;
        }
    }

    bool collab = sp_playlist_is_collaborative(m_sp_playlist);
    if (m_collaborative != collab) {
        m_collaborative = collab;
        updated = true;
    }

    if (m_trackList && m_trackList->isEmpty() && !m_skipUpdateTracks) {
        int count = sp_playlist_num_tracks(m_sp_playlist);
        int insertPos = -1; // Append
        if (m_type == Starred || m_type == Inbox) insertPos = 0; // Prepend
        for (int i = 0; i < count; ++i) {
            auto strack = sp_playlist_track(m_sp_playlist, i);
            if(!strack) {
                qWarning() << "###No strack";
                continue;
            }
            addTrack(strack, insertPos);
        }
        updated = true;
    }

    OfflineStatus os = OfflineStatus(sp_playlist_get_offline_status(QSpotifySession::instance()->spsession(), m_sp_playlist));
    if (m_offlineStatus != os) {
        if (os == Waiting && m_offlineTracks.count() == m_availableTracks.count())
            m_offlineStatus = Yes;
        else if (os == Yes && m_offlineTracks.count() < m_availableTracks.count())
            m_offlineStatus = Waiting;
        else
            m_offlineStatus = os;

        if (m_offlineStatus != No) {
            m_availableOffline = true;
            emit availableOfflineChanged();
        }

        updated = true;
    }

    if (m_offlineStatus == Downloading) {
        int dp = sp_playlist_get_offline_download_completed(QSpotifySession::instance()->spsession(), m_sp_playlist);
        if (m_offlineDownloadProgress != dp) {
            m_offlineDownloadProgress = dp;
            updated = true;
        }
    }

    if (!m_callbacks) {
        m_callbacks = new sp_playlist_callbacks;
        memset(m_callbacks, 0, sizeof(sp_playlist_callbacks));
        m_callbacks->playlist_state_changed = callback_playlist_state_changed;
        m_callbacks->description_changed = 0;
        m_callbacks->image_changed = 0;
        m_callbacks->playlist_metadata_updated = callback_playlist_metadata_updated;
        m_callbacks->playlist_renamed = callback_playlist_renamed;
        m_callbacks->playlist_update_in_progress = 0;
        m_callbacks->subscribers_changed = 0;
        m_callbacks->tracks_added = callback_tracks_added;
        m_callbacks->tracks_moved = callback_tracks_moved;
        m_callbacks->tracks_removed = callback_tracks_removed;
        m_callbacks->track_created_changed = 0;
        m_callbacks->track_message_changed = 0;
        m_callbacks->track_seen_changed = callback_track_seen_changed;
        sp_playlist_add_callbacks(m_sp_playlist, m_callbacks, this);
    }

    return updated;
}

std::shared_ptr<QSpotifyTrack> QSpotifyPlaylist::addTrack(sp_track *track, int pos)
{
    auto qtrack = QSpotifyCacheManager::instance().getTrack(track, this);

    if (pos == -1)
        m_trackList->appendRow(qtrack);
    else
        m_trackList->insertRow(pos, qtrack);
    m_tracksSet.insert(track);
    connect(qtrack.get(), SIGNAL(trackDataChanged()), this, SIGNAL(playlistDataChanged()));
    connect(qtrack.get(), SIGNAL(offlineStatusChanged()), this, SLOT(onTrackChanged()));
    connect(qtrack.get(), SIGNAL(isAvailableChanged()), this, SLOT(onTrackChanged()));
    if (m_type != Starred) {
        connect(QSpotifySession::instance()->user()->starredList(), SIGNAL(tracksAdded(QVector<sp_track*>)), qtrack.get(), SLOT(onStarredListTracksAdded(QVector<sp_track*>)));
        connect(QSpotifySession::instance()->user()->starredList(), SIGNAL(tracksRemoved(QVector<sp_track*>)), qtrack.get(), SLOT(onStarredListTracksRemoved(QVector<sp_track*>)));
    }
    if (m_type == Inbox) {
        connect(qtrack.get(), SIGNAL(seenChanged()), this, SIGNAL(seenCountChanged()));
    }
    qtrack->metadataUpdated();
    return qtrack;
}

bool QSpotifyPlaylist::event(QEvent *e)
{

    // FIXME correctly pass events to playqueue tracklist.
    if (e->type() == QEvent::User) {
        m_skipUpdateTracks = true;
        metadataUpdated();
        m_skipUpdateTracks = false;
        e->accept();
        return true;
    } else if (e->type() == QEvent::User + 1) {
        // TracksMetadata updated
        if (m_trackList) {
            for (int i = 0; i < m_trackList->count(); ++i) {
                m_trackList->at(i)->metadataUpdated();
            }
        }
        e->accept();
        return true;
    } else if (e->type() == QEvent::User + 2) {
        // Playlist renamed
        m_name = QString::fromUtf8(sp_playlist_name(m_sp_playlist));
        postUpdateEvent();
        emit nameChanged();
        e->accept();
        return true;
    } else if (e->type() == QEvent::User + 3) {
        qDebug() << "Track add start";
        // TracksAdded event
        QSpotifyTracksAddedEvent *ev = static_cast<QSpotifyTracksAddedEvent *>(e);
        QVector<sp_track*> tracks = ev->tracks();
        int pos = ev->position();
        bool currentList = QSpotifySession::instance()->playQueue()->isCurrentTrackList(m_trackList);
        int amount = sp_playlist_num_tracks(m_sp_playlist);
        for (int i = 0; i < tracks.count(); ++i) {
            auto strack = tracks.at(i);
            if(!strack) {
                qWarning() << "## No track";
                continue;
            }
            if (pos < amount && strack == sp_playlist_track(m_sp_playlist, pos)) {
                auto t = addTrack(strack, pos++);
                if(currentList)
                    QSpotifySession::instance()->playQueue()->m_implicitTracks->appendRow(t);
            } else {
                qDebug() << "Unmatched track found";
            }
        }
        if(currentList)
            QSpotifySession::instance()->playQueue()->m_implicitTracks->setShuffle(QSpotifySession::instance()->playQueue()->m_implicitTracks->isShuffle());
        postUpdateEvent();
        if (m_type == Starred || m_type == Inbox)
            emit tracksAdded(tracks);
        m_trackList->setShuffle(m_trackList->isShuffle());
        qDebug() << "Track add end";
        e->accept();
        return true;
    } else if (e->type() == QEvent::User + 4) {
        // TracksRemoved event
        QSpotifyTracksRemovedEvent *ev = static_cast<QSpotifyTracksRemovedEvent *>(e);
        QVector<int> tracks = ev->positions();
        std::sort(tracks.begin(), tracks.end(), std::greater<int>());
        QVector<sp_track *> tracksSignal;

        bool isCurrentList = QSpotifySession::instance()->playQueue()->isCurrentTrackList(m_trackList);

        for (int i = 0; i < tracks.count(); ++i) {
            int pos = tracks.at(i);
            if (pos < 0 || pos >= m_trackList->count())
                continue;
            if (auto tr = m_trackList->takeRow(pos)) {
                unregisterTrackType(tr);
                disconnect(tr.get(), SIGNAL(offlineStatusChanged()), this, SLOT(onTrackChanged()));
                disconnect(tr.get(), SIGNAL(isAvailableChanged()), this, SLOT(onTrackChanged()));
                tracksSignal.append(tr->m_sp_track);
                m_tracksSet.remove(tr->m_sp_track);

                if(isCurrentList) {
                    auto playQueueList = QSpotifySession::instance()->playQueue()->m_implicitTracks;
                    playQueueList->removeRow(playQueueList->indexOf(tr));
                }
            }
        }
        postUpdateEvent();
        if (m_type == Starred)
            emit tracksRemoved(tracksSignal);
        e->accept();
        m_trackList->setShuffle(m_trackList->isShuffle());
        auto playQueueList = QSpotifySession::instance()->playQueue()->m_implicitTracks;
        playQueueList->setShuffle(playQueueList->isShuffle());
        return true;
    } else if (e->type() == QEvent::User + 5) {
        // TracksMoved event
        QSpotifyTracksMovedEvent *ev = static_cast<QSpotifyTracksMovedEvent *>(e);
        QVector<int> positions = ev->positions();
        int newpos = ev->newPosition();
        QVector<std::shared_ptr<QSpotifyTrack> > tracks;
        for (int i = 0; i < positions.count(); ++i) {
            int pos = positions.at(i);
            if (pos < 0 || pos >= m_trackList->count())
                continue;
            tracks.append(m_trackList->at(pos));
            m_trackList->replace(pos, std::shared_ptr<QSpotifyTrack>());
        }
        for (int i = 0; i < tracks.count(); ++i)
            m_trackList->insertRow(newpos++, tracks.at(i));
        m_trackList->removeAll(std::shared_ptr<QSpotifyTrack>());
        postUpdateEvent();
        if (QSpotifySession::instance()->playQueue()->isCurrentTrackList(m_trackList))
            QSpotifySession::instance()->playQueue()->tracksUpdated();
        e->accept();
        return true;
    } else if (e->type() == QEvent::User + 6) {
        // TrackSeen event
        if (m_type == Inbox) {
            QSpotifyTrackSeenEvent *ev = static_cast<QSpotifyTrackSeenEvent*>(e);
            m_trackList->at(ev->position())->updateSeen(ev->seen());
        }
        e->accept();
        return true;
    } else if (e->type() == QEvent::User + 7) {
        emit dataChanged();
        m_updateEventPosted = false;
        e->accept();
        return true;
    }
    return QSpotifyObject::event(e);
}

void QSpotifyPlaylist::postUpdateEvent()
{
    if (!m_updateEventPosted) {
        QCoreApplication::postEvent(this, new QEvent(QEvent::Type(QEvent::User + 7)));
        m_updateEventPosted = true;
    }
}

void QSpotifyPlaylist::add(QSpotifyTrack *track)
{
    if (!track)
        return;

    sp_playlist_add_tracks(m_sp_playlist, const_cast<sp_track* const*>(&track->m_sp_track), 1, m_trackList->count(), QSpotifySession::instance()->spsession());
}

void QSpotifyPlaylist::remove(QSpotifyTrack *track)
{
    if (!track)
        return;

    int i = m_trackList->indexOf(track->shared_from_this());
    if (i > -1)
        sp_playlist_remove_tracks(m_sp_playlist, &i, 1);
}

void QSpotifyPlaylist::addAlbum(QSpotifyAlbumBrowse *album)
{
    if (!album || !album->m_albumTracks)
        return;

    int c = album->m_albumTracks->count();
    if (c < 1)
        return;

    const sp_track *tracks[c];
    for (int i = 0; i < c; ++i)
        tracks[i] = album->m_albumTracks->at(i)->sptrack();
    sp_playlist_add_tracks(m_sp_playlist, const_cast<sp_track* const*>(tracks), c, m_trackList->count(), QSpotifySession::instance()->spsession());
}

void QSpotifyPlaylist::rename(const QString &name)
{
    if (name.trimmed().isEmpty())
        return;

    QString n = name;
    if (n.size() > 255)
        n.resize(255);

    sp_playlist_rename(m_sp_playlist, n.toUtf8().constData());
}

int QSpotifyPlaylist::trackCount() const
{
    int c = 0;
    if (m_trackList) {
        for (int i = 0; i < m_trackList->count(); ++i) {
            if (m_trackList->at(i)->error() == QSpotifyTrack::Ok)
                ++c;
        }
    }
    return c;
}

int QSpotifyPlaylist::totalDuration() const
{
    if (!m_trackList)
        return 0;

    return m_trackList->totalDuration();
}

void QSpotifyPlaylist::removeFromContainer()
{
    QSpotifySession::instance()->user()->removePlaylist(this);
}

void QSpotifyPlaylist::deleteFolderContent()
{
    QSpotifySession::instance()->user()->deleteFolderAndContent(this);
}

bool QSpotifyPlaylist::isCurrentPlaylist() const
{
    return QSpotifySession::instance()->m_playQueue->m_sourceTrackList == m_trackList;
}

byte *QSpotifyPlaylist::getImageIdPtr(const QString &key)
{
    return m_imagePointers.value(key, nullptr);
}

void QSpotifyPlaylist::setCollaborative(bool c)
{
    sp_playlist_set_collaborative(m_sp_playlist, c);
}

void QSpotifyPlaylist::setAvailableOffline(bool offline)
{
    if (m_type == Folder) {
        for (int i = 0; i < m_availablePlaylists.count(); ++i)
            dynamic_cast<QSpotifyPlaylist *>(m_availablePlaylists.at(i))->setAvailableOffline(offline);
    } else {
        if (m_availableOffline == offline)
            return;

        m_availableOffline = offline;
        sp_playlist_set_offline_mode(QSpotifySession::instance()->spsession(), m_sp_playlist, offline);
    }
    emit availableOfflineChanged();
}

void QSpotifyPlaylist::play()
{
    if (!m_trackList || m_trackList->isEmpty())
        return;

    int i = (m_type == Starred || m_type == Inbox) ? m_trackList->previousAvailable(m_trackList->count())
                                                   : m_trackList->nextAvailable(-1);
    QSpotifySession::instance()->m_playQueue->playTrack(m_trackList, i);
}

void QSpotifyPlaylist::enqueue()
{
    if (m_type == Folder) {
        for (int i = 0; i < m_availablePlaylists.count(); ++i)
            dynamic_cast<QSpotifyPlaylist *>(m_availablePlaylists.at(i))->enqueue();
    } else {
        QSpotifySession::instance()->playQueue()->enqueueTracks(m_trackList);
    }
}

int QSpotifyPlaylist::unseenCount() const
{
    if (m_type != Inbox)
        return 0;

    int c = 0;
    for (int i = 0; i < m_trackList->count(); ++i) {
        std::shared_ptr<QSpotifyTrack> t = m_trackList->at(i);
        if (t->error() == QSpotifyTrack::Ok && !t->seen())
            ++c;
    }
    return c;
}

void QSpotifyPlaylist::onTrackChanged()
{
    if (!sender())
        return;

    std::shared_ptr<QSpotifyTrack> tr = (dynamic_cast<QSpotifyTrack *>(sender()))->shared_from_this();
    if (!tr)
        return;

    registerTrackType(tr);
}

void QSpotifyPlaylist::registerTrackType(std::shared_ptr<QSpotifyTrack> t)
{
    int oldCount = m_offlineTracks.count();
    if (t->offlineStatus() == QSpotifyTrack::Yes)
        m_offlineTracks.insert(t);
    else
        m_offlineTracks.remove(t);
    if ((oldCount == 0 && m_offlineTracks.count() > 0) || (oldCount == 1 && m_offlineTracks.count() == 0))
        emit hasOfflineTracksChanged();

    if (t->m_isAvailable) {
        m_availableTracks.insert(t);
    }
}

void QSpotifyPlaylist::unregisterTrackType(std::shared_ptr<QSpotifyTrack> t)
{
    m_offlineTracks.remove(t);
    m_availableTracks.remove(t);
}
