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
#include <QSettings>
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

    archBar = this->addToolBar(tr("Archive"));
    archBar->setWindowIcon(QIcon(":/root"));

    curFolder = new QLabel;
    curFolder->setMinimumWidth(150);
    curFolder->setTextInteractionFlags(Qt::TextSelectableByMouse);
    selected = new QLabel;

    ////////////////////////////////////////////////////////////////
    this->exitAction = new QAction(tr("Exit"), this);
    this->recentFileActs.clear();
    for (int i = 0; i < maxRecentFiles; ++i)
    {
        recentFileActs.push_back(new QAction(this));
        recentFileActs[i]->setVisible(false);
        connect(recentFileActs[i], SIGNAL(triggered()), this, SLOT(openRecentFile()));
    }

    separatorAct = this->ui->menu->addSeparator();
    for (int i = 0; i < maxRecentFiles; ++i)
        this->ui->menu->addAction(recentFileActs[i]);
    this->ui->menu->addSeparator();
    this->ui->menu->addAction(this->exitAction);
    updateRecentFileActions();
    ////////////////////////////////////////////////////////////////

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
    connect(this->exitAction, SIGNAL(triggered()), this, SLOT(close()));

    connect(this->ui->actionStandart, SIGNAL(triggered(bool)), this, SLOT(toolBarVisTriggered(bool)));
    connect(this->ui->actionArchive, SIGNAL(triggered(bool)), this, SLOT(toolBarVisTriggered(bool)));

    this->setAutoFillBackground(false);
}

void MainWindow::openRecentFile()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if(action)
        open(action->data().toString());
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

void MainWindow::open(QString fileName)
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

        QSettings settings;
        QStringList files = settings.value("recentFileList").toStringList();
        files.removeAll(fileName);
        files.prepend(fileName);
        while (files.size() > maxRecentFiles)
            files.removeLast();

        settings.setValue("recentFileList", files);
        this->updateRecentFileActions();
    }
    else
        child->close();
}

void MainWindow::open()
{
    QString fileName = QFileDialog::getOpenFileName(this);
    if(!fileName.isEmpty())
        this->open(fileName);
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

void MainWindow::updateRecentFileActions()
{
    QSettings settings;
    QStringList files = settings.value("recentFileList").toStringList();

    int numRecentFiles = qMin(files.size(), (int)maxRecentFiles);

    for (int i = 0; i < numRecentFiles; ++i) {
        QString text = tr("&%1 %2").arg(i + 1).arg(QFileInfo(files[i]).fileName());
        recentFileActs[i]->setText(text);
        recentFileActs[i]->setData(files[i]);
        recentFileActs[i]->setVisible(true);
    }
    for (int j = numRecentFiles; j < maxRecentFiles; ++j)
        recentFileActs[j]->setVisible(false);
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
