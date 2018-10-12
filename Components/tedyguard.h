#ifndef TEDYGUARD_H
#define TEDYGUARD_H

#include <QMainWindow>


namespace Ui {
class TedyGuard;
}

class TedyGuard : public QMainWindow
{
    Q_OBJECT

public:
    explicit TedyGuard(QWidget *parent = 0);
    ~TedyGuard();

private:
    Ui::TedyGuard *ui;

private slots:
    void on_buttonScan_clicked();
    void on_buttonStopScan_clicked();
    void on_buttonCancelClean_clicked();
    void on_buttoStopClean_clicked();
    void on_buttonReturn_clicked();
};

#endif // TEDYGUARD_H
