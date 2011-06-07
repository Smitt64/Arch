#ifndef CONTENTVIEW_H
#define CONTENTVIEW_H

#include <QWidget>
#include <QList>
#include <QUrl>
#include <filesystem.h>
#include "filesviewwidget.h"

namespace Ui {
    class ContentView;
}

class QAction;
class QTreeWidgetItem;

class ContentView : public QWidget
{
    Q_OBJECT

public:
    explicit ContentView(QWidget *parent = 0);
    ~ContentView();

    bool loadFile(const QString &fileName);

    QString currentFile() { return curFile; }

    QList<QAction*> getActions();

signals:
    void currentFolderChanged(QString folder);
    void fileSelected(QString name);

public slots:
    void setCurrentFolder(QString folder = "");
    void cdUp();
    void removeFiles();
    void viewEdit();

private slots:
    void open(QString fname);
    void updateFileTree();
    void onClickFolderTree(QTreeWidgetItem *item, int column);
    void onDoubleClick(QListWidgetItem *item);
    void onEndEditFile(QListWidgetItem *item);

    void onAddDragedFiles(const QMimeData *data);

    void onClickItem(QListWidgetItem *item);

private:
    void makeFileItem(QString text);
    void setCurrentFile(const QString &fileName);
    void updateFileList(QString folder = "");
    void selectTreeItemByPath(QString path);
    QTreeWidgetItem *addItem(QString lpszItem, QTreeWidgetItem *hRoot);
    FSHANDLE handle;
    QString currFolder;
    QString curFile;
    FilesViewWidget *fileList;
    QString oldFile;
    QList<QAction*> menuactions;
    Ui::ContentView *ui;
};

#endif // CONTENTVIEW_H
