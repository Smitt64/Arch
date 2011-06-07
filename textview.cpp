#include "textview.h"
#include <QTextStream>
#include "ui_textview.h"

TextView::TextView(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TextView)
{
    ui->setupUi(this);
}

TextView::~TextView()
{
    delete ui;
}

void TextView::setText(QByteArray value)
{
    this->ui->plainTextEdit->setPlainText(value);
}

QByteArray TextView::getText()
{
    QByteArray data;
    QTextStream stream(&data);

    stream << this->ui->plainTextEdit->toPlainText();

    return data;
}
