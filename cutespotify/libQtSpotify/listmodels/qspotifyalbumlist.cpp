#include "qspotifyalbumlist.h"

#include "../qspotifyalbumbrowse.h"

QSpotifyAlbumList::QSpotifyAlbumList(QObject *parent) :
    ListModelBase<QSpotifyAlbum>(parent)
{
    m_roles[IsAvailableRole] = "isAvailable";
    m_roles[ArtistRole] = "artist";
    m_roles[NameRole] = "name";
    m_roles[YearRole] = "year";
    m_roles[TypeRole] = "type";
    m_roles[SectionTypeRole] = "sectionType";
    m_roles[CoverIdRole] = "coverId";
}

QVariant QSpotifyAlbumList::data(const QModelIndex &index, int role) const
{
    if(index.row() < 0 || index.row() >= m_dataList.size())
        return QVariant();
    auto album = m_dataList.at(index.row());
    switch(role) {
    case IsAvailableRole:
        return album->isAvailable();
    case ArtistRole:
        return album->artist();
    case NameRole:
        return album->name();
    case YearRole:
        return album->year();
    case TypeRole:
        return album->type();
    case SectionTypeRole:
        return album->sectionType();
    case CoverIdRole:
        return album->coverId();
    default:
        return QVariant();
    }
}

QSpotifyAlbumBrowse *QSpotifyAlbumList::albumBrowse(const int idx)
{
    auto browse = new QSpotifyAlbumBrowse();
    browse->setAlbum(m_dataList.at(idx));
    return browse;
}
