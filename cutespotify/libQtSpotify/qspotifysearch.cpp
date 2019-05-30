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


#include "qspotifysearch.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QEvent>
#include <QtCore/QMutexLocker>

#include <QtCore/QDebug>

#include <libspotify/api.h>

#include "qspotifyalbum.h"
#include "qspotifyartist.h"
#include "qspotifyplaylist.h"
#include "qspotifyplaylistsearchentry.h"
#include "qspotifysession.h"
#include "qspotifytrack.h"
#include "qspotifytracklist.h"
#include "qspotifyuser.h"
#include "qspotifyplayqueue.h"
#include "qspotifycachemanager.h"

#include "listmodels/qspotifyartistlist.h"
#include "listmodels/qspotifyalbumlist.h"
#include "listmodels/qspotifyplaylistsearchlist.h"

enum SearchType{
    Albums = 0,
    Artists,
    Playlists,
    Tracks
};

struct SearchTypePass {
    SearchType mType;
    SearchTypePass(SearchType type) : mType(type) {}
};

class SearchResultEvent : public QEvent {
public:
    SearchResultEvent(sp_search *s, SearchTypePass *ptr) :
        QEvent(Type(QEvent::User)), m_search(s), m_Ptr(ptr) { Q_ASSERT(s); }
    ~SearchResultEvent() {
        if (m_Ptr) delete m_Ptr;
    }

    sp_search *getSearch() const {return m_search;}
    SearchTypePass *getPtr() const { return m_Ptr;}

private:
    sp_search *m_search;
    SearchTypePass *m_Ptr;
};

static QHash<sp_search *, QSpotifySearch *> g_searchObjects;
static QMutex g_mutex;

static void callback_search_complete(sp_search *result, void *opPtr)
{
    QMutexLocker lock(&g_mutex);
    QSpotifySearch *s = g_searchObjects.value(result);
    if (s)
        QCoreApplication::postEvent(s, new SearchResultEvent(result, static_cast<SearchTypePass*>(opPtr)));
}

QSpotifySearch::QSpotifySearch(QObject *parent, SearchType stype, bool preview)
    : QObject(parent)
    , m_sp_search(nullptr)
    , m_busy(false)
    , m_tracksLimit(100)
    , m_albumsLimit(50)
    , m_artistsLimit(50)
    , m_playlistsLimit(50)
    , m_numPreviewItems(3)
    , m_searchType(stype)
    , m_enablePreview(preview)
{
    m_trackResults = new QSpotifyTrackList(this);
    m_albumResults = new QSpotifyAlbumList(this);
    m_artistResults = new QSpotifyArtistList(this);
    m_playlistResults = new QSpotifyPlaylistSearchList(this);

    if(m_enablePreview) {
        m_trackResultsPreview = new QSpotifyTrackList(this);
        m_albumResultsPreview = new QSpotifyAlbumList(this);
        m_artistResultsPreview = new QSpotifyArtistList(this);
        m_playlistResultsPreview = new QSpotifyPlaylistSearchList(this);
    }
}

QSpotifySearch::~QSpotifySearch()
{
}

void QSpotifySearch::setQuery(const QString &q)
{
    if (q == m_query)
        return;

    m_query = q;
    emit queryChanged();
}

void QSpotifySearch::search(bool preview)
{
    setBusy(true);

    QMutexLocker lock(&g_mutex);
    if (!m_query.isEmpty()) {
        if(preview && m_enablePreview) {
            m_sp_search = sp_search_create(
                        QSpotifySession::instance()->m_sp_session,
                        m_query.toUtf8().constData(),
                        0, m_numPreviewItems, 0, m_numPreviewItems,
                        0, m_numPreviewItems, 0, m_numPreviewItems,
                        sp_search_type(m_searchType), callback_search_complete, nullptr);
        } else {
            m_sp_search = sp_search_create(
                        QSpotifySession::instance()->m_sp_session,
                        m_query.toUtf8().constData(),
                        0, m_tracksLimit, 0, m_albumsLimit,
                        0, m_artistsLimit, 0, m_playlistsLimit,
                        sp_search_type(m_searchType), callback_search_complete, nullptr);
        }
        g_searchObjects.insert(m_sp_search, this);
    } else {
        populateResults(nullptr);
    }
}

