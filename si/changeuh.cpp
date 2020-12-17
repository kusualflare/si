#include "changeuh.h"
#include "ui_changeuh.h"
#include <msg.h>
#include <kdlcore.h>
extern map<QString,KDlcore*>all_tasks;

changeuh::changeuh(const QString &taskid,QWidget *parent) :
    QWidget(parent),
    ui(new Ui::changeuh)
{
    this->taskid=taskid;
    d=all_tasks[taskid];
    info=d->info();
    ui->setupUi(this);
    ui->plainTextEdit->setPlainText(to_QString(info->url_header()));
    dct(ui->ok,btnc,this,&changeuh::cpk);
}

void changeuh::cpk(){
    KUHG p=to_KUHG(ui->plainTextEdit->toPlainText());
    bool flag=true;
    for(auto &i:p)if(i.first.size()){
        flag=false;
        break;
    }
    if(flag){
        msg("请至少填写一条链接")->exec();
        return;
    }
    if(d->is_stopped())d->set_url_and_header(p);
    else{
        d->stop();
        d->set_url_and_header(p);
        d->start();
    }
    deleteLater();
}

changeuh::~changeuh()
{
    delete ui;
}
