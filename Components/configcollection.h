#ifndef CONFIGCOLLECTION_H
#define CONFIGCOLLECTION_H

#include <QObject>
#include "../Utils/environment.h"


class ConfigCollectionPrivate;

class ConfigCollection : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(ConfigCollection)
protected:
    ConfigCollectionPrivate* d_ptr;

public:
    enum ConfigEntryKey
    {
        CONFIG_SERVICE_TIMEOUT = 1,
        CONFIG_SHUTDOWN_TIMEOUT = 2,
        CONFIG_CPU_INTEL_TURBO = 3,
        CONFIG_KEYBD_COMPOSE = 4,
        CONFIG_DISK_PHYSICS = 5,
        CONFIG_BOOT_TIMEOUT = 6,
        CONFIG_BOOT_RESOLUTION = 7,
        CONFIG_WIFI_INTEL_80211n = 8,
        CONFIG_DISP_SCALE_GNOME_WINDOW = 11,
        CONFIG_DISP_SCALE_GNOME_TEXT = 12,
        CONFIG_DISP_RESOLUTION = 13,
        CONFIG_DISK_SWAP = 14,
        CONFIG_ACPI_OS = 15,
        CONFIG_DISP_GAMMA = 16
    };

    explicit ConfigCollection();
    void loadConfig();
    bool applyConfig();

    QVariant getValue(ConfigEntryKey key);
    QVariant getDefaultValue(ConfigEntryKey key);
    bool setValue(ConfigEntryKey key, QVariant value);
    void resetValue(ConfigEntryKey key);
    bool isModified(ConfigEntryKey key);
    QList<ConfigEntryKey> modifiedEntryList();

    bool setEnvironment(QList<Utils::EnvironmentItem> changes,
                        bool systemScope = false);

signals:
    void configApplied(bool successful);
    void promptNeedReboot();
};

#endif // CONFIGCOLLECTION_H
