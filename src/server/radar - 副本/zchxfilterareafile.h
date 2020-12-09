#ifndef ZCHXFILTERAREAFILE_H
#define ZCHXFILTERAREAFILE_H

#include "zchxdatadef.h"

class zchxFilterAreaFile
{
public:
    explicit zchxFilterAreaFile(const QString& fileName);
    bool    removeArea(const QList<int> ids);
    bool    addArea(const zchxCommon::zchxfilterAreaList& area);
    zchxCommon::zchxfilterAreaList  getFilterAreaList() const;
    QByteArray getFilterAreaByteArray() const;


private:
    void    init();
    bool    updateFile();

private:
    QString mFileName;
    QMap<qint64, zchxCommon::zchxFilterArea> mAreaMap;
};

#endif // ZCHXFILTERAREAFILE_H
