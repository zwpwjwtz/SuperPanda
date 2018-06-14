#include <QMessageBox>
#include "magickonfug.h"
#include "ui_magickonfug.h"

#define SPANDA_MGCKF_CONFIG_NONE 0
#define SPANDA_MGCKF_CONFIG_SERVICE_TIMEOUT 1
#define SPANDA_MGCKF_CONFIG_SHUTDOWN_TIMEOUT 2
#define SPANDA_MGCKF_CONFIG_CPU_INTEL_TURBO 3
#define SPANDA_MGCKF_CONFIG_IO_KEYBD_COMPOSE 4
#define SPANDA_MGCKF_CONFIG_DISK_PHYSICS 5

#define SPANDA_MGCKF_FILE_SYSTEMD_SYSTEM "/home/user/system.conf"
#define SPANDA_MGCKF_FILE_SYSTEMD_USER "/home/user/user.conf"


MagicKonfug::MagicKonfug(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MagicKonfug)
{
    ui->setupUi(this);

    setFixedSize(width(), height());
    ui->frameStatus->hide();
    ui->listWidget->setStyleSheet("background-color: transparent;");
    ui->groupPage->setCurrentIndex(pageGroupCount);

    loadConfig();
}

MagicKonfug::~MagicKonfug()
{
    delete ui;
}

void MagicKonfug::loadConfig()
{

}

void MagicKonfug::applyConfig(int configIndex)
{
    QString configValue;
    QString backupName;
    switch (configIndex)
    {
        case SPANDA_MGCKF_CONFIG_SERVICE_TIMEOUT:
            if (!configFile.fileExists(SPANDA_MGCKF_FILE_SYSTEMD_SYSTEM))
            {
                  warnMissingFile(SPANDA_MGCKF_FILE_SYSTEMD_SYSTEM);
                  break;
            }
            if (!configFile.fileExists(SPANDA_MGCKF_FILE_SYSTEMD_USER))
            {
                  warnMissingFile(SPANDA_MGCKF_FILE_SYSTEMD_USER);
                  break;
            }
            configFile.backupFile(SPANDA_MGCKF_FILE_SYSTEMD_SYSTEM, backupName);
            configFile.backupFile(SPANDA_MGCKF_FILE_SYSTEMD_USER, backupName);
            configValue = ui->textTimeoutSrvStart->text();
            configFile.regexpReplaceLine(SPANDA_MGCKF_FILE_SYSTEMD_SYSTEM,
                                        "DefaultTimeoutStartSec=",
                                        "#*DefaultTimeoutStartSec=\\w*",
                                        QString("DefaultTimeoutStartSec=%1s")
                                                .arg(configValue));
            configFile.regexpReplaceLine(SPANDA_MGCKF_FILE_SYSTEMD_SYSTEM,
                                         "DefaultTimeoutStopSec=",
                                         "#*DefaultTimeoutStopSec=\\w*",
                                         QString("DefaultTimeoutStopSec=%1s")
                                                 .arg(configValue));
            configFile.regexpReplaceLine(SPANDA_MGCKF_FILE_SYSTEMD_USER,
                                         "DefaultTimeoutStartSec=",
                                         "#*DefaultTimeoutStartSec=\\w*",
                                         QString("DefaultTimeoutStartSec=%1s")
                                                 .arg(configValue));
            configFile.regexpReplaceLine(SPANDA_MGCKF_FILE_SYSTEMD_USER,
                                         "DefaultTimeoutStopSec=",
                                         "#*DefaultTimeoutStopSec=\\w*",
                                         QString("DefaultTimeoutStopSec=%1s")
                                                 .arg(configValue));
            configValue = ui->textTimeoutShutdown->text();
            configFile.regexpReplaceLine(SPANDA_MGCKF_FILE_SYSTEMD_SYSTEM,
                                         "ShutdownWatchdogSec=",
                                         "#*ShutdownWatchdogSec=\\w*",
                                         QString("ShutdownWatchdogSec=%1s")
                                                 .arg(configValue));
            break;
        default:;
    }
}

void MagicKonfug::warnMissingFile(QString fileName, bool aborted)
{
    if (aborted)
    {
        QMessageBox::critical(this, "Missing file",
                              QString("Cannot continue due to a missing file: "
                                      "\n%1").arg(fileName));
    }
    else
    {
        QMessageBox::warning(this, "Missing file",
                             QString("File %1 does not exists. We will try to "
                                     "create it if possible.").arg(fileName));
    }
}

void MagicKonfug::warnPermission(QString objectName)
{
    QMessageBox::critical(this, "Permission denied",
                          QString("Cannot continue due to denied access to "
                                  "\n%1").arg(objectName));
}

void MagicKonfug::on_listWidget_clicked(const QModelIndex &index)
{
    ui->groupPage->setCurrentIndex(index.row());
}

void MagicKonfug::on_buttonAbout_clicked()
{
    QList<QListWidgetItem*> selectedItems = ui->listWidget->selectedItems();
    if (selectedItems.count() > 0)
    {
        for (int i=0; i<selectedItems.count(); i++)
            selectedItems[i]->setSelected(false);
    }

    ui->groupPage->setCurrentIndex(pageGroupCount + 1);
}

void MagicKonfug::on_buttonExit_clicked()
{
    close();
}


void MagicKonfug::on_groupPage_currentChanged(int arg1)
{
    if (arg1 >= 0 && arg1 < pageGroupCount)
        ui->buttonBox->show();
    else
        ui->buttonBox->hide();
}

void MagicKonfug::on_buttonBox_clicked(QAbstractButton *button)
{
    if (ui->buttonBox->buttonRole(button) ==
                        QDialogButtonBox::ButtonRole::ApplyRole)
    {
        switch (ui->groupPage->currentIndex())
        {
            case 0: // Startup
                break;
            case 1: // Hardware
                applyConfig(SPANDA_MGCKF_CONFIG_CPU_INTEL_TURBO);
                break;
            case 2: // Service
                applyConfig(SPANDA_MGCKF_CONFIG_SERVICE_TIMEOUT);
                applyConfig(SPANDA_MGCKF_CONFIG_SHUTDOWN_TIMEOUT);
                break;
            case 3: // I/O
                applyConfig(SPANDA_MGCKF_CONFIG_IO_KEYBD_COMPOSE);
                break;
            case 4: // Disk
                applyConfig(SPANDA_MGCKF_CONFIG_DISK_PHYSICS);
                break;
            default:;
        }
    }
}
