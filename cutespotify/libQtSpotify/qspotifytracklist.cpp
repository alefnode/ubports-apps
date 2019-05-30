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


#include "qspotifytracklist.h"

#include "qspotifysession.h"
#include "qspotifyplayqueue.h"

QSpotifyTrackList::QSpotifyTrackList(QObject *parent, bool reverse)
    : ListModelBase<QSpotifyTrack>(parent)
    , m_reverse(reverse)
    , m_currentIndex(0)
    , m_currentTrack(0)
    , m_shuffle(false)
    , m_shuffleIndex(0)
{
    m_roles[NameRole] = "trackName";
    m_roles[ArtistsRole] = "artists";
    m_roles[AlbumRole] = "album";
    m_roles[AlbumCoverRole] = "albumCoverId";
    m_roles[DiscNumberRole] = "discNumber";
    m_roles[DurationRole] = "trackDuration";
    m_roles[DurationMsRole] = "durationMs";
    m_roles[ErrorRole] = "error";
    m_roles[DiscIndexRole] = "discIndex";
    m_roles[IsAvailableRole] = "isAvailable";
    m_roles[IsStarredRole] = "isStarred";
    m_roles[PopularityRole] = "popularity";
    m_roles[IsCurrentPlayingTrackRole] = "isCurrentPlayingTrack";
    m_roles[SeenRole] = "seen";
    m_roles[CreatorRole] = "creator";
    m_roles[CreationDateRole] = "creationDate";
    m_roles[AlbumObjectRole] = "albumObject";
    m_roles[ArtistObjectRole] = "artistObject";
    m_roles[OfflineStatusRole] = "offlineStatus";
    m_roles[RawPtrRole] = "rawPtr";
}

QVariant QSpotifyTrackList::data(const QModelIndex &index, int role) const
{
    if(index.row() < 0 || index.row() >= m_dataList.size())
        return QVariant();
    auto track = m_dataList.at(index.row());
    switch(role) {
    case NameRole:
        return track->name();
    case ArtistsRole:
        return track->artists();
    case AlbumRole:
        return track->album();
    case AlbumCoverRole:
        return track->albumCoverId();
    case DiscNumberRole:
        return track->discNumber();
    case DurationRole:
        return track->durationString();
    case DurationMsRole:
        return track->duration();
    case ErrorRole:
        return track->error();
    case DiscIndexRole:
        return track->discIndex();
    case IsAvailableRole:
        return track->isAvailable();
    case IsStarredRole:
        return track->isStarred();
    case PopularityRole:
        return track->popularity();
    case IsCurrentPlayingTrackRole:
        return track->isCurrentPlayingTrack();
    case SeenRole:
        return track->seen();
    case CreatorRole:
        return track->creator();
    case CreationDateRole:
        return track->creationDate();
        // TODO
    case AlbumObjectRole:
        return QVariant();
    case ArtistObjectRole:
        return QVariant();
    case OfflineStatusRole:
        return QVariant();
    case RawPtrRole:
        return QVariant::fromValue<QSpotifyTrack *>(track.get());
    default:
        return QVariant();
    }
}

void QSpotifyTrackList::play()
{
    if (count() == 0)
        return;

    if (m_shuffle)
        playTrackAtIndex(m_shuffleList.first());
    else
        playTrackAtIndex(m_reverse ? previousAvailable(count()) : nextAvailable(-1));
}

void QSpotifyTrackList::playTrack(int index)
{
    if(at(index)->isCurrentPlayingTrack()) {
        // just adapt the tracklist (it might be that we have now a diffrent tracklist)
        QSpotifySession::instance()->m_playQueue->playFromDifferentTrackList(this);
    } else {
        QSpotifySession::instance()->m_playQueue->playTrack(this, index);
    }
}

bool QSpotifyTrackList::playTrackAtIndex(int i)
{
    if (i < 0 || i >= count()) {
        m_currentTrack.reset();
        m_currentIndex = 0;
        emit currentPlayIndexChanged();
        return false;
    }

    if (m_shuffle)
        m_shuffleIndex = m_shuffleList.indexOf(i);
    m_currentTrack = at(i);
    m_currentIndex = i;
    emit currentPlayIndexChanged();
    playCurrentTrack();
    return true;
}

