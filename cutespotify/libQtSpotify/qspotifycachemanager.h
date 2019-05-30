#ifndef QSPOTIFYCACHEMANAGER_H
#define QSPOTIFYCACHEMANAGER_H

#include <memory>

class QMutex;
class QThread;

class QSpotifyTrack;
class QSpotifyPlaylist;
class QSpotifyArtist;
class QSpotifyAlbum;

struct sp_track;
struct sp_artist;
struct sp_album;

class CleanerWorker;

class QSpotifyCacheManager
{
public:

    static QSpotifyCacheManager& instance() {
        static QSpotifyCacheManager inst;
        return inst;
    }

    std::shared_ptr<QSpotifyTrack> getTrack(sp_track *t, QSpotifyPlaylist *playlist = nullptr);
    std::shared_ptr<QSpotifyArtist> getArtist(sp_artist *a);
    std::shared_ptr<QSpotifyAlbum> getAlbum(sp_album *a);

    void clean();

    void clearTables();


private:
    QSpotifyCacheManager();
    ~QSpotifyCacheManager();

    template<class T> void cleanTable(T &hash, QMutex *mutex);
    void cleanUp();

    int numTracks();
    int numAlbums();
    int numArtists();

    QThread *m_cleanerThread;

    friend class CleanerWorker;
    CleanerWorker *m_cleanerWorker;
};

#endif // QSPOTIFYCACHEMANAGER_H
