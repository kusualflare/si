#ifndef CHANGEUH_H
#define CHANGEUH_H

#include <QWidget>
class KDlcore;
class KDlinfo;

namespace Ui {
class changeuh;
}

class changeuh : public QWidget
{
    Q_OBJECT

public:
    explicit changeuh(const QString &taskid,QWidget *parent = nullptr);
    ~changeuh();
    KDlinfo *info;
    KDlcore *d;
    QString taskid;
    void cpk();
private:
    Ui::changeuh *ui;
};

#endif // CHANGEUH_H
