#include <QDesktopWidget>
#include <QMessageBox>
#include "magickonfug.h"
#include "ui_magickonfug.h"
#include "../Utils/dialogutils.h"
#include "../Utils/diskutils.h"

#define SPANDA_MGCKF_CONFIG_NONE 0
#define SPANDA_MGCKF_CONFIG_SERVICE_TIMEOUT 1
#define SPANDA_MGCKF_CONFIG_SHUTDOWN_TIMEOUT 2
#define SPANDA_MGCKF_CONFIG_CPU_INTEL_TURBO 3
#define SPANDA_MGCKF_CONFIG_KEYBD_COMPOSE 4
#define SPANDA_MGCKF_CONFIG_DISK_PHYSICS 5
#define SPANDA_MGCKF_CONFIG_BOOT_TIMEOUT 6
#define SPANDA_MGCKF_CONFIG_BOOT_RESOLUTION 7
#define SPANDA_MGCKF_CONFIG_WIFI_INTEL_80211n 8

#define SPANDA_MGCKF_FILE_GRUB_DEFAULT "/etc/default/grub"
#define SPANDA_MGCKF_FILE_KEYBD_DEFAULT "/etc/default/keyboard"
#define SPANDA_MGCKF_FILE_MOUNT_ROOT "/etc/fstab"
#define SPANDA_MGCKF_FILE_SYSTEMD_SYSTEM "/etc/systemd/system.conf"
#define SPANDA_MGCKF_FILE_SYSTEMD_USER "/etc/systemd/user.conf"
#define SPANDA_MGCKF_FILE_UDEV_DISK "/etc/udev/rules.d/95-superpanda.rules"
#define SPANDA_MGCKF_FILE_MODCONF_IWLWIFI "/etc/modprobe.d/iwlwifi.conf"


MagicKonfug::MagicKonfug(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MagicKonfug)
{
    ui->setupUi(this);

    setFixedSize(width(), height());
    move((QApplication::desktop()->width() - width()) / 2,
         (QApplication::desktop()->height() - height()) / 2);

    ui->frameStatus->hide();
    ui->listWidget->setStyleSheet("background-color: transparent;");
    ui->groupPage->setCurrentIndex(pageGroupCount);
    ui->buttonBox->setEnabled(false);

    loadConfig();

    connect(&bootConfig,
            SIGNAL(commandFinished(bool)),
            this,
            SLOT(onCommandFinished(bool)));
}

MagicKonfug::~MagicKonfug()
{
    delete ui;
}

void MagicKonfug::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::LanguageChange)
        ui->retranslateUi(this);
}

void MagicKonfug::showEvent(QShowEvent *event)
{
    Q_UNUSED(event)
    loadConfig();
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
        setWidgetDisabled(ui->comboKeySequence);

    errCode = configFile.findLine(SPANDA_MGCKF_FILE_UDEV_DISK,
                                  "ATTR{queue/rotational}==\"0\"",
                                  configValue);
    if (errCode == ConfigFileEditor::FileOk)
    {
        if (configValue.contains("deadline"))
            ui->comboDiskType->setCurrentIndex(1);
    }

    errCode = configFile.findLine(SPANDA_MGCKF_FILE_GRUB_DEFAULT,
                                  "GRUB_TIMEOUT=",
                                  configValue);
    if (errCode == ConfigFileEditor::FileOk)
    {
        if (configValue.at(12) == '=')
        {
            configValue = configValue.mid(13);
            ui->textTimeoutBoot->setValue(configValue.toInt());
        }
    }

    errCode = configFile.findLine(SPANDA_MGCKF_FILE_GRUB_DEFAULT,
                                  "GRUB_GFXMODE=",
                                  configValue);
    if (errCode == ConfigFileEditor::FileOk)
    {
        if (configValue.at(12) == '=')
        {
            ui->comboBootResolution->setCurrentIndex(
                                    bootResolutionStringToIndex(configValue));
        }
    }

    errCode = configFile.findLine(SPANDA_MGCKF_FILE_MODCONF_IWLWIFI,
                                  "11n_disable=",
                                  configValue);
    if (errCode == ConfigFileEditor::FileOk)
    {
        if (configValue.at(11) == '=')
        {
            configValue = configValue.mid(12);
            ui->checkIWiFi80211n->setChecked(configValue.toInt() == 0);
        }
    }

    for (int i=0; i<pageGroupCount; i++)
        configPageMoidified[i] = false;
    for (int i=0; i<configEntryCount; i++)
        configMoidified[i] = false;
    needUpdatingBoot = false;
}

