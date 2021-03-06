#include <QSize>

#include "configcollection.h"
#include "configcollection_p.h"
#include "config_files.h"
#include "config_gconf.h"
#include "../Utils/dialogutils.h"
#include "../Utils/diskutils.h"
#include "../Utils/screenutils.h"
#include "../Utils/swaputils.h"


ConfigCollection::ConfigCollection()
{
    this->d_ptr = new ConfigCollectionPrivate(this);
}

void ConfigCollection::loadConfig()
{
    for (int i=1; i<=ConfigCollectionPrivate::MaxConfigEntry; i++)
        resetValue(ConfigEntryKey(i));
}

bool ConfigCollection::applyConfig()
{
    using namespace Utils;
    Q_D(ConfigCollection);

    bool successful;
    bool valueExists;
    QString configValue;
    QString oldValue;
    QString fileName;
    QString backupName;
    QRegExp expression;
    ConfigFileEditor::FileErrorCode errCode;
    QList<QString> tempStringList;

    QMap<int, QVariant>::const_iterator i;
    for (i=d->configList.constBegin(); i!=d->configList.constEnd(); i++)
    {
        if (!d->configModified[i.key()])
            continue;
        switch (i.key())
        {
            case CONFIG_SERVICE_TIMEOUT:
                fileName = SPANDA_CONFIG_FILE_SYSTEMD_SYSTEM;
                d->configFile.backupFile(fileName, backupName);
                configValue = QString::number(i.value().toInt());
                errCode = d->configFile.regexpReplaceLine(fileName,
                                            "DefaultTimeoutStartSec=",
                                            "#*DefaultTimeoutStartSec=\\w*",
                                            QString("DefaultTimeoutStartSec=%1s")
                                                    .arg(configValue));
                d->configFile.regexpReplaceLine(fileName,
                                             "DefaultTimeoutStopSec=",
                                             "#*DefaultTimeoutStopSec=\\w*",
                                             QString("DefaultTimeoutStopSec=%1s")
                                                     .arg(configValue));
                successful = d->testConfigFileError(errCode, fileName);

                fileName = SPANDA_CONFIG_FILE_SYSTEMD_USER;
                d->configFile.backupFile(fileName, backupName);
                errCode = d->configFile.regexpReplaceLine(fileName,
                                             "DefaultTimeoutStartSec=",
                                             "#*DefaultTimeoutStartSec=\\w*",
                                             QString("DefaultTimeoutStartSec=%1s")
                                                     .arg(configValue));
                d->configFile.regexpReplaceLine(fileName,
                                             "DefaultTimeoutStopSec=",
                                             "#*DefaultTimeoutStopSec=\\w*",
                                             QString("DefaultTimeoutStopSec=%1s")
                                                     .arg(configValue));
                successful &= d->testConfigFileError(errCode, fileName);
                break;
            case CONFIG_SHUTDOWN_TIMEOUT:
                fileName = SPANDA_CONFIG_FILE_SYSTEMD_SYSTEM;
                d->configFile.backupFile(fileName, backupName);
                configValue = QString::number(i.value().toInt());
                errCode = d->configFile.regexpReplaceLine(fileName,
                                             "ShutdownWatchdogSec=",
                                             "#*ShutdownWatchdogSec=\\w*",
                                             QString("ShutdownWatchdogSec=%1s")
                                                     .arg(configValue));
                successful = d->testConfigFileError(errCode, fileName);
                break;
            case CONFIG_CPU_INTEL_TURBO:
                fileName = SPANDA_CONFIG_FILE_GRUB_DEFAULT;
                d->configFile.backupFile(fileName, backupName);
                if (i.value().toBool())
                    errCode = d->configFile.regexpReplaceLine(fileName,
                                                 "GRUB_CMDLINE_LINUX_DEFAULT=",
                                                 "\"\\n",
                                                 " intel_pstate=enable\"\n");
                else
                    errCode = d->configFile.regexpReplaceLine(fileName,
                                                 "GRUB_CMDLINE_LINUX_DEFAULT=",
                                                 "\\s*intel_pstate=enable",
                                                 "");
                if (d->testConfigFileError(errCode, fileName))
                {
                    d->needUpdatingBoot = true;
                    successful = true;
                }
                break;
            case CONFIG_KEYBD_COMPOSE:
                    fileName = SPANDA_CONFIG_FILE_KEYBD_DEFAULT;
                    d->configFile.backupFile(fileName, backupName);
                    configValue = i.value().toString();
                    if (!configValue.isEmpty())
                        configValue.prepend("compose:");
                    d->configFile.exists(fileName, "XKBOPTIONS", valueExists);
                    if (valueExists)
                    {
                        errCode = d->configFile.regexpReplaceLine(fileName,
                                                "XKBOPTIONS=",
                                                "\\s*compose:\\w*",
                                                "");
                        d->configFile.regexpReplaceLine(fileName,
                                                "XKBOPTIONS=",
                                                "\"\\n",
                                                QString(" %1\"\n")
                                                       .arg(configValue));
                    }
                    else
                        errCode = d->configFile.append(fileName,
                                                    QString("\nXKBOPTIONS=\"%1\"")
                                                           .arg(configValue));
                    successful = d->testConfigFileError(errCode, fileName);
                break;
            case CONFIG_DISK_PHYSICS:
                // Get mount entry for the root partition
                d->exeFile.runCommand("mount | grep \" on / \"", true);
                tempStringList = QString(d->exeFile.getOutput())
                                        .split(' ').toVector().toList();
                expression.setPattern("^/dev/([a-zA-Z]+)\\d*");
                if (expression.indexIn(tempStringList[0]) < 0)
                    break;

                // Set scheduler for disk I/O
                fileName = SPANDA_CONFIG_FILE_UDEV_DISK;
                configValue.clear();
                if (i.value().toInt() == 1) // Using SSD
                    configValue = QString("ACTION==\"add|change\", "
                                          "KERNEL==\"%1\", "
                                          "ATTR{queue/rotational}==\"0\", "
                                          "ATTR{queue/scheduler}=\"deadline\"")
                                         .arg(expression.cap(1));
                d->configFile.exists(fileName,
                                  QString("KERNEL==\"%1\", ATTR{queue/rotational}")
                                         .arg(expression.cap(1)),
                                  valueExists);
                if (valueExists)
                    errCode = d->configFile.regexpReplaceLine(fileName,
                                            QString("KERNEL==\"%1\", "
                                                    "ATTR{queue/rotational}")
                                                  .arg(expression.cap(1)),
                                            ".*", configValue);
                else
                    errCode = d->configFile.append(fileName,
                                                configValue.prepend("\n"));
                successful = d->testConfigFileError(errCode, fileName);

                // Adjust mount parameters for root partition
                fileName = SPANDA_CONFIG_FILE_MOUNT_ROOT;
                d->configFile.backupFile(fileName, backupName);
                d->configFile.findLine(fileName, tempStringList[0], oldValue);
                if (oldValue.isEmpty() || oldValue.indexOf(tempStringList[0]) > 0)
                    d->configFile.findLine(fileName,
                                        DiskUtils::getUUIDByBlock(
                                                            tempStringList[0]),
                                        oldValue);
                if (i.value().toInt() == 1) // Using SSD
                {
                    configValue = " defaults";
                    if (!oldValue.contains(",discard"))
                        configValue.append(",discard");
                    if (!oldValue.contains(",noatime"))
                        configValue.append(",noatime");
                    QString newValue(oldValue);
                    newValue.replace(" defaults", configValue);
                    errCode = d->configFile.replace(fileName, oldValue, newValue);
                }
                else // Using HDD
                {
                    d->configFile.regexpReplaceLine(fileName, oldValue,
                                                 ",discard", "");
                }
                successful &= d->testConfigFileError(errCode, fileName);
                break;
            case CONFIG_BOOT_TIMEOUT:
                fileName = SPANDA_CONFIG_FILE_GRUB_DEFAULT;
                d->configFile.backupFile(fileName, backupName);
                configValue = QString::number(i.value().toInt());
                errCode = d->configFile.regexpReplaceLine(fileName,
                                         "GRUB_TIMEOUT=",
                                         "#*GRUB_TIMEOUT=\\d*",
                                         QString("GRUB_TIMEOUT=%1")
                                                 .arg(configValue));

                if (d->testConfigFileError(errCode, fileName))
                {
                    d->needUpdatingBoot = true;
                    successful = true;
                }
                break;
            case CONFIG_BOOT_RESOLUTION:
                fileName = SPANDA_CONFIG_FILE_GRUB_DEFAULT;
                d->configFile.backupFile(fileName, backupName);
                configValue = i.value().toString();
                errCode = d->configFile.regexpReplaceLine(fileName,
                                         "GRUB_GFXMODE=",
                                         "#*GRUB_GFXMODE=\\w*",
                                         QString("GRUB_GFXMODE=%1")
                                                 .arg(configValue));

                if (d->testConfigFileError(errCode, fileName))
                {
                    d->needUpdatingBoot = true;
                    successful = true;
                }
                break;
            case CONFIG_WIFI_INTEL_80211n:
                fileName = SPANDA_CONFIG_FILE_MODCONF_IWLWIFI;
                d->configFile.backupFile(fileName, backupName);
                if (i.value().toBool())
                    configValue = "0";
                else
                    configValue = "1";
                d->configFile.exists(fileName, "11n_disable=", valueExists);
                if (valueExists)
                    errCode = d->configFile.regexpReplaceLine(fileName,
                                         "11n_disable=",
                                         "11n_disable=\\w*",
                                         QString("11n_disable=%1")
                                                .arg(configValue));
                else
                    errCode = d->configFile.append(fileName,
                                         QString("\noptions iwlwifi 11n_disable=%1")
                                                .arg(configValue));
                successful = d->testConfigFileError(errCode, fileName);
                break;
            case CONFIG_DISP_SCALE_GNOME_WINDOW:
                GSettingsEditor::setValue(SPANDA_CONFIG_GCONF_SCHEMA_GNOME_IFACE,
                                      SPANDA_CONFIG_GCONF_KEY_GNOME_SCALING,
                                      i.value());
                GSettingsEditor::setValue(SPANDA_CONFIG_GCONF_SCHEMA_DDE_GNOME,
                                      SPANDA_CONFIG_GCONF_KEY_GNOME_SCALING,
                                      i.value());
                successful = true;
                break;
            case CONFIG_DISP_SCALE_GNOME_TEXT:
                GSettingsEditor::setValue(SPANDA_CONFIG_GCONF_SCHEMA_GNOME_IFACE,
                                      SPANDA_CONFIG_GCONF_KEY_GNOME_TEXTSCALING,
                                      i.value());
                GSettingsEditor::setValue(SPANDA_CONFIG_GCONF_SCHEMA_DDE_GNOME,
                                      SPANDA_CONFIG_GCONF_KEY_GNOME_TEXTSCALING,
                                      i.value());
                successful = true;
                break;
            case CONFIG_DISP_RESOLUTION:
                fileName = SPANDA_CONFIG_FILE_AUTOSTART_SCREEN_USER;
                if (i.value().toSize().isValid())
                {
                    if (!d->configFile.fileExists(fileName))
                    {
                        d->configFile.append(fileName,
                                             "[Desktop Entry]\n"
                                             "Exec=bash -c \". ~/.xsession\"\n"
                                             "Name=SuperPanda-ScreenConfig\n"
                                             "Type=Application");
                    }

                    fileName = SPANDA_CONFIG_FILE_XSESSION_USER;
                    configValue = ScreenUtils::getModeLine(i.value().toSize());
                    configValue = QString("xrandr --newmode %1\n"
                                          "xrandr --addmode %2 %3\n"
                                          "xrandr --output %2 --mode %3")
                                  .arg(configValue)
                                  .arg(ScreenUtils::currentMonitor())
                                  .arg(ScreenUtils::getModeName(configValue));
                    if (!d->configFile.fileExists(fileName))
                    {
                        errCode = d->configFile.append(fileName, configValue);
                    }
                    else
                    {
                        tempStringList = configValue.split('\n');
                        errCode = d->configFile.replaceLine(fileName,
                                                "xrandr --newmode ",
                                                tempStringList[0].append('\n'));
                        errCode = d->configFile.replaceLine(fileName,
                                                "xrandr --addmode ",
                                                tempStringList[1].append('\n'));
                        errCode = d->configFile.replaceLine(fileName,
                                                "xrandr --output ",
                                                tempStringList[2].append('\n'));
                    }
                    successful = d->testConfigFileError(errCode, fileName);
                }
                else
                {
                    if (!d->configList[CONFIG_DISP_GAMMA].isValid())
                        d->configFile.deleteFile(fileName);

                    fileName = SPANDA_CONFIG_FILE_XSESSION_USER;
                    errCode = d->configFile.replaceLine(fileName,
                                                        "xrandr --output ",
                                                        "");
                    successful = d->testConfigFileError(errCode, fileName);
                }
                d->needResetScreen |= successful;
                break;
            case CONFIG_DISK_SWAP:
                if (i.value().toInt() > 0)
                {
                    // See if the free space is enough
                    FileName swapFileName = FileName::fromUtf8(
                                            SPANDA_CONFIG_FILE_SWAPFILE_ROOT);
                    qint64 available = DiskUtils::getFreeSpace(
                                        swapFileName.parentDir().toString()) +
                            ConfigFileEditor::fileSize(swapFileName.toString());
                    qint64 required = qint64(i.value().toInt()) * 1024 * 1024;
                    if (available < required)
                    {
                        DialogUtils::warnInsufficientSpace(
                                            swapFileName.parentDir().toString(),
                                            required);
                        break;
                    }
                }

                // Add an entry in /etc/fstab for the swap file
                fileName = SPANDA_CONFIG_FILE_MOUNT_ROOT;
                d->configFile.backupFile(fileName, backupName);
                if (i.value().toInt() > 0)
                    configValue = QString("%1 none swap defaults 0 0")
                                         .arg(SPANDA_CONFIG_FILE_SWAPFILE_ROOT);
                else
                    configValue.clear();
                errCode = d->configFile.replaceLine(fileName,
                                    QString("%1 none swap")
                                        .arg(SPANDA_CONFIG_FILE_SWAPFILE_ROOT),
                                    configValue);
                successful = d->testConfigFileError(errCode, fileName);
                d->needResetSwapFile = successful;
                break;
            case  CONFIG_ACPI_OS:
                fileName = SPANDA_CONFIG_FILE_GRUB_DEFAULT;
                d->configFile.backupFile(fileName, backupName);
                switch (i.value().toInt())
                {
                    case 1: // Disabled
                        configValue = " acpi_osi=!";
                        break;
                    case 2: // Linux
                        configValue = " acpi_osi=Linux";
                        break;
                    case 3: // Windows
                        configValue = " acpi_osi=Windows";
                        break;
                    case 0:
                    default:
                        configValue.clear();

                }
                d->configFile.exists(fileName, "acpi_osi=", valueExists);
                if (valueExists)
                    errCode = d->configFile.regexpReplaceLine(fileName,
                                                "GRUB_CMDLINE_LINUX_DEFAULT=",
                                                "\\s*acpi_osi=!?\\w*",
                                                configValue);
                else
                    errCode = d->configFile.regexpReplaceLine(fileName,
                                                "GRUB_CMDLINE_LINUX_DEFAULT=",
                                                "\"\\n",
                                                configValue.append("\"\n"));
                successful = d->testConfigFileError(errCode, fileName);
                d->needUpdatingBoot = successful;
                break;
            case CONFIG_DISP_GAMMA:
                fileName = SPANDA_CONFIG_FILE_AUTOSTART_SCREEN_USER;
                tempStringList = i.value().toString().split(',');
                if (tempStringList.count() >= 3)
                {
                    if (!d->configFile.fileExists(fileName))
                    {
                        d->configFile.append(fileName,
                                             "[Desktop Entry]\n"
                                             "Exec=bash -c \". ~/.xsession\"\n"
                                             "Name=SuperPanda-ScreenConfig\n"
                                             "Type=Application");
                    }

                    fileName = SPANDA_CONFIG_FILE_XSESSION_USER;
                    configValue = QString("xgamma "
                                          "-rgamma %1 -ggamma %2 -bgamma %3\n")
                                         .arg(tempStringList[0])
                                         .arg(tempStringList[1])
                                         .arg(tempStringList[2]);
                    if (!d->configFile.fileExists(fileName))
                    {
                        errCode = d->configFile.append(fileName, configValue);
                    }
                    else
                    {
                        errCode = d->configFile.replaceLine(fileName,
                                                            "xgamma -rgamma ",
                                                            configValue);
                    }
                    successful = d->testConfigFileError(errCode, fileName);
                }
                else
                {
                    if (!d->configList[CONFIG_DISP_RESOLUTION].isValid())
                        d->configFile.deleteFile(fileName);

                    fileName = SPANDA_CONFIG_FILE_XSESSION_USER;
                    errCode = d->configFile.replaceLine(fileName,
                                                        "xgamma -rgamma ",
                                                        "");
                    successful = d->testConfigFileError(errCode, fileName);
                }
                d->needResetScreen |= successful;
                break;
            default:;
        }
        if (successful)
            d->configModified[i.key()] = false;
    }
    d->doUpdating();
    return successful;
}

