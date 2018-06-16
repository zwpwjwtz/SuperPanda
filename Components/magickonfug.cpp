﻿#include <QMessageBox>
#include "magickonfug.h"
#include "ui_magickonfug.h"

#define SPANDA_MGCKF_CONFIG_NONE 0
#define SPANDA_MGCKF_CONFIG_SERVICE_TIMEOUT 1
#define SPANDA_MGCKF_CONFIG_SHUTDOWN_TIMEOUT 2
#define SPANDA_MGCKF_CONFIG_CPU_INTEL_TURBO 3
#define SPANDA_MGCKF_CONFIG_KEYBD_COMPOSE 4
#define SPANDA_MGCKF_CONFIG_DISK_PHYSICS 5

#define SPANDA_MGCKF_EXEC_GRUB_UPDATE "/usr/sbin/grub2-mkconfig"
#define SPANDA_MGCKF_FILE_SYSTEMD_SYSTEM "/etc/systemd/system.conf"
#define SPANDA_MGCKF_FILE_SYSTEMD_USER "/etc/systemd/user.conf"
#define SPANDA_MGCKF_FILE_GRUB_DEFAULT "/etc/default/grub"
#define SPANDA_MGCKF_FILE_GRUB_CONFIG "/boot/grub2/grub.cfg"
#define SPANDA_MGCKF_FILE_KEYBD_DEFAULT "/etc/default/keyboard"


MagicKonfug::MagicKonfug(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MagicKonfug)
{
    ui->setupUi(this);

    setFixedSize(width(), height());
    ui->frameStatus->hide();
    ui->listWidget->setStyleSheet("background-color: transparent;");
    ui->groupPage->setCurrentIndex(pageGroupCount);
    ui->buttonBox->setEnabled(false);

    loadConfig();
    for (int i=0; i<pageGroupCount; i++)
        configPageMoidified[i] = false;
    for (int i=0; i<configEntryCount; i++)
        configMoidified[i] = false;

    connect(&exeFile,
            SIGNAL(finished(ExeLauncher::ExecErrorCode)),
            this,
            SLOT(onExeFinished(ExeLauncher::ExecErrorCode)));
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
            intValue = expression.cap(1).toInt();
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
            intValue = expression.cap(1).toInt();
            ui->textTimeoutShutdown->setValue(intValue);
        }
    }

    errCode = configFile.findLine(SPANDA_MGCKF_FILE_GRUB_DEFAULT,
                                  "intel_pstate=enable",
                                  configValue);
    if (errCode == ConfigFileEditor::FileOk)
    {
        ui->checkTurboFreq->setChecked(!configValue.isEmpty());
    }

    errCode = configFile.findLine(SPANDA_MGCKF_FILE_KEYBD_DEFAULT,
                                  "XKBOPTIONS=",
                                  configValue);
    if (errCode == ConfigFileEditor::FileOk)
    {
        expression.setPattern("compose:(\\w+)");
        if (expression.indexIn(configValue) >= 0)
            configValue = expression.cap(1);
        ui->comboKeySequence->setCurrentIndex(
                                    composeKeyStringToIndex(configValue));
    }
    else
    {
        ui->comboKeySequence->setEnabled(false);
        ui->comboKeySequence->setToolTip("Not supported on your system.");
    }
}

