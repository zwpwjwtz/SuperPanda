#include <QDesktopWidget>
#include <QTranslator>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "global.h"
#include "aboutwindow.h"
#include "Components/magickonfug.h"
#include "Components/tedyguard.h"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    move((QApplication::desktop()->width() - width()) / 2,
         (QApplication::desktop()->height() - height()) / 2);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::LanguageChange)
        ui->retranslateUi(this);
}

void MainWindow::on_toolButton_2_clicked()
{
    if (!windowMgckf)
        windowMgckf = new MagicKonfug;
    windowMgckf->show();
}

void MainWindow::on_toolButton_3_clicked()
{
    if (!windowTdgrd)
        windowTdgrd = new TedyGuard;
    windowTdgrd->show();
}

void MainWindow::on_comboBox_currentIndexChanged(int index)
{
    appTranslator.load(QString(":/Translations/SuperPanda_")
                       .append(spanda_language_string[index]));
}

void MainWindow::on_pushButton_clicked()
{
    if (!windowAbout)
        windowAbout = new AboutWindow;
    windowAbout->show();
}