QVariant ConfigCollection::getValue(ConfigEntryKey key)
{
    Q_D(ConfigCollection);
    if (d->configList.value(int(key)).isValid())
        return d->configList.value(int(key));
    else
        return getDefaultValue(key);
}

QVariant ConfigCollection::getDefaultValue(ConfigEntryKey key)
{
    switch(key)
    {
        case CONFIG_SERVICE_TIMEOUT:
            return 10;
        case CONFIG_SHUTDOWN_TIMEOUT:
            return 30;
        case CONFIG_CPU_INTEL_TURBO:
            return false;
        case CONFIG_KEYBD_COMPOSE:
            return QString();
        case CONFIG_DISK_PHYSICS:
            return 0;
        case CONFIG_BOOT_TIMEOUT:
            return 10;
        case CONFIG_BOOT_RESOLUTION:
            return QString();
        case CONFIG_WIFI_INTEL_80211n:
            return true;
        case CONFIG_DISP_SCALE_GNOME_WINDOW:
            return 1;
        case CONFIG_DISP_SCALE_GNOME_TEXT:
            return 1.0;
        case CONFIG_DISP_RESOLUTION:
            return QSize();
        case CONFIG_DISK_SWAP:
            return 0;
        case CONFIG_ACPI_OS:
            return 0;
        case CONFIG_DISP_GAMMA:
            return QString("1,1,1");
        default:
            return QVariant();
    }
}

