#include "tracklistfiltermodel.h"

#include "../qspotifytrack.h"

TrackListFilterModel::TrackListFilterModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    setDynamicSortFilter(true);
    setFilterCaseSensitivity(Qt::CaseInsensitive);
    setFilterRole(QSpotifyTrackList::NameRole);
    setSortRole(QSpotifyTrackList::NameRole);
}

int TrackListFilterModel::getSourceIndex(const int idx)
{
    return mapToSource(index(idx, 0)).row();
}

bool TrackListFilterModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    QModelIndex idx  = sourceModel()->index(source_row, 0, source_parent);
    bool ret = (sourceModel()->data(idx, QSpotifyTrackList::ErrorRole) == QSpotifyTrack::Ok && (
                    sourceModel()->data(idx, QSpotifyTrackList::NameRole).toString().contains(filterRegExp()) ||
                    sourceModel()->data(idx, QSpotifyTrackList::ArtistsRole).toString().contains(filterRegExp()) ||
                    sourceModel()->data(idx, QSpotifyTrackList::AlbumRole).toString().contains(filterRegExp())));
    return ret;
}
