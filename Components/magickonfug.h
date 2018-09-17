#ifndef MagicKonfug_H
#define MagicKonfug_H

#include <QMainWindow>
#include "Interfaces/configfileeditor.h"
#include "Utils/bootutils.h"


class QAbstractButton;
class EnvironmentWidget;

namespace Ui {
class MagicKonfug;
}

class MagicKonfug : public QMainWindow
{
    Q_OBJECT

public:
    explicit MagicKonfug(QWidget *parent = 0);
    ~MagicKonfug();

protected:
    void changeEvent(QEvent* event);
    void closeEvent(QCloseEvent* event);
    void showEvent(QShowEvent* event);

private:
    Ui::MagicKonfug *ui;
    EnvironmentWidget* envEditor;
    ConfigFileEditor configFile;
    BootUtils bootConfig;
    static const int pageGroupCount = 5;
    static const int configEntryCount = 9;
    bool configPageMoidified[pageGroupCount];
    bool configMoidified[configEntryCount];
    bool needUpdatingBoot;

    void loadConfig();
    bool applyConfig(int configIndex);
    void setConfigModified(int configIndex, bool modified = true);
    void setConfigPageModified(int pageIndex, bool modified = true);
    void setWidgetDisabled(QWidget* widget);
    void showStatusPage(bool pageVisible, QString text = QString());
    static bool testConfigFileError(ConfigFileEditor::FileErrorCode errCode,
                                    const QString& fileName,
                                    const bool aborted = true);
    static int composeKeyStringToIndex(const QString& str);
    static QString composeKeyIndexToString(int index);
    static int bootResolutionStringToIndex(const QString& str);
    static QString bootResolutionIndexToString(int index);

private slots:
    void onCommandFinished(bool successful);
    void on_listWidget_clicked(const QModelIndex &index);
    void on_buttonAbout_clicked();
    void on_buttonExit_clicked();
    void on_buttonBox_clicked(QAbstractButton *button);
    void on_groupPage_currentChanged(int arg1);
    void on_textTimeoutSrvStart_valueChanged(int arg1);
    void on_textTimeoutShutdown_valueChanged(int arg1);
    void on_checkTurboFreq_toggled(bool checked);
    void on_comboKeySequence_currentIndexChanged(int index);
    void on_comboDiskType_currentIndexChanged(const QString &arg1);
    void on_textTimeoutBoot_valueChanged(int arg1);
    void on_comboBootResolution_currentIndexChanged(int index);
    void on_checkIWiFi80211n_toggled(bool checked);
    void on_buttonEnvEdit_clicked();
};

#endif // MagicKonfug_H