bool ConfigCollection::setValue(ConfigEntryKey key, QVariant value)
{
    Q_D(ConfigCollection);
    if (!d->configList.contains(key))
        resetValue(key);
    if (getValue(key) != value)
    {
        if (getDefaultValue(key) == value)
            d->configList[int(key)] = QVariant();
        else
            d->configList[int(key)] = value;
        d->configModified[int(key)] = true;
    }
    return true;
}

void ConfigCollection::resetValue(ConfigEntryKey key)
{
    Q_D(ConfigCollection);

    QVariant value;
    QString configValue;
    QRegExp expression;
    ConfigFileEditor::FileErrorCode errCode;
    switch(key)
    {
        case CONFIG_SERVICE_TIMEOUT:
            errCode = d->configFile.findLine(SPANDA_CONFIG_FILE_SYSTEMD_SYSTEM,
                                          "DefaultTimeoutStartSec=",
                                          configValue);
            if (errCode == ConfigFileEditor::FileOk)
            {
                expression.setPattern("^DefaultTimeoutStartSec=(\\d+)");
                if (expression.indexIn(configValue) >= 0)
                    value = expression.cap(1).toInt();
            }
            break;
        case CONFIG_SHUTDOWN_TIMEOUT:
            errCode = d->configFile.findLine(SPANDA_CONFIG_FILE_SYSTEMD_SYSTEM,
                                          "ShutdownWatchdogSec=",
                                          configValue);
            if (errCode == ConfigFileEditor::FileOk)
            {
                expression.setPattern("^ShutdownWatchdogSec=(\\d+)");
                if (expression.indexIn(configValue) >= 0)
                    value = expression.cap(1).toInt();
            }
            break;
        case CONFIG_CPU_INTEL_TURBO:
            errCode = d->configFile.findLine(SPANDA_CONFIG_FILE_GRUB_DEFAULT,
                                          "intel_pstate=enable",
                                          configValue);
            if (errCode == ConfigFileEditor::FileOk)
                value = !configValue.isEmpty();
            break;
        case CONFIG_KEYBD_COMPOSE:
            if (!d->configFile.fileExists(SPANDA_CONFIG_FILE_KEYBD_DEFAULT))
            {
                value = "unavailable";
                break;
            }
            errCode = d->configFile.findLine(SPANDA_CONFIG_FILE_KEYBD_DEFAULT,
                                          "XKBOPTIONS=",
                                          configValue);
            if (errCode == ConfigFileEditor::FileOk)
            {
                expression.setPattern("compose:(\\w+)");
                if (expression.indexIn(configValue) >= 0)
                    value = expression.cap(1);
            }
            break;
        case CONFIG_DISK_PHYSICS:
            errCode = d->configFile.findLine(SPANDA_CONFIG_FILE_UDEV_DISK,
                                          "ATTR{queue/rotational}==\"0\"",
                                          configValue);
            if (errCode == ConfigFileEditor::FileOk)
            {
                if (configValue.contains("deadline"))
                    value = 1;
            }
            break;
        case CONFIG_BOOT_TIMEOUT:
            errCode = d->configFile.findLine(SPANDA_CONFIG_FILE_GRUB_DEFAULT,
                                          "GRUB_TIMEOUT=",
                                          configValue);
            if (errCode == ConfigFileEditor::FileOk)
            {
                if (configValue.at(12) == '=')
                    value = configValue.mid(13).toInt();
            }
            break;
        case CONFIG_BOOT_RESOLUTION:
            errCode = d->configFile.findLine(SPANDA_CONFIG_FILE_GRUB_DEFAULT,
                                          "GRUB_GFXMODE=",
                                          configValue);
            if (errCode == ConfigFileEditor::FileOk)
            {
                if (configValue.at(12) == '=')
                    value = configValue.mid(13);
            }
            break;
        case CONFIG_WIFI_INTEL_80211n:
            errCode = d->configFile.findLine(SPANDA_CONFIG_FILE_MODCONF_IWLWIFI,
                                          "11n_disable=",
                                          configValue);
            if (errCode == ConfigFileEditor::FileOk)
            {
                if (configValue.at(11) == '=')
                    value = (configValue.toInt() == 0);
            }
            break;
        case CONFIG_DISP_SCALE_GNOME_WINDOW:
            value = GSettingsEditor::getValue(SPANDA_CONFIG_GCONF_SCHEMA_DDE_GNOME,
                                              SPANDA_CONFIG_GCONF_KEY_GNOME_SCALING);
            if (!value.isValid())
                value = GSettingsEditor::getValue(SPANDA_CONFIG_GCONF_SCHEMA_GNOME_IFACE,
                                                  SPANDA_CONFIG_GCONF_KEY_GNOME_SCALING);
            break;
        case CONFIG_DISP_SCALE_GNOME_TEXT:
            value = GSettingsEditor::getValue(SPANDA_CONFIG_GCONF_SCHEMA_DDE_GNOME,
                                              SPANDA_CONFIG_GCONF_KEY_GNOME_TEXTSCALING);
            if (!value.isValid())
                value = GSettingsEditor::getValue(SPANDA_CONFIG_GCONF_SCHEMA_GNOME_IFACE,
                                                  SPANDA_CONFIG_GCONF_KEY_GNOME_TEXTSCALING);
            break;
        case CONFIG_DISP_RESOLUTION:
            if (d->configFile.fileExists(SPANDA_CONFIG_FILE_AUTOSTART_SCREEN_USER))
            {
                d->configFile.findLine(SPANDA_CONFIG_FILE_XSESSION_USER,
                                       "xrandr --output",
                                       configValue);
                if (configValue.isEmpty())
                    break;

                // Assuming user do not change resolution manually, so that
                // current resolution is exactly the one defined in config file
                value = ScreenUtils::currentResolution();
            }
            break;
        case CONFIG_DISK_SWAP:
            value = d->configFile.fileSize(SPANDA_CONFIG_FILE_SWAPFILE_ROOT)
                                 / 1024 / 1024;
            break;
        case CONFIG_ACPI_OS:
            errCode = d->configFile.findLine(SPANDA_CONFIG_FILE_GRUB_DEFAULT,
                                             "acpi_osi=",
                                             configValue);
            if (errCode == ConfigFileEditor::FileOk)
            {
                if (configValue.contains("=Linux"))
                    value = 2;
                else if (configValue.contains("=Windows"))
                    value = 3;
                else if (configValue.contains("acpi_osi=!"))
                    value = 1;
            }
            break;
        case CONFIG_DISP_GAMMA:
            if (d->configFile.fileExists(SPANDA_CONFIG_FILE_AUTOSTART_SCREEN_USER))
            {
                // Assuming user do not change gamma manually, so that
                // current resolution is exactly the one defined in config file
                QString gammaString;
                QList<double> gammaComponent = ScreenUtils::currentGamma();
                for (int i=0; i<gammaComponent.count(); i++)
                {
                    gammaString.append(',')
                               .append(QString::number(gammaComponent[i]));
                }
                value = gammaString.mid(1);
            }
            break;
        default:
            return;
    }

    d->configList[int(key)] = value;
    d->configModified[int(key)] = false;
}

