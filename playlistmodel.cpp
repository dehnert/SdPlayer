#include <QMessageBox>
#include "playlistmodel.h"
#include <QFileInfo>
#include <QUrl>
#include <QDir>

// rowcount is size of list
int PlaylistModel::rowCount(const QModelIndex &parent) const
{
    return Playlist.count();
}

// data is string if index is valid, row is valid, role is DisplayRole
QVariant PlaylistModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    int row=index.row();
    if (row >= Playlist.size())
        return QVariant();

    if (role == Qt::DisplayRole) {
        QFileInfo finfo(Playlist.at(row));
        return QString("%1) %2").arg(row+1).arg(finfo.fileName());
    } else if (role == Qt::ToolTipRole) {
        return Playlist.at(row); // tool tip shows complete path
    } else if (role == Qt::DecorationRole) {
        if (row==PlayPos) {
            if (PlayMode==2) {
                return IconSpeaker; // playing icon
            } else {
                return IconMute; // paused icon
            }
        } else {
            return QColor(255,255,255); // not the "playing" file
        }
    } else
        return QVariant();
}

// header data is used by table or tree, not used by list
QVariant PlaylistModel::headerData(int section, Qt::Orientation orientation,
                                     int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    if (orientation == Qt::Horizontal)
        return QString("Column %1").arg(section);
    else
        return QString("Row %1").arg(section);
}

// this tells the view that all items may be edited
Qt::ItemFlags PlaylistModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags defaultFlags = QAbstractListModel::flags(index);

    if (index.isValid())
        return Qt::ItemIsDragEnabled /*| Qt::ItemIsDropEnabled*/ | defaultFlags; // cannot drop ON existing items
    else
        return Qt::ItemIsDropEnabled | defaultFlags;
}

// this lets the view set my data for editing and drag/drop operations
bool PlaylistModel::setData(const QModelIndex &index,
                              const QVariant &value, int role)
{
    if (index.isValid() && role == Qt::EditRole) {

        Playlist.replace(index.row(), value.toString()); // QVarient contains a string
        emit dataChanged(index, index);
        return true;
    }
    return false;
}

bool PlaylistModel::insertRows(int position, int rows, const QModelIndex &parent)
{
    beginInsertRows(QModelIndex(), position, position+rows-1);

    for (int row = 0; row < rows; ++row) {
        Playlist.insert(position, "");
    }

    endInsertRows();
    if (PlayPos>=position) { // if inserting rows before PlayPos
        PlayPos+=rows;  // bump PlayPos up by rows inserted
    }
    return true;
}

bool PlaylistModel::removeRows(int position, int rows, const QModelIndex &parent)
{
    int flag=0;
    beginRemoveRows(QModelIndex(), position, position+rows-1);

    for (int row = 0; row < rows; ++row) {
        Playlist.removeAt(position);
        flag=1;
    }

    endRemoveRows();
    if (flag) {
        emit playlistChanged();
    }
    if (PlayPos>=position) { // PlayPos in or after deleted rows
        if (PlayPos>=position+rows) { // PlayPos after deleted rows
            PlayPos-=rows;  // adjust PlayPos for deleted rows
        } else {
            PlayPos=-1; // PlayPos row was deleted! (is there a better way to handle this?)
        }
    }
    return true;
}

// indicate to view that copy/move are supported
Qt::DropActions PlaylistModel::supportedDropActions() const
{
    return Qt::CopyAction | Qt::MoveAction;
}

QStringList PlaylistModel::mimeTypes() const
{
    QStringList types;
    types << "application/x-qabstractitemmodeldatalist";
    types << "text/uri-list";
    return types;
}

// return list of all supported files in specified directory and all nested directories
QStringList PlaylistModel::filesInDir(QString dirname)
{
    QStringList filelist;
    QFileInfoList infolist;
    QFileInfo fileinfo;
    int i;

    QDir dir=QDir(dirname);
    infolist=dir.entryInfoList(QDir::Dirs|QDir::Files|QDir::NoDotAndDotDot,QDir::DirsLast); // files before directories, no . or ..
    for (i=0; i<infolist.size(); i++) { // process all files first (keep files in each directory together)
        fileinfo=infolist[i];
        if (fileinfo.isFile()) {
            if (FileTypes.contains(fileinfo.suffix().toLower())) {
                filelist+=fileinfo.absoluteFilePath();
            }
        }
        if (fileinfo.isDir()) {
            filelist+=filesInDir(fileinfo.absoluteFilePath()); // recursively get files in nested dirs
        }
    }
    return filelist;
}

