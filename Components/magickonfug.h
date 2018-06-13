#ifndef MagicKonfug_H
#define MagicKonfug_H

#include <QMainWindow>

namespace Ui {
class MagicKonfug;
}

class MagicKonfug : public QMainWindow
{
    Q_OBJECT

public:
    explicit MagicKonfug(QWidget *parent = 0);
    ~MagicKonfug();

private slots:
    void on_listWidget_clicked(const QModelIndex &index);
    void on_buttonAbout_clicked();
    void on_buttonExit_clicked();

private:
    Ui::MagicKonfug *ui;
};

#endif // MagicKonfug_H
