#include "zchxtileimagethread.h"
#include "qhttpget.h"
#include <QFile>
#include <QPixmapCache>

namespace qt {
zchxTileImageThread::zchxTileImageThread(const QString& url, const QString& name,int pos_x, int pos_y, bool imgSync, QObject* retobj, QObject *parent) : QObject(parent),QRunnable(),
    mImgSync(imgSync),
    mName(name),
    mUrl(url),
    mPx(pos_x),
    mPy(pos_y),
    mReturnObj(retobj)

{

}

void zchxTileImageThread::run()
{    
    QPixmap *img = loadImage();
    if(img)
    {
        if(!mImgSync)
        {
            emit signalSend(*img, mPx, mPy);
        } else
        {
            if(mReturnObj)
            QMetaObject::invokeMethod(mReturnObj,
                                      "appendTileImg",
                                      Qt::DirectConnection,
                                      Q_ARG(QPixmap, *img),
                                      Q_ARG(int, mPx),
                                      Q_ARG(int, mPy),
                                      Q_ARG(QString, mName)
                                      );
        }


        delete img;
    }
}

QPixmap* zchxTileImageThread::loadImage()
{
    return loadImageFromUrl(mUrl);
}

int Byte2Int(const QByteArray& src)
{
    QByteArray bytes = src;
    while(bytes.size() < 4)
    {
        bytes.insert(0, char(0));
    }
    int addr = bytes[3] & 0x000000FF;
    addr |= ((bytes[2] << 8) & 0x0000FF00);
    addr |= ((bytes[1] << 16) & 0x00FF0000);
    addr |= ((bytes[0] << 24) & 0xFF000000);
    return addr;
}
QPixmap* zchxTileImageThread::loadImageFromUrl(const QString &url)
{
    //qDebug()<<"start load image:"<<QDateTime::currentDateTime()<<url;
    QPixmap *img = new QPixmap(256, 256);
    //QPixmapCache::setCacheLimit(1);
    bool sts = false;
    QString format = "undef";
    if(!url.isEmpty()){
        if(url.contains("http"))
        {
            QByteArray recv = QHttpGet::getContentOfURL(url);
            QByteArray JPG = QByteArray::number(65496, 16).toUpper();
            QByteArray GIF = QByteArray::number(4671814, 16).toUpper();
            QByteArray PNG = QByteArray::number(2303741511, 16).toUpper();
            if(65496 == Byte2Int(recv.mid(0, 2)))
            {
                sts = img->loadFromData(recv, "JPG");
                format = "jpg";
            } else if(4671814 == Byte2Int(recv.mid(0, 3)))
            {
                sts = img->loadFromData(recv, "GIF");
                format = "gif";
            } else if(2303741511 == (uint)(Byte2Int(recv.mid(0, 4))))
            {
                sts = img->loadFromData(recv, "PNG");
                format = "png";
            }
        } else if(QFile::exists(url))
        {
            sts = img->load(url, "PNG");
        }
    }
    if(!sts)
    {
        img->fill(Qt::transparent);
    }
//    qDebug()<<"end load image:"<<format<<" img:"<<url<<"sts:"<<sts;
    return img;
}
}
