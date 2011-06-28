#include <QtGui>
#include <QFile>
#include <QKeyEvent>
#include "filesviewwidget.h"

FilesViewWidget::FilesViewWidget(QWidget *parent) :
    QListWidget(parent)
{
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setAcceptDrops(true);
    setGridSize(QSize(80, 80));
    setIconSize(QSize(64, 64));
    setWordWrap(true);
    setViewMode(QListWidget::IconMode);

    setContextMenuPolicy(Qt::ActionsContextMenu);
    //addAction(new QAction("add file", NULL));
}

void FilesViewWidget::dragEnterEvent(QDragEnterEvent *event)
{
    setBackgroundRole(QPalette::Highlight);
    event->acceptProposedAction();
    //emit dragedFiles(event->mimeData());
}

void FilesViewWidget::dragMoveEvent(QDragMoveEvent *event)
{
    event->acceptProposedAction();
}

void FilesViewWidget::dropEvent(QDropEvent *event)
{
    mimeData = (QMimeData*)event->mimeData();

    //QFile f(mimeData->urls()[0].path().remove(0, 1));
    //if(f.open(QIODevice::ReadOnly))
        //qDebug() << mimeData->text() << mimeData->urls()[0].path();
    /*     if (mimeData->hasImage()) {
             setPixmap(qvariant_cast<QPixmap>(mimeData->imageData()));
         } else if (mimeData->hasHtml()) {
             setText(mimeData->html());
             setTextFormat(Qt::RichText);
         } else if (mimeData->hasText()) {
             setText(mimeData->text());
             setTextFormat(Qt::PlainText);
         } else if (mimeData->hasUrls()) {
             QList<QUrl> urlList = mimeData->urls();
             QString text;
             for (int i = 0; i < urlList.size() && i < 32; ++i) {
                 QString url = urlList.at(i).path();
                 text += url + QString("\n");
             }
             setText(text);
         } else {
             setText(tr("Cannot display data"));
         }*/
    emit dragedFiles(mimeData);

    setBackgroundRole(QPalette::Dark);
    event->acceptProposedAction();
}

void FilesViewWidget::dragLeaveEvent(QDragLeaveEvent *event)
{
    event->accept();
}

void FilesViewWidget::contextMenuEvent(QContextMenuEvent *event)
{

}

void FilesViewWidget::keyPressEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_Escape)
    {
        this->closePersistentEditor(this->currentItem());
    }
    else if(event->key() == Qt::Key_Delete)
    {
        emit deleteFile();
    }
}