void QSpotifySearch::searchAlbums()
{
    setBusy(true);

    if (!m_query.isEmpty()) {
        QMutexLocker lock(&g_mutex);
        auto typePtr = new SearchTypePass(Albums);
        m_sp_search = sp_search_create(
                    QSpotifySession::instance()->m_sp_session,
                    m_query.toUtf8().constData(),
                    0, 0, 0, m_albumsLimit,
                    0, 0, 0, 0,
                    sp_search_type(m_searchType), callback_search_complete, typePtr);
        g_searchObjects.insert(m_sp_search, this);
    }
}

void QSpotifySearch::searchArtists()
{
    setBusy(true);

    if (!m_query.isEmpty()) {
        QMutexLocker lock(&g_mutex);
        auto typePtr = new SearchTypePass(Artists);
        m_sp_search = sp_search_create(
                    QSpotifySession::instance()->m_sp_session,
                    m_query.toUtf8().constData(),
                    0, 0, 0, 0,
                    0, m_artistsLimit, 0, 0,
                    sp_search_type(m_searchType), callback_search_complete, typePtr);
        g_searchObjects.insert(m_sp_search, this);
    }
}

void QSpotifySearch::searchPlaylists()
{
    setBusy(true);

    if (!m_query.isEmpty()) {
        QMutexLocker lock(&g_mutex);
        auto typePtr = new SearchTypePass(Playlists);
        m_sp_search = sp_search_create(
                    QSpotifySession::instance()->m_sp_session,
                    m_query.toUtf8().constData(),
                    0, 0, 0, 0,
                    0, 0, 0, m_playlistsLimit,
                    sp_search_type(m_searchType), callback_search_complete, typePtr);
        g_searchObjects.insert(m_sp_search, this);
    }
}

void QSpotifySearch::searchTracks()
{
    setBusy(true);

    if (!m_query.isEmpty()) {
        QMutexLocker lock(&g_mutex);
        auto typePtr = new SearchTypePass(Tracks);
        m_sp_search = sp_search_create(
                    QSpotifySession::instance()->m_sp_session,
                    m_query.toUtf8().constData(),
                    0, m_tracksLimit, 0, 0,
                    0, 0, 0, 0,
                    sp_search_type(m_searchType), callback_search_complete, typePtr);
        g_searchObjects.insert(m_sp_search, this);
    }
}

void QSpotifySearch::clearSearch(sp_search *search)
{
    QMutexLocker lock(&g_mutex);
    if (search)
        sp_search_release(search);
    g_searchObjects.remove(search);
    if (search == m_sp_search)
        m_sp_search = nullptr;
}

bool QSpotifySearch::event(QEvent *e)
{
    if (e->type() == QEvent::User) {
        auto ev = static_cast<SearchResultEvent*>(e);
        if (ev) {
            if (auto search = ev->getSearch()) {
                g_mutex.lock();
                bool is_current = (m_sp_search == search);
                g_mutex.unlock();
                if (sp_search_error(search) == SP_ERROR_OK && is_current) {
                    if(ev->getPtr()) {
                        switch(ev->getPtr()->mType) {
                        case Albums:
                            populateAlbums(search);
                            break;
                        case Artists:
                            populateArtists(search);
                            break;
                        case Playlists:
                            populatePlaylists(search);
                            break;
                        case Tracks:
                            populateTracks(search);
                            break;
                        default:
                            populateResults(search);
                            break;
                        }
                    } else
                        populateResults(search);
                }
                clearSearch(search);
            }
        }
        setBusy(false);

        emit resultsChanged();

        e->accept();
        return true;
    }
    return QObject::event(e);
}

void QSpotifySearch::populateAlbums(sp_search *search)
{
    m_albumResults->clear();
    if (m_enablePreview)
        m_albumResultsPreview->clear();


    if (search) {
        int c = sp_search_num_albums(search);
        for (int i = 0; i < c; ++i) {
            sp_album *a = sp_search_album(search, i);
            if (!sp_album_is_available(a))
                continue;
            std::shared_ptr<QSpotifyAlbum> album = QSpotifyCacheManager::instance().getAlbum(a);
            m_albumResults->appendRow(album);
            if(m_enablePreview && i < m_numPreviewItems)
                m_albumResultsPreview->appendRow(album);
        }
    }
}