bool ConfigCollection::isModified(ConfigEntryKey key)
{
    Q_D(ConfigCollection);
    return d->configModified.value(int(key));
}

QList<ConfigCollection::ConfigEntryKey> ConfigCollection::modifiedEntryList()
{
    Q_D(ConfigCollection);

    QList<ConfigEntryKey> list;
    QMap<int, bool>::const_iterator i;
    for (i=d->configModified.begin(); i!=d->configModified.end(); i++)
    {
        if (i.value())
            list.push_back(ConfigEntryKey(i.key()));
    }
    return list;
}

bool ConfigCollection::setEnvironment(QList<Utils::EnvironmentItem> changes,
                                      bool systemScope)
{
    using namespace Utils;
    Q_D(ConfigCollection);

    bool successful = true;
    FileName filePath;
    ConfigFileEditor::FileErrorCode errCode;

    if (systemScope)
        filePath = FileName::fromUserInput(SPANDA_CONFIG_FILE_ENVIRONMENT_SYS);
    else
        filePath = FileName::fromUserInput(SPANDA_CONFIG_FILE_ENVIRONMENT_USER);

    Environment tempEnv(Environment::systemEnvironment());
    for (int i=0; i<changes.count(); i++)
        changes[i].apply(&tempEnv);
    changes = Environment::systemEnvironment().diff(tempEnv, true);

    for (int i=0; i<changes.count(); i++)
    {
        // Deal with prepending and appending for certain variables
        switch (changes[i].operation)
        {
            case EnvironmentItem::Prepend:
                if (changes[i].name == "PATH")
                    changes[i].value.append(
                                    QString(":\"$%1\"").arg(changes[i].name));
                else
                    changes[i].value.append(
                                    d->getEnvironmentVariable(changes[i].name));
                break;
            case EnvironmentItem::Append:
                if (changes[i].name == "PATH")
                    changes[i].value.prepend(
                                    QString("\"$%1\":").arg(changes[i].name));
                else
                    changes[i].value.prepend(
                                    d->getEnvironmentVariable(changes[i].name));
                break;
            default:;
        }
        if (!systemScope)
            changes[i].name.prepend("export ");
        errCode = ConfigFileEditor::regexpWriteLine(filePath.toString(),
                                        QString("%1=").arg(changes[i].name),
                                        QString("\\s*%1=[^\n]*")
                                               .arg(changes[i].name),
                                        QString("%1=%2")
                                               .arg(changes[i].name)
                                               .arg(changes[i].value));
        successful = d->testConfigFileError(errCode, filePath.toString());
        if (!successful)
            break;
    }

    return successful;
}

