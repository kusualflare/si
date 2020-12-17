#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
class QPushButton;
class KPanAc;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    void resizeEvent(QResizeEvent*) override;
    ~MainWindow();
    void contextMenuEvent(QContextMenuEvent *event) override;
    QPushButton *btn;
    void sndl(QString);
    int pathit;
    QMenu *menu;

    QAction *ac_openpath,*ac_setuh,*ac_wcat;
    int crr;
    void snac(KPanAc*);
    void build(KPanAc*ii);
private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
