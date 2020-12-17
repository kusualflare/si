#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <kdlcore.h>
#include <newtask.h>
#include <newtaskx.h>
#include <changeuh.h>
#include <dlcftw.h>
#include <msg.h>
#include <kclock.h>
extern QString cupt,othercfpath,taskindexpath,dlconfigpath,allheader,headerpath;
extern vector<QString>stv;
extern map<QString,KDlcore*>all_tasks;
extern map<QString,pair<QString,QString>>dl_st;
vector<QString>dl_st_l;
map<QString,int>dl_st_r;
extern map<QString,QString>other_config;
extern map<pair<int,int>,QString>failtip;
vector<QString>taskid_v;
const QPoint ttw(37,96);//放弃治疗了，用这个表示任务列表中左上第一个单元格可选区域左上角对于主窗体的相对坐标，来作为右键菜单的偏移量
inline void save_otherconfig(){
    QFile file(othercfpath);
    file.open(QIODevice::WriteOnly);
    QTextStream outfile(&file);
    outfile.setCodec("utf8");
    for(auto &i:other_config)outfile<<i.first<<'='<<i.second<<Qt::endl;
}
inline void beep(){
    if(other_config["beep_at_suc"].toUInt())QApplication::beep();
}
inline QString gtid(){
    return QDateTime::currentDateTime().toString("yyyyMMddhhmmsszzz");
}
inline void write_taskindex(){
    QFile file(taskindexpath);
    file.open(QIODevice::WriteOnly);
    QTextStream outfile(&file);
    outfile.setCodec("utf8");
    for(auto &i:all_tasks)outfile<<i.first<<Qt::endl;
}
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle("si");
    setWindowIcon(QIcon(cupt+"/si-data/ico/siico.png"));
    menu=new QMenu("选项");
    ac_openpath=new QAction("打开下载目录");
    ac_setuh=new QAction("更改链接-请求头");
    ac_wcat=new QAction("查看配置");
    dct(ac_openpath,&QAction::triggered,[this]{
        QDesktopServices::openUrl(QUrl("file:"+all_tasks[taskid_v[crr]]->path(), QUrl::TolerantMode));
        menu->deleteLater();
    });
    dct(ac_setuh,&QAction::triggered,[this]{
        (new changeuh(taskid_v[crr]))->show();
        menu->deleteLater();
    });
    dct(ac_wcat,&QAction::triggered,[this]{
        (new dlcftw(taskid_v[crr]))->show();
        menu->deleteLater();
    });

    QSystemTrayIcon *ic=new QSystemTrayIcon(QIcon(cupt+"/si-data/ico/siico.png"),this);
    dct(ic,&QSystemTrayIcon::activated,[this](QSystemTrayIcon::ActivationReason reason){
        if(reason==QSystemTrayIcon::DoubleClick)show();
    });
    ic->show();

    btn=new QPushButton("隐藏至托盘",this);
    dct(btn,btnc,this,&MainWindow::hide);

    dct(ui->tbwd,&QTabWidget::currentChanged,[this](int index){
        ui->statusbar->showMessage(index==0?"“失败”属于一种暂停，程序重启之后所有的失败会变成已暂停":index==1?"设置将自动保存，下载相关设置仅在部分时候自动填写进输入框或表格内":"“重建”的账号会被放到列表末尾");
    });
    if(ui->tbwd->currentIndex()==0)ui->tbwd->currentChanged(0);
    else ui->tbwd->setCurrentIndex(0);
    ui->tasktable->setColumnCount(10);
    ui->tasktable->setHorizontalHeaderLabels(QStringList()<<"保存路径"<<"文件名"<<"状态"<<"已下载"<<"总大小"<<"平均速度"<<"即时速度"<<"连接数"<<"已耗时(s)"<<"预计剩余(s)");
    ui->tasktable->horizontalHeader()->setSectionResizeMode(9,QHeaderView::Stretch);
    ui->tasktable->setSelectionBehavior(QAbstractItemView::SelectRows);

    ui->addx->setIcon(QApplication::style()->standardIcon(QStyle::SP_FileDialogEnd));
    ui->add->setIcon(QApplication::style()->standardIcon(QStyle::SP_FileDialogNewFolder));
    ui->del->setIcon(QApplication::style()->standardIcon(QStyle::SP_TrashIcon));
    ui->stop->setIcon(QApplication::style()->standardIcon(QStyle::SP_MediaPause));
    ui->ctne->setIcon(QApplication::style()->standardIcon(QStyle::SP_MediaPlay));
    ui->cancel->setIcon(QApplication::style()->standardIcon(QStyle::SP_BrowserStop));
    ui->restart->setIcon(QApplication::style()->standardIcon(QStyle::SP_BrowserReload));

    ui->settingtool->setCurrentIndex(0);

    ui->checkBox->setChecked(other_config["start_at_boot"].toUInt());
    ui->checkBox_2->setChecked(other_config["desktop_link"].toUInt());
    ui->checkBox_3->setChecked(other_config["startmenu_link"].toUInt());
    ui->checkBox_4->setChecked(other_config["beep_at_suc"].toUInt());
    ui->checkBox_5->setChecked(other_config["tip_at_fail"].toUInt());
    ui->checkBox_6->setChecked(other_config["onlyone"].toUInt());
    dct(ui->checkBox,&QCheckBox::stateChanged,[](bool checked){//开机启动（当前用户）
        QString baseUrl("HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run");
        QSettings settingClasses(baseUrl,QSettings::NativeFormat);
        if(checked){
            QString u=cupt;
            settingClasses.setValue("si",'"'+u.replace('/','\\')+"\\si\" /b");
        }
        else settingClasses.remove("si");
        other_config["start_at_boot"]=QString::number(checked);
        save_otherconfig();
    });
    dct(ui->checkBox_2,&QCheckBox::stateChanged,[this](bool checked){//桌面快捷方式
        QString tPath=QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
        if(checked){
            if(QFile::link(cupt+"/si.exe",tPath+"/si.lnk")==false){
                msg("固定失败")->exec();
                ui->checkBox_2->setChecked(false);
            }
        }else QFile::remove(tPath+"/si.lnk");
        other_config["desktop_link"]=QString::number(checked);
        save_otherconfig();
    });
    dct(ui->checkBox_3,&QCheckBox::stateChanged,[this](bool checked){//开始菜单快捷方式
        QString tPath=QStandardPaths::writableLocation(QStandardPaths::ApplicationsLocation);
        if(checked){
            QDir aw;
            if(aw.mkpath(tPath+"/si")==false||QFile::link(cupt+"/si.exe",tPath+"/si.lnk")==false){
                msg("固定失败")->exec();
                ui->checkBox_3->setChecked(false);
            }
        }else QFile::remove(tPath+"/si.lnk");
        other_config["startmenu_link"]=QString::number(checked);
        save_otherconfig();
    });
    dct(ui->checkBox_4,&QCheckBox::stateChanged,[](bool checked){
        other_config["beep_at_suc"]=QString::number(checked);
        save_otherconfig();
    });
    dct(ui->checkBox_5,&QCheckBox::stateChanged,[](bool checked){
        other_config["tip_at_fail"]=QString::number(checked);
        save_otherconfig();
    });
    dct(ui->checkBox_6,&QCheckBox::stateChanged,[](bool checked){
        other_config["onlyone"]=QString::number(checked);
        save_otherconfig();
    });

    ui->configtable->setColumnCount(2);
    ui->configtable->setHorizontalHeaderLabels(QStringList()<<"项"<<"值");
    ui->configtable->horizontalHeader()->setSectionResizeMode(1,QHeaderView::Stretch);
    ui->configtable->setRowCount(dl_st.size());
    int j=0;
    for(auto &i:dl_st){
        ui->configtable->setItem(j,0,new QTableWidgetItem(i.second.second));
        ui->configtable->item(j,0)->setFlags(ui->configtable->item(j,0)->flags()&~Qt::ItemIsEditable);
        ui->configtable->setItem(j,1,new QTableWidgetItem(i.second.first));
        if(i.first=="path"){
            ui->configtable->item(j,1)->setFlags(ui->configtable->item(j,1)->flags()&~Qt::ItemIsEditable);
            pathit=j;
        }
        ++j;
        dl_st_l.push_back(i.first);
    }
    for(size_t i=0;i<dl_st_l.size();++i)dl_st_r[dl_st_l[i]]=i;
    dct(ui->configtable,&QTableWidget::cellDoubleClicked,[this](int row,int col){
        if(row==pathit&&col==1){
            QString getpath=QFileDialog::getExistingDirectory(this,"请选择目录",ui->configtable->item(row,col)->text());
            if(getpath=="")return;
            if(getpath.back()=='/')getpath=getpath.left(getpath.size()-1);
            ui->configtable->item(row,col)->setText(getpath);
            ui->configtable->cellChanged(row,col);
        }
    });
    dct(ui->configtable,&QTableWidget::cellChanged,[this](int row){
        dl_st[dl_st_l[row]].first=ui->configtable->item(row,1)->text();
        QFile file(dlconfigpath);
        file.open(QIODevice::WriteOnly);
        QTextStream outfile(&file);
        outfile.setCodec("utf8");
        for(auto &i:dl_st)outfile<<i.second.second<<'|'<<i.first<<'='<<i.second.first<<Qt::endl;
    });
    ui->headeredit->setPlainText(allheader);
    dct(ui->headeredit,&QPlainTextEdit::textChanged,[this]{
        allheader=ui->headeredit->toPlainText();
        QFile file(headerpath);
        file.open(QIODevice::WriteOnly);
        QTextStream outfile(&file);
        outfile.setCodec("utf8");
        outfile<<allheader;
    });

    j=0;
    for(auto &i:all_tasks){
        taskid_v.push_back(i.first);
        auto u=i.second;
        u->setParent(this);
        ui->tasktable->insertRow(j);
        for(int i=0;i<10;++i){
            QTableWidgetItem *wi;
            ui->tasktable->setItem(j,i,wi=new QTableWidgetItem);
            wi->setFlags(wi->flags()&~Qt::ItemIsEditable);
        }
        ui->tasktable->item(j,0)->setText(u->path());
        ui->tasktable->item(j,1)->setText(u->out());
        ui->tasktable->item(j,2)->setText(u->is_suc()?"已完成":"已暂停");
        ui->tasktable->item(j,3)->setText(to_sz(u->all_dm()));
        ui->tasktable->item(j,4)->setText(to_sz(u->all_size()));
        ui->tasktable->item(j,5)->setText(u->scds()==0?"0":to_sz(u->all_dm()/(ld)u->scds())+"/s");
        ui->tasktable->item(j,6)->setText("0");
        ui->tasktable->item(j,7)->setText("0");
        ui->tasktable->item(j,8)->setText(QString::number(u->scds()));
        ui->tasktable->item(j,9)->setText(u->is_suc()?"0":"未知");
        dct(u,&KDlcore::advanced,[this,j,u](const QString &text){
            ui->tasktable->item(j,0)->setText(u->path());
            ui->tasktable->item(j,1)->setText(u->out());
            ui->tasktable->item(j,2)->setText(text=="downloading"?"下载中":text=="fail"?"失败":text=="merging"?"正在合并":"已完成");
            ui->tasktable->item(j,3)->setText(to_sz(u->all_dm()));
            ui->tasktable->item(j,4)->setText(to_sz(u->all_size()));
            ui->tasktable->item(j,5)->setText(u->scds()==0?"0":to_sz(u->all_dm()/(ld)u->scds())+"/s");
            ui->tasktable->item(j,6)->setText(to_sz(u->dm_ls_sc()));
            ui->tasktable->item(j,7)->setText(QString::number(u->cnct_cnt()));
            ui->tasktable->item(j,8)->setText(QString::number(u->scds()));
            ui->tasktable->item(j,9)->setText((u->all_dm()==0?"未知":QString::number(u->scds()*(u->all_size()-u->all_dm())/u->all_dm())));
        });
        dct(u,&KDlcore::success,&beep);
        dct(u,&KDlcore::fail,[](int type,int type2){
            auto o=failtip.find({type,type2});
            if(o!=failtip.end())msg("失败："+o->second);
        });
        ++j;
    }
    dct(ui->addx,btnc,[this]{
        newtaskx *n=new newtaskx(gtid());
        n->show();
        dct(n,&newtaskx::ok,this,&MainWindow::sndl);
    });
    dct(ui->add,btnc,[this]{
        newtask *n=new newtask(gtid());
        n->show();
        dct(n,&newtask::ok,this,&MainWindow::sndl);
    });
    dct(ui->del,btnc,[this]{
        set<int>ids;
        for(auto &i:ui->tasktable->selectedItems())ids.insert(i->row());
        int j=0;
        for(auto &i:ids){
            auto u=all_tasks[taskid_v[i-j]];
            u->stop();
            u->removeparts();
            u->removefile();
            u->removercd();
            u->deleteLater();
            all_tasks.erase(taskid_v[i-j]);
            ui->tasktable->removeRow(i-j);
            taskid_v.erase(taskid_v.begin()+i-j);
            ++j;
        }
        write_taskindex();
    });
    dct(ui->stop,btnc,[this]{
        set<int>ids;
        for(auto &i:ui->tasktable->selectedItems())ids.insert(i->row());
        for(auto &i:ids){
            auto u=all_tasks[taskid_v[i]];
            if(u->is_suc()==false){
                all_tasks[taskid_v[i]]->stop();
                ui->tasktable->item(i,2)->setText("已暂停");
            }
        }
    });
    dct(ui->ctne,btnc,[this]{
        set<int>ids;
        for(auto &i:ui->tasktable->selectedItems())ids.insert(i->row());
        for(auto &i:ids){
            auto u=all_tasks[taskid_v[i]];
            if(u->is_suc()==false){
                all_tasks[taskid_v[i]]->start();
                ui->tasktable->item(i,2)->setText("请求中");
            }
        }
    });
    dct(ui->cancel,btnc,[this]{
        set<int>ids;
        for(auto &i:ui->tasktable->selectedItems())ids.insert(i->row());
        int j=0;
        for(auto &i:ids){
            auto u=all_tasks[taskid_v[i-j]];
            u->stop();
            u->removeparts();
            u->removercd();
            u->deleteLater();
            all_tasks.erase(taskid_v[i-j]);
            ui->tasktable->removeRow(i-j);
            taskid_v.erase(taskid_v.begin()+i-j);
            ++j;
        }
        write_taskindex();
    });
    dct(ui->restart,btnc,[this]{
        set<int>ids;
        for(auto &i:ui->tasktable->selectedItems())ids.insert(i->row());
        for(auto &i:ids){
            all_tasks[taskid_v[i]]->restart();
            ui->tasktable->item(i,2)->setText("请求中");
        }
    });

    dct(ui->tasktable,&QTableWidget::cellDoubleClicked,[](int row){
        auto d=all_tasks[taskid_v[row]];
        if(d->is_suc())QProcess::execute("explorer",QStringList()<<QString(d->path()+'/'+d->out()).replace('/','\\'));
    });
}
void MainWindow::sndl(QString taskid){
    taskid_v.push_back(taskid);
    write_taskindex();
    auto u=all_tasks[taskid];
    int j=ui->tasktable->rowCount();
    ui->tasktable->insertRow(j);
    for(int i=0;i<10;++i){
        QTableWidgetItem *wi;
        ui->tasktable->setItem(j,i,wi=new QTableWidgetItem);
        wi->setFlags(wi->flags()&~Qt::ItemIsEditable);
    }
    ui->tasktable->item(j,0)->setText(u->path());
    ui->tasktable->item(j,1)->setText(u->out());
    ui->tasktable->item(j,2)->setText("请求中");
    ui->tasktable->item(j,4)->setText(to_sz(u->all_size()));
    dct(u,&KDlcore::advanced,[this,j,u](const QString &text){
        ui->tasktable->item(j,0)->setText(u->path());
        ui->tasktable->item(j,1)->setText(u->out());
        ui->tasktable->item(j,2)->setText(text=="downloading"?"下载中":text=="fail"?"失败":text=="merging"?"正在合并":"已完成");
        ui->tasktable->item(j,3)->setText(to_sz(u->all_dm()));
        ui->tasktable->item(j,4)->setText(to_sz(u->all_size()));
        ui->tasktable->item(j,5)->setText(u->scds()==0?"0":to_sz(u->all_dm()/(ld)u->scds())+"/s");
        ui->tasktable->item(j,6)->setText(to_sz(u->dm_ls_sc()));
        ui->tasktable->item(j,7)->setText(QString::number(u->cnct_cnt()));
        ui->tasktable->item(j,8)->setText(QString::number(u->scds()));
        ui->tasktable->item(j,9)->setText((u->all_dm()==0?"未知":QString::number(u->scds()*(u->all_size()-u->all_dm())/u->all_dm())));
    });
    dct(u,&KDlcore::success,&beep);
    dct(u,&KDlcore::fail,[](int type,int type2){
        auto o=failtip.find({type,type2});
        if(o!=failtip.end())msg("失败："+o->second);
    });
    u->start();
}
MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::resizeEvent(QResizeEvent *){
    btn->move(width()-btn->width(),0);
}

void MainWindow::contextMenuEvent(QContextMenuEvent *ev){
    if(ui->tbwd->currentIndex()!=0)return;
    auto l=ui->tasktable->itemAt(ev->pos()-ttw);
    if(l==nullptr)return;
    crr=l->row();
    menu=new QMenu;
    menu->addAction(ac_openpath);
    menu->addAction(ac_setuh);
    menu->addAction(ac_wcat);
    menu->exec(ev->globalPos());
    menu->show();
}
