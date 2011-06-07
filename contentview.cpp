#include "contentview.h"
#include <QDebug>
#include <QMessageBox>
#include <QPainter>
#include <QFileInfo>
#include <QList>
#include <QLabel>
#include <QDir>
#include <QUrl>
#include <QFileInfo>
#include <QMimeData>
#include "textview.h"
#include <QProgressDialog>
#include "ui_contentview.h"

ContentView::ContentView(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ContentView),
    currFolder("[ROOT]\\")
{
    ui->setupUi(this);

    this->fileList = new FilesViewWidget(this);
    this->ui->horizontalLayout->addWidget(this->fileList);

    connect(this->fileList, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(onClickItem(QListWidgetItem*)));
    connect(this->fileList, SIGNAL(dragedFiles(const QMimeData*)), this, SLOT(onAddDragedFiles(const QMimeData*)));
    connect(this->fileList, SIGNAL(itemChanged(QListWidgetItem*)), this, SLOT(onEndEditFile(QListWidgetItem*)));
    connect(this->fileList, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(onDoubleClick(QListWidgetItem*)));
    connect(this->ui->treeWidget, SIGNAL(itemClicked(QTreeWidgetItem*,int)), this, SLOT(onClickFolderTree(QTreeWidgetItem*,int)));
}

ContentView::~ContentView()
{
    FileSystem::getInst()->fsClose(&this->handle);
    delete ui;
}

bool ContentView::loadFile(const QString &fileName)
{
    setCurrentFile(fileName);

    if(FileSystem::getInst()->fsOpen(fileName, &this->handle))
    {
        updateFileTree();
        FileSystem::getInst()->fsClose(&this->handle);
    }
    else
        return false;

    return true;
}

void ContentView::setCurrentFile(const QString &fileName)
{
    curFile = QFileInfo(fileName).canonicalFilePath();

    this->setWindowTitle(curFile);
}

void ContentView::updateFileTree()
{
    if(this->ui->treeWidget->topLevelItemCount() != 0)
        this->ui->treeWidget->clear();
    QTreeWidgetItem *rootItem = new QTreeWidgetItem(0);
    rootItem->setText(0, "[ROOT]");
    rootItem->setIcon(0, QIcon(":/root"));

    this->ui->treeWidget->addTopLevelItem(rootItem);

    for(int i = 0; i < this->handle.dataHeader->m_pFileCount; i++)
    {
        QString file(this->handle.fileList[i].m_pName);
        QTreeWidgetItem *hFolder = rootItem;
        QStringList list = file.split('\\');

        for(int j = 0; j < list.count() - 1; j++)
        {
            hFolder = this->addItem(list[j], hFolder);
        }
    }

    this->ui->treeWidget->sortItems(0, Qt::AscendingOrder);
}

QTreeWidgetItem *ContentView::addItem(QString lpszItem, QTreeWidgetItem *hRoot)
{
    QTreeWidgetItem *hChild = NULL;
    QString label("");

    //Существует ли такая ветка
    for(int i = 0; i < hRoot->childCount(); i++)
        if(hRoot->child(i)->text(0) == lpszItem)
        {
            hChild = hRoot->child(i);
            break;
        }

    //Если не существует создать
    if(!hChild)
    {
        hChild = new QTreeWidgetItem();
        hChild->setText(0, lpszItem);
        hChild->setIcon(0, QIcon(":/folder"));
        hRoot->addChild(hChild);
    }

    return hChild;
}

void ContentView::open(QString fname)
{
}

void ContentView::onClickFolderTree(QTreeWidgetItem *item, int column)
{
    QString text;
    QTreeWidgetItem *bufItem = item;

    while(bufItem)
    {
        text.insert(0, bufItem->text(0) + "\\");
        bufItem = bufItem->parent();
    }

    this->currFolder = text;

    emit currentFolderChanged(this->currFolder);

    this->updateFileList(text);
}