ConfigCollectionPrivate::ConfigCollectionPrivate(ConfigCollection *parent)
{
    this->q_ptr = parent;
    needUpdatingBoot = false;
    needResetScreen = false;
    needResetSwapFile = false;

    connect(&bootConfig,
            SIGNAL(commandFinished(bool)),
            this,
            SLOT(onUpdatingBootFinished()));
    connect(&swapConfig,
            SIGNAL(finishedMakingSwapfile()),
            this,
            SLOT(onFinishedMakingSwapfile()));
    connect(&swapConfig,
            SIGNAL(finishedRemovingSwapfile()),
            this,
            SLOT(onFinishedRemovingSwapfile()));
    connect(&swapConfig,
            SIGNAL(finishedTurnOnSwap()),
            this,
            SLOT(onFinishedTurnOnSwap()));
}

void ConfigCollectionPrivate::doUpdating()
{
    Q_Q(ConfigCollection);

    bool finished = true;
    if (needUpdatingBoot)
    {
        needUpdatingBoot = false;
        if (bootConfig.updateBootMenu())
            finished = false;
    }
    if (needResetScreen)
    {
        needResetScreen = false;
        QSize newResolution =
                q->getValue(ConfigCollection::CONFIG_DISP_RESOLUTION).toSize();
        ScreenUtils::setMode(ScreenUtils::currentMonitor(), newResolution);
        QList<QString> newGamma = q->getValue(ConfigCollection::CONFIG_DISP_GAMMA)
                                    .toString().split(',');
        ScreenUtils::setGamma(ScreenUtils::currentMonitor(),
                              newGamma[0].toDouble(),
                              newGamma[1].toDouble(),
                              newGamma[2].toDouble());
    }
    if (needResetSwapFile)
    {
        needResetSwapFile = false;
        bool successful;
        QString swapFile(SPANDA_CONFIG_FILE_SWAPFILE_ROOT);
        qint64 swapSize = q->getValue(ConfigCollection::CONFIG_DISK_SWAP).toInt();
        if (swapSize > 0)
        {
            swapSize *= 1024 * 1024; // Convert from MiB to Byte
            successful = swapConfig.makeSwapFile(swapFile, swapSize);
        }
        else
            successful = swapConfig.removeSwapFile(swapFile);
        if (!successful)
        {
            // Assuming permission denied when modifying the swap file
            testConfigFileError(ConfigFileEditor::NoPermission, swapFile);
        }
        else
            finished = false;
    }

    if (finished)
    {
        QMap<int, bool>::const_iterator i;
        for (i=configModified.constBegin(); i!=configModified.end(); i++)
        {
            if (i.value())
            {
                emit q->configApplied(false);
                return;
            }
        }
        emit q->configApplied(true);
    }
}

QString ConfigCollectionPrivate::getEnvironmentVariable(QString key)
{
    using namespace Utils;

    static Environment::const_iterator i;
    Environment currentEnv(Environment::systemEnvironment());
    for (i=currentEnv.constBegin(); i!=currentEnv.constEnd(); i++)
    {
        if (i.key() == key)
            return i.value();
    }
    return "";
}

bool ConfigCollectionPrivate::testConfigFileError(
                                        ConfigFileEditor::FileErrorCode errCode,
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

void ConfigCollectionPrivate::onUpdatingBootFinished()
{
    Q_Q(ConfigCollection);
    emit q->promptNeedReboot();
    doUpdating();
}

void ConfigCollectionPrivate::onFinishedMakingSwapfile()
{
    swapConfig.turnOnSwap(SPANDA_CONFIG_FILE_SWAPFILE_ROOT);
}

void ConfigCollectionPrivate::onFinishedRemovingSwapfile()
{
    doUpdating();
}

void ConfigCollectionPrivate::onFinishedTurnOnSwap()
{
    doUpdating();
}
