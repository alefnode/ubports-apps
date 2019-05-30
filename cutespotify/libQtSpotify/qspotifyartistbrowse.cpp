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


#include "qspotifyartistbrowse.h"

#include <QtConcurrent/QtConcurrentRun>
#include <QtCore/QCoreApplication>
#include <QtCore/QEvent>
#include <QtCore/QHash>

#include <libspotify/api.h>

#include "qspotifyalbum.h"
#include "qspotifyartist.h"
#include "qspotifyplaylist.h"
#include "qspotifysession.h"
#include "qspotifytrack.h"
#include "qspotifytracklist.h"
#include "qspotifyuser.h"
#include "qspotifycachemanager.h"

#include "listmodels/qspotifyartistlist.h"
#include "listmodels/qspotifyalbumlist.h"

static QHash<sp_artistbrowse*, QSpotifyArtistBrowse*> g_artistBrowseObjects;
static QMutex g_mutex;

static void callback_artistbrowse_complete(sp_artistbrowse *result, void *)
{
    QMutexLocker lock(&g_mutex);
    QSpotifyArtistBrowse *o = g_artistBrowseObjects.value(result);
    if (o)
        QCoreApplication::postEvent(o, new QEvent(QEvent::User));
}

QSpotifyArtistBrowse::QSpotifyArtistBrowse(QObject *parent)
    : QObject(parent)
    , m_sp_artistbrowse(nullptr)
    , m_artist(nullptr)
    , m_busy(false)
    , m_topHitsReady(false)
    , m_dataReady(false)
{
    m_topTracks = new QSpotifyTrackList(this);
    m_similarArtists = new QSpotifyArtistList(this);
    m_albums = new QSpotifyAlbumList(this);
    m_topHitsSearch = new QSpotifySearch(this, QSpotifySearch::Standard);
    m_topHitsSearch->setTracksLimit(50);
    connect(m_topHitsSearch, SIGNAL(resultsChanged()), this, SLOT(processTopHits()));
}

QSpotifyArtistBrowse::~QSpotifyArtistBrowse()
{
    clearData();
}

void QSpotifyArtistBrowse::setArtist(std::shared_ptr<QSpotifyArtist> artist)
{
    if (m_artist == artist)
        return;
    clearData();
    m_artist = artist;
    emit artistChanged();

    if (!m_artist)
        return;

    m_topHitsReady = false;
    m_dataReady = false;
    m_busy = true;
    emit busyChanged();

    QMutexLocker lock(&g_mutex);
    m_sp_artistbrowse = sp_artistbrowse_create(QSpotifySession::instance()->spsession(),
                                               m_artist->spartist(),
                                               SP_ARTISTBROWSE_NO_TRACKS,
                                               callback_artistbrowse_complete, nullptr);
    Q_ASSERT(m_sp_artistbrowse);
    g_artistBrowseObjects.insert(m_sp_artistbrowse, this);

    m_topHitsSearch->setQuery(QString(QLatin1String("artist:\"%1\"")).arg(m_artist->name()));
    m_topHitsSearch->searchTracks();
}

bool QSpotifyArtistBrowse::event(QEvent *e)
{
    if (e->type() == QEvent::User) {
        g_mutex.lock();
        g_artistBrowseObjects.remove(m_sp_artistbrowse);
        g_mutex.unlock();
        processData();
        e->accept();
        return true;
    }
    return QObject::event(e);
}

void QSpotifyArtistBrowse::clearData()
{
    if (m_sp_artistbrowse) {
        QMutexLocker lock(&g_mutex);
        g_artistBrowseObjects.remove(m_sp_artistbrowse);
        sp_artistbrowse_release(m_sp_artistbrowse);
        m_sp_artistbrowse = nullptr;
    }
    m_biography.clear();
    m_topTracks->clear();
    m_albums->clear();
    m_albumsCount = 0;
    m_appearsOnCount = 0;
    m_singlesCount = 0;
    m_compilationsCount = 0;
    m_similarArtists->clear();
}

void QSpotifyArtistBrowse::processData()
{
    if (m_sp_artistbrowse) {
        m_dataReady = true;

        if (sp_artistbrowse_error(m_sp_artistbrowse) != SP_ERROR_OK)
            return;

        m_biography = QString::fromUtf8(sp_artistbrowse_biography(m_sp_artistbrowse)).split(QLatin1Char('\n'), QString::SkipEmptyParts);

        if (sp_artistbrowse_num_portraits(m_sp_artistbrowse) > 0) {
            sp_link *link = sp_link_create_from_artistbrowse_portrait(m_sp_artistbrowse, 0);
            if (link) {
                char buffer[200];
                int uriSize = sp_link_as_string(link, &buffer[0], 200);
                m_pictureId = QString::fromUtf8(&buffer[0], uriSize);
                sp_link_release(link);
            }
        }

        int c = qMin(80, sp_artistbrowse_num_albums(m_sp_artistbrowse));
        QList<std::shared_ptr<QSpotifyAlbum> > albums, singles, compilations, appearsOn;
        for (int i = 0; i < c; ++i) {
            sp_album *album = sp_artistbrowse_album(m_sp_artistbrowse, i);
            if (!sp_album_is_available(album))
                continue;
            std::shared_ptr<QSpotifyAlbum> qalbum = QSpotifyCacheManager::instance().getAlbum(album);

            if ((qalbum->type() == QSpotifyAlbum::Album || qalbum->type() == QSpotifyAlbum::Unknown) && qalbum->artist() == m_artist->name()) {
                qalbum->setSectionType("Albums");
                albums.append(qalbum);
                m_albumsCount++;
            } else if (qalbum->type() == QSpotifyAlbum::Single) {
                qalbum->setSectionType("Singles");
                singles.append(qalbum);
                m_singlesCount++;
            } else if (qalbum->type() == QSpotifyAlbum::Compilation) {
                qalbum->setSectionType("Compilations");
                compilations.append(qalbum);
                m_compilationsCount++;
            } else {
                qalbum->setSectionType("Appears on");
                appearsOn.append(qalbum);
                m_appearsOnCount++;
            }
        }
        m_albums->appendRows(albums + singles + compilations + appearsOn);

        c = sp_artistbrowse_num_similar_artists(m_sp_artistbrowse);
        for (int i = 0; i < c; ++i) {
            std::shared_ptr<QSpotifyArtist> artist = QSpotifyCacheManager::instance().getArtist(sp_artistbrowse_similar_artist(m_sp_artistbrowse, i));
            m_similarArtists->appendRow(artist);
        }

        if (m_topHitsReady) {
            m_busy = false;
            emit busyChanged();
            emit dataChanged();
        }
    }
}

void QSpotifyArtistBrowse::processTopHits()
{
    m_topHitsReady = true;
    m_topTracks->clear();
    int c = m_topHitsSearch->trackResults()->count();
    for (int i = 0; i < c && m_topTracks->count() < 10; ++i) {
        std::shared_ptr<QSpotifyTrack> t = m_topHitsSearch->trackResults()->at(i);
        QStringList artists = t->artists().split(", ");
        if (artists.contains(m_artist->name())) {
            m_topTracks->appendRow(t);
        }
    }

    if (m_dataReady) {
        m_busy = false;
        emit busyChanged();
        emit dataChanged();
    }
}
