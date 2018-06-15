#include <QMessageBox>
#include "magickonfug.h"
#include "ui_magickonfug.h"

#define SPANDA_MGCKF_CONFIG_NONE 0
#define SPANDA_MGCKF_CONFIG_SERVICE_TIMEOUT 1
#define SPANDA_MGCKF_CONFIG_SHUTDOWN_TIMEOUT 2
#define SPANDA_MGCKF_CONFIG_CPU_INTEL_TURBO 3
#define SPANDA_MGCKF_CONFIG_IO_KEYBD_COMPOSE 4
#define SPANDA_MGCKF_CONFIG_DISK_PHYSICS 5

#define SPANDA_MGCKF_FILE_SYSTEMD_SYSTEM "/etc/systemd/system.conf"
#define SPANDA_MGCKF_FILE_SYSTEMD_USER "/etc/systemd/user.conf"


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
    QString configValue;
    QRegExp expression;
    int intValue;
    ConfigFileEditor::FileErrorCode errCode;

    errCode = configFile.findLine(SPANDA_MGCKF_FILE_SYSTEMD_SYSTEM,
                                  "DefaultTimeoutStartSec=",
                                  configValue);
    if (errCode == ConfigFileEditor::FileOk)
    {
        expression.setPattern("^DefaultTimeoutStartSec=(\\d+)");
        if (expression.indexIn(configValue) >= 0)
        {
            intValue = expression.cap(0).toInt();
            ui->textTimeoutSrvStart->setValue(intValue);
        }
    }
    errCode = configFile.findLine(SPANDA_MGCKF_FILE_SYSTEMD_SYSTEM,
                                  "ShutdownWatchdogSec=",
                                  configValue);
    if (errCode == ConfigFileEditor::FileOk)
    {
        expression.setPattern("^ShutdownWatchdogSec=(\\d+)");
        if (expression.indexIn(configValue) >= 0)
        {
            intValue = expression.cap(0).toInt();
            ui->textTimeoutShutdown->setValue(intValue);
        }
    }
}

void MagicKonfug::applyConfig(int configIndex)
{
    QString configValue;
    QString fileName;
    QString backupName;
    ConfigFileEditor::FileErrorCode errCode;
    switch (configIndex)
    {
        case SPANDA_MGCKF_CONFIG_SERVICE_TIMEOUT:
            fileName = SPANDA_MGCKF_FILE_SYSTEMD_SYSTEM;
            configFile.backupFile(fileName, backupName);
            configValue = ui->textTimeoutSrvStart->value();
            errCode = configFile.regexpReplaceLine(fileName,
                                        "DefaultTimeoutStartSec=",
                                        "#*DefaultTimeoutStartSec=\\w*",
                                        QString("DefaultTimeoutStartSec=%1s")
                                                .arg(configValue));
            configFile.regexpReplaceLine(fileName,
                                         "DefaultTimeoutStopSec=",
                                         "#*DefaultTimeoutStopSec=\\w*",
                                         QString("DefaultTimeoutStopSec=%1s")
                                                 .arg(configValue));
            testConfigFileError(errCode, fileName);

            fileName = SPANDA_MGCKF_FILE_SYSTEMD_USER;
            configFile.backupFile(fileName, backupName);
            errCode = configFile.regexpReplaceLine(fileName,
                                         "DefaultTimeoutStartSec=",
                                         "#*DefaultTimeoutStartSec=\\w*",
                                         QString("DefaultTimeoutStartSec=%1s")
                                                 .arg(configValue));
            configFile.regexpReplaceLine(fileName,
                                         "DefaultTimeoutStopSec=",
                                         "#*DefaultTimeoutStopSec=\\w*",
                                         QString("DefaultTimeoutStopSec=%1s")
                                                 .arg(configValue));
            testConfigFileError(errCode, fileName);
            break;
        case SPANDA_MGCKF_CONFIG_SHUTDOWN_TIMEOUT:
            fileName = SPANDA_MGCKF_FILE_SYSTEMD_SYSTEM;
            configValue = ui->textTimeoutShutdown->value();
            errCode = configFile.regexpReplaceLine(fileName,
                                         "ShutdownWatchdogSec=",
                                         "#*ShutdownWatchdogSec=\\w*",
                                         QString("ShutdownWatchdogSec=%1s")
                                                 .arg(configValue));
            testConfigFileError(errCode, fileName);
            break;
        default:;
    }
}

bool MagicKonfug::testConfigFileError(ConfigFileEditor::FileErrorCode errCode,
                                      const QString& fileName,
                                      const bool aborted)
{
    bool ret = false;
    switch (errCode)
    {
        case ConfigFileEditor::FileOk:
            ret = true;
            break;
        case ConfigFileEditor::FileNotFound:
            MagicKonfug::warnMissingFile(fileName, aborted);
        break;
        case ConfigFileEditor::NoPermission:
            MagicKonfug::warnPermission(fileName);
        default:;
    }
    return ret;
}


void MagicKonfug::warnMissingFile(QString fileName, bool aborted)
{
    if (aborted)
    {
        QMessageBox::critical(nullptr, "Missing file",
                              QString("Cannot continue due to a missing file: "
                                      "\n%1").arg(fileName));
    }
    else
    {
        QMessageBox::warning(nullptr, "Missing file",
                             QString("File %1 does not exists. We will try to "
                                     "create it if possible.").arg(fileName));
    }
}

void MagicKonfug::warnPermission(QString objectName)
{
    QMessageBox::critical(nullptr, "Permission denied",
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
