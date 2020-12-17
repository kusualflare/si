#include "kdownloader.h"
#include <kclock.h>
#include <kcnc.h>
#include <kdlinfo.h>
#include <base.h>

const QSet<QByteArray>st_dcd{"200","301","302","303","307","308"};

KDownloader::KDownloader(KDlinfo *info,QObject *parent):QObject(parent)
{
    this->info=info;
    line=new KClock(this);
}

KDownloader::KDownloader(const QString &fromrcd,QObject *parent):QObject(parent){
    info=new KDlinfo;
    line=new KClock(this);

    part_cnt=0;
    c_ini();

    QFile file(fromrcd);
    if(file.open(QIODevice::ReadOnly)==false)throw "日志路径无法打开";
    QTextStream infile(&file);
    infile.setCodec("utf8");
    QString info_u,info_key,info_val,stt;
    int eq_index,cl_index,sp_index,ba_index,s_id;
    vector<pair<QByteArray,QByteArray>>theader;
    while(infile.atEnd()==false){
        info_u=infile.readLine();
        if(info_u.isEmpty()||info_u[0]=='#')continue;
        if(info_u=='<'){
            theader.clear();
            while(info_u!='>'&&infile.atEnd()==false){
                info_u=infile.readLine();
                if(info_u.isEmpty()||info_u[0]=='#'||(eq_index=info_u.indexOf('='))==-1)continue;
                info_key=info_u.left(eq_index);
                info_val=info_u.mid(eq_index+1);
                if(info_key=="url"){
                    info->url.push_back(info_val);
                    info->header_pt.push_back(info->header.size());
                }
                else if(info_key=="header"){
                    cl_index=info_val.indexOf(':');
                    theader.push_back({info_val.left(cl_index).toUtf8().data(),info_val.mid(cl_index+1).toUtf8().data()});
                    s_id=0;
                    while(theader.back().second[s_id]==' ')++s_id;
                    theader.back().second=theader.back().second.mid(s_id);
                }
            }
            info->header.push_back(theader);
            continue;
        }
        if(info_u=="ok"||info_u=="downloading"||info_u=="merging"||info_u=="fail"){
            stt=info_u;
            continue;
        }
        if(info_u=="do_not_merge")do_not_merge=true;
        if((eq_index=info_u.indexOf('='))==-1)continue;
        info_key=info_u.left(eq_index);
        info_val=info_u.mid(eq_index+1);
        if(info_key=="path")info->path=info_val;
        else if(info_key=="out")info->out=info_val;
        else if(info_key=="rcd")info->rcd.push_back(info_val);
        else if(info_key=="all_start")info->all_start=info_val.toULongLong();
        else if(info_key=="all_end")info->all_end=info_val.toULongLong();
        else if(info_key=="min_part_sz")info->min_part_sz=info_val.toULongLong();
        else if(info_key=="max_part_cnt")info->max_part_cnt=info_val.toUInt();
        else if(info_key=="max_cnct_run")info->max_cnct_run=info_val.toUInt();
        else if(info_key=="max_cnct_per_url")info->max_cnct_per_url=info_val.toUInt();
        else if(info_key=="meta_tmot")info->meta_tmot=info_val.toInt();
        else if(info_key=="body_tmot")info->body_tmot=info_val.toInt();
        else if(info_key=="dl_tmot")info->dl_tmot=info_val.toInt();

        else if(info_key=="scds")scds=info_val.toInt();
        else if(info_key=="part"){
            if((ba_index=info_val.indexOf('-'))==-1||(sp_index=info_val.indexOf(' ',ba_index))==-1)continue;
            ++part_cnt;
            part_start.push_back(info_val.left(ba_index).toULongLong());
            part_end.push_back(info_val.left(sp_index).mid(ba_index+1).toULongLong());
            part_hv_suc.push_back(info_val.mid(sp_index+1).toInt());
            if(part_hv_suc.back())all_dm+=part_end.back()-part_start.back()+1;
            suc_cnt+=part_hv_suc.back();
        }
    }
    fdd(false);
    is_divided=true;
    if(stt=="ok")is_suc=true;
}