bool MagicKonfug::applyConfig(int configIndex)
{
    static bool successful;
    static bool valueExists;
    static QString configValue;
    static QString fileName;
    static QString backupName;
    static ConfigFileEditor::FileErrorCode errCode;
    static QList<QString> tempStringList;

    if (!configMoidified[configIndex])
        return true;

    successful = false;
    switch (configIndex)
    {
        case SPANDA_MGCKF_CONFIG_SERVICE_TIMEOUT:
            fileName = SPANDA_MGCKF_FILE_SYSTEMD_SYSTEM;
            configFile.backupFile(fileName, backupName);
            configValue = QString::number(ui->textTimeoutSrvStart->value());
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
            successful = testConfigFileError(errCode, fileName);

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
            successful &= testConfigFileError(errCode, fileName);
            break;
        case SPANDA_MGCKF_CONFIG_SHUTDOWN_TIMEOUT:
            fileName = SPANDA_MGCKF_FILE_SYSTEMD_SYSTEM;
            configFile.backupFile(fileName, backupName);
            configValue = QString::number(ui->textTimeoutShutdown->value());
            errCode = configFile.regexpReplaceLine(fileName,
                                         "ShutdownWatchdogSec=",
                                         "#*ShutdownWatchdogSec=\\w*",
                                         QString("ShutdownWatchdogSec=%1s")
                                                 .arg(configValue));
            successful = testConfigFileError(errCode, fileName);
            break;
        case SPANDA_MGCKF_CONFIG_CPU_INTEL_TURBO:
            fileName = SPANDA_MGCKF_FILE_GRUB_DEFAULT;
            configFile.backupFile(fileName, backupName);
            if (ui->checkTurboFreq->isChecked())
                errCode = configFile.regexpReplaceLine(fileName,
                                             "GRUB_CMDLINE_LINUX_DEFAULT=",
                                             "\"\\n",
                                             " intel_pstate=enable\"\n");
            else
                errCode = configFile.regexpReplaceLine(fileName,
                                             "GRUB_CMDLINE_LINUX_DEFAULT=",
                                             "\\s*intel_pstate=enable",
                                             "");
            if (!testConfigFileError(errCode, fileName))
                break;
            tempStringList.clear();
            tempStringList.append("-o");
            tempStringList.append(SPANDA_MGCKF_FILE_GRUB_CONFIG);
            if (exeFile.runFile(SPANDA_MGCKF_EXEC_GRUB_UPDATE,
                                         tempStringList) == ExeLauncher::ExecOk)
                successful = true;
            showStatusPage(true);
            break;
            case SPANDA_MGCKF_CONFIG_KEYBD_COMPOSE:
                fileName = SPANDA_MGCKF_FILE_KEYBD_DEFAULT;
                configFile.backupFile(fileName, backupName);
                configValue = composeKeyIndexToString(
                                        ui->comboKeySequence->currentIndex());
                if (!configValue.isEmpty())
                    configValue.prepend("compose:");
                configFile.exists(fileName, "XKBOPTIONS", valueExists);
                if (valueExists)
                {
                    errCode = configFile.regexpReplaceLine(fileName,
                                            "XKBOPTIONS=",
                                            "\\s*compose:\\w*",
                                            "");
                    configFile.regexpReplaceLine(fileName,
                                            "XKBOPTIONS=",
                                            "\"\\n",
                                            QString(" %1\"\n")
                                                   .arg(configValue));
                }
                else
                    errCode = configFile.append(fileName,
                                                QString("XKBOPTIONS=\"%1\"")
                                                       .arg(configValue));
                successful = testConfigFileError(errCode, fileName);
            break;
        default:;
    }
    if (successful)
        setConfigModified(configIndex, false);
    return successful;
}

void MagicKonfug::setConfigModified(int configIndex, bool modified)
{
    configMoidified[configIndex] = modified;

    // Update modification state of the related page
    // Note: the page flag is not set to "false" even when
    //       all related config entries are set to "false".
    //       The cleaning of page flag is done in on_buttonBox_clicked()
    switch (configIndex)
    {
        case SPANDA_MGCKF_CONFIG_CPU_INTEL_TURBO:
            setConfigPageModified(1, modified);
            break;
        case SPANDA_MGCKF_CONFIG_SERVICE_TIMEOUT:
        case SPANDA_MGCKF_CONFIG_SHUTDOWN_TIMEOUT:
            setConfigPageModified(2, modified);
            break;
        case SPANDA_MGCKF_CONFIG_KEYBD_COMPOSE:
            setConfigPageModified(3, modified);
            break;
        default:;
    }
}

void MagicKonfug::setConfigPageModified(int pageIndex, bool modified)
{
    configPageMoidified[pageIndex] |= modified;
    if (ui->groupPage->currentIndex() == pageIndex)
        ui->buttonBox->setEnabled(modified);
}

