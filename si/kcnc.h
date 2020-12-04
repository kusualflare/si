#ifndef KCNC_H
#define KCNC_H

#include <QObject>
class QNetworkAccessManager;
class QNetworkReply;
class QFile;

class KDownloader;

class KCnc : public QObject
{
    Q_OBJECT
public:
    explicit KCnc(std::pair<size_t,size_t>pri,KDownloader* d);
    void start_dl();
    void stop();
    ///
    size_t part_id,url_id;
    QString my_url;//当前链接。重定向修改这里的，而不是url向量里的，确保重定向不会改变原始的链接数据且每次都重新进行重定向
    KDownloader *d;
    quint64 sz_got,sz_pea;//已得到的字节数，（第二个变量是用于辅助检查有无新的数据到来的）

    QNetworkAccessManager *n_mn;
    QNetworkReply *n_rp;
    int ti1=0;
    void n_tmot(QSet<int>*);
    void n_dlmt();
    void n_fail();
    void robk();
    void n_disc();
    bool ctrg_is_wr();
    void finish();
    bool trans;
    QFile *ot=nullptr;

    void p_fail();
    void p_dlfn();
    void p_tmot(QSet<int>*);
    void p_disc();
    void p_fins();
    void write_part();
    quint64 y;

signals:

};

#endif // KCNC_H
