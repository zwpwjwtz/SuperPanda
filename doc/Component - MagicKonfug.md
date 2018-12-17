# SuperPanda - MagicKunfug

The component "MagicKunfug" helps tweaking system parameters, by modifying configuration files via standard file IO functions, or binary configuration via command line interface. The idea of the design is to utilize file/command as native APIs for system configuration, contrary to that of Windows(R) system which largely depends on application binary interfaces (ABI), as ABIs in Linux world seem impossible to be conciliated due to the incompatibility among vast variation of standard libraries. As a consequence, MagicKunfug requires nothing other than a Qt shared library of minimum build version, with basic or optional command line tools for extended functions to work.

This document is devoted to elucidate the technical details of each tweaking functions built in MagicKunfug.


## Cheat sheet

### Configuration files

All file IDs are defined in [config_files.h](./Component/config_files.h).

|No.|File ID|File Type|File Scope|Default Path|
|---|---|---|---|---|
|1|SPANDA_CONFIG_FILE_GRUB_DEFAULT|Text|System|/etc/default/grub|
|2|SPANDA_CONFIG_FILE_KEYBD_DEFAULT|Text|System|/etc/default/keyboard|
|3|SPANDA_CONFIG_FILE_MOUNT_ROOT|Text|System|/etc/fstab|
|4|SPANDA_CONFIG_FILE_SYSTEMD_SYSTEM|Text|System|/etc/systemd/system.conf|
|5|SPANDA_CONFIG_FILE_SYSTEMD_USER|Text|System|/etc/systemd/user.conf|
|6|SPANDA_CONFIG_FILE_UDEV_DISK|Text|System|/etc/udev/rules.d/95-superpanda.rules|
|7|SPANDA_CONFIG_FILE_MODCONF_IWLWIFI|Text|System|/etc/modprobe.d/iwlwifi.conf|
|8|SPANDA_CONFIG_FILE_ENVIRONMENT_SYS|Text|System|/etc/environment|
|9|SPANDA_CONFIG_FILE_ENVIRONMENT_USER|Text|System|~/.xsessionrc|
|10|SPANDA_CONFIG_FILE_XSESSION_USER|Text|System|~/.xsession|
|11|SPANDA_CONFIG_FILE_AUTOSTART_SCREEN_USER|Text|System|~/.config/autostart/spanda-screen-config.desktop|
|12|SPANDA_CONFIG_FILE_SWAPFILE_ROOT|Binary|System|/swapfile|


### Configurations

All config IDs are defined in [configcollection.h](./Component/configcollection.h).

|No.|Name|Category|Config ID|Config File No.|Value Type|Default Value|
|---|---|---|---|---|---|
|1|Service timeout|Service|CONFIG_SERVICE_TIMEOUT|4,5|Int|10|
|2|System shutdown timeout|Service|CONFIG_SHUTDOWN_TIMEOUT|4|Int|30|
|3|Intel CPU Turbo|CPU|CONFIG_CPU_INTEL_TURBO|1|Bool|false|
|4|Keboard Compose Key|Keyboard|CONFIG_KEYBD_COMPOSE|2|String|""|
|5|Disk physics|Disk|CONFIG_DISK_PHYSICS|3,6|Int|0|
|6|Boot screen timeout|Boot|CONFIG_BOOT_TIMEOUT|1|Int|10|
|7|Boot screen resolution|Boot|CONFIG_BOOT_RESOLUTION|1|String|""|
|8|Intel Wifi 802.11n|Network|CONFIG_WIFI_INTEL_80211n|7|Bool|true|
|11|Gnome desktop scaling - Window|Screen|CONFIG_DISP_SCALE_GNOME_WINDOW|(Gsettings)|Int|1|
|12|Gnome desktop scaling - Text|Screen|CONFIG_DISP_SCALE_GNOME_TEXT|(Gsettings)|Double|1.0|
|13|Screen resolution|Screen|CONFIG_DISP_RESOLUTION|10,11|{Int,Int}|{0,0}|
|14|System swap file|Swap|CONFIG_DISK_SWAP|3,12|Int|0|
|15|ACPI OS reporting|ACPI|CONFIG_ACPI_OS|1|String|0|
|16|Screen gamma|Screen|CONFIG_DISP_GAMMA|10,11|String|"1,1,1"|


## Implementation

### Hardware

#### ACPI

#### CPU

#### Disk

#### Keyboard

#### Network

#### Screen


### System Management

#### Boot

#### Service

#### Swap


### Application

#### Environment variables