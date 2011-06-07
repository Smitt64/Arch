#include "mainwindow.h"
#include <QDebug>
#include <QLabel>
#include <QMdiArea>
#include <QMdiSubWindow>
#include <QSignalMapper>
#include <contentview.h>
#include <QFileDialog>
#include <QFileInfo>
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    mdiArea = new QMdiArea(this);
    mdiArea->setDocumentMode(false);
    mdiArea->setViewMode(QMdiArea::TabbedView);
    this->setCentralWidget(this->mdiArea);

    windowMapper = new QSignalMapper(this);

    /*foreach(QTabBar* tab, mdiArea->findChildren())
    {
        tab->setTabsClosable(true);
    }*/
    archBar = this->addToolBar("Архив");

    curFolder = new QLabel;
    curFolder->setMinimumWidth(150);
    curFolder->setTextInteractionFlags(Qt::TextSelectableByMouse);
    selected = new QLabel;

    this->ui->statusBar->addWidget(this->curFolder);
    this->ui->statusBar->addWidget(this->selected);

    this->ui->mainToolBar->addAction(this->ui->create);
    this->ui->mainToolBar->addAction(this->ui->open);
    this->ui->mainToolBar->addAction(this->ui->close);

    archBar->addAction(this->ui->upDir);
    archBar->addSeparator();
    archBar->addAction(this->ui->add_file);
    archBar->addAction(this->ui->remove);
    archBar->addAction(this->ui->rename);
    archBar->addSeparator();
    archBar->addAction(this->ui->view_edit);

    updateActions();

    connect(this->ui->open, SIGNAL(triggered()), this, SLOT(open()));
    connect(this->ui->upDir, SIGNAL(triggered()), this, SLOT(upFolder()));
    connect(this->ui->remove, SIGNAL(triggered()), this, SLOT(removeFiles()));
    connect(this->ui->view_edit, SIGNAL(triggered()), this, SLOT(viewedit()));
    connect(mdiArea, SIGNAL(subWindowActivated(QMdiSubWindow*)), this, SLOT(updateActions()));

    this->setAutoFillBackground(false);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setActiveSubWindow(QWidget *window)
{
    if(!window)
        return;

    mdiArea->setActiveSubWindow(qobject_cast<QMdiSubWindow*>(window));
}

ContentView *MainWindow::activeMdiChild()
{
    if(QMdiSubWindow *activeSubWindow = mdiArea->activeSubWindow())
        return qobject_cast<ContentView*>(activeSubWindow->widget());

    return NULL;
}

QMdiSubWindow *MainWindow::findMdiChild(const QString &fileName)
{
    QString canonicalFilePath = QFileInfo(fileName).canonicalFilePath();

    foreach(QMdiSubWindow *window, mdiArea->subWindowList())
    {
        ContentView *mdiChild = qobject_cast<ContentView*>(window->widget());
        if(mdiChild->currentFile() == canonicalFilePath)
        return window;
    }

    return NULL;
}

void MainWindow::open()
{
    QString fileName = QFileDialog::getOpenFileName(this);
    if(!fileName.isEmpty())
    {
        QMdiSubWindow *existing = findMdiChild(fileName);
        if(existing)
        {
            mdiArea->setActiveSubWindow(existing);
            return;
        }

        ContentView *child = createMdiChild();
        if(child->loadFile(fileName))
        {
            child->show();
            child->setCurrentFolder("[ROOT]\\");
            this->curFolder->setText("[ROOT]\\");
        }
        else
            child->close();
     }
    /*ContentView *child = createMdiChild();
    if(child->loadFile("D:/ArchV2/debug/test.tst"))
        child->showMaximized();*/
}

ContentView *MainWindow::createMdiChild()
{
    ContentView *child = new ContentView(this->mdiArea);
    QMdiSubWindow *subWindow = mdiArea->addSubWindow(child);
    subWindow->setAttribute(Qt::WA_DeleteOnClose);

    connect(child, SIGNAL(currentFolderChanged(QString)), this, SLOT(setCurFolder(QString)));
    connect(child, SIGNAL(fileSelected(QString)), this, SLOT(setCurFile(QString)));
    /*connect(child, SIGNAL(copyAvailable(bool)),
                 cutAct, SLOT(setEnabled(bool)));
         connect(child, SIGNAL(copyAvailable(bool)),
                 copyAct, SLOT(setEnabled(bool)));*/

    return child;
}

void MainWindow::upFolder()
{
    if(activeMdiChild())
    {
        activeMdiChild()->cdUp();
    }
}

void MainWindow::setCurFolder(QString text)
{
    this->curFolder->setText(text);
}

void MainWindow::updateActions()
{
    bool hasMdiChild = (activeMdiChild() != 0);

    this->ui->close->setEnabled(hasMdiChild);
    this->ui->add_file->setEnabled(hasMdiChild);
    this->ui->remove->setEnabled(hasMdiChild);
    this->ui->rename->setEnabled(hasMdiChild);
    this->ui->upDir->setEnabled(hasMdiChild);
}

void MainWindow::setCurFile(QString name)
{
    this->selected->setText(name);
}

void MainWindow::removeFiles()
{
    if(activeMdiChild())
    {
        activeMdiChild()->removeFiles();
    }
}

void MainWindow::viewedit()
{
    if(activeMdiChild())
    {
        activeMdiChild()->viewEdit();
    }
}
