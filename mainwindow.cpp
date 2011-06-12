#include "mainwindow.h"
#include <QDebug>
#include <QLabel>
#include <QMdiArea>
#include <QMdiSubWindow>
#include <QSignalMapper>
#include "contentview.h"
#include <QFileDialog>
#include <QFileInfo>
#include <QPushButton>
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
    archBar = this->addToolBar(tr("Archive"));
    archBar->setWindowIcon(QIcon(":/root"));

    curFolder = new QLabel;
    curFolder->setMinimumWidth(150);
    curFolder->setTextInteractionFlags(Qt::TextSelectableByMouse);
    selected = new QLabel;

    this->ui->statusBar->addWidget(this->curFolder);
    this->ui->statusBar->addWidget(this->selected);

    this->ui->mainToolBar->setWindowTitle(tr("Standart"));
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

    connect(this->ui->mainToolBar, SIGNAL(visibilityChanged(bool)), this, SLOT(toolBarVisibitity(bool)));
    connect(archBar, SIGNAL(visibilityChanged(bool)), this, SLOT(toolBarVisibitity(bool)));

    connect(this->ui->actionStandart, SIGNAL(triggered(bool)), this, SLOT(toolBarVisTriggered(bool)));
    connect(this->ui->actionArchive, SIGNAL(triggered(bool)), this, SLOT(toolBarVisTriggered(bool)));

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
}

ContentView *MainWindow::createMdiChild()
{
    ContentView *child = new ContentView(this->mdiArea);
    QMdiSubWindow *subWindow = mdiArea->addSubWindow(child);
    subWindow->setAttribute(Qt::WA_DeleteOnClose);

    connect(child, SIGNAL(currentFolderChanged(QString)), this, SLOT(setCurFolder(QString)));
    connect(child, SIGNAL(fileSelected(QString)), this, SLOT(setCurFile(QString)));

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

    QString caption;
    if(hasMdiChild)
    {
        caption = activeMdiChild()->windowTitle();
        this->setWindowTitle(QString("MyArchiver - %1").arg(caption));
    }
    else
        this->setWindowTitle(QString("MyArchiver"));

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

void MainWindow::toolBarVisibitity(bool value)
{
    QWidget *w = (QWidget*)this->sender();
    QList<QAction*> list = this->ui->menuView->actions();
    for(int i = 0; i < list.count(); i++)
        if(list[i]->text() == w->windowTitle())
            list[i]->setChecked(value);
}

void MainWindow::toolBarVisTriggered(bool value)
{
    QAction *w = (QAction*)this->sender();

    if(w->text() == this->ui->mainToolBar->windowTitle())
        this->ui->mainToolBar->setVisible(value);

    if(w->text() == this->archBar->windowTitle())
        this->archBar->setVisible(value);
}
