#ifndef KDLCORE_H
#define KDLCORE_H

#include <kdlinfo.h>
class KDownloader;

class KDlcore : public QObject
{
    Q_OBJECT
public:
    explicit KDlcore(KDlinfo *info,QObject *parent=nullptr);//新建，注意info不会随着对象的析构一起被删除
    explicit KDlcore(const QString &rcdpath,QObject *parent=nullptr);//从日志中恢复，未下完的分片会被直接丢弃
    void start();//开始（继续）
    void fill_only();//仅自动填充信息而不自动开始下载
    void stop();//暂停
    void removefile();//删除下好的文件（同步调用）
    void removercd();//删除记录（同步调用）
    void removeparts();//删除分片文件夹（连同分片一起，同步调用）
    bool notmerge();//是否下完不要进行合并（和删除）分片
    void set_notmerge(bool f=false);//设置是否下完不要进行合并（和删除）分片
    void restart(bool rt=true);//中断、移除已下好的文件或分片 以及 记录、让一切回到最初已填充好的状态，并重新开始（如果rt为假则不自动开始）
    void set_url_and_header(const KUHG &url_header);//更改链接和请求头（注意请在暂停时使用此函数，否则一切不会被更改）
    void set_rcd(std::vector<QString>rcd);//更改日志路径（无需暂停）
    bool is_stopped();//是否处于暂停状态
    bool is_suc();//是否处于成功状态
    KDlinfo* info();//获取当前配置信息

    QString path();//下载目录
    QString out();//文件名
    std::vector<QString> rcd();//日志路径
    quint64 all_start();//起始字节数
    quint64 all_end();//终止字节数
    quint64 all_size();//总大小
    quint64 all_dm();//已下载的大小
    quint64 dm_ls_sc();//即时速度（上一秒内下载的字节数）
    int scds();//已下载的秒数
    size_t cnct_cnt();//当前真实连接数
private:
    KDownloader *d;
    void cct();

signals:
    void fail(int type,int type2);//0:超时，1:读写出错，2:得到的响应不符合预期，3:主动暂停（特别注意这点，主动暂停也会发送fail信号），4:已经下载成功（这个类弄出类的对象是一次性的，下完一次就不能继续下了）
    void success();//标志着成功
    void advanced(const QString&);//约每秒发送一次，表示进度往前了 fail:失败，downloading:正在下载，ok:成功，merging:合并中
    void filled_ok();//表示填充完毕
};

#endif // KDLCORE_H
