#ifndef FILESVIEWWIDGET_H
#define FILESVIEWWIDGET_H

#include <QListWidget>

class FilesViewWidget : public QListWidget
{
    Q_OBJECT
public:
    explicit FilesViewWidget(QWidget *parent = 0);

signals:
    void dragedFiles(const QMimeData*);

public slots:

protected:
    QMimeData *mimeData;
    //QListWidgetItem *cur;
    void dragEnterEvent(QDragEnterEvent *event);
    void dragMoveEvent(QDragMoveEvent *event);
    void dropEvent(QDropEvent *event);
    void dragLeaveEvent(QDragLeaveEvent *event);
    void keyPressEvent(QKeyEvent *event);
    void contextMenuEvent(QContextMenuEvent *event);
};

#endif // FILESVIEWWIDGET_H
