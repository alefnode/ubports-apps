#ifndef TRACKLISTFILTERMODEL_H
#define TRACKLISTFILTERMODEL_H

#include <QtCore/QSortFilterProxyModel>

#include "../qspotifytracklist.h"

class TrackListFilterModel : public QSortFilterProxyModel
{
   Q_OBJECT

    Q_PROPERTY(QSpotifyTrackList *trackList READ tracks WRITE setTracks NOTIFY tracksChanged)
    Q_PROPERTY(QString filter READ filter WRITE setFilter NOTIFY filterChanged)
public:
    explicit TrackListFilterModel(QObject *parent = nullptr);

    QSpotifyTrackList *tracks() const { return m_trackList;}
    void setTracks(QSpotifyTrackList *tracks) { m_trackList = tracks; Q_EMIT tracksChanged();}

    QString filter() const { return filterRegExp().pattern(); }
    void setFilter(const QString &filter) { setFilterFixedString(filter); Q_EMIT filterChanged();}

    Q_INVOKABLE void init() {
        setSourceModel(m_trackList);
    }

    Q_INVOKABLE int getSourceIndex(const int idx);

signals:
    void tracksChanged();
    void filterChanged();

protected:
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const;
    // We only care about filtering
    bool lessThan(const QModelIndex &, const QModelIndex &) const { return false;}

private:
    QSpotifyTrackList *m_trackList;
};

#endif // TRACKLISTFILTERMODEL_H
