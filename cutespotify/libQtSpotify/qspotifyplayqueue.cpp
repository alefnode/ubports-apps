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


#include "qspotifyplayqueue.h"

#include "qspotifysession.h"
#include "qspotifytracklist.h"

QSpotifyPlayQueue::QSpotifyPlayQueue(QObject *parent)
    : QObject(parent)
    , m_currentExplicitTrack(nullptr)
    , m_sourceTrackList(nullptr)
    , m_currentTrackIndex(0)
    , m_shuffle(false)
    , m_repeat(false)
{
    m_implicitTracks = new QSpotifyTrackList(this);
}

QSpotifyPlayQueue::~QSpotifyPlayQueue()
{
    clear();
}

void QSpotifyPlayQueue::playTrack(QSpotifyTrackList *list, int index)
{
    if (m_currentExplicitTrack) {
        m_currentExplicitTrack.reset();
    }

    if (m_sourceTrackList != list) {
        clearTrackList();

        m_sourceTrackList = list;

        if(m_sourceTrackList) {
            int c = m_sourceTrackList->count();
            for(int i = 0; i < c; ++i)
                m_implicitTracks->appendRow(m_sourceTrackList->at(i));
        }

    }
    m_implicitTracks->playTrackAtIndex(index);
    m_implicitTracks->setShuffle(m_shuffle);

    emit tracksChanged();
}

void QSpotifyPlayQueue::playFromDifferentTrackList(QSpotifyTrackList *list)
{
    if (m_sourceTrackList != list) {
        clearTrackList();

        m_sourceTrackList = list;

        if(m_sourceTrackList) {
            int c = m_sourceTrackList->count();
            for(int i = 0; i < c; ++i)
                m_implicitTracks->appendRow(m_sourceTrackList->at(i));
        }
    }
}

void QSpotifyPlayQueue::enqueueTrack(std::shared_ptr<QSpotifyTrack> track)
{
    m_explicitTracks.enqueue(track);

    emit tracksChanged();
}

void QSpotifyPlayQueue::enqueueTracks(QSpotifyTrackList *tracks, bool reverse)
{
    if(reverse) {
        for(int i = tracks->count(); i >= 0; --i) {
            std::shared_ptr<QSpotifyTrack> t = tracks->at(i);
            m_explicitTracks.enqueue(t);
        }
    } else {
        for (int i = 0; i < tracks->count(); ++i) {
            std::shared_ptr<QSpotifyTrack> t = tracks->at(i);
            m_explicitTracks.enqueue(t);
        }
    }
    emit tracksChanged();
}

void QSpotifyPlayQueue::selectTrack(int index)
{
    auto track = m_implicitTracks->at(index);
    if (m_currentExplicitTrack == track || m_implicitTracks->m_currentTrack == track)
        return;

    if (m_currentExplicitTrack) {
        m_currentExplicitTrack.reset();
    }

    int explicitPos = m_explicitTracks.indexOf(track);
    if (explicitPos != -1) {
        m_explicitTracks.removeAt(explicitPos);
        m_currentExplicitTrack = track;
        if (m_currentExplicitTrack->isLoaded())
            onTrackReady();
        else
            connect(m_currentExplicitTrack.get(), SIGNAL(isLoadedChanged()), this, SLOT(onTrackReady()));
    } else {
        m_implicitTracks->playTrackAtIndex(m_implicitTracks->indexOf(track));
    }

    emit tracksChanged();
}

bool QSpotifyPlayQueue::isExplicitTrack(int index)
{
    return index > m_currentTrackIndex && index <= m_currentTrackIndex + m_explicitTracks.count();
}

void QSpotifyPlayQueue::playNext(bool repeatOne)
{
    if (repeatOne) {
        QSpotifySession::instance()->play(m_currentExplicitTrack ? m_currentExplicitTrack : m_implicitTracks->m_currentTrack, true);
    } else {
        if (m_currentExplicitTrack) {
            m_currentExplicitTrack.reset();
        }

        if (m_explicitTracks.isEmpty()) {
            if (!m_implicitTracks->next()) {
                if (m_repeat) {
                    m_implicitTracks->play();
                } else {
                    QSpotifySession::instance()->stop();
                    clearTrackList();
                }
            }
        } else {
            m_currentExplicitTrack = m_explicitTracks.dequeue();
            if (m_currentExplicitTrack->isLoaded())
                onTrackReady();
            else
                connect(m_currentExplicitTrack.get(), SIGNAL(isLoadedChanged()), this, SLOT(onTrackReady()));
        }
    }

    emit tracksChanged();
}

