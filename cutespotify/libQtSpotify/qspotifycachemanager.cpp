#include "qspotifycachemanager.h"

#include <QtCore/QObject>
#include <QtCore/QEvent>
#include <QtCore/QCoreApplication>
#include <QtCore/QTime>
#include <QtCore/QHash>
#include <QtCore/QMutex>
#include <QtCore/QMutexLocker>
#include <QtCore/QThread>

#include <QtCore/QDebug>

#include <libspotify/api.h>

#include "qspotifytrack.h"
#include "qspotifyplaylist.h"
#include "qspotifyartist.h"
#include "qspotifyalbum.h"

static QHash<sp_track *, std::shared_ptr<QSpotifyTrack> > m_tracks;
static QMutex trackMutex;

static QHash<sp_artist *, std::shared_ptr<QSpotifyArtist> > m_artists;
static QMutex artistMutex;

static QHash<sp_album *, std::shared_ptr<QSpotifyAlbum> > m_albums;
static QMutex albumMutex;

class CleanerWorker : public QObject {
public:
    CleanerWorker(QObject *parent = nullptr) :
        QObject(parent)
    {
        m_lastClean.start();
    }

    bool event(QEvent *e) {
        if (e->type() == QEvent::User) {
            if(m_lastClean.elapsed() > 5000)
            {
                m_lastClean.restart();
                QSpotifyCacheManager::instance().cleanUp();
            }
            return true;
        }
        return QObject::event(e);
    }
private:
    QTime m_lastClean;
};

std::shared_ptr<QSpotifyTrack> QSpotifyCacheManager::getTrack(sp_track *t, QSpotifyPlaylist *playlist)
{
    Q_ASSERT(t);
    QMutexLocker lock(&trackMutex);
    auto iter = m_tracks.find(t);
    if(iter != m_tracks.end()) {
        if(auto sptr = iter.value()) {
            return sptr;
        }
    }

    std::shared_ptr<QSpotifyTrack> qtrack(
                new QSpotifyTrack(t, playlist),
                [] (QSpotifyTrack *track) {track->destroy();});
    qtrack->init();

    m_tracks.insert(t, qtrack);

    if(playlist) {
        playlist->registerTrackType(qtrack);
    }
    return qtrack;
}

std::shared_ptr<QSpotifyArtist> QSpotifyCacheManager::getArtist(sp_artist *a)
{
    Q_ASSERT(a);
    QMutexLocker lock(&artistMutex);
    if (!a) {
        return std::shared_ptr<QSpotifyArtist>();
    }

    auto iter = m_artists.find(a);
    if(iter != m_artists.end()) {
        if(auto sptr = iter.value()) {
            return sptr;
        }
    }

    std::shared_ptr<QSpotifyArtist> artPtr(
                new QSpotifyArtist(a),
                [] (QSpotifyArtist *artist) {artist->deleteLater();});
    artPtr->init();

    m_artists.insert(a, artPtr);

    return artPtr;
}

std::shared_ptr<QSpotifyAlbum> QSpotifyCacheManager::getAlbum(sp_album *a)
{
    Q_ASSERT(a);
    QMutexLocker lock(&albumMutex);
    if (!a) {
        return std::shared_ptr<QSpotifyAlbum>();
    }

    auto iter = m_albums.find(a);
    if(iter != m_albums.end()) {
        if(auto sptr = iter.value()) {
            return sptr;
        }
    }

    std::shared_ptr<QSpotifyAlbum> albPtr(
                new QSpotifyAlbum(a),
                [] (QSpotifyAlbum *album) {album->deleteLater();});
    albPtr->init();

    m_albums.insert(a, albPtr);

    return albPtr;
}

void QSpotifyCacheManager::clean()
{
    QCoreApplication::postEvent(m_cleanerWorker, new QEvent(QEvent::User));
}

void QSpotifyCacheManager::clearTables()
{
    m_cleanerThread->quit();
    m_cleanerThread->wait();

    m_tracks.clear();
    m_albums.clear();
    m_artists.clear();
}

QSpotifyCacheManager::QSpotifyCacheManager()
{
    m_cleanerThread = new QThread();
    m_cleanerWorker = new CleanerWorker();
    m_cleanerWorker->moveToThread(m_cleanerThread);
    QObject::connect(m_cleanerThread, SIGNAL(finished()), m_cleanerWorker, SLOT(deleteLater()));
    QObject::connect(m_cleanerThread, SIGNAL(finished()), m_cleanerThread, SLOT(deleteLater()));
    m_cleanerThread->start(QThread::LowPriority);
}

QSpotifyCacheManager::~QSpotifyCacheManager()
{
    // You should manually call clearTables before quitting.
}

template <class T>
void QSpotifyCacheManager::cleanTable(T &hash, QMutex *mutex)
{
    QMutexLocker lock(mutex);
    auto iter = hash.begin();
    while(iter != hash.end()) {
        if(iter.value().unique()) {
            iter.value().reset();
            iter = hash.erase(iter);
        } else
            ++iter;
    }
}

void QSpotifyCacheManager::cleanUp()
{
    qDebug() << "#Before Clean: Tracks" << m_tracks.size() << "Artists" << m_artists.size() << "Ablums" << m_albums.size();
    cleanTable<QHash<sp_track *, std::shared_ptr<QSpotifyTrack> > >(m_tracks, &trackMutex);
    cleanTable<QHash<sp_artist *, std::shared_ptr<QSpotifyArtist> > >(m_artists, &artistMutex);
    cleanTable<QHash<sp_album *, std::shared_ptr<QSpotifyAlbum> > >(m_albums, &albumMutex);
    qDebug() << "#After Clean: Tracks" << m_tracks.size() << "Artists" << m_artists.size() << "Ablums" << m_albums.size();
}

int QSpotifyCacheManager::numTracks()
{
    return m_tracks.size();
}

int QSpotifyCacheManager::numAlbums()
{
    return m_albums.size();
}

int QSpotifyCacheManager::numArtists()
{
    return m_artists.size();
}
