#include "configcollection.h"
#include "configcollection_p.h"
#include "config_files.h"
#include "config_gconf.h"
#include "../Utils/dialogutils.h"
#include "../Utils/diskutils.h"


ConfigCollection::ConfigCollection()
{
    this->d_ptr = new ConfigCollectionPrivate(this);
}

void ConfigCollection::loadConfig()
{
    for (int i=1; i<ConfigCollectionPrivate::MaxConfigEntry; i++)
        resetValue(ConfigEntryKey(i));
}

bool ConfigCollection::applyConfig()
{
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
    return d->configList.value(int(key));
}

bool ConfigCollection::setValue(ConfigEntryKey key, QVariant value)
{
    Q_D(ConfigCollection);
    if (d->configList.value(key) != value)
    {
        d->configList[int(key)] = value;
        d->configModified[int(key)] = true;
    }
    return true;
}

void ConfigCollection::resetValue(ConfigEntryKey key)
{
    Q_D(ConfigCollection);

    static QVariant value;
    static QString configValue;
    static QRegExp expression;
    static ConfigFileEditor::FileErrorCode errCode;
    switch(key)
    {
        case CONFIG_SERVICE_TIMEOUT:
            value = 10;
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
            value = 30;
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
            value = false;
            errCode = d->configFile.findLine(SPANDA_CONFIG_FILE_GRUB_DEFAULT,
                                          "intel_pstate=enable",
                                          configValue);
            if (errCode == ConfigFileEditor::FileOk)
                value = !configValue.isEmpty();
            break;
        case CONFIG_KEYBD_COMPOSE:
            value = QString();
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
            value = 0;
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
            value = 10;
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
            value = QString();
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
            value = true;
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
            value = GSettingsEditor::getValue(SPANDA_CONFIG_GCONF_SCHEMA_GNOME_IFACE,
                                              SPANDA_CONFIG_GCONF_KEY_GNOME_TEXTSCALING);
            if (!value.isValid())
                value = GSettingsEditor::getValue(SPANDA_CONFIG_GCONF_SCHEMA_DDE_GNOME,
                                                  SPANDA_CONFIG_GCONF_KEY_GNOME_TEXTSCALING);
            break;
        default:;
    }

    if (value.isValid())
    {
        d->configList[int(key)] = value;
        d->configModified[int(key)] = false;
    }
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
        if (changes[i].name == "PATH")
        switch (changes[i].operation)
        {
            case EnvironmentItem::Prepend:
                changes[i].value.append(
                                    QString(":\"$%1\"").arg(changes[i].name));
                break;
            case EnvironmentItem::Append:
                changes[i].value.prepend(
                                    QString("\"$%1\":").arg(changes[i].name));
                break;
            default:;
        }
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

    connect(&bootConfig,
            SIGNAL(commandFinished(bool)),
            this,
            SLOT(onUpdatingBootFinished()));
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
