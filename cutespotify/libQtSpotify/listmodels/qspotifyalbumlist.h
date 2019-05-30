#ifndef QSPOTIFYALBUMLIST_H
#define QSPOTIFYALBUMLIST_H

#include "../qspotifyalbum.h"
#include "listmodelbase.h"

class QSpotifyAlbumBrowse;

class QSpotifyAlbumList : public ListModelBase<QSpotifyAlbum>
{
    Q_OBJECT
public:
    explicit QSpotifyAlbumList(QObject *parent = nullptr);

    enum Roles {
        IsAvailableRole = Qt::UserRole + 1,
        ArtistRole,
        NameRole,
        YearRole,
        TypeRole,
        SectionTypeRole,
        CoverIdRole
    };

    QVariant data(const QModelIndex &index, int role) const;
    QHash<int, QByteArray> roleNames() const { return m_roles; }

    Q_INVOKABLE QSpotifyAlbumBrowse *albumBrowse(const int idx);

private:
    QHash<int, QByteArray> m_roles;
};

#endif // QSPOTIFYALBUMLIST_H
