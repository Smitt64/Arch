#include "listbuilder.h"
#include <QPainter>
#include <QLabel>
#include <QUrl>
#include <QDebug>
#include <QMessageBox>
#include <QFileInfo>
#include <QTextEdit>
#include <QPixmap>
#include <QMenuItem>
#include <QApplication>

ListBuilder::ListBuilder(FSHANDLE *arch, QObject *parent) :
    QThread(parent),
    handle(arch),
    custom(false)
{
    this->progress = new QProgressDialog(qApp->activeWindow());
    this->progress->setWindowModality(Qt::WindowModal);
}

void ListBuilder::setList(QListWidget *value)
{
    this->list = value;
}

void ListBuilder::setCurrentFolder(QString value)
{
    this->currFolder = value;
}

QListWidgetItem *ListBuilder::makeItem(QString text)
{
    QListWidgetItem *item = new QListWidgetItem;
    QString ext = FileSystem::fsGetExstension(text);
    ext = ext.toLower();
    QString name = this->currFolder.remove("[ROOT]\\") + text;

    item->setText(text);

    QPixmap map(64, 64);
    map.fill(Qt::transparent);

    QPainter paint(&map);
    paint.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
    paint.drawPixmap(QRect(-4, -6, 64, 64), QPixmap(":/document"), QRect(0, 0, 48, 48));
    paint.setPen(Qt::darkBlue);
    paint.setFont(QFont("Times", 8, QFont::Bold));
    paint.drawText(7, 10, ext);

    paint.setPen(Qt::darkRed);
    paint.drawLine(7, 12, 30, 12);

    if(FileSystem::getInst()->fsOpen(this->handle))
    {
        QPixmap m;
        if(ext == "html" || ext == "htm")
        {
            QTextEdit *label = new QTextEdit;
            label->setText(QString(FileSystem::getInst()->fsGetFile(name, this->handle)));
            m = QPixmap::grabWidget(label);
            delete label;
            paint.drawPixmap(QRect(7, 13, 40, 40), m, QRect(0, 0, 40, 40));
            //paint.drawPixmapFragments(&QPainter::PixmapFragment::create(QPoint(7, 13), QRect(0, 0, 30, 30)), 1, map);
        }
        else if(ext == "bmp" || ext == "jpg" || ext == "jpeg" || ext == "png")
        {
            m.loadFromData(FileSystem::getInst()->fsGetFile(name, this->handle));
            paint.drawPixmap(QRect(7, 13, 40, 40), m);
        }
        else
        {
            QFont font("Arial");
            font.setPointSizeF(8);
            QString text = FileSystem::getInst()->fsGetFile(name, this->handle);
            paint.setFont(font);
            paint.drawText(7, 13, 40, 40, Qt::AlignLeft, text);
        }
        FileSystem::getInst()->fsClose(this->handle);
    }

    item->setIcon(QIcon(map));
    item->setData(Qt::UserRole, 1);

    return item;
}

void ListBuilder::setFiles(QList<QUrl> list)
{
    this->customFiles.clear();
    for(int i = 0; i < list.count(); i++)
    {
        this->customFiles.append(list[i].path().remove(0, 1));
    }
    this->custom = true;
}

void ListBuilder::run()
{
    if(!custom)
    {
        this->list->clear();

        if(FileSystem::getInst()->fsOpen(this->handle))
        {
            QString buf = this->currFolder;

            if(buf.contains("[ROOT]\\"))
                buf.remove(0, 7);

            QStringList folders = FileSystem::getInst()->fsGetFolders(buf, this->handle);
            QStringList files = FileSystem::getInst()->fsGetListFiles(buf, this->handle);

            int max = folders.count() + files.count();
            this->progress->setMaximum(max);
            this->progress->setWindowTitle(tr("Building file list..."));

            int steps = 0;
            this->progress->setLabelText(tr("Reading folders..."));
            for(int i = 0; i < folders.count(); i++)
            {
                QListWidgetItem *item = new QListWidgetItem;
                item->setText(folders[i]);
                item->setIcon(QIcon(":/folder"));
                item->setData(Qt::UserRole, 0);

                this->list->addItem(item);

                steps ++;
                this->progress->setValue(steps);
            }

            for(int i = 0; i < files.count(); i++)
            {
                this->list->addItem(this->makeItem(files[i]));
                this->progress->setLabelText(QString(tr("File: %1")).arg(files[i]));
                steps ++;
                this->progress->setValue(steps);
            }
            this->progress->setValue(max);

            FileSystem::getInst()->fsClose(this->handle);
        }
    }
    else
    {
        this->progress->setWindowTitle(tr("Addition files..."));
        this->progress->setMaximum(this->customFiles.count());

        if(FileSystem::getInst()->fsOpen(this->handle))
        {
            for(int i = 0; i < this->customFiles.count(); i++)
            {
                QFileInfo inf(this->customFiles[i]);
                QString name = this->currFolder + inf.fileName();
                name.remove("[ROOT]\\");
                if(FileSystem::getInst()->fsAddFile(inf.absoluteFilePath(),
                                                 name.toLocal8Bit(),
                                                 this->handle))
                    this->list->addItem(this->makeItem(inf.fileName()));
                this->progress->setValue(i);
            }
            this->progress->setMaximum(this->customFiles.count());
            FileSystem::getInst()->fsClose(this->handle);
        }
        else
            QMessageBox::critical(this->list, tr("Error!"), tr("Can't open archive!"));

        this->custom = false;
    }
}
