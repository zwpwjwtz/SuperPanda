#include "magickonfug.h"
#include "ui_magickonfug.h"


MagicKonfug::MagicKonfug(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MagicKonfug)
{
    ui->setupUi(this);

    setFixedSize(width(), height());
    ui->listWidget->setStyleSheet("background-color: transparent;");
}

MagicKonfug::~MagicKonfug()
{
    delete ui;
}

void MagicKonfug::on_listWidget_clicked(const QModelIndex &index)
{
    ui->frameRight->setCurrentIndex(index.row() + 1);
}

void MagicKonfug::on_buttonAbout_clicked()
{
    QList<QListWidgetItem*> selectedItems = ui->listWidget->selectedItems();
    if (selectedItems.count() > 0)
    {
        for (int i=0; i<selectedItems.count(); i++)
            selectedItems[i]->setSelected(false);
    }

    ui->frameRight->setCurrentIndex(ui->frameRight->count() - 1);
}

void MagicKonfug::on_buttonExit_clicked()
{
    close();
}