void ContentView::makeFileItem(QString text)
{
    QListWidgetItem *item = new QListWidgetItem;
    QString ext = FileSystem::fsGetExstension(text);
    ext = ext.toLower();
    QString name = this->currFolder.remove("[ROOT]\\") + text;

    item->setText(text);

    QPixmap map(48, 48);
    map.fill(Qt::transparent);

    QPainter paint(&map);
    paint.drawPixmap(-4, -6, 55, 55, QPixmap(":/document"));
    paint.setPen(Qt::darkBlue);
    paint.setFont(QFont("Times", 8, QFont::Bold));
    paint.drawText(7, 10, ext);

    paint.setPen(Qt::darkRed);
    paint.drawLine(7, 12, 30, 12);

    if(FileSystem::getInst()->fsOpen(&this->handle))
    {
        QPixmap map;
        if(ext == "html" || ext == "htm")
        {
            QLabel *label = new QLabel;
            label->setAutoFillBackground(false);
            label->setText(QString(FileSystem::getInst()->fsGetFile(name, &this->handle)));
            map = QPixmap::grabWidget(label);
            paint.drawPixmap(QRect(7, 13, 33, 33), map, QRect(0, 0, 100, 100));
        }
        else if(ext == "bmp" || ext == "jpg" || ext == "jpeg" || ext == "png")
        {
            map.loadFromData(FileSystem::getInst()->fsGetFile(name, &this->handle));
            paint.drawPixmap(QRect(7, 13, 33, 33), map);
        }
        else
        {
            QFont font("Arial");
            font.setPointSizeF(5);
            QString text = FileSystem::getInst()->fsGetFile(name, &this->handle);
            paint.setFont(font);
            paint.drawText(7, 13, 33, 33, Qt::AlignLeft, text);
        }
        FileSystem::getInst()->fsClose(&this->handle);
    }

    item->setIcon(QIcon(map));
    item->setData(Qt::UserRole, 1);

    this->fileList->addItem(item);
}

void ContentView::updateFileList(QString folder)
{
    this->fileList->clear();

    if(FileSystem::getInst()->fsOpen(&this->handle))
    {
        QString buf = folder;

        if(buf.contains("[ROOT]\\"))
            buf.remove(0, 7);

        QStringList folders = FileSystem::getInst()->fsGetFolders(buf, &this->handle);
        QStringList files = FileSystem::getInst()->fsGetListFiles(buf, &this->handle);

        for(int i = 0; i < folders.count(); i++)
        {
            QListWidgetItem *item = new QListWidgetItem;
            item->setText(folders[i]);
            item->setIcon(QIcon(":/folder"));
            item->setData(Qt::UserRole, 0);

            this->fileList->addItem(item);
        }

        for(int i = 0; i < files.count(); i++)
        {
            this->makeFileItem(files[i]);
        }

        FileSystem::getInst()->fsClose(&this->handle);
    }
}

void ContentView::setCurrentFolder(QString folder)
{
    this->currFolder = folder;
    updateFileList(this->currFolder);
    emit currentFolderChanged(this->currFolder);
}

void ContentView::onDoubleClick(QListWidgetItem *item)
{
    if(item->data(Qt::UserRole).toInt() == 0)
    {
        QString buf = this->currFolder + item->text();
        setCurrentFolder(buf + "\\");
    }
    else
    {
        this->oldFile = item->text();
    }
}

void ContentView::onEndEditFile(QListWidgetItem *item)
{
    if(item->text() != this->oldFile)
    {
        if(QMessageBox::question(this, "Изменение файла", "Переименовать файл?",
                                 QMessageBox::Yes | QMessageBox::No, QMessageBox::No)
                == QMessageBox::Yes)
        {
            if(FileSystem::getInst()->fsOpen(&this->handle))
            {
                FileSystem::getInst()->fsRenameFile(this->oldFile, item->text(), &this->handle);
                FileSystem::getInst()->fsClose(&this->handle);
            }
        }
        else
        {
            item->setText(this->oldFile);
        }
    }
    this->fileList->closePersistentEditor(item);
}

QList<QAction*> ContentView::getActions()
{
    return this->menuactions;
}

