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
|9|SPANDA_CONFIG_FILE_ENVIRONMENT_USER|Text|User|~/.xsessionrc|
|10|SPANDA_CONFIG_FILE_XSESSION_USER|Text|User|~/.xsession|
|11|SPANDA_CONFIG_FILE_AUTOSTART_SCREEN_USER|Text|User|~/.config/autostart/spanda-screen-config.desktop|
|12|SPANDA_CONFIG_FILE_SWAPFILE_ROOT|Binary|System|/swapfile|


### Configurations

All config IDs are defined in [configcollection.h](./Component/configcollection.h).

|No.|Name|Category|Config ID|Config File No.|Value Type|Default Value|
|---|---|---|---|---|---|---|
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

##### ACPI OS reporting

Report OS type and version to BIOS during boot. The OS type string is passed as kernel boot parameter with following possible values:

|Value|Kernel boot parameter|Description|
|---|---|---|
|0|""|Use default setting|
|1|"acpi_osi=!"|Disable reporting|
|2|"acpi_osi=Linux"|Linux|
|3|"acpi_osi=Windows"|Windows(R)|

More details can be found on [ArchWiki](https://wiki.archlinux.org/index.php/Talk:ASUS_E403SA).


#### CPU

##### Intel CPU Turbo

Enabled/disable dynamic frequency/power management ("Turbo") on Intel(R) CPUs. The setting is passed as kernel boot parameter with following possible values:

|Value|Kernel boot parameter|Description|
|---|---|---|
|False|""|Use default setting|
|True|"intel_pstate=enable"|Enable CPU Turbo|


#### Disk

##### Disk physics

Enable/disable optimization for solid state drive (SSD). A series of parameters concerning disk IO is set, either in UDEV config file or in system mount config file (fstab), with following possible values:

Value=0: Using traditional hard disk drive (HDD), with config line

```
ACTION=="add|change", KERNEL==\"%1\", ATTR{queue/rotational}
```

and fstab config parameters without "discard, noatime" for root partition.


Value=1: Using SSD for root partition, with UDEV config line

```
"ACTION=="add|change", "KERNEL=="%1", "ATTR{queue/rotational}=="0", "ATTR{queue/scheduler}="deadline"
```
and fstab config parameters with "discard,noatime" for root partition.


#### Keyboard

##### Keboard Compose Key

Set compose key for inputing non-ASCII characters, i.e. "€", "é" or "ĺ". The parameter is set in system config file of keyboard, with following possible values:

|Value|Description|
|---|---|
|""|Use default setting|
|"XKBOPTIONS=ralt"|Use right Alt key|
|"XKBOPTIONS=lwin"|Use left Win/Super key|
|"XKBOPTIONS=rwin"|Use right Win/Super key|
|"XKBOPTIONS=lctrl"|Use left Ctrl key|
|"XKBOPTIONS=rctrl"|Use right Ctrl key|
|"XKBOPTIONS=caps"|Use Caps Lock key|
|"XKBOPTIONS=menu"|Use Menu key|


#### Network

##### Intel Wifi 802.11n

Enable/disable 802.11n support (if any) on Intel(R) wireless wetwork adapters. This may help improve the stability of wireless connection when connecting to 802.11b/g network. The parameter is set in config file of IWLWIFI module, with following possible values:

|Value|Module config parameter|Description|
|---|---|---|
|True|"11n_disable=0"|Enable 802.11n support|
|False|"11n_disable=1"|Disable 802.11n support|


#### Screen

##### Gnome desktop scaling

Set desktop scaling factors for desktop environment (which is generally required by HiDPI display) via GSettings CUI interface. The config entries concerning screen scalings are:

|Schema|Key|Type|Default value|
|---|---|---|---|
|org.gnome.desktop.interface|scaling-factor|Int|1|
|org.gnome.desktop.interface|text-scaling-factor|Double|1.0|
|com.deepin.wrap.gnome.desktop.interface|scaling-factor|1|
|com.deepin.wrap.gnome.desktop.interface|text-scaling-factor|1.0|

Note: the schema "com.deepin.wrap.gnome.desktop.interface" is a wrapper for standard gnome desktop settings. 

##### Screen resolution

Adjust screen resolution via xrandr command. The commands for adjusting are stored in an auto-starting bash script which is launched after user's login to X desktop environment. The following commands are used:

```
xrandr --newmode MODE_LINE
xrandr --addmode MONITOR MODE_NAME
xrandr --output MONITOR --mode MODE_NAME
```

where "MODE_LINE", "MONITOR" and "MODE_NAME" are complete config string of display mode, monitor ID and name of the display mode, respectively, detected via "cvt" command and "xrandr" command.

##### Screen gamma

Adjust screen gamma via xrandr command. The commands for adjusting are stored in an auto-starting bash script which is launched after user's login to X desktop environment. The following commands are used:

```
xgamma -rgamma RED -ggamma GREEN -bgamma BLUE
```

where "RED", "GREEN" and "BLUE" are float values corresponding to red, green and blue component of gamma. The default values are all "1.0".


### System Management

#### Boot

##### Boot screen timeout

Set timeout for GRUB OS selection screen. Following line in GRUB config file is modified:

```
GRUB_TIMEOUT=SECOND
```

where "SECOND" is the timeout value in second. Any leading hastags in this line will be removed.

##### Boot screen resolution

Set resolution for GRUB OS selection screen. Following line in GRUB config file is modified:

```
GRUB_GFXMODE=WIDTHxHEIGHT
```

where "WIDTH" and "HEIGHT" are width and height of the boot screen, respectively. Any leading hastags in this line will be removed.

Currently, only "640x480", "800x600" and "1024x768" are available as they are the most commonly supported modes.


#### Service

##### Service timeout

Set timeout for system service. The following line in systemd config file (system-scope and user-scope config) is modified:

```
DefaultTimeoutStartSec=SECOND
```

where "SECOND" is the timeout value in second. Any leading hastags in this line will be removed.

##### System shutdown timeout

Set timeout for system shutdown. This is specially useful when the shutdown process is stucked by hardware failure or driver error. The following line in systemd config file (system-scope and user-scope config) is modified:

```
ShutdownWatchdogSec=SECOND
```

where "SECOND" is the timeout value in second. Any leading hastags in this line will be removed.


#### Swap

##### System swap file

Create/destroy swap file for system, and enable/disable them. The swap file is created in root partition, with size (in MiB) specified by user. Then a mount entry is added to system mount config file (fstab) in order to enable it during boot. The following line in fstab is modified:

```
/swapfile none swap defaults 0 0
```

"dd", "mkswap", "swapon" and "swapoff" command are needed to manipulate the swap file.


### Application

#### Environment variables

The environment variables (or simply "environments") are pre-defined key-value pairs that tells applications specified information during their execution. Environment variables are defined either as system-scope or user-scope configuration, stored separately in config files, each with following format:

```
KEY=VALUE
```

where "KEY" and "VALUE" are variable name and variable value in string,respectively.