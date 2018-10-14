#include <QDesktopWidget>
#include <QMessageBox>

#include "tedyguard.h"
#include "ui_tedyguard.h"

#define SPANDA_TDGRD_STATE_READY 0
#define SPANDA_TDGRD_STATE_SCANNING 1
#define SPANDA_TDGRD_STATE_SCAN_RESULT 2
#define SPANDA_TDGRD_STATE_CLEANING 3
#define SPANDA_TDGRD_STATE_CLEAN_RESULT 4
#define SPANDA_TDGRD_STATE_STOPPED 5


TedyGuard::TedyGuard(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::TedyGuard)
{
    ui->setupUi(this);

    setFixedSize(width(), height());
    move((QApplication::desktop()->width() - width()) / 2,
         (QApplication::desktop()->height() - height()) / 2);

    currentState = SPANDA_TDGRD_STATE_READY;

    connect(&cleaner,
            SIGNAL(scanFinished()),
            this,
            SLOT(onCleanerScanFinished()));
    connect(&cleaner,
            SIGNAL(cleaningFinished()),
            this,
            SLOT(onCleanerCleaningFinished()));
}

TedyGuard::~TedyGuard()
{
    delete ui;
}

void TedyGuard::onCleanerScanFinished()
{
    currentState = SPANDA_TDGRD_STATE_SCAN_RESULT;
    ui->groupPage->setCurrentIndex(2);
}

void TedyGuard::onCleanerCleaningFinished()
{
    currentState = SPANDA_TDGRD_STATE_CLEAN_RESULT;
    ui->groupPage->setCurrentIndex(4);
}

void TedyGuard::on_buttonScan_clicked()
{
    QSet<FileCleaner::ScanCategory> categories;
    for (int i=0; i<ui->listScanCategoryList->count(); i++)
    {
        if (ui->listScanCategoryList->item(i)->checkState() == Qt::Checked)
        switch (i)
        {
            case 0:
                categories.insert(FileCleaner::ScanAppCache);
                break;
            case 1:
                categories.insert(FileCleaner::ScanDownloadPackage);
                break;
            case 2:
                categories.insert(FileCleaner::ScanUnusedPackageConf);
                break;
            default:;
        }
    }
    if (categories.count() < 1)
    {
        QMessageBox::information(this, tr("No item choosed"),
                                 tr("Please choose at least one item to "
                                    "do scanning."));
        return;
    }

    ui->buttonStopScan->setText(tr("Stop"));
    ui->groupPage->setCurrentIndex(1);
    cleaner.scanByCategory(categories);
}

void TedyGuard::on_buttonStopScan_clicked()
{
    if (currentState == SPANDA_TDGRD_STATE_SCANNING)
    {
        currentState = SPANDA_TDGRD_STATE_STOPPED;
        cleaner.stop();
        ui->progressScan->setValue(0);
        ui->buttonStopScan->setText(tr("Return"));
    }
    else
    {
        ui->groupPage->setCurrentIndex(0);
        ui->buttonStopScan->setText(tr("Stop"));
    }
}

void TedyGuard::on_buttonCancelClean_clicked()
{
    currentState = SPANDA_TDGRD_STATE_READY;
    ui->groupPage->setCurrentIndex(0);
}

void TedyGuard::on_buttonClean_clicked()
{
    currentState = SPANDA_TDGRD_STATE_CLEANING;
    ui->buttonStopClean->setText(tr("Stop"));
    ui->groupPage->setCurrentIndex(3);
    cleaner.clean();
}

void TedyGuard::on_buttonStopClean_clicked()
{
    if (currentState == SPANDA_TDGRD_STATE_CLEANING)
    {
        currentState = SPANDA_TDGRD_STATE_STOPPED;
        cleaner.stop();
        ui->progressScan->setValue(0);
        ui->buttonStopClean->setText(tr("Return"));
    }
    else
    {
        ui->groupPage->setCurrentIndex(0);
        ui->buttonStopClean->setText(tr("Stop"));
    }
}

void TedyGuard::on_buttonReturn_clicked()
{
    currentState = SPANDA_TDGRD_STATE_READY;
    ui->groupPage->setCurrentIndex(0);
}
