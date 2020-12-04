#include "kcnc.h"
#include <base.h>
#include <kclock.h>
#include <kdlinfo.h>
#include <kdownloader.h>

const QSet<QByteArray>st_wcd{"206","301","302","303","307","308"};

KCnc::KCnc(std::pair<size_t, size_t> pri, KDownloader *d) : QObject()
{
    this->d=d;
    part_id=pri.first;
    url_id=pri.second;
    my_url=d->info->url[url_id];
    start_dl();
}

void KCnc::start_dl(){
    trans=false;
    sz_got=0;//把已得到且写入了的字节数初始化为0，注意这行不要放到f_dlmt函数中
    sz_pea=0;

    ++d->cnct_cnt;//真实连接数+1
    ++d->url_cnct_cnt[url_id];//当前链接真实连接数+1
    d->part_run[part_id]=true;//标记当前分片正在下

    QNetworkRequest n_req(my_url);
    for(auto &i:d->info->header[d->info->header_pt[url_id]])n_req.setRawHeader(i.first,i.second);
    n_req.setRawHeader("Range","bytes="+QByteArray::number(d->part_start[part_id])+'-'+QByteArray::number(d->part_end[part_id]));
    n_mn=new QNetworkAccessManager;
    n_rp=n_mn->get(n_req);
    //发送请求
    dct(n_rp,&QNetworkReply::metaDataChanged,this,&KCnc::n_dlmt);
    //得到元数据就开始处理响应头

    if(ti1==0)ti1=d->line->getid();
    d->line->insert(ti1,d->info->meta_tmot);
    //计时1：如果单连接元数据获取超时秒内没能获取到元数据，算作失败
    dct(d->line,&KClock::tick,this,&KCnc::n_tmot);
}

void KCnc::n_tmot(QSet<int>*st){
    if(st->contains(ti1))n_fail();
}

void KCnc::n_fail(){
    n_disc();
    robk();
    finish();
}

void KCnc::n_disc(){
    disconnect(d->line,&KClock::tick,this,&KCnc::n_tmot);
    d->line->erase(ti1);
}

void KCnc::robk(){
    n_rp->disconnect();
    if(n_rp->isRunning())n_rp->abort();
    n_mn->deleteLater();
    if(ot!=nullptr){
        ot->close();
        ot->deleteLater();
        ot=nullptr;
    }
    --d->cnct_cnt;//真实连接数-1
    --d->url_cnct_cnt[url_id];//当前链接真实连接数-1
    d->part_run[part_id]=false;
}

void KCnc::finish(){
    d->dvk[part_id]=nullptr;
    if(d->is_stopped==false){
        d->b_queue.push({part_id,url_id});
        d->d_c();
    }
    deleteLater();
}

void KCnc::n_dlmt(){
    if(n_rp->hasRawHeader("status")&&st_wcd.contains(n_rp->rawHeader("status").left(3))){
        n_fail();
        return;
    }
    if(n_rp->hasRawHeader("location")){
        my_url=n_rp->rawHeader("location");
        //把当前链接设置成location字段所含的链接
        robk();
        start_dl();
        //重新开始下载
        return;
    }
    //如果响应头中含有location字段则通过它重定向。程序仅会处理这一类型的重定向
    if(ctrg_is_wr()){
        n_fail();
        return;
    }
    //如果Content-Range字段不正确，算作失败。（当然有location字段重定向的刚刚已经处理过了）

    QString tstart=QString::number(d->part_start[part_id]),tend=QString::number(d->part_end[part_id]);
    ot=new QFile(d->info->path+'/'+d->info->out+"-parts/"+d->info->out+'.'+QString(d->endws-tstart.size(),'0')+tstart+'-'+QString(d->endws-tend.size(),'0')+tend);
    //ot->setParent(this);
    if(ot->open(QIODevice::WriteOnly)==false){
        n_fail();
        return;
    }
    //打开文件准备写入

    dct(n_rp,&QNetworkReply::finished,this,&KCnc::p_dlfn);
    //连接结束

    n_disc();
    d->line->insert(ti1,d->info->body_tmot);
    dct(d->line,&KClock::tick,this,&KCnc::p_tmot);
    //如果每单连接最大响应间隔秒内没有得到进一步的返回，算作失败
}

bool KCnc::ctrg_is_wr(){
    QByteArray ctrg=n_rp->rawHeader("Content-Range");
    if(ctrg.size()<6)return true;
    static ull lf,rt;
    lf=0;
    rt=0;
    int b=6;
    for(;b<ctrg.size()&&ctrg[b]!='-';++b)lf=(lf<<1)+(lf<<3)+(ctrg[b]^48);
    if(b==ctrg.size()||lf!=d->part_start[part_id])return true;//起始字节数不正确
    for(++b;b<ctrg.size()&&ctrg[b]!='/';++b)rt=(rt<<1)+(rt<<3)+(ctrg[b]^48);
    if(b==ctrg.size()||rt!=d->part_end[part_id])return true;//终止字节数不正确
    return false;
}

void KCnc::p_tmot(QSet<int>*st){
    if(st->contains(ti1))p_fail();
    else write_part();
}

void KCnc::p_fail(){
    d->all_dm-=sz_got;
    p_fins();
}

void KCnc::p_disc(){
    disconnect(d->line,&KClock::tick,this,&KCnc::p_tmot);
    d->line->erase(ti1);
}

void KCnc::write_part(){
    y=n_rp->bytesAvailable();
    if(sz_got+y>sz_pea){
        sz_pea=sz_got+y;
        d->line->restart(ti1);
        d->line->restart(d->ti3);
    }else return;
    if(y<min(52428800ull,d->part_end[part_id]-d->part_start[part_id]+1-sz_got))return;
    ot->write(n_rp->readAll());
    sz_got+=y;
    d->dm_cur_sc+=y;
    d->all_dm+=y;
    return;
}

void KCnc::p_dlfn(){
    write_part();
    if(sz_got==d->part_end[part_id]-d->part_start[part_id]+1){//如果字节数对上了，算下载成功
        d->part_hv_suc[part_id]=true;
        ++d->suc_cnt;
        p_fins();
    }else p_fail();
}

void KCnc::p_fins(){
    p_disc();
    robk();
    finish();
}

void KCnc::stop(){
    if(trans)p_fail();
    else n_fail();
}
