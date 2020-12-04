#ifndef DLCFTW_H
#define DLCFTW_H

#include <QWidget>

namespace Ui {
class dlcftw;
}

class dlcftw : public QWidget
{
    Q_OBJECT

public:
    explicit dlcftw(const QString &taskid,QWidget *parent = nullptr);
    ~dlcftw();

private:
    Ui::dlcftw *ui;
};

#endif // DLCFTW_H