bool PlaylistModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
    QString message;
    QByteArray mimedata;
    QMap<int,QString> DroppedItems; // QMap needed to keep items moved via drag-drop in playlist in original order
    QString filename;
    QFileInfo fileinfo;
    int newrows;
    bool deleteflag=false;

    newrows=0;
    QDataStream stream(&mimedata, QIODevice::ReadOnly);
    if (data->hasFormat("application/x-qabstractitemmodeldatalist")) { // moving items within the list
        mimedata = data->data("application/x-qabstractitemmodeldatalist");
        while (!stream.atEnd())
        {
            int r, c;
            QMap<int,  QVariant> roleDataMap;
            stream >> r >> c >> roleDataMap; // get the row, column and roledatamap
            DroppedItems.insert(r,Playlist[r]); // store dropped items in a map to keep them sorted by row
            newrows++;
            deleteflag=true; // flag that I must delete dropped items after they are inserted at new row
        }
    } else {
        if (data->hasUrls()) { // adding files to the list (for now only files, not folders)
            // recursion must be used here to allow nested folders
            // file order not importand, so perhaps just building a QString list of all files firs, then entering them in QMap
            // some redesign required
            if (data->urls().size()==1) { // if a single file is dropped
                foreach (QUrl url, data->urls()) {
                    filename=url.toLocalFile();
                    fileinfo.setFile(filename);
                    if (fileinfo.isFile()) {
                        if (fileinfo.suffix().toLower()=="m3u") {
                            emit playlistDropped(filename); // let the main window handle dropped playlist file
                            return false; // cannot handle a playlist file (emit a signal)
                        }
                    }
                }
            }
            foreach (QUrl url, data->urls()) { // process all top level files first
                filename=url.toLocalFile();
                fileinfo.setFile(filename); // get information about file
                if (fileinfo.isFile()) { // if it is a file (not a directory or symlink)
                    if (FileTypes.contains(fileinfo.suffix().toLower())) { // it must be a supported file (all others are ignored)
                        DroppedItems.insert(newrows,filename);
                        newrows++;
                    }
                }
            }
            QStringList filelist;
            foreach (QUrl url, data->urls()) { // process all selected folders next
                filename=url.toLocalFile();
                fileinfo.setFile(filename);
                if (fileinfo.isDir()) {
                    filelist+=filesInDir(filename); // returns list of all supported files in directory
                }
            }
            // now I must add all files in filelist to the DroppedItems list
            for (int i=0; i<filelist.size(); i++) {
                DroppedItems.insert(newrows,filelist[i]);
                newrows++;
            }
        }
    }
    if (newrows>0) {
        if (row==-1) { // inserting into empty list, or past end of list
            row=rowCount();
        }
        int insertrow=row;
        insertRows(insertrow, newrows, QModelIndex());
        QMapIterator<int,QString> i(DroppedItems);
        while (i.hasNext()) { // iterate over DroppedItems map, inserting items at row keeping them in order
            i.next();
            QModelIndex idx = index(insertrow, 0, QModelIndex());
            setData(idx, i.value());
            insertrow++;
        }
        if (deleteflag) {
            QMapIterator<int,QString> j(DroppedItems);
            int deletecount=0;
            int rowdeleted;
            int rowadjusted=row;
            while (j.hasNext()) { // iterate over DroppedItems map (in row order), deleting items that were moved
                j.next();
                rowdeleted=j.key()-deletecount;
                if (j.key()>=row) {
                    rowdeleted+=newrows; // adjust row to delete if it is after the insert point
                }
                if (rowdeleted==PlayPos) { // if moving the PlayPos row
                    PlayPos=rowadjusted+deletecount;
                }
                removeRows(rowdeleted,1,QModelIndex());
                if (rowdeleted<rowadjusted) {
                    rowadjusted--; // position of newly inserted rows has moved up
                }
                deletecount++; // count deleted rows so I can adjust the position of the next row(s) to delete above
            }
            // scroll to make rowadjusted visible
            emit(scrollto(rowadjusted));
        }
        emit playlistChanged(); // items added or moved via drag-drop
        return true;
    }
    return true;
}

void PlaylistModel::setPlayPos(int pos)
{
    if (pos!=PlayPos) {
       QModelIndex idx = index(PlayPos, 0, QModelIndex());
       emit dataChanged(idx,idx);
       PlayPos=pos;
       idx = index(PlayPos, 0, QModelIndex());
       emit dataChanged(idx,idx);
    }
}

int PlaylistModel::playPos()
{
    return PlayPos;
}

// return filename at playlist position pos
QString PlaylistModel::getFilename(int pos)
{
    if (pos<0 || pos>=Playlist.size()) {
        return "";
    }
    return Playlist[pos];
}

void PlaylistModel::setPlayMode(int mode)
{
    if (mode!=PlayMode) {
        PlayMode=mode;
        QModelIndex idx = index(PlayPos, 0, QModelIndex());
        emit dataChanged(idx,idx);
    }
}

void PlaylistModel::setFileTypes(QStringList newFileTypes)
{
    FileTypes=newFileTypes;
}
