#include "kclock.h"
#include <base.h>

KClock::KClock(QObject *parent, Qt::TimerType tp) : QObject(parent)
{
    st=new QSet<int>;
    tm=new QTimer(this);
    tm->setTimerType(tp);
    tm->start(1000);
    dct(tm,&QTimer::timeout,this,&KClock::timeout);
}

void KClock::timeout(){
    st->clear();
    for(auto &i:task_state){
        if(--i.second==0){
            i.second=task_info[i.first];
            st->insert(i.first);
        }
    }
    tick(st);
}

void KClock::insert(int timeid, int sec){
    task_info[timeid]=sec;
    task_state[timeid]=sec;
}

void KClock::erase(int timeid){
    task_info.erase(timeid);
    task_state.erase(timeid);
}

void KClock::restart(int timeid){
    task_state[timeid]=task_info[timeid];
}

int KClock::getid(){
    return ++cnt;
}

KClock::~KClock(){
    delete st;
    tm->stop();
}
