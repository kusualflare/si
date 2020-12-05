#ifndef KDOWNLOADER_H
#define KDOWNLOADER_H

#include <QObject>
#include <queue>
class QNetworkAccessManager;
class QNetworkReply;
class QProcess;

class KClock;
class KCnc;
class KDlinfo;

class KDownloader : public QObject
{
    Q_OBJECT
public:
    explicit KDownloader(KDlinfo *info,QObject *parent=nullptr);//新建下载
    explicit KDownloader(const QString &fromrcd,QObject *parent=nullptr);//从日志中恢复下载，注意代码并不会对其进行校验
    ~KDownloader();
    void start();//开始（继续）
    void stop();//暂停
    void removefile();//删除下好的文件（同步调用）
    void removercd();//删除记录（同步调用）
    void removeparts();//删除分片文件夹（连同分片一起，同步调用）
    void change_url_and_header(const std::vector<std::pair<std::vector<QString>,std::vector<std::pair<QByteArray,QByteArray>>>> &url_header);//更改链接和请求头（注意请在暂停时使用此函数，否则一切不会被更改）
    bool do_not_merge=false;//是否下完不要进行合并（和删除）分片
    bool is_stopped=true;//是否处于暂停状态
    ///
    KDlinfo *info;
    KClock *line;

    std::queue<std::pair<size_t,size_t>>part_url_queue,b_queue;//分片-链接队列，缓冲用队列
    std::vector<quint64>part_start,part_end;//每个分片的真实起始、终止字节
    std::vector<size_t>url_cnct_cnt;//每条链接的真实连接数
    std::vector<bool>part_hv_suc,part_run;//每个分片是否已经成功，每个分片是否正在下

    //用于自动填充<
    bool is_filled=false;//信息是否已自动填充完成
    void fill_and_startdl(bool sd);//为true则填充完后自动开始下载
    int ti1=0,ti2=0;
    QString f_murl;
    QNetworkAccessManager *f_mn;
    QNetworkReply *f_rp;
    void f_cnct(bool sd);
    void f_tmot(QSet<int>*);
    bool tsd;
    void f_dlmt();
    void f_fail(int,int);
    void f_robk();
    void f_disc();
    void fdd(bool sd);
    void name();
    int endws;
    //>

    //用于下载<
    bool is_divided=false,is_suc=false;
    void divide();
    void startdl();
    size_t part_cnt,suc_cnt=0,cnct_cnt;
    int ti3=0,scds=0;
    quint64 all_dm,dm_cur_sc,dm_ls_sc;
    void d_tmot(QSet<int>*);
    void write_rcd(const QByteArray&);
    void d_fail(int,int);
    std::vector<KCnc*>dvk;
    std::pair<size_t,size_t>pri;
    void d_c();
    void d_cp();
    void d_suc();
    void c_ini();
    QString kkk_u;
    void mgok(int);
    void delok();
    //>
    QProcess *pr;

signals:
    void fail(int type,int type2);//type: 0:超时，1:读写出错，2:得到的响应不符合预期，3:主动暂停（特别注意这点，主动暂停也会发送fail信号），4:已经下载成功（这个类弄出类的对象是一次性的，下完一次就不能继续下了）
    void success();//标志着成功
    void advanced(const QString&);//约每秒发送一次，表示进度往前了（如果正在下载但即时速度为0则不会发送此信号） downloading:正在下载，fail:失败，ok:成功，merging:合并中
    void filled_ok();
};

#endif // KDOWNLOADER_H
