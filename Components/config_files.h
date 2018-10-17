#ifndef CONFIG_FILES_H
#define CONFIG_FILES_H

#define SPANDA_CONFIG_FILE_GRUB_DEFAULT "/etc/default/grub"
#define SPANDA_CONFIG_FILE_KEYBD_DEFAULT "/etc/default/keyboard"
#define SPANDA_CONFIG_FILE_MOUNT_ROOT "/etc/fstab"
#define SPANDA_CONFIG_FILE_SYSTEMD_SYSTEM "/etc/systemd/system.conf"
#define SPANDA_CONFIG_FILE_SYSTEMD_USER "/etc/systemd/user.conf"
#define SPANDA_CONFIG_FILE_UDEV_DISK "/etc/udev/rules.d/95-superpanda.rules"
#define SPANDA_CONFIG_FILE_MODCONF_IWLWIFI "/etc/modprobe.d/iwlwifi.conf"
#define SPANDA_CONFIG_FILE_ENVIRONMENT_SYS "/etc/environment"
#define SPANDA_CONFIG_FILE_ENVIRONMENT_USER "~/.xsessionrc"
#define SPANDA_CONFIG_FILE_XSESSION_USER "~/.xsession"
#define SPANDA_CONFIG_FILE_AUTOSTART_SCREEN_USER "~/.config/autostart/" \
                                            "spanda-screen-config.desktop"

#define SPANDA_CONFIG_FILE_SWAPFILE_ROOT "/swapfile"

#endif // CONFIG_FILES_H
