#include <QDesktopWidget>
#include <QMessageBox>
#include "magickonfug.h"
#include "ui_magickonfug.h"
#include "../Utils/dialogutils.h"
#include "../Widgets/environmentwidget.h"

#define SPANDA_MGCKF_CONFIG_NONE 0
#define SPANDA_MGCKF_CONFIG_SERVICE_TIMEOUT 1
#define SPANDA_MGCKF_CONFIG_SHUTDOWN_TIMEOUT 2
#define SPANDA_MGCKF_CONFIG_CPU_INTEL_TURBO 3
#define SPANDA_MGCKF_CONFIG_KEYBD_COMPOSE 4
#define SPANDA_MGCKF_CONFIG_DISK_PHYSICS 5
#define SPANDA_MGCKF_CONFIG_BOOT_TIMEOUT 6
#define SPANDA_MGCKF_CONFIG_BOOT_RESOLUTION 7
#define SPANDA_MGCKF_CONFIG_WIFI_INTEL_80211n 8
#define SPANDA_MGCKF_CONFIG_APP_ENV_SYS 9
#define SPANDA_MGCKF_CONFIG_APP_ENV_USER 10
#define SPANDA_MGCKF_CONFIG_DISP_SCALE_GNOME 11
#define SPANDA_MGCKF_CONFIG_DISP_RESOLUTION 12


MagicKonfug::MagicKonfug(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MagicKonfug)
{
    ui->setupUi(this);

    setFixedSize(width(), height());
    move((QApplication::desktop()->width() - width()) / 2,
         (QApplication::desktop()->height() - height()) / 2);

    ui->listWidget->setStyleSheet("background-color: transparent;");
    ui->groupPage->setCurrentIndex(pageGroupCount);
    envEditor = nullptr;
    currentEnvEditMode = EnvEditMode::NotEditting;

    connect(&configEditor,
            SIGNAL(configApplied(bool)),
            this,
            SLOT(onConfigEditorApplied(bool)));
    connect(&configEditor,
            SIGNAL(promptNeedReboot()),
            this,
            SLOT(onConfigEditorPromptReboot()));
}

MagicKonfug::~MagicKonfug()
{
    destroyWidget(envEditor);
    delete ui;
}

void MagicKonfug::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);

        // Some widgets lack retranslateUi() method,
        // so we just close and destroy them
        destroyWidget(envEditor);
        envEditor = nullptr;
    }
}

void MagicKonfug::closeEvent(QCloseEvent *event)
{
    Q_UNUSED(event)
}

void MagicKonfug::showEvent(QShowEvent *event)
{
    Q_UNUSED(event)
    loadConfig();
}

void MagicKonfug::loadConfig()
{
    QVariant value;
    configEditor.loadConfig();

    value = configEditor.getValue(ConfigCollection::CONFIG_SERVICE_TIMEOUT);
    ui->textTimeoutSrvStart->setValue(value.toInt());

    value = configEditor.getValue(ConfigCollection::CONFIG_SHUTDOWN_TIMEOUT);
    ui->textTimeoutShutdown->setValue(value.toInt());

    value = configEditor.getValue(ConfigCollection::CONFIG_CPU_INTEL_TURBO);
    ui->checkTurboFreq->setChecked(value.toBool());

    value = configEditor.getValue(ConfigCollection::CONFIG_KEYBD_COMPOSE);
    ui->comboKeySequence->setCurrentIndex(value.toInt());

    value = configEditor.getValue(ConfigCollection::CONFIG_KEYBD_COMPOSE);
    if (value.isValid())
        ui->comboKeySequence->setCurrentIndex(
                                composeKeyStringToIndex(value.toString()));
    else
        setWidgetDisabled(ui->comboKeySequence);

    value = configEditor.getValue(ConfigCollection::CONFIG_DISK_PHYSICS);
    ui->comboDiskType->setCurrentIndex(value.toInt());

    value = configEditor.getValue(ConfigCollection::CONFIG_BOOT_TIMEOUT);
    ui->textTimeoutBoot->setValue(value.toInt());

    value = configEditor.getValue(ConfigCollection::CONFIG_BOOT_RESOLUTION);
    ui->comboBootResolution->setCurrentIndex(
                                bootResolutionStringToIndex(value.toString()));

    value = configEditor.getValue(ConfigCollection::CONFIG_WIFI_INTEL_80211n);
    ui->checkIWiFi80211n->setChecked(value.toBool());

    value = configEditor.getValue(
                            ConfigCollection::CONFIG_DISP_SCALE_GNOME_WINDOW);
    ui->textWindowScaling->setValue(value.toInt());

    value = configEditor.getValue(
                            ConfigCollection::CONFIG_DISP_SCALE_GNOME_TEXT);
    ui->textWindowTextScaling->setValue(value.toDouble());

    value = configEditor.getValue(ConfigCollection::CONFIG_DISP_RESOLUTION);
    if (value.toSize().isValid())
    {
        ui->radioCustomizedResolution->setChecked(true);
        ui->textScreenWidth->setValue(value.toSize().width());
        ui->textScreenHeight->setValue(value.toSize().height());
    }
    else
        ui->radioDefaultResolution->setChecked(true);

    for (int i=0; i<pageGroupCount; i++)
        configPageMoidified[i] = false;
    ui->buttonBox->setEnabled(false);
}

