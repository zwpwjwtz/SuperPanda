#ifndef MagicKonfug_H
#define MagicKonfug_H

#include <QMainWindow>
#include "Interfaces/configfileeditor.h"
#include "Interfaces/exelauncher.h"


class QAbstractButton;

namespace Ui {
class MagicKonfug;
}

class MagicKonfug : public QMainWindow
{
    Q_OBJECT

public:
    explicit MagicKonfug(QWidget *parent = 0);
    ~MagicKonfug();

private:
    Ui::MagicKonfug *ui;
    ConfigFileEditor configFile;
    ExeLauncher exeFile;
    static const int pageGroupCount = 5;

    void loadConfig();
    void applyConfig(int configIndex);
    void warnMissingFile(QString fileName, bool aborted = false);
    void warnPermission(QString objectName);

private slots:
    void on_listWidget_clicked(const QModelIndex &index);
    void on_buttonAbout_clicked();
    void on_buttonExit_clicked();
    void on_buttonBox_clicked(QAbstractButton *button);
    void on_groupPage_currentChanged(int arg1);
};

#endif // MagicKonfug_H