bool MagicKonfug::applyConfig(int configIndex)
{
    static bool successful;
    static bool valueExists;
    static QString configValue;
    static QString oldValue;
    static QString fileName;
    static QString backupName;
    static QRegExp expression;
    static ConfigFileEditor::FileErrorCode errCode;
    static QList<QString> tempStringList;
    static ExeLauncher exeFile;

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
            if (testConfigFileError(errCode, fileName))
            {
                needUpdatingBoot = true;
                successful = true;
            }
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
                                                QString("\nXKBOPTIONS=\"%1\"")
                                                       .arg(configValue));
                successful = testConfigFileError(errCode, fileName);
            break;
        case SPANDA_MGCKF_CONFIG_DISK_PHYSICS:
            // Get mount entry for the root partition
            exeFile.runCommand("mount | grep \" on / \"", true);
            tempStringList = QString(exeFile.getOutput())
                                    .split(' ').toVector().toList();
            expression.setPattern("^/dev/([a-zA-Z]+)\\d*");
            if (expression.indexIn(tempStringList[0]) < 0)
                break;

            // Set scheduler for disk I/O
            fileName = SPANDA_MGCKF_FILE_UDEV_DISK;
            configValue.clear();
            if (ui->comboDiskType->currentIndex() == 1) // Using SSD
                configValue = QString("ACTION==\"add|change\", "
                                      "KERNEL==\"%1\", "
                                      "ATTR{queue/rotational}==\"0\", "
                                      "ATTR{queue/scheduler}=\"deadline\"")
                                     .arg(expression.cap(1));
            configFile.exists(fileName,
                              QString("KERNEL==\"%1\", ATTR{queue/rotational}")
                                     .arg(expression.cap(1)),
                              valueExists);
            if (valueExists)
                errCode = configFile.regexpReplaceLine(fileName,
                                        QString("KERNEL==\"%1\", "
                                                "ATTR{queue/rotational}")
                                              .arg(expression.cap(1)),
                                        ".*", configValue);
            else
                errCode = configFile.append(fileName,
                                            configValue.prepend("\n"));
            successful = testConfigFileError(errCode, fileName);

            // Adjust mount parameters for root partition
            fileName = SPANDA_MGCKF_FILE_MOUNT_ROOT;
            configFile.backupFile(fileName, backupName);
            configFile.findLine(fileName, tempStringList[0], oldValue);
            if (oldValue.isEmpty() || oldValue.indexOf(tempStringList[0]) > 0)
                configFile.findLine(fileName,
                                    DiskUtils::getUUIDByBlock(
                                                        tempStringList[0]),
                                    oldValue);
            if (ui->comboDiskType->currentIndex() == 1) // Using SSD
            {
                configValue = " defaults";
                if (!oldValue.contains(",discard"))
                    configValue.append(",discard");
                if (!oldValue.contains(",noatime"))
                    configValue.append(",noatime");
                QString newValue(oldValue);
                newValue.replace(" defaults", configValue);
                errCode = configFile.replace(fileName, oldValue, newValue);
            }
            else // Using HDD
            {
                configFile.regexpReplaceLine(fileName, oldValue,
                                             ",discard", "");
            }
            successful &= testConfigFileError(errCode, fileName);
            break;
        case SPANDA_MGCKF_CONFIG_BOOT_TIMEOUT:
            fileName = SPANDA_MGCKF_FILE_GRUB_DEFAULT;
            configFile.backupFile(fileName, backupName);
            configValue = QString::number(ui->textTimeoutBoot->value());
            errCode = configFile.regexpReplaceLine(fileName,
                                     "GRUB_TIMEOUT=",
                                     "#*GRUB_TIMEOUT=\\d*",
                                     QString("GRUB_TIMEOUT=%1")
                                             .arg(configValue));

            if (testConfigFileError(errCode, fileName))
            {
                needUpdatingBoot = true;
                successful = true;
            }
            break;
        case SPANDA_MGCKF_CONFIG_BOOT_RESOLUTION:
            fileName = SPANDA_MGCKF_FILE_GRUB_DEFAULT;
            configFile.backupFile(fileName, backupName);
            configValue = bootResolutionIndexToString(
                                    ui->comboBootResolution->currentIndex());
            errCode = configFile.regexpReplaceLine(fileName,
                                     "GRUB_GFXMODE=",
                                     "#*GRUB_GFXMODE=\\w*",
                                     QString("GRUB_GFXMODE=%1")
                                             .arg(configValue));

            if (testConfigFileError(errCode, fileName))
            {
                needUpdatingBoot = true;
                successful = true;
            }
            break;
        case SPANDA_MGCKF_CONFIG_WIFI_INTEL_80211n:
            fileName = SPANDA_MGCKF_FILE_MODCONF_IWLWIFI;
            configFile.backupFile(fileName, backupName);
            if (ui->checkIWiFi80211n->isChecked())
                configValue = "0";
            else
                configValue = "1";
            configFile.exists(fileName, "11n_disable=", valueExists);
            if (valueExists)
                errCode = configFile.regexpReplaceLine(fileName,
                                     "11n_disable=",
                                     "11n_disable=\\w*",
                                     QString("11n_disable=%1")
                                            .arg(configValue));
            else
                errCode = configFile.append(fileName,
                                     QString("\noptions iwlwifi 11n_disable=%1")
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
        // Startup
        case SPANDA_MGCKF_CONFIG_BOOT_TIMEOUT:
        case SPANDA_MGCKF_CONFIG_BOOT_RESOLUTION:
            setConfigPageModified(0, modified);
            break;

        // Hardware
        case SPANDA_MGCKF_CONFIG_CPU_INTEL_TURBO:
        case SPANDA_MGCKF_CONFIG_WIFI_INTEL_80211n:
            setConfigPageModified(1, modified);
            break;

        // Service
        case SPANDA_MGCKF_CONFIG_SERVICE_TIMEOUT:
        case SPANDA_MGCKF_CONFIG_SHUTDOWN_TIMEOUT:
            setConfigPageModified(2, modified);
            break;

        // I/O
        case SPANDA_MGCKF_CONFIG_KEYBD_COMPOSE:
            setConfigPageModified(3, modified);
            break;

        // Disk
        case SPANDA_MGCKF_CONFIG_DISK_PHYSICS:
            setConfigPageModified(4, modified);
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

void MagicKonfug::setWidgetDisabled(QWidget *widget)
{
    widget->setEnabled(false);
    widget->setToolTip(tr("Not supported on your system."));
}

void MagicKonfug::showStatusPage(bool pageVisible, QString text)
{
    if (pageVisible)
    {
        ui->groupPage->hide();
        ui->frameStatus->show();
        ui->buttonBox->setEnabled(false);
        if (text.isEmpty())
            text = tr("Processing configuration, please wait...");
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
            DialogUtils::warnMissingFile(fileName, aborted);
        break;
        case ConfigFileEditor::NoPermission:
            DialogUtils::warnPermission(fileName);
        default:;
    }
    return ret;
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

int MagicKonfug::bootResolutionStringToIndex(const QString &str)
{
    if (str.contains("640x480"))
        return 1;
    else if (str.contains("800x600"))
        return 2;
    else if (str.contains("1024x768"))
        return 3;
    else
        return 0;
}

QString MagicKonfug::bootResolutionIndexToString(int index)
{
    switch (index)
    {
        case 1:
            return "640x480";
        case 2:
            return "800x600";
        case 3:
            return "1024x768";
        default:
            return "auto";
    }
}

void MagicKonfug::onCommandFinished(bool successful)
{
    if (successful)
    {
            QMessageBox::information(this,
                                     tr("Configuration(s) applied"),
                                     tr("Finish applying configuration. "
                                     "You may need to reboot to have them "
                                     "take effect."));
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
                applied = applyConfig(SPANDA_MGCKF_CONFIG_BOOT_TIMEOUT);
                applied &= applyConfig(SPANDA_MGCKF_CONFIG_BOOT_RESOLUTION);
                break;
            case 1: // Hardware
                applied = applyConfig(SPANDA_MGCKF_CONFIG_CPU_INTEL_TURBO);
                applied &= applyConfig(SPANDA_MGCKF_CONFIG_WIFI_INTEL_80211n);
                break;
            case 2: // Service
                applied = applyConfig(SPANDA_MGCKF_CONFIG_SERVICE_TIMEOUT);
                applied &= applyConfig(SPANDA_MGCKF_CONFIG_SHUTDOWN_TIMEOUT);
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
        if (needUpdatingBoot)
        {
            if (bootConfig.updateBootMenu())
                showStatusPage(true);
            needUpdatingBoot = false;
        }
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

void MagicKonfug::on_comboDiskType_currentIndexChanged(const QString &arg1)
{
    Q_UNUSED(arg1)
    setConfigModified(SPANDA_MGCKF_CONFIG_DISK_PHYSICS);
}

void MagicKonfug::on_textTimeoutBoot_valueChanged(int arg1)
{
    Q_UNUSED(arg1)
    setConfigModified(SPANDA_MGCKF_CONFIG_BOOT_TIMEOUT);
}

void MagicKonfug::on_comboBootResolution_currentIndexChanged(int index)
{
    Q_UNUSED(index)
    setConfigModified(SPANDA_MGCKF_CONFIG_BOOT_RESOLUTION);
}

void MagicKonfug::on_checkIWiFi80211n_toggled(bool checked)
{
    Q_UNUSED(checked)
    setConfigModified(SPANDA_MGCKF_CONFIG_WIFI_INTEL_80211n);
}