bool MagicKonfug::applyConfig(int configIndex)
{
    // Set value only.
    // The real "apply" operation is done in on_buttonBox_clicked()
    typedef ConfigCollection::ConfigEntryKey Key;

    bool successful;
    switch (configIndex)
    {
        case SPANDA_MGCKF_CONFIG_SERVICE_TIMEOUT:
            successful = configEditor.setValue(Key::CONFIG_SERVICE_TIMEOUT,
                                               ui->textTimeoutSrvStart->value());
            break;
        case SPANDA_MGCKF_CONFIG_SHUTDOWN_TIMEOUT:
            successful = configEditor.setValue(Key::CONFIG_SHUTDOWN_TIMEOUT,
                                               ui->textTimeoutShutdown->value());
            break;
        case SPANDA_MGCKF_CONFIG_CPU_INTEL_TURBO:
            successful = configEditor.setValue(Key::CONFIG_CPU_INTEL_TURBO,
                                               ui->checkTurboFreq->isChecked());
            break;
        case SPANDA_MGCKF_CONFIG_KEYBD_COMPOSE:
            if (ui->comboKeySequence->isEnabled())
            successful = configEditor.setValue(Key::CONFIG_KEYBD_COMPOSE,
                                        composeKeyIndexToString(
                                        ui->comboKeySequence->currentIndex()));
            else
                successful = true;
            break;
        case SPANDA_MGCKF_CONFIG_DISK_PHYSICS:
            successful = configEditor.setValue(Key::CONFIG_DISK_PHYSICS,
                                               ui->comboDiskType->currentIndex());
            break;
        case SPANDA_MGCKF_CONFIG_BOOT_TIMEOUT:
            successful = configEditor.setValue(Key::CONFIG_BOOT_TIMEOUT,
                                               ui->textTimeoutBoot->value());
            break;
        case SPANDA_MGCKF_CONFIG_BOOT_RESOLUTION:
            successful = configEditor.setValue(Key::CONFIG_BOOT_RESOLUTION,
                                        bootResolutionIndexToString(
                                        ui->comboBootResolution->currentIndex()));
            break;
        case SPANDA_MGCKF_CONFIG_WIFI_INTEL_80211n:
            successful = configEditor.setValue(Key::CONFIG_WIFI_INTEL_80211n,
                                               ui->checkIWiFi80211n->isChecked());
            break;
        case SPANDA_MGCKF_CONFIG_APP_ENV_SYS:
            successful = configEditor.setEnvironment(envVarChanges, true);
            break;
        case SPANDA_MGCKF_CONFIG_APP_ENV_USER:
            successful = configEditor.setEnvironment(envVarChanges, false);
            break;
        case SPANDA_MGCKF_CONFIG_DISP_SCALE_GNOME:
            successful = configEditor.setValue(Key::CONFIG_DISP_SCALE_GNOME_WINDOW,
                                               ui->textWindowScaling->value());
            successful &= configEditor.setValue(Key::CONFIG_DISP_SCALE_GNOME_TEXT,
                                        float(ui->textWindowTextScaling->value()));
            break;
        case SPANDA_MGCKF_CONFIG_DISP_RESOLUTION:
        {
            QSize resolution;
            if (ui->radioCustomizedResolution->isChecked())
            {
                resolution.setHeight(ui->textScreenHeight->value());
                resolution.setWidth(ui->textScreenWidth->value());
            }
            successful = configEditor.setValue(Key::CONFIG_DISP_RESOLUTION,
                                               resolution);
            break;
        }
        default:;
    }
    return successful;
}

