#ifndef QSPOTIFYPLAYLISTSEARCHLIST_H
#define QSPOTIFYPLAYLISTSEARCHLIST_H

#include "../qspotifyplaylistsearchentry.h"
#include "listmodelbase.h"

class QSpotifyPlaylistSearchList : public ListModelBase<QSpotifyPlaylistSearchEntry>
{
    Q_OBJECT
public:
    explicit QSpotifyPlaylistSearchList(QObject *parent = nullptr);

    enum Roles {
        NameRole = Qt::UserRole + 1
    };

    QVariant data(const QModelIndex &index, int role) const;
    QHash<int, QByteArray> roleNames() const { return m_roles; }

    Q_INVOKABLE QSpotifyPlaylist *playlist(const int idx);

private:
    QHash<int, QByteArray> m_roles;
};

#endif // QSPOTIFYPLAYLISTSEARCHLIST_H
