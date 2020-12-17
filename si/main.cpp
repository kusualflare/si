#include "mainwindow.h"
#include <msg.h>
#include <kdlcore.h>
QString cupt,start_mode,dlconfigpath,headerpath,allheader,taskindexpath,othercfpath;
extern map<QString,pair<QString,QString>>dl_st;
extern map<QString,int>stv;
map<QString,QString>other_config;
map<QString,KDlcore*>all_tasks;
inline void rd(){
    cupt=QApplication::applicationDirPath();
    dlconfigpath=cupt+"/si-data/settings/dlconfig.txt",headerpath=cupt+"/si-data/settings/header.txt",taskindexpath=cupt+"/si-data/taskindex.txt",othercfpath=cupt+"/si-data/settings/otherconfig.txt";
    QFile header_file(headerpath);
    if(header_file.open(QIODevice::ReadOnly)==false){
        msg("请求头配置文件缺失")->exec();
        throw -1;
    }
    QTextStream header_infile(&header_file);
    header_infile.setCodec("utf8");
    QString u;
    allheader=header_infile.readAll();

    QFile config_file(dlconfigpath);
    if(config_file.open(QIODevice::ReadOnly)==false){
        msg("下载配置文件缺失")->exec();
        throw -1;
    }
    QTextStream config_infile(&config_file);
    config_infile.setCodec("utf8");
    while(config_infile.atEnd()==false){
        u=config_infile.readLine();
        if(u.isEmpty())continue;
        int p=u.indexOf('|'),g=u.indexOf('=',p);
        if(p==-1||g==-1||p>g)continue;
        dl_st[u.mid(p+1,g-p-1)]={u.mid(g+1),u.left(p)};
    }
    stv["out"]=0;
    stv["all_start"]=1;
    stv["all_end"]=2;
    int j=2;
    for(auto &i:dl_st)stv[i.first]=++j;

    QFile taskid_file(taskindexpath);
    if(taskid_file.open(QIODevice::ReadOnly)==false){
        msg("下载记录文件缺失")->exec();
        throw -1;
    }
    QTextStream taskid_infile(&taskid_file);
    taskid_infile.setCodec("utf8");
    bool flag=false;
    while(taskid_infile.atEnd()==false){
        u=taskid_infile.readLine();
        if(u.isEmpty())continue;
        try {
            auto d=new KDlcore(cupt+"/si-data/tasks/"+u);
            all_tasks[u]=d;
        } catch (...) {
            flag=true;
        }
    }
    if(flag)msg("部分或全部记录文件不存在，如果这不是你手动删除的并且你也没有强行退出程序，请与我联系。除记录丢失外，继续使用不会受到影响");

    QFile othercf_file(othercfpath);
    if(othercf_file.open(QIODevice::ReadOnly)==false){
        msg("杂项配置文件缺失")->exec();
        throw -1;
    }
    int i;
    QTextStream othercf_infile(&othercf_file);
    othercf_infile.setCodec("utf8");
    while(othercf_infile.atEnd()==false){
        u=othercf_infile.readLine();
        if(u.isEmpty()||(i=u.indexOf('='))==-1)continue;
        other_config[u.left(i)]=u.mid(i+1);
    }

    if(other_config["onlyone"].toUInt()){
        auto *shared=new QSharedMemory(cupt+"/si");
        if(shared->create(1)==false){
            msg("已存在同一目录下的si实例，请检查托盘并双击图标以显示主窗口。如果想运行多份，请将文件夹完整地复制至其它目录（设置中部分选项如快捷方式等需重新打勾才有效）。如果想同时运行同一目录下的多份，请在设置中调整")->exec();
            throw -1;
        }
    }
}
int main(int argc, char *argv[])
{
    try {
        QApplication a(argc, argv);
        rd();
        auto *w=new MainWindow;
        if(argc>1)start_mode=argv[1];
        if(start_mode!="/b")w->show();//开机自启时不显示窗口
        return a.exec();
    } catch (int x) {
        return x;
    }
}