void MagicKonfug::setConfigModified(int configIndex, bool modified)
{
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

        // Application
        case SPANDA_MGCKF_CONFIG_APP_ENV_SYS:
        case SPANDA_MGCKF_CONFIG_APP_ENV_USER:
            setConfigPageModified(3, modified);
            break;

        // I/O
        case SPANDA_MGCKF_CONFIG_KEYBD_COMPOSE:
        case SPANDA_MGCKF_CONFIG_DISP_SCALE_GNOME:
        case SPANDA_MGCKF_CONFIG_DISP_RESOLUTION:
            setConfigPageModified(4, modified);
            break;

        // Disk
        case SPANDA_MGCKF_CONFIG_DISK_PHYSICS:
            setConfigPageModified(5, modified);
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

void MagicKonfug::showEnvEditor(bool systemScope)
{
    if (currentEnvEditMode != EnvEditMode::NotEditting)
        return;

    if (envEditor == nullptr)
    {
        envEditor = new EnvironmentWidget(nullptr);
        envEditor->setWindowTitle(tr("Environment Variable Editor"));
        connect(envEditor,
                SIGNAL(closing()),
                this,
                SLOT(onEnvEditorClosing()));
    }
    if (systemScope)
    {
        currentEnvEditMode = EnvEditMode::SystemScope;
        envEditor->setBaseEnvironmentText(tr("System Environment"));
    }
    else
    {
        currentEnvEditMode = EnvEditMode::UserScope;
        envEditor->setBaseEnvironmentText(tr("User Environment"));
    }
    systemScopeEnvEdit = systemScope;

    Utils::Environment env(Utils::Environment::systemEnvironment());
    envEditor->setBaseEnvironment(env);
    envEditor->show();
    envEditor->move(QCursor::pos());
}

void MagicKonfug::showStatusPage(bool pageVisible, QString text)
{
    if (pageVisible && ui->groupPage->currentIndex() < pageGroupCount)
    {
        lastPageGroupIndex = ui->groupPage->currentIndex();
        ui->groupPage->setCurrentIndex(pageGroupCount + 2);
        ui->buttonBox->setEnabled(false);
        if (text.isEmpty())
            text = tr("Processing configuration, please wait...");
        ui->labelStatus->setText(text);
    }
    else
    {
        ui->groupPage->setCurrentIndex(lastPageGroupIndex);
        ui->buttonBox->setEnabled(true);
    }
}

void MagicKonfug::destroyWidget(QWidget* widget)
{
    if (widget)
    {
        widget->close();
        delete widget;
    }
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

void MagicKonfug::onConfigEditorApplied(bool successful)
{
    showStatusPage(false);
    int pageIndex = ui->groupPage->currentIndex();
    configPageMoidified[pageIndex] = !successful;
    if (pageIndex < pageGroupCount)
        ui->buttonBox->setEnabled(!successful);
}

void MagicKonfug::onConfigEditorPromptReboot()
{
    QMessageBox::information(this,
                             tr("Reboot needed"),
                             tr("Some options need a reboot to take effect."));
}

void MagicKonfug::onEnvEditorClosing()
{
    QList<Utils::EnvironmentItem> tempEnvChanges(envEditor->userChanges());
    bool modified = tempEnvChanges != envVarChanges;

    if (modified)
    {
        envVarChanges = tempEnvChanges;
        if (currentEnvEditMode == EnvEditMode::SystemScope)
            setConfigModified(SPANDA_MGCKF_CONFIG_APP_ENV_SYS);
        else
            setConfigModified(SPANDA_MGCKF_CONFIG_APP_ENV_USER);
    }

    currentEnvEditMode = EnvEditMode::NotEditting;
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
            case 3: // Application
                if (systemScopeEnvEdit)
                    applied = applyConfig(SPANDA_MGCKF_CONFIG_APP_ENV_SYS);
                else
                    applied = applyConfig(SPANDA_MGCKF_CONFIG_APP_ENV_USER);
                break;
            case 4: // I/O
                applied = applyConfig(SPANDA_MGCKF_CONFIG_KEYBD_COMPOSE);
                applied &= applyConfig(SPANDA_MGCKF_CONFIG_DISP_SCALE_GNOME);
                applied &= applyConfig(SPANDA_MGCKF_CONFIG_DISP_RESOLUTION);
                break;
            case 5: // Disk
                applied = applyConfig(SPANDA_MGCKF_CONFIG_DISK_PHYSICS);
                break;
            default:;
        }
        if (!applied)
        {
            QMessageBox::warning(this, tr("Configuration not applied"),
                                 tr("Certain value(s) cannot not be set due to "
                                    "incorrect format or range. Please check "
                                    "then try it again."));
            return;
        }
        showStatusPage(true);
        configEditor.applyConfig();
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

void MagicKonfug::on_buttonEditSysEnv_clicked()
{
    showEnvEditor(true);
}

void MagicKonfug::on_buttonEditUserEnv_clicked()
{
    showEnvEditor(false);
}

void MagicKonfug::on_textWindowScaling_valueChanged(int arg1)
{
    Q_UNUSED(arg1)
    setConfigModified(SPANDA_MGCKF_CONFIG_DISP_SCALE_GNOME);
}

void MagicKonfug::on_textWindowTextScaling_valueChanged(double arg1)
{
    Q_UNUSED(arg1)
    setConfigModified(SPANDA_MGCKF_CONFIG_DISP_SCALE_GNOME);
}

void MagicKonfug::on_radioDefaultResolution_clicked()
{
    ui->textScreenWidth->setEnabled(false);
    ui->textScreenHeight->setEnabled(false);
    setConfigModified(SPANDA_MGCKF_CONFIG_DISP_RESOLUTION);
}

void MagicKonfug::on_radioCustomizedResolution_clicked()
{
    ui->textScreenWidth->setEnabled(true);
    ui->textScreenHeight->setEnabled(true);
    setConfigModified(SPANDA_MGCKF_CONFIG_DISP_RESOLUTION);
}

void MagicKonfug::on_textScreenWidth_valueChanged(int arg1)
{
    Q_UNUSED(arg1)
    setConfigModified(SPANDA_MGCKF_CONFIG_DISP_RESOLUTION);
}

void MagicKonfug::on_textScreenHeight_valueChanged(int arg1)
{
    Q_UNUSED(arg1)
    setConfigModified(SPANDA_MGCKF_CONFIG_DISP_RESOLUTION);
}
