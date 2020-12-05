#ifndef KCLOCK_H
#define KCLOCK_H

#include <QObject>

class QTimer;

class KClock : public QObject
{
    Q_OBJECT
public:
    explicit KClock(QObject *parent=nullptr,Qt::TimerType tp=Qt::VeryCoarseTimer);
    ~KClock();
    void insert(int timeid,int sec);
    void erase(int timeid);
    void restart(int timeid);
    int getid();
private:
    void timeout();
    QTimer *tm;
    std::map<int,int>task_info;
    std::map<int,int>task_state;
    QSet<int>*st;
    int cnt=0;

signals:
    void tick(QSet<int>*);
};

#endif // KCLOCK_H
