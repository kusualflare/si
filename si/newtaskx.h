#ifndef NEWTASKX_H
#define NEWTASKX_H

#include <QWidget>
class KDlinfo;
class KDlcore;

namespace Ui {
class newtaskx;
}

class newtaskx : public QWidget
{
    Q_OBJECT

public:
    explicit newtaskx(const QString &taskid,QWidget *parent = nullptr);
    ~newtaskx();
    KDlinfo *info;
    KDlcore *d;
    void fillfailed(int type,int type2);
    void fillsuc();
    void fill();
    QString taskid;
    int pathit;
private:
    Ui::newtaskx *ui;
signals:
    void ok(QString);
};

#endif // NEWTASKX_H
