#ifndef NEWTASK_H
#define NEWTASK_H

#include <QWidget>
class KDlinfo;
class KDlcore;

namespace Ui {
class newtask;
}

class newtask : public QWidget
{
    Q_OBJECT

public:
    explicit newtask(const QString &taskid,QWidget *parent = nullptr);
    ~newtask();
    KDlinfo *info;
    KDlcore *d;
    void fillfailed(int type,int type2);
    void fillsuc();
    void fill();
    QString taskid;
    int pathit;
private:
    Ui::newtask *ui;
signals:
    void ok(QString);
};

#endif // NEWTASK_H
