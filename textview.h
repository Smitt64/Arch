#ifndef TEXTVIEW_H
#define TEXTVIEW_H

#include <QDialog>

namespace Ui {
    class TextView;
}

class TextView : public QDialog
{
    Q_OBJECT

public:
    explicit TextView(QWidget *parent = 0);
    ~TextView();

    void setText(QByteArray value);
    QByteArray getText();

private:
    Ui::TextView *ui;
};

#endif // TEXTVIEW_H