void MagicKonfug::showStatusPage(bool pageVisible, QString text)
{
    if (pageVisible)
    {
        ui->groupPage->hide();
        ui->frameStatus->show();
        ui->buttonBox->setEnabled(false);
        if (text.isEmpty())
            text = "Processing configuration, please wait...";
        ui->labelStatus->setText(text);
    }
    else
    {
        ui->groupPage->show();
        ui->frameStatus->hide();
        ui->buttonBox->setEnabled(true);
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

void MagicKonfug::warnExecPermission(QString objectName)
{
    QMessageBox::critical(nullptr, "Permission denied",
                          QString("Cannot execute %1.\n"
                                  "Please make sure that you have the "
                                  "right permission to do it.")
                                 .arg(objectName));
}

int MagicKonfug::composeKeyStringToIndex(const QString &str)
{
    if (str == "ralt")
        return 1;
    else if (str == "lwin")
        return 2;
    else if (str == "rwin")
        return 3;
    else if (str == "lctrl")
        return 4;
    else if (str == "rctrl")
        return 5;
    else if (str == "caps")
        return 6;
    else if (str == "menu")
        return 7;
    else
        return 0;
}

QString MagicKonfug::composeKeyIndexToString(int index)
{
    switch (index)
    {
        case 1: // Right Alt
            return "ralt";
        case 2: // Left Win
            return "lwin";
        case 3: // Right Win
            return "rwin";
        case 4: // Left Ctrl
            return "lctrl";
        case 5: // Right Ctrl
            return "rctrl";
        case 6: // Caps Lock
            return "caps";
        case 7: // Menu
            return "menu";
        default:
            return "";
    }
}

void MagicKonfug::onExeFinished(ExeLauncher::ExecErrorCode errCode)
{
    switch (errCode)
    {
        case ExeLauncher::ExecOk:
            QMessageBox::information(this, "Configuration(s) applied",
                                     "Finish applying configuration. "
                                     "You may need to reboot to have them "
                                     "take effect.");
            break;
        case ExeLauncher::FileNotFound:
            warnMissingFile(exeFile.getExeFilePath(), true);
            break;
        case ExeLauncher::NoPermission:
            warnExecPermission(exeFile.getExeFilePath());
            break;
        case ExeLauncher::Crashed:
        case ExeLauncher::UnknownError:
        default:
            QMessageBox::critical(this, "Unknown error occured",
                                  QString("Magic Panda encountered an exception "
                                          "when trying to execute program \n%1\n"
                                          "Exit code: %2\n")
                                         .arg(exeFile.getCommand())
                                         .arg(exeFile.getExitCode()));
    }
    showStatusPage(false);
    if (ui->groupPage->currentIndex() < pageGroupCount)
        ui->buttonBox->setEnabled(
                        configPageMoidified[ui->groupPage->currentIndex()]);
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
    {
        ui->buttonBox->show();
        ui->buttonBox->setEnabled(configPageMoidified[arg1]);
    }
    else
        ui->buttonBox->hide();
}

void MagicKonfug::on_buttonBox_clicked(QAbstractButton *button)
{
    static bool applied;
    if (ui->buttonBox->buttonRole(button) ==
                        QDialogButtonBox::ButtonRole::ApplyRole)
    {
        int pageIndex = ui->groupPage->currentIndex();
        switch (pageIndex)
        {
            case 0: // Startup
                break;
            case 1: // Hardware
                applied = applyConfig(SPANDA_MGCKF_CONFIG_CPU_INTEL_TURBO);
                break;
            case 2: // Service
                applied = applyConfig(SPANDA_MGCKF_CONFIG_SERVICE_TIMEOUT);
                applied = applyConfig(SPANDA_MGCKF_CONFIG_SHUTDOWN_TIMEOUT);
                break;
            case 3: // I/O
                applied = applyConfig(SPANDA_MGCKF_CONFIG_KEYBD_COMPOSE);
                break;
            case 4: // Disk
                applied = applyConfig(SPANDA_MGCKF_CONFIG_DISK_PHYSICS);
                break;
            default:;
        }
        if (pageIndex < pageGroupCount && applied)
            configPageMoidified[pageIndex] = false;
    }
}

void MagicKonfug::on_textTimeoutSrvStart_valueChanged(int arg1)
{
    Q_UNUSED(arg1)
    setConfigModified(SPANDA_MGCKF_CONFIG_SERVICE_TIMEOUT);
}

void MagicKonfug::on_textTimeoutShutdown_valueChanged(int arg1)
{
    Q_UNUSED(arg1)
    setConfigModified(SPANDA_MGCKF_CONFIG_SHUTDOWN_TIMEOUT);
}

void MagicKonfug::on_checkTurboFreq_toggled(bool checked)
{
    Q_UNUSED(checked)
    setConfigModified(SPANDA_MGCKF_CONFIG_CPU_INTEL_TURBO);
}

void MagicKonfug::on_comboKeySequence_currentIndexChanged(int index)
{
    Q_UNUSED(index)
    setConfigModified(SPANDA_MGCKF_CONFIG_KEYBD_COMPOSE);
}