void KDownloader::start(){
    if(is_suc){
        fail(4,0);//"已成功"
        return;
    }
    if(is_stopped==false)return;
    is_stopped=false;
    if(is_filled)startdl();
    else fill_and_startdl(true);
}

void KDownloader::fill_and_startdl(bool sd){
    if(info->path.isEmpty())info->path=QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
    if(info->out.isEmpty()==false){
        if(info->all_end!=-1ull){
            fdd(sd);
            return;
        }
    }
    if(ti1==0)ti1=line->getid();
    line->insert(ti1,info->dl_tmot);
    dct(line,&KClock::tick,this,&KDownloader::f_tmot);
    f_murl=info->url[0];
    f_cnct(sd);
}

void KDownloader::fdd(bool sd){
    info->path.replace('\\','/');
    if(info->path.back()=='/')info->path=info->path.left(info->path.size()-1);

    kkk_u=info->path+'/'+info->out;
    kkk_u.replace('/','\\');

    endws=QString::number(info->all_end).size();

    for(auto &i:info->rcd)i.replace('\\','/');

    is_filled=true;
    filled_ok();
    if(sd)startdl();
}

void KDownloader::f_tmot(QSet<int>*st){
    if(st->contains(ti1)||st->contains(ti2))f_fail(0,0);//"在程序尝试自动填充时，主链接（第一条链接）响应超时"
}

void KDownloader::f_cnct(bool sd){
    tsd=sd;
    QNetworkRequest f_rq(f_murl);
    for(auto &i:info->header[0])f_rq.setRawHeader(i.first,i.second);
    f_mn=new QNetworkAccessManager;
    f_rp=f_mn->get(f_rq);
    dct(f_rp,&QNetworkReply::metaDataChanged,this,&KDownloader::f_dlmt);
    if(ti2==0)ti2=line->getid();
    line->insert(ti2,info->meta_tmot);
}

void KDownloader::f_dlmt(){
    if(f_rp->hasRawHeader("status")&&st_dcd.contains(f_rp->rawHeader("status").left(3))==false){
        f_fail(2,1);//"在程序尝试自动填充时，通过主链接（第一条链接）得到的响应头中的状态码不正确，为"+f_rp->rawHeader("status").left(3)
        return;
    }
    if(f_rp->hasRawHeader("location")){
        f_murl=f_rp->rawHeader("location");
        f_robk();
        f_cnct(tsd);
        return;
    }
    if(info->out.isEmpty()){
        if(f_rp->hasRawHeader("Content-Disposition")){
            QByteArray CD=f_rp->rawHeader("Content-Disposition");
            if(CD.left(11)!="attachment;"&&CD.indexOf(";attachment;")==-1&&CD.indexOf("; attachment;")==-1)throw "服务器希望你在浏览器中打开链接而不是进行下载";
            int uo=CD.indexOf(";filename="),bg=CD.left(9)=="filename="?0:uo==-1?CD.indexOf("; filename="):uo;
            if(bg==-1)name();
            else{
                int ed=CD.indexOf(';',bg+1);
                QByteArray rs=CD.mid(bg,max(-1,ed-bg));
                int tb=rs.indexOf('=');
                QByteArray to=rs.right(rs.size()-tb-1);
                if(to[0]=='\"'&&to.back()=='\"')to=to.mid(1,to.size()-2);
                info->out=to;
            }
        }else name();
    }
    if(info->all_end==-1ull){
        if(f_rp->hasRawHeader("Content-Length")==false){
            f_fail(2,0);//"在程序尝试自动填充终止字节数时，通过主链接（第一条链接）得到的响应头中没有Content-Length字段"
            return;
        }
        info->all_end=f_rp->rawHeader("Content-Length").toULongLong()-1;
    }
    f_disc();
    fdd(tsd);
}

void KDownloader::f_disc(){
    disconnect(line,&KClock::tick,this,&KDownloader::f_tmot);
    line->erase(ti1);
    line->erase(ti2);
}

void KDownloader::f_fail(int type, int type2){
    is_stopped=true;
    f_robk();
    f_disc();
    fail(type,type2);
}

void KDownloader::f_robk(){
    f_rp->disconnect();
    if(f_rp->isRunning())f_rp->abort();
    f_mn->deleteLater();
}

