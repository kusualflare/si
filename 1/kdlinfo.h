#ifndef KDLINFO_H
#define KDLINFO_H
#include <QObject>

typedef std::vector<std::pair<std::vector<QString>,std::vector<std::pair<QByteArray,QByteArray>>>> KUHG;

inline KUHG to_KUHG(const QString &text){
    KUHG x;
    static QString u,p,key;
    static QByteArray val;
    static int ei,ci,s_id;
    static std::vector<QString>url;
    static std::vector<std::pair<QByteArray,QByteArray>>header;
    for(int i=0;i<text.size();++i){
        u.clear();
        while(i<text.size()&&text[i]!='\n')u+=text[i++];
        if(u!="<")continue;
        url.clear();
        header.clear();
        while(++i<text.size()&&text[i]!='>'){
            p.clear();
            while(i<text.size()&&text[i]!='\n')p+=text[i++];
            if((ei=p.indexOf('='))==-1)continue;
            key=p.left(ei);
            val=p.mid(ei+1).toUtf8();
            if(key=="url")url.push_back(val);
            else if(key=="header"){
                if((ci=val.indexOf(':'))==-1)continue;
                header.push_back({val.left(ci),val.mid(ci+1)});
                s_id=0;
                while(header.back().second[s_id]==' ')++s_id;
                header.back().second=header.back().second.mid(s_id);
            }
        }
        x.push_back({url,header});
    }
    return x;
}

inline QString to_QString(const KUHG &x){
    QString text;
    for(auto &i:x){
        text+="<\n";
        for(auto &j:i.first)text+="url="+j+'\n';
        for(auto &j:i.second)text+="header="+j.first+':'+j.second+'\n';
        text+=">\n";
    }
    return text;
}

class KDlinfo
{
public:
    ////注意：传进来的任何（包括下载目录、文件名在内的）字符串应当是utf8编码的而不是系统编码，如有需要请使用把base.h中的syscd_to_utf8函数取消注释并使用它将字符串转为utf8编码
    explicit KDlinfo(
            const std::vector<QString>url,//链接
            const std::vector<std::pair<QByteArray,QByteArray>>header={},//请求头（该种构造方法为所有链接共用所有请求头）
            const QString &path="",//下载目录，为空则自动改为C:/users/<USER>/Downloads
            const QString &out="",//文件名，为空则自动通过Content-Disposition取名，如果无法通过Content-Disposition取名，则根据第一个链接取名，如果无法通过第一个链接取名，则自动通过时间取名
            const std::vector<QString> &rcd={},//日志路径（含文件名）。注意从当前版本开始代码不再自动在下载目录生成日志，也不会主动在下载完成后清楚日志
            quint64 all_start=0,//起始字节
            quint64 all_end=-1,//终止字节，为-1ull则自动改为文件大小-1
            quint64 min_part_sz=1,//最小分片大小
            size_t max_part_cnt=32,//最大分片数，建议不要超过1000
            size_t max_cnct_run=32,//最大连接数，建议不要超过400
            size_t max_cnct_per_url=32,//单链接最大连接数
            int meta_tmot=5,//单链接元数据获取超时
            int body_tmot=5,//单链接传输持续响应超时
            int dl_tmot=15//传输总持续响应超时
            ){
        this->url=url;
        this->header.push_back(header);
        this->header_pt.push_back(0);
        this->path=path;
        this->out=out;
        this->rcd=rcd;
        this->all_start=all_start;
        this->all_end=all_end;
        this->min_part_sz=min_part_sz;
        this->max_part_cnt=max_part_cnt;
        this->max_cnct_run=max_cnct_run;
        this->max_cnct_per_url=max_cnct_per_url;
        this->meta_tmot=meta_tmot;
        this->body_tmot=body_tmot;
        this->dl_tmot=dl_tmot;
    }
    explicit KDlinfo(
            const KUHG &url_header={},//链接-请求头组（支持为链接分组设置请求头）
            const QString &path="",//下载目录，为空则自动改为C:/users/<USER>/Downloads
            const QString &out="",//文件名，为空则自动通过Content-Disposition取名，如果无法通过Content-Disposition取名，则根据第一个链接取名，如果无法通过第一个链接取名，则自动通过时间取名
            const std::vector<QString> &rcd={},//日志路径（含文件名）。注意从当前版本开始代码不再自动在下载目录生成日志，也不会主动在下载完成后清楚日志
            quint64 all_start=0,//起始字节
            quint64 all_end=-1,//终止字节，为-1ull则自动改为文件大小-1
            quint64 min_part_sz=1,//最小分片大小
            size_t max_part_cnt=32,//最大分片数，建议不要超过1000
            size_t max_cnct_run=32,//最大连接数，建议不要超过400
            size_t max_cnct_per_url=32,//单链接最大连接数
            int meta_tmot=5,//单链接元数据获取超时
            int body_tmot=5,//单链接传输持续响应超时
            int dl_tmot=15//传输总持续响应超时
            ){
        for(size_t i=0;i<url_header.size();++i){
            for(auto &j:url_header[i].first){
                url.push_back(j);
                header_pt.push_back(i);
            }
            header.push_back(url_header[i].second);
        }
        this->path=path;
        this->out=out;
        this->rcd=rcd;
        this->all_start=all_start;
        this->all_end=all_end;
        this->min_part_sz=min_part_sz;
        this->max_part_cnt=max_part_cnt;
        this->max_cnct_run=max_cnct_run;
        this->max_cnct_per_url=max_cnct_per_url;
        this->meta_tmot=meta_tmot;
        this->body_tmot=body_tmot;
        this->dl_tmot=dl_tmot;
    }
    std::vector<QString>url;
    std::vector<std::vector<std::pair<QByteArray,QByteArray>>>header;
    std::vector<size_t>header_pt;
    QString path;
    QString out;
    std::vector<QString> rcd;
    quint64 all_start;
    quint64 all_end;
    quint64 min_part_sz;
    size_t max_part_cnt;
    size_t max_cnct_run;
    size_t max_cnct_per_url;
    int meta_tmot;
    int body_tmot;
    int dl_tmot;
    KUHG url_header(){
        KUHG uurl_header;
        for(auto &i:header)uurl_header.push_back({{},i});
        for(size_t i=0;i<url.size();++i)uurl_header[header_pt[i]].first.push_back(url[i]);
        return uurl_header;
    }
};

#endif // KDLINFO_H
