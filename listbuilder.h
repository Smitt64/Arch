#ifndef LISTBUILDER_H
#define LISTBUILDER_H

#include <QThread>
#include <QListWidget>
#include <QProgressDialog>
#include "filesystem.h"

class ListBuilder : public QThread
{
    Q_OBJECT
public:
    ListBuilder(FSHANDLE *arch, QObject *parent = 0);
    void setList(QListWidget *value);
    void setFiles(QList<QUrl> list);
    void setFiles(QStringList list);
    void run();

signals:

public slots:
    void setCurrentFolder(QString value);

//protected:
    //virtual void run();

private:
    QProgressDialog *progress;
    QListWidgetItem *makeItem(QString text);
    QListWidget *list;
    FSHANDLE *handle;
    bool custom;
    QString currFolder;
    QStringList customFiles;
};

#endif // LISTBUILDER_H
