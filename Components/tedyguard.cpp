#include <QDesktopWidget>
#include "tedyguard.h"
#include "ui_tedyguard.h"


TedyGuard::TedyGuard(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::TedyGuard)
{
    ui->setupUi(this);

    setFixedSize(width(), height());
    move((QApplication::desktop()->width() - width()) / 2,
         (QApplication::desktop()->height() - height()) / 2);
}

TedyGuard::~TedyGuard()
{
    delete ui;
}

void TedyGuard::on_buttonScan_clicked()
{
    ui->groupPage->setCurrentIndex(1);
}

void TedyGuard::on_buttonStopScan_clicked()
{
    ui->groupPage->setCurrentIndex(2);
}

void TedyGuard::on_buttonCancelClean_clicked()
{
    ui->groupPage->setCurrentIndex(3);
}

void TedyGuard::on_buttoStopClean_clicked()
{
    ui->groupPage->setCurrentIndex(4);
}

void TedyGuard::on_buttonReturn_clicked()
{
    ui->groupPage->setCurrentIndex(0);
}
