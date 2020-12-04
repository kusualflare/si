#include "dlcftw.h"
#include "ui_dlcftw.h"
#include <kdlcore.h>
#include <base.h>
extern map<QString,KDlcore*>all_tasks;
extern map<QString,pair<QString,QString>>dl_st;
extern map<QString,int>dl_st_r;
dlcftw::dlcftw(const QString &taskid,QWidget *parent) :
    QWidget(parent),
    ui(new Ui::dlcftw)
{
    ui->setupUi(this);
    auto f=all_tasks[taskid]->info();
    ui->configtable->setColumnCount(2);
    ui->configtable->setHorizontalHeaderLabels(QStringList()<<"项"<<"值");
    ui->configtable->horizontalHeader()->setSectionResizeMode(1,QHeaderView::Stretch);
    ui->configtable->setRowCount(dl_st.size());
    int j=0;
    for(auto &i:dl_st){
        ui->configtable->setItem(j,0,new QTableWidgetItem(i.second.second));
        ++j;
    }

    ui->configtable->setItem(dl_st_r["body_tmot"],1,new QTableWidgetItem(QString::number(f->body_tmot)));
    ui->configtable->setItem(dl_st_r["dl_tmot"],1,new QTableWidgetItem(QString::number(f->dl_tmot)));
    ui->configtable->setItem(dl_st_r["max_cnct_per_url"],1,new QTableWidgetItem(QString::number(f->max_cnct_per_url)));
    ui->configtable->setItem(dl_st_r["max_cnct_run"],1,new QTableWidgetItem(QString::number(f->max_cnct_run)));
    ui->configtable->setItem(dl_st_r["max_part_cnt"],1,new QTableWidgetItem(QString::number(f->max_part_cnt)));
    ui->configtable->setItem(dl_st_r["meta_tmot"],1,new QTableWidgetItem(QString::number(f->meta_tmot)));
    ui->configtable->setItem(dl_st_r["min_part_sz"],1,new QTableWidgetItem(QString::number(f->min_part_sz)));
    ui->configtable->setItem(dl_st_r["path"],1,new QTableWidgetItem(f->path));

    ui->configtable->insertRow(0);
    ui->configtable->setItem(0,0,new QTableWidgetItem("终止字节"));
    ui->configtable->setItem(0,1,new QTableWidgetItem(QString::number(f->all_end)));
    ui->configtable->insertRow(0);
    ui->configtable->setItem(0,0,new QTableWidgetItem("起始字节"));
    ui->configtable->setItem(0,1,new QTableWidgetItem(QString::number(f->all_start)));
    ui->configtable->insertRow(0);
    ui->configtable->setItem(0,0,new QTableWidgetItem("文件名"));
    ui->configtable->setItem(0,1,new QTableWidgetItem(f->out));

    for(int i=0;i<ui->configtable->rowCount();++i)for(int j=0;j<ui->configtable->columnCount();++j)ui->configtable->item(i,j)->setFlags(ui->configtable->item(i,j)->flags()&~Qt::ItemIsEditable);
}

dlcftw::~dlcftw()
{
    delete ui;
}
