template <class ItemType>
ListModelBase<ItemType>::ListModelBase(QObject *parent)
    : QAbstractListModel(parent)
{ }

template <class ItemType>
int ListModelBase<ItemType>::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_dataList.size();
}

template <class ItemType>
ListModelBase<ItemType>::~ListModelBase()
{
    clear();
}

template <class ItemType>
void ListModelBase<ItemType>::appendRow(std::shared_ptr<ItemType> item)
{
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    connect(item.get(), &QSpotifyObject::dataChanged, this, &ListModelBase<ItemType>::itemDataChanged);
    m_dataList.append(item);
    endInsertRows();
}

template <class ItemType>
void ListModelBase<ItemType>::appendRows(const QList<std::shared_ptr<ItemType> > &items)
{
    if (items.isEmpty()) return;
    beginInsertRows(QModelIndex(),rowCount(),rowCount()+items.size()-1);
    for(auto item : items) {
        connect(item.get(), &QSpotifyObject::dataChanged, this, &ListModelBase<ItemType>::itemDataChanged);
        m_dataList.append(item);
    }
    endInsertRows();
}

template <class ItemType>
void ListModelBase<ItemType>::insertRow(int row, std::shared_ptr<ItemType> item)
{
    beginInsertRows(QModelIndex(),row,row);
    connect(item.get(), &QSpotifyObject::dataChanged, this, &ListModelBase<ItemType>::itemDataChanged);
    m_dataList.insert(row,item);
    endInsertRows();
}

template <class ItemType>
std::shared_ptr<ItemType> ListModelBase<ItemType>::find(const QString &id) const
{
    for(auto item : m_dataList)
        if(item->getId() == id) return item;
    return 0;
}

template <class ItemType>
QModelIndex ListModelBase<ItemType>::indexFromItem(const std::shared_ptr<ItemType> item) const
{
    Q_ASSERT(item);
    for(int row=0; row<m_dataList.size();++row)
        if(m_dataList.at(row) == item) return index(row);

    return QModelIndex();
}

template <class ItemType>
void ListModelBase<ItemType>::clear()
{
    if (m_dataList.isEmpty()) return;
    beginRemoveRows(QModelIndex(),0, m_dataList.size()-1);
    while(!m_dataList.isEmpty()) {
        auto i = m_dataList.takeFirst();
        disconnect(i.get(), &QSpotifyObject::dataChanged, this, &ListModelBase<ItemType>::itemDataChanged);
        i.reset();
    }
    endRemoveRows();
}

template <class ItemType>
bool ListModelBase<ItemType>::removeRows(int row, int count, const QModelIndex &parent)
{
    Q_UNUSED(parent);
    if (count == 0) return true;
    if(row < 0 || (row+count) >= m_dataList.size()) return false;
    beginRemoveRows(QModelIndex(),row,row+count-1);
    for(int i=0; i<count; ++i){
        auto j = m_dataList.takeAt(row);
        disconnect(j.get(), &QSpotifyObject::dataChanged, this, &ListModelBase<ItemType>::itemDataChanged);
    }
    endRemoveRows();
    return true;

}

template <class ItemType>
std::shared_ptr<ItemType> ListModelBase<ItemType>::takeRow(int row){
    beginRemoveRows(QModelIndex(),row,row);
    auto item = m_dataList.takeAt(row);
    disconnect(item.get(), &QSpotifyObject::dataChanged, this, &ListModelBase<ItemType>::itemDataChanged);
    endRemoveRows();
    return item;
}

template <class ItemType>
void ListModelBase<ItemType>::replaceData(const QList<std::shared_ptr<ItemType> > &newData)
{
    clear();
    appendRows(newData);
}

template <class ItemType>
void ListModelBase<ItemType>::itemDataChanged()
{
    auto sndr = dynamic_cast<ItemType*>(sender());
    if(sndr) {
        int idx = m_dataList.indexOf(sndr->shared_from_this());
        if(idx > -1 && idx < count()) {
            auto modelIdx = index(idx);
            emit dataChanged(modelIdx, modelIdx);
        }
    }
}
