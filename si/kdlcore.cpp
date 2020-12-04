#include "kdlcore.h"
#include <base.h>
#include <kdownloader.h>

KDlcore::KDlcore(KDlinfo *info,QObject *parent):QObject(parent){
    d=new KDownloader(info,this);
    cct();
}

KDlcore::KDlcore(const QString &rcdpath,QObject *parent):QObject(parent){
    d=new KDownloader(rcdpath,this);
    cct();
}

void KDlcore::cct(){
    dct(d,&KDownloader::fail,this,&KDlcore::fail);
    dct(d,&KDownloader::success,this,&KDlcore::success);
    dct(d,&KDownloader::advanced,this,&KDlcore::advanced);
    dct(d,&KDownloader::filled_ok,this,&KDlcore::filled_ok);
}

void KDlcore::start(){
    d->start();
}
void KDlcore::fill_only(){
    d->fill_and_startdl(false);
}
void KDlcore::stop(){
    d->stop();
}
void KDlcore::removefile(){
    d->removefile();
}
void KDlcore::removercd(){
    d->removercd();
}
void KDlcore::removeparts(){
    d->removeparts();
}
bool KDlcore::notmerge(){
    return d->do_not_merge;
}
void KDlcore::set_notmerge(bool f){
    d->do_not_merge=f;
}
void KDlcore::restart(bool rt){
    if(d->is_filled==false)return;
    d->stop();
    d->removefile();
    d->removercd();
    d->removeparts();
    auto u=new KDownloader(d->info);
    u->do_not_merge=d->do_not_merge;
    d->disconnect();
    d->deleteLater();
    d=u;
    cct();
    d->is_stopped=false;
    d->fill_and_startdl(rt);
}
void KDlcore::set_url_and_header(const KUHG &url_header){
    d->change_url_and_header(url_header);
}
void KDlcore::set_rcd(std::vector<QString> rcd){
    d->info->rcd=rcd;
}
bool KDlcore::is_stopped(){
    return d->is_stopped;
}
bool KDlcore::is_suc(){
    return d->is_suc;
}
KDlinfo* KDlcore::info(){
    return d->info;
}

QString KDlcore::path(){
    return d->info->path;
}
QString KDlcore::out(){
    return d->info->out;
}
vector<QString> KDlcore::rcd(){
    return d->info->rcd;
}

ull KDlcore::all_start(){
    return d->info->all_start;
}
ull KDlcore::all_end(){
    return d->info->all_end;
}
ull KDlcore::all_size(){
    return d->info->all_end-d->info->all_start+1;
}
ull KDlcore::all_dm(){
    return d->all_dm;
}
ull KDlcore::dm_ls_sc(){
    return d->dm_ls_sc;
}
int KDlcore::scds(){
    return d->scds;
}

size_t KDlcore::cnct_cnt(){
    return d->cnct_cnt;
}