void ContentView::onAddDragedFiles(const QMimeData *data)
{
    QString files;
    QList<QUrl> list = data->urls();

    for(int i = 0; i < list.count(); i++)
    {
        QFileInfo inf(list[i].path().remove(0, 1));
        files += inf.fileName() + "\n";
    }

    if(FileSystem::getInst()->fsOpen(&this->handle))
    {
        qDebug() << "add files" << list;
        for(int i = 0; i < list.count(); i++)
        {
            QFileInfo inf(list[i].path().remove(0, 1));
            qDebug() << inf.absoluteFilePath();

            QString name = this->currFolder + inf.fileName();
            name.remove("[ROOT]\\");
            if(FileSystem::getInst()->fsAddFile(inf.absoluteFilePath(),
                                             name.toLocal8Bit(),
                                             &this->handle))
                makeFileItem(inf.fileName());
        }
        FileSystem::getInst()->fsClose(&this->handle);
    }
    else
        QMessageBox::critical(this, "Добавление файлов...", "Не уалось открыть архив для записи!");

    this->fileList->sortItems();
}

void ContentView::onClickItem(QListWidgetItem *item)
{
    emit fileSelected(item->text());
}

void ContentView::selectTreeItemByPath(QString path)
{
    QStringList names = path.remove(0, 7).split("\\");
    qDebug() << names;
    int s = 0;
    bool finded = false;

    QTreeWidgetItem *item = this->ui->treeWidget->topLevelItem(0);
    while(!finded)
    {
        for(int i = 0; i < item->childCount(); i++)
        {
            if(item->child(i)->text(0) == names[s])
            {
                item = item->child(i);
                s++;
                break;
            }
        }

        if(s == names.count())
        {
            this->ui->treeWidget->setCurrentItem(item);
            finded = true;
        }
    }
}

void ContentView::cdUp()
{
    if(this->currFolder != "[ROOT]\\")
    {
        this->currFolder.chop(1);
        int pos = this->currFolder.lastIndexOf('\\');
        this->currFolder.remove(pos + 1, this->currFolder.length() - pos);
        //this->currFolder += "\\";
        this->setCurrentFolder(this->currFolder);
    }
}

void ContentView::removeFiles()
{
    QList<QListWidgetItem*> list = this->fileList->selectedItems();

    if(list.count())
    {
        QMessageBox msg(this);
        msg.setWindowTitle("Удаление...");
        msg.setText("Удалить выбраные файлы/каталоги?");

        QString files;
        bool hasFolders = false;

        for(int i = 0; i < list.count(); i++)
        {
            files += list[i]->text() + "\n";
            if(list[i]->data(Qt::UserRole).toInt() == 0)
                hasFolders = true;
        }
        msg.setInformativeText(files);
        msg.setIcon(QMessageBox::Question);
        msg.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msg.setDefaultButton(QMessageBox::No);

        if(msg.exec() == QMessageBox::Yes)
        {
            if(FileSystem::getInst()->fsOpen(&this->handle))
            {
                QString folder = this->currFolder.remove("[ROOT]\\");
                QStringList f = FileSystem::getInst()->fsGetFileNamesList(&this->handle);

                for(int i = 0; i < list.count(); i++)
                {
                    if(list[i]->data(Qt::UserRole).toInt() == 0)
                    {
                        foreach(QString value, f)
                        {
                            QString fname = QString(folder + list[i]->text() + "\\");
                            if(value.contains(fname))
                            {
                                FileSystem::getInst()->fsDelete(value, &this->handle);
                            }
                        }
                    }
                    else
                    {
                        FileSystem::getInst()->fsDelete(QString(folder + list[i]->text()), &this->handle);
                        delete list[i];
                    }
                }
                FileSystem::getInst()->fsClose(&this->handle);
            }
        }
    }
}

void ContentView::viewEdit()
{
    if(FileSystem::getInst()->fsOpen(&this->handle))
    {
        TextView dlg(this);
        QString fname = this->currFolder.remove("[ROOT]\\") + this->fileList->currentItem()->text();
        dlg.setText(FileSystem::getInst()->fsGetFile(fname, &this->handle));

        if(dlg.exec() == QDialog::Accepted)
        {
            FileSystem::getInst()->fsRewriteFile(dlg.getText(), fname, &this->handle, 2);
        }
        FileSystem::getInst()->fsClose(&this->handle);
    }
}
