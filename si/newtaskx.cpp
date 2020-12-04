#include "newtaskx.h"
#include "ui_newtaskx.h"
#include <kdlcore.h>
#include <msg.h>
extern map<QString,pair<QString,QString>>dl_st;
extern QString allheader;
extern QString cupt;
extern map<QString,int>stv;
extern map<QString,KDlcore*>all_tasks;
extern map<pair<int,int>,QString>failtip;
newtaskx::newtaskx(const QString &taskid, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::newtaskx)
{
    ui->setupUi(this);
    ui->uhedit->setPlaceholderText("请填写至少一个指向同一文件的链接-请求头组，示例：\n\n<\nurl=https://www.123.com/test.zip\nurl=https://www.456.com/test/0_1\nheader=User-Agent:1\nUser-Agent=Cookie:2\n>\n<\nurl=http://www.789.io/file/test?id=0\nheader=1: 234\nurl=https://www.000.com.cn/t\n>");
    this->taskid=taskid;
    d=new KDlcore(info=new KDlinfo);

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
    dct(ui->start,btnc,this,&newtaskx::fill);
}

void newtaskx::fill(){
    ui->start->setEnabled(false);
    ui->start->setText("正在获取文件信息");

    KUHG p=to_KUHG(ui->uhedit->toPlainText());
    bool flag=true;
    for(auto &i:p)if(i.first.size()){
        flag=false;
        break;
    }
    if(flag){
        msg("请至少填写一条链接")->exec();
        ui->start->setEnabled(true);
        ui->start->setText("开始");
        return;
    }
    d->set_url_and_header(p);

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

    dct(d,&KDlcore::fail,this,&newtaskx::fillfailed);
    dct(d,&KDlcore::filled_ok,this,&newtaskx::fillsuc);
    d->fill_only();
}

void newtaskx::fillfailed(int type, int type2){
    disconnect(d,&KDlcore::fail,this,&newtaskx::fillfailed);
    disconnect(d,&KDlcore::filled_ok,this,&newtaskx::fillsuc);
    msg("填充（获取文件信息）失败："+failtip[{type,type2}]);
    ui->start->setEnabled(true);
    ui->start->setText("开始");
}

void newtaskx::fillsuc(){
    disconnect(d,&KDlcore::fail,this,&newtaskx::fillfailed);
    disconnect(d,&KDlcore::filled_ok,this,&newtaskx::fillsuc);
    ui->start->setEnabled(true);
    all_tasks[taskid]=d;
    ok(taskid);
    deleteLater();
}

newtaskx::~newtaskx()
{
    delete ui;
}
