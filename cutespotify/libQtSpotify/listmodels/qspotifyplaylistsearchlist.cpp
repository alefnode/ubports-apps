#include "qspotifyplaylistsearchlist.h"

QSpotifyPlaylistSearchList::QSpotifyPlaylistSearchList(QObject *parent)
    : ListModelBase<QSpotifyPlaylistSearchEntry>(parent)
{
    m_roles[NameRole] = "name";
}

QVariant QSpotifyPlaylistSearchList::data(const QModelIndex &index, int role) const
{
    if(index.row() < 0 || index.row() >= m_dataList.size())
        return QVariant();
    auto playlistSearch = m_dataList.at(index.row());
    switch(role) {
    case NameRole:
        return playlistSearch->name();
    default:
        return QVariant();
    }
}

QSpotifyPlaylist *QSpotifyPlaylistSearchList::playlist(const int idx)
{
    return m_dataList.at(idx)->playlist();
}
