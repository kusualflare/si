#include "msg.h"
QMessageBox *msgBox=nullptr;
QMessageBox *msg(const QString &info){
    if(msgBox!=nullptr){
        delete msgBox;
        msgBox=nullptr;
    }
#ifdef TEST
    qDebug()<<info;
#endif
    msgBox=new QMessageBox("提示",info,QMessageBox::Information,QMessageBox::Ok|QMessageBox::Escape,0,0);
    msgBox->setTextInteractionFlags(Qt::TextSelectableByMouse|Qt::TextSelectableByKeyboard);
    msgBox->show();
    return msgBox;
}
void closemsg(){
    if(msgBox!=nullptr){
        delete msgBox;
        msgBox=nullptr;
    }
}

