# si
一个轻量的下载器，支持多链接下载同一文件、自定义起止字节数

最低配置：win7 32位 1G内存

通过Qt5.15.2·C++编写

下载部分核心代码见1文件夹，demo代码见si文件夹

从2.0版本开始，不再单独发布控制台应用程序，仅发布窗口程序

如果你想简单地使用这个代码，这样做（以简单的控制台应用为例）：

把1文件夹下的所有文件添加进你的项目

在.pro里加QT += network

主程序这样写：
```
#include<KDlcore.h>
#include<base.h>

map<pair<int,int>,QString>failtip={{{0,0},"在程序尝试自动填充时，主链接（第一条链接）响应超时"},{{0,1},"传输持续响应超时"},{{1,0},"下载目录无法创建"},{{1,1},"日志目录无法创建"},{{1,2},"无法写入日志"},{{1,3},"合并时出错"},{{2,0},"在程序尝试自动填充终止字节数时，通过主链接（第一条链接）得到的响应头中没有Content-Length字段"},{{2,1},"在程序尝试自动填充时，通过主链接（第一条链接）得到的响应头中的状态码不正确"}};


int main(int argc, char *argv[]){
    QCoreApplication a(argc, argv);
    auto *ifo=new KDlinfo(KUHG({{{"（你要下的链接）"},{{"User-Agent","（对应的ua）"},{"Cookie","（对应的饼干）"}}}}));
    auto *dl=new KDlcore(ifo);
    qDebug()<<"正在填充";
    vector<ull>dmpss;
    dct(dl,&KDlcore::filled_ok,[]{
        cls();
        qDebug()<<"正在连接";
    });
    dct(dl,&KDlcore::success,[&a,dl,&dmpss]{
        cls();
        sort(dmpss.begin(),dmpss.end());
        qDebug()<<(dl->path()+'/'+dl->out()).toUtf8().data()<<"下载完成";
        qDebug()<<"共"<<dl->all_size()/1048576.0<<"M 耗时"<<dl->scds()<<"s 平均速度"<<dl->all_size()/1048576.0/dl->scds()<<"M/s 即时速度的中位数"<<(dmpss.size()&1?dmpss[dmpss.size()>>1]:(dmpss[(dmpss.size()-1)>>1]+dmpss[dmpss.size()>>1])>>1)/1048576.0<<"M/s";
        a.exit();
    });
    dct(dl,&KDlcore::fail,[&a,dl](int type,int type2){
        auto k=failtip.find({type,type2});
        if(k==failtip.end())return;
        cls();
        qDebug()<<(dl->path()+'/'+dl->out()).toUtf8().data()<<"下载失败(类型"<<type<<','<<type2<<"):"<<k->second;
        a.exit(1);
    });
    dct(dl,&KDlcore::advanced,[dl,&dmpss](const QString &text){
        cls();
        qDebug()<<(dl->path()+'/'+dl->out()).toUtf8().data()<<(text=="downloading"?"正在下载":text=="fail"?"下载失败":text=="merging"?"正在合并":"合并成功");
        qDebug()<<dl->all_dm()/1048576.0<<"M /"<<dl->all_size()/1048576.0<<"M ("<<dl->all_dm()*100.0/dl->all_size()<<"%) 平均速度"<<dl->all_dm()/1048576.0/dl->scds()<<"M/s";
        if(text=="downloading"){
            qDebug()<<"即时速度"<<dl->dm_ls_sc()/1048576.0<<"M/s 连接数"<<dl->cnct_cnt()<<"已下载了"<<dl->scds()<<"s 剩余"<<(dl->all_dm()==0?-1ull:dl->scds()*(dl->all_size()-dl->all_dm())/dl->all_dm())<<'s';
            dmpss.push_back(dl->dm_ls_sc());
        }
    });
    dl->start();
    a.exec();
    delete ifo;
}
```
