//一些头文件和函数
#ifndef BASE_H
#define BASE_H

#define TEST

#include <queue>
#include <random>
#include <set>
#include <QApplication>
#include <QTextCodec>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTimer>
#include <QFile>
#include <QDir>
#include <QStandardPaths>
#include <QProcess>
#include <QSettings>

#include <QFileDialog>
#include <QApplication>
#include <QMessageBox>
#include <QMainWindow>
#include <QSystemTrayIcon>
#include <QSharedMemory>
#include <QContextMenuEvent>
#include <QDesktopServices>

using namespace std;
typedef unsigned long long ull;
typedef long double ld;

template<typename Ta,typename Tb,typename Tc>inline void qct(Ta a,Tb b,Tc c){QObject::connect(a,b,a,c,Qt::QueuedConnection);}
template<typename Ta,typename Tb,typename Tc,typename Td>inline void qct(Ta a,Tb b,Tc c,Td d){QObject::connect(a,b,c,d,Qt::QueuedConnection);}
//队列连接
template<typename Ta,typename Tb,typename Tc>inline void dct(Ta a,Tb b,Tc c){QObject::connect(a,b,a,c,Qt::DirectConnection);}
template<typename Ta,typename Tb,typename Tc,typename Td>inline void dct(Ta a,Tb b,Tc c,Td d){QObject::connect(a,b,c,d,Qt::DirectConnection);}
//直接连接

inline bool have_any(QString &x,const string &y){for(auto &i:y)if(x.indexOf(i)!=-1)return true;return false;}

#define btnc &QPushButton::clicked

inline QString syscd_to_utf8(const string &text){static QTextStream t;return t.codec()->toUnicode(text.c_str());}//系统编码转utf8编码
//inline string utf8_to_syscd(const QString &text){static QTextStream t;return QTextCodec::codecForName(t.codec()->name())->fromUnicode(text).toStdString();}//utf8编码转系统编码
#endif // BASE_H
