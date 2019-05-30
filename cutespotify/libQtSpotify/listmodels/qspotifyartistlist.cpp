#include "qspotifyartistlist.h"

#include "../qspotifyartistbrowse.h"

QSpotifyArtistList::QSpotifyArtistList(QObject *parent) :
    ListModelBase<QSpotifyArtist>(parent)
{
    m_roles[NameRole] = "name";
    m_roles[PictureIdRole] = "pictureId";
}

QVariant QSpotifyArtistList::data(const QModelIndex &index, int role) const
{
    if(index.row() < 0 || index.row() >= m_dataList.size())
        return QVariant();
    auto artist = m_dataList.at(index.row());
    switch(role) {
    case NameRole:
        return artist->name();
    case PictureIdRole:
        return artist->pictureId();
    default:
        return QVariant();
    }
}

QSpotifyArtistBrowse *QSpotifyArtistList::artistBrowse(const int idx)
{
    auto artistBrowse = new QSpotifyArtistBrowse();
    artistBrowse->setArtist(m_dataList.at(idx));
    return artistBrowse;
}