void KDownloader::name(){
    QString fg=info->url[0];
    while(fg.back()=='/')fg=fg.left(fg.size()-1);
    fg=fg.mid(fg.lastIndexOf('/')+1);
    if(have_any(fg,"\\/:*?\"<>|"))info->out=QString::number(time(0));
    else info->out=fg;
}

void KDownloader::divide(){//划定分片
    is_divided=true;
    ull part_size=max(info->min_part_sz,(info->all_end-info->all_start+1)/info->max_part_cnt);
    part_cnt=(info->all_end-info->all_start+1)/part_size;
    for(size_t i=0;i<part_cnt;++i){
        part_start.push_back(info->all_start+i*part_size);
        part_end.push_back((i+1)*part_size-1);
    }
    part_end.back()=info->all_end;
    part_hv_suc=vector<bool>(part_cnt,false);
}

void KDownloader::startdl(){
    if(is_divided==false)divide();

    QDir aw;
    for(auto &i:info->rcd)
        if(aw.mkpath(i.left(i.lastIndexOf('/')))==false){
            fail(1,0);//"下载目录无法创建"
            return;
        }
    if(aw.mkpath(info->path)==false||aw.mkpath(info->path+'/'+info->out+"-parts")==false){
        fail(1,1);//"日志目录无法创建"
        return;
    }

    part_url_queue=b_queue=queue<pair<size_t,size_t>>();
    vector<pair<size_t,size_t>>q0;
    for(size_t i=0;i<part_cnt;++i){
        if(part_hv_suc[i])continue;
        for(size_t j=0;j<info->url.size();++j)q0.push_back({i,j});
    }
    shuffle(q0.begin(),q0.end(),default_random_engine(chrono::system_clock::now().time_since_epoch().count()));
    for(auto &i:q0)part_url_queue.push(i);
    //随机生成队列

    c_ini();
    for(size_t i=0;i<part_cnt;++i)all_dm+=part_hv_suc[i]*(part_end[i]-part_start[i]+1);//当前已下载的字节数初始化为已成功的分片大小总和
    part_run=vector<bool>(part_cnt,false);//初始化为所有分片不在下载
    url_cnct_cnt=vector<size_t>(info->url.size(),0);//初始化为所有链接当前真实连接数为0
    dvk=vector<KCnc*>(part_cnt,nullptr);

    if(ti3==0)ti3=line->getid();
    line->insert(ti3,info->dl_tmot);
    dct(line,&KClock::tick,this,&KDownloader::d_tmot);
    d_c();
}

void KDownloader::c_ini(){
    all_dm=0;
    dm_ls_sc=0;//上一秒下载的字节数初始化为0
    dm_cur_sc=0;//当前秒下载的字节数初始化为0
    cnct_cnt=0;//当前真实连接数初始化为0
}

void KDownloader::d_tmot(QSet<int>*st){
    if(st->contains(ti3)){
        write_rcd("fail");//注意这行不能写在d_fail函数里
        d_fail(0,1);//"传输持续响应超时"
        return;
    }
    ++scds;
    write_rcd("downloading");
}

void KDownloader::write_rcd(const QByteArray &state){
    if(state=="downloading"&&dm_cur_sc==0)return;
    dm_ls_sc=dm_cur_sc;
    advanced(state);
    dm_cur_sc=0;
    if(info->rcd.empty())return;

    QString s;
    QTextStream word(&s);
    if(do_not_merge)word<<"do_not_merge"<<Qt::endl;
    word<<state<<Qt::endl;
    word<<to_QString(info->url_header());
    word<<"path="<<info->path<<Qt::endl;
    word<<"out="<<info->out<<Qt::endl;
    for(auto &i:info->rcd)word<<"rcd="<<i<<Qt::endl;
    word<<"all_start="<<info->all_start<<Qt::endl;
    word<<"all_end="<<info->all_end<<Qt::endl;
    word<<"min_part_sz="<<info->min_part_sz<<Qt::endl;
    word<<"max_part_cnt="<<info->max_part_cnt<<Qt::endl;
    word<<"max_cnct_run="<<info->max_cnct_run<<Qt::endl;
    word<<"max_cnct_per_url="<<info->max_cnct_per_url<<Qt::endl;
    word<<"meta_tmot="<<info->meta_tmot<<Qt::endl;
    word<<"body_tmot="<<info->body_tmot<<Qt::endl;
    word<<"dl_tmot="<<info->dl_tmot<<Qt::endl;
    for(size_t i=0;i<part_cnt;++i)word<<"part="<<part_start[i]<<'-'<<part_end[i]<<' '<<part_hv_suc[i]<<Qt::endl;
    word<<"scds="<<scds<<Qt::endl;
    word<<"cnct_cnt="<<cnct_cnt<<Qt::endl;
    //生成日志内容

    for(auto &i:info->rcd){
        QFile rcd_file(i);
        if(rcd_file.open(QIODevice::WriteOnly)==false){
            d_fail(1,2);//"无法写入日志"
            return;
        }
        rcd_file.write(s.toUtf8().data());
    }
}

