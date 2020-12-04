#include "newtask.h"
#include "ui_newtask.h"
#include <kdlcore.h>
#include <msg.h>
map<QString,pair<QString,QString>>dl_st;
extern QString allheader;
extern QString cupt;
map<QString,int>stv;
extern map<QString,KDlcore*>all_tasks;
map<pair<int,int>,QString>failtip={{{0,0},"在程序尝试自动填充时，主链接（第一条链接）响应超时"},{{0,1},"传输持续响应超时"},{{1,0},"下载目录无法创建"},{{1,1},"日志目录无法创建"},{{1,2},"无法写入日志"},{{1,3},"合并时出错"},{{2,0},"在程序尝试自动填充终止字节数时，通过主链接（第一条链接）得到的响应头中没有Content-Length字段"},{{2,1},"在程序尝试自动填充时，通过主链接（第一条链接）得到的响应头中的状态码不正确"}};
newtask::newtask(const QString &taskid, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::newtask)
{
    this->taskid=taskid;
    d=new KDlcore(info=new KDlinfo);

    ui->setupUi(this);
    setWindowTitle("新任务");
    ui->toolBox->setCurrentIndex(0);
    ui->configtable->setColumnCount(2);
    ui->configtable->setHorizontalHeaderLabels(QStringList()<<"项"<<"值");
    ui->configtable->setColumnWidth(0,200);
    ui->configtable->horizontalHeader()->setSectionResizeMode(1,QHeaderView::Stretch);
    ui->configtable->setRowCount(dl_st.size()+3);
    ui->configtable->setItem(0,0,new QTableWidgetItem("文件名(为空则自动获取)"));
    ui->configtable->item(0,0)->setFlags(ui->configtable->item(0,0)->flags()&~Qt::ItemIsEditable);
    ui->configtable->setItem(0,1,new QTableWidgetItem(""));
    ui->configtable->setItem(1,0,new QTableWidgetItem("起始字节"));
    ui->configtable->item(1,0)->setFlags(ui->configtable->item(1,0)->flags()&~Qt::ItemIsEditable);
    ui->configtable->setItem(1,1,new QTableWidgetItem("0"));
    ui->configtable->setItem(2,0,new QTableWidgetItem("终止字节(为-1则自动获取)"));
    ui->configtable->item(2,0)->setFlags(ui->configtable->item(2,0)->flags()&~Qt::ItemIsEditable);
    ui->configtable->setItem(2,1,new QTableWidgetItem("-1"));
    int j=3;
    for(auto &i:dl_st){
        ui->configtable->setItem(j,0,new QTableWidgetItem(i.second.second));
        ui->configtable->item(j,0)->setFlags(ui->configtable->item(j,0)->flags()&~Qt::ItemIsEditable);
        ui->configtable->setItem(j,1,new QTableWidgetItem(i.second.first));
        if(i.first=="path"){
            ui->configtable->item(j,1)->setFlags(ui->configtable->item(j,1)->flags()&~Qt::ItemIsEditable);
            pathit=j;
        }
        ++j;
    }
    dct(ui->configtable,&QTableWidget::cellDoubleClicked,[this](int row,int col){
        if(row==pathit&&col==1){
            QString getpath=QFileDialog::getExistingDirectory(this,"请选择目录",ui->configtable->item(row,col)->text());
            if(getpath=="")return;
            if(getpath.back()=='/')getpath=getpath.left(getpath.size()-1);
            ui->configtable->item(row,col)->setText(getpath);
        }
    });
    ui->headeredit->setText(allheader);
    dct(ui->start,btnc,this,&newtask::fill);
}

void newtask::fill(){
    ui->start->setEnabled(false);
    ui->start->setText("正在获取文件信息");

    vector<QString>ur;
    QString uim;
    for(auto &i:string(ui->urlsedit->toPlainText().toUtf8().data())){
        if(i!='\n')uim+=i;
        else if(!uim.isEmpty()){
            ur.push_back(uim);
            uim.clear();
        }
    }
    if(!uim.isEmpty())ur.push_back(uim);

    if(ur.empty()){
        msg("请至少填写一条链接")->exec();
        ui->start->setEnabled(true);
        ui->start->setText("开始");
        return;
    }

    vector<pair<QByteArray,QByteArray>>hd;
    vector<QByteArray>hr;
    QByteArray him;
    for(auto &i:string(ui->headeredit->toPlainText().toUtf8().data())){
        if(i!='\n')him+=i;
        else if(!him.isEmpty()){
            hr.push_back(him);
            him.clear();
        }
    }
    if(!him.isEmpty())hr.push_back(him);
    for(auto &i:hr){
        int g=i.indexOf(':');
        if(g>=i.size())continue;
        QByteArray key=i.left(g),val=i.mid(g+1);
        while(val[0]==' ')val=val.mid(1);
        hd.push_back({key,val});
    }

    d->set_url_and_header({{ur,hd}});

    info->out=ui->configtable->item(stv["out"],1)->text();
    info->all_start=ui->configtable->item(stv["all_start"],1)->text().toULongLong();
    info->all_end=[](const QString &x){return x=="-1"?-1ull:x.toULongLong();}(ui->configtable->item(stv["all_end"],1)->text());
    info->path=ui->configtable->item(stv["path"],1)->text();
    info->rcd={cupt+"/si-data/tasks/"+taskid};
    info->min_part_sz=ui->configtable->item(stv["min_part_sz"],1)->text().toULongLong();
    info->max_part_cnt=ui->configtable->item(stv["max_part_cnt"],1)->text().toUInt();
    info->max_cnct_run=ui->configtable->item(stv["max_cnct_run"],1)->text().toUInt();
    info->max_cnct_per_url=ui->configtable->item(stv["max_cnct_per_url"],1)->text().toUInt();
    info->meta_tmot=ui->configtable->item(stv["meta_tmot"],1)->text().toInt();
    info->body_tmot=ui->configtable->item(stv["body_tmot"],1)->text().toInt();
    info->dl_tmot=ui->configtable->item(stv["dl_tmot"],1)->text().toInt();

    dct(d,&KDlcore::fail,this,&newtask::fillfailed);
    dct(d,&KDlcore::filled_ok,this,&newtask::fillsuc);
    d->fill_only();
}

void newtask::fillfailed(int type, int type2){
    disconnect(d,&KDlcore::fail,this,&newtask::fillfailed);
    disconnect(d,&KDlcore::filled_ok,this,&newtask::fillsuc);
    msg("填充（获取文件信息）失败："+failtip[{type,type2}]);
    ui->start->setEnabled(true);
    ui->start->setText("开始");
}

void newtask::fillsuc(){
    disconnect(d,&KDlcore::fail,this,&newtask::fillfailed);
    disconnect(d,&KDlcore::filled_ok,this,&newtask::fillsuc);
    ui->start->setEnabled(true);
    all_tasks[taskid]=d;
    ok(taskid);
    deleteLater();
}

newtask::~newtask()
{
    delete ui;
}