bool QSpotifyTrackList::next()
{
    if (m_shuffle) {
        if (m_shuffleIndex + 1 >= m_shuffleList.count()) {
            m_currentTrack.reset();
            return false;
        }
        return playTrackAtIndex(m_shuffleList.at(m_shuffleIndex + 1));
    } else {
        int index = indexOf(m_currentTrack);
        if (index == -1) {
            int newi = qMin(m_currentIndex, count() - 1);
            return playTrackAtIndex(m_reverse ? previousAvailable(newi) : nextAvailable(newi - 1));
        }
        return playTrackAtIndex(m_reverse ? previousAvailable(index) : nextAvailable(index));
    }
}

bool QSpotifyTrackList::previous()
{
    if (m_shuffle) {
        if (m_shuffleIndex - 1 < 0) {
            m_currentTrack.reset();
            return false;
        }
        return playTrackAtIndex(m_shuffleList.at(m_shuffleIndex - 1));
    } else {
        int index = indexOf(m_currentTrack);
        if (index == -1) {
            int newi = qMin(m_currentIndex, count() - 1);
            return playTrackAtIndex(m_reverse ? nextAvailable(newi - 1) : previousAvailable(newi));
        }
        return playTrackAtIndex(m_reverse ? nextAvailable(index) : previousAvailable(index));
    }
}

void QSpotifyTrackList::playLast()
{
    if (count() == 0)
        return;

    if (m_shuffle)
        playTrackAtIndex(m_shuffleList.last());
    else
        playTrackAtIndex(m_reverse ? nextAvailable(-1) : previousAvailable(count()));
}

void QSpotifyTrackList::playCurrentTrack()
{
    if (!m_currentTrack)
        return;

    if (m_currentTrack->isLoaded())
        onTrackReady();
    else
        connect(m_currentTrack.get(), SIGNAL(isLoadedChanged()), this, SLOT(onTrackReady()));
}

void QSpotifyTrackList::onTrackReady()
{
    disconnect(this, SLOT(onTrackReady()));
    QSpotifySession::instance()->play(m_currentTrack);
}

void QSpotifyTrackList::setShuffle(bool s)
{
    m_shuffle = s;

    m_shuffleList.clear();
    m_shuffleIndex = 0;
    bool currentTrackStillExists = m_currentTrack && contains(m_currentTrack);

    if (m_shuffle) {
        qsrand(QTime::currentTime().msec());
        int currentTrackIndex = 0;
        if (currentTrackStillExists) {
            currentTrackIndex = indexOf(m_currentTrack);
            m_shuffleList.append(currentTrackIndex);
        }
        QList<int> indexes;
        for (int i = 0; i < count(); ++i) {
            if ((currentTrackStillExists && i == currentTrackIndex) || !at(i)->isAvailable())
                continue;
            indexes.append(i);
        }
        while (!indexes.isEmpty()) {
            int i = indexes.takeAt(indexes.count() == 1 ? 0 : (qrand() % (indexes.count() - 1)));
            m_shuffleList.append(i);
        }
    }
}

int QSpotifyTrackList::removeAll(const std::shared_ptr<QSpotifyTrack> ptr)
{
    int count = 0;
    beginResetModel();
    count = m_dataList.removeAll(ptr);
    endResetModel();
    return count;
}

int QSpotifyTrackList::totalDuration() const
{
    qint64 total = 0;
    for (int i = 0; i < count(); ++i)
        total += at(i)->duration();

    return total;
}

int QSpotifyTrackList::nextAvailable(int i)
{
    do {
        ++i;
    } while (i < count() && !at(i)->isAvailable());
    return i;
}

int QSpotifyTrackList::previousAvailable(int i)
{
    do {
        --i;
    } while (i > -1 && !at(i)->isAvailable());
    return i;
}
