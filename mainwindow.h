#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
    class MainWindow;
}

class QLabel;
class QMdiArea;
class QMdiSubWindow;
class QSignalMapper;
class ContentView;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void setActiveSubWindow(QWidget *window);
    void setCurFolder(QString text);
    void open();
    void updateActions();
    void setCurFile(QString name);
    void upFolder();
    void removeFiles();
    void viewedit();

    void toolBarVisibitity(bool value);
    void toolBarVisTriggered(bool value);

private:
    ContentView *activeMdiChild();
    QMdiSubWindow *findMdiChild(const QString &fileName);
    ContentView *createMdiChild();

    QSignalMapper *windowMapper;
    QMdiArea *mdiArea;
    QLabel *curFolder, *selected;
    QToolBar *archBar;
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