void QSpotifyPlayQueue::playPrevious()
{
    if (m_currentExplicitTrack) {
        m_currentExplicitTrack.reset();
    }

    if (!m_implicitTracks->previous()) {
        if (m_repeat) {
            m_implicitTracks->playLast();
        } else {
            QSpotifySession::instance()->stop();
            clearTrackList();
        }
    }

    emit tracksChanged();
}

void QSpotifyPlayQueue::clear()
{
    if (m_currentExplicitTrack) {
        m_currentExplicitTrack.reset();
    }

    clearTrackList();

    m_explicitTracks.clear();
}

void QSpotifyPlayQueue::setShuffle(bool s)
{
    if (m_shuffle == s)
        return;
    m_shuffle = s;
    m_implicitTracks->setShuffle(s);

    emit tracksChanged();
}

void QSpotifyPlayQueue::setRepeat(bool r)
{
    if (m_repeat == r)
        return;

    m_repeat = r;

    emit tracksChanged();
}

void QSpotifyPlayQueue::onTrackReady()
{
    disconnect(this, SLOT(onTrackReady()));
    if (m_currentExplicitTrack)
        QSpotifySession::instance()->play(m_currentExplicitTrack);
}

QSpotifyTrackList *QSpotifyPlayQueue::tracks() const
{
    return m_implicitTracks;
}
// TODO implement explicit track behaviour again
//{
//    QList<QObject *> list;

//    if (!m_implicitTracks)
//        return list;

//    int currIndex = 0;

//    if (m_shuffle) {
//        for (int i = 0; i < m_implicitTracks->m_shuffleList.count(); ++i) {
//            std::shared_ptr<QSpotifyTrack>  t = m_implicitTracks->at(m_implicitTracks->m_shuffleList.at(i));
//            // FIXME POINTER escape
//            list.append((QObject*)t.get());
//            if (t == m_implicitTracks->m_currentTrack)
//                currIndex = i;
//        }
//    } else {
//        if (m_implicitTracks->m_reverse) {
//            int i = m_implicitTracks->previousAvailable(m_implicitTracks->count());
//            while (i >= 0) {
//                std::shared_ptr<QSpotifyTrack>  t = m_implicitTracks->at(i);
//                // FIXME POINTER escape
//                list.append((QObject*)t.get());
//                if (t == m_implicitTracks->m_currentTrack)
//                    currIndex = m_implicitTracks->count() - 1 - i;
//                i = m_implicitTracks->previousAvailable(i);
//            }
//        } else {
//            int i = m_implicitTracks->nextAvailable(-1);
//            while (i < m_implicitTracks->count()) {
//                std::shared_ptr<QSpotifyTrack>  t = m_implicitTracks->at(i);
//                // FIXME POINTER escape
//                list.append((QObject*)t.get());
//                if (t == m_implicitTracks->m_currentTrack)
//                    currIndex = i;
//                i = m_implicitTracks->nextAvailable(i);
//            }
//        }
//    }

//    if (m_currentExplicitTrack)
//        // FIXME POINTER escape
//        list.insert(++currIndex, (QObject*)m_currentExplicitTrack.get());
//    for (int i = 0; i < m_explicitTracks.count(); ++i)
//        // FIXME POINTER escape
//        list.insert(++currIndex, (QObject*)m_explicitTracks.at(i).get());

//    if (m_currentExplicitTrack)
//        // FIXME POINTER escape
//        m_currentTrackIndex = list.indexOf((QObject *)m_currentExplicitTrack.get());
//    else if (m_implicitTracks->m_currentTrack)
//        // FIXME POINTER escape
//        m_currentTrackIndex = list.indexOf((QObject *)m_implicitTracks->m_currentTrack.get());

//    return list;
//}

bool QSpotifyPlayQueue::isCurrentTrackList(QSpotifyTrackList *tl)
{
    return m_sourceTrackList == tl;
}

void QSpotifyPlayQueue::tracksUpdated()
{
    emit tracksChanged();
}

void QSpotifyPlayQueue::onOfflineModeChanged()
{
    if (m_shuffle && m_implicitTracks)
        m_implicitTracks->setShuffle(true);
    emit tracksChanged();
}

void QSpotifyPlayQueue::clearTrackList()
{
    m_implicitTracks->clear();
    m_sourceTrackList = nullptr;
}
