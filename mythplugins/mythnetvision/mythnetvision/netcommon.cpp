#include <QDir>

#include <libmythbase/mythcorecontext.h>
#include <libmythbase/mythdirs.h>
#include <libmythbase/remotefile.h>

#include "netcommon.h"

QString GetThumbnailFilename(const QString& url, const QString& title)
{
    QString fileprefix = GetConfDir();

    QDir dir(fileprefix);
    if (!dir.exists())
        dir.mkdir(fileprefix);

    fileprefix += "/cache/netvision-thumbcache";

    dir = QDir(fileprefix);
    if (!dir.exists())
        dir.mkdir(fileprefix);

    quint16 urlChecksum = qChecksum(url.toLocal8Bit());
    quint16 titleChecksum = qChecksum(title.toLocal8Bit());
    QString sFilename = QString("%1/%2_%3")
        .arg(fileprefix).arg(urlChecksum).arg(titleChecksum);
    return sFilename;
}

QString GetMythXMLURL(void)
{
    QString MasterIP = gCoreContext->GetMasterServerIP();
    int MasterStatusPort = gCoreContext->GetMasterServerStatusPort();

    return QString("http://%1:%2/InternetContent/").arg(MasterIP)
        .arg(MasterStatusPort);
}

QUrl GetMythXMLSearch(const QString& url, const QString& query, const QString& grabber,
                         const QString& pagenum)
{
    QString tmp = QString("%1GetInternetSearch?Query=%2&Grabber=%3&Page=%4")
        .arg(url, query, grabber, pagenum);
    QUrl tmpUrl {tmp};
    return tmpUrl;
}
