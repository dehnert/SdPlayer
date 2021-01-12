#ifndef PLAYLISTMODEL_H
#define PLAYLISTMODEL_H

#include <QAbstractListModel>
#include <QStringList>
#include <QList>
#include <QMimeData>
#include <QIcon>
#include <QApplication>

class PlaylistModel : public QAbstractListModel
{
    Q_OBJECT

public:
    PlaylistModel(const QStringList &strings, QObject *parent = 0)
        : QAbstractListModel(parent), Playlist(strings) {PlayPos=-1; PlayMode=0; IconSpeaker=QIcon(QCoreApplication::applicationDirPath()+"/sound.png"); IconMute=QIcon(QCoreApplication::applicationDirPath()+"/sound_mute.png");}
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    bool setData(const QModelIndex &index, const QVariant &value,
                 int role = Qt::EditRole);
    bool insertRows(int position, int rows, const QModelIndex &index = QModelIndex());
    bool removeRows(int position, int rows, const QModelIndex &index = QModelIndex());
    Qt::DropActions supportedDropActions() const;
    QStringList mimeTypes() const;
    bool dropMimeData ( const QMimeData * data, Qt::DropAction action, int row, int column, const QModelIndex & parent );
    void setPlayPos(int pos);
    int playPos();
    QString getFilename(int pos);
    void setPlayMode(int mode);
    void setFileTypes(QStringList newFileTypes);

signals:
    void playlistChanged(void);
    void playlistDropped(QString filename);
    void scrollto(int row);

private:
    QStringList Playlist;
    QStringList FileTypes;
    int PlayPos;
    int PlayMode;
    QStringList filesInDir(QString dirname);
    QIcon IconSpeaker,IconMute;
};

#endif // PLAYLISTMODEL_H