void QSpotifySearch::populateArtists(sp_search *search)
{
    m_artistResults->clear();
    if (m_enablePreview)
        m_artistResultsPreview->clear();

    if (search) {
        int c = sp_search_num_artists(search);
        for (int i = 0; i < c; ++i) {
            std::shared_ptr<QSpotifyArtist> artist = QSpotifyCacheManager::instance().getArtist(sp_search_artist(search, i));
            m_artistResults->appendRow(artist);
            if(m_enablePreview && i < m_numPreviewItems)
                m_artistResultsPreview->appendRow(artist);
        }
    }
}

void QSpotifySearch::populatePlaylists(sp_search *search)
{
    m_playlistResults->clear();
    if (m_enablePreview)
        m_playlistResultsPreview->clear();

    if (search) {
        int c = sp_search_num_playlists(search);
        for (int i = 0; i < c; ++i) {
            auto playlist = std::shared_ptr<QSpotifyPlaylistSearchEntry>(
                        new QSpotifyPlaylistSearchEntry(sp_search_playlist_name(search, i), sp_search_playlist(search, i)),
                        [] (QSpotifyPlaylistSearchEntry *pl) {pl->deleteLater();});
            playlist->init();
            m_playlistResults->appendRow(playlist);
            if(m_enablePreview && i < m_numPreviewItems)
                m_playlistResultsPreview->appendRow(playlist);
        }
    }
}

void QSpotifySearch::populateTracks(sp_search *search)
{
    m_trackResults->clear();
    if (m_enablePreview)
        m_trackResultsPreview->clear();

    auto sourceList = QSpotifySession::instance()->playQueue()->m_sourceTrackList;
    if(sourceList == m_trackResults || sourceList == m_trackResultsPreview) {
        QSpotifySession::instance()->playQueue()->m_sourceTrackList = nullptr;
    }

    if (search) {
        int c = sp_search_num_tracks(search);
        for (int i = 0; i < c; ++i) {
            if (auto strack = sp_search_track(search, i)) {
                auto track = QSpotifyCacheManager::instance().getTrack(strack);
                if(m_enablePreview && i < m_numPreviewItems)
                    m_trackResultsPreview->appendRow(track);
                m_trackResults->appendRow(track);
                connect(QSpotifySession::instance()->user()->starredList(), SIGNAL(tracksAdded(QVector<sp_track*>)), track.get(), SLOT(onStarredListTracksAdded(QVector<sp_track*>)));
                connect(QSpotifySession::instance()->user()->starredList(), SIGNAL(tracksRemoved(QVector<sp_track*>)), track.get(), SLOT(onStarredListTracksRemoved(QVector<sp_track*>)));
            }
        }
    }
}

void QSpotifySearch::setDidYouMean(sp_search *search)
{
    if (search)
        m_didYouMean = QString::fromUtf8(sp_search_did_you_mean(search));
}

void QSpotifySearch::populateResults(sp_search *search)
{
    if (search) {
        if (sp_search_error(search) != SP_ERROR_OK)
            return;

        populateAlbums(search);
        populateArtists(search);
        populatePlaylists(search);
        populateTracks(search);

        setDidYouMean(search);
    } else {
        m_albumResults->clear();
        m_artistResults->clear();
        m_playlistResults->clear();
        m_trackResults->clear();

        auto sourceList = QSpotifySession::instance()->playQueue()->m_sourceTrackList;
        if(sourceList == m_trackResults || sourceList == m_trackResultsPreview) {
            QSpotifySession::instance()->playQueue()->m_sourceTrackList = nullptr;
        }

        if (m_enablePreview) {
            m_albumResultsPreview->clear();
            m_artistResultsPreview->clear();
            m_playlistResultsPreview->clear();
            m_trackResultsPreview->clear();
        }
    }

    setBusy(false);

    emit resultsChanged();
}

void QSpotifySearch::setBusy(bool busy)
{
    m_busy = busy;
    emit busyChanged();
}