void KDownloader::d_fail(int type, int type2){
    is_stopped=true;
    d_cp();
    fail(type,type2);
}

void KDownloader::d_cp(){
    line->erase(ti3);
    disconnect(line,&KClock::tick,this,&KDownloader::d_tmot);
    for(auto &i:dvk)if(i!=nullptr)i->stop();
}

void KDownloader::d_c(){
    if(suc_cnt==part_cnt){
        is_stopped=true;
        d_suc();
        return;
    }
    while(b_queue.empty()==false){
        part_url_queue.push(b_queue.front());
        b_queue.pop();
    }
    while(part_url_queue.empty()==false&&cnct_cnt<info->max_cnct_run){
        pri=part_url_queue.front();
        part_url_queue.pop();
        if(part_hv_suc[pri.first])continue;//如果某个分片已经成功，则跳过这个二元组
        if(url_cnct_cnt[pri.second]>=info->max_cnct_per_url||part_run[pri.first]){//如果该链接连接数已达到单链接最大连接数，或该分片已存在连接，那么该二元组放进缓冲用队列等到下次信号被触发的时候再压入分片-链接队列
            b_queue.push(pri);
            continue;
        }
        dvk[pri.first]=new KCnc(pri,this);
    }
}

void KDownloader::stop(){
    if(is_stopped||is_suc)return;
    if(is_filled)d_fail(3,0);//"在下载时主动暂停"
    else f_fail(3,1);//"在自动填充时主动暂停"
}

void KDownloader::d_suc(){
    d_cp();
    if(do_not_merge){
        line->disconnect();
        line->deleteLater();
        is_suc=true;
        write_rcd("ok");
        success();
        return;
    }
    write_rcd("merging");
    pr=new QProcess(this);
    pr->start("cmd",QStringList()<<"/c"<<"copy"<<"/b"<<"/y"<<kkk_u+"-parts\\"+info->out+".*"<<kkk_u);
    qct(pr,QOverload<int,QProcess::ExitStatus>::of(&QProcess::finished),this,&KDownloader::mgok);
}

void KDownloader::mgok(int exitcode){
    if(exitcode!=0){
        fail(1,3);//"合并时出错，copy命令提示："+syscd_to_utf8(pr->readAllStandardOutput().toStdString())
        pr->disconnect();
        pr->deleteLater();
        return;
    }
    is_suc=true;
    pr->disconnect();
    pr->deleteLater();
    line->disconnect();
    line->deleteLater();
    write_rcd("ok");
    removeparts();
    success();
}

KDownloader::~KDownloader(){
    stop();
}

void KDownloader::removefile(){
    QFile::remove(info->path+'/'+info->out);
}

void KDownloader::removercd(){
    for(auto &i:info->rcd)QFile::remove(i);
}

void KDownloader::removeparts(){
    QDir aw(info->path+'/'+info->out+"-parts");
    aw.removeRecursively();
}

void KDownloader::change_url_and_header(const KUHG &url_header){
    if(is_stopped==false)return;
    info->url.clear();
    info->header.clear();
    info->header_pt.clear();
    for(size_t i=0;i<url_header.size();++i){
        for(auto &j:url_header[i].first){
            info->url.push_back(j);
            info->header_pt.push_back(i);
        }
        info->header.push_back(url_header[i].second);
    }
}
