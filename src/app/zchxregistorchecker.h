#ifndef ZCHXREGISTORCHECKER_H
#define ZCHXREGISTORCHECKER_H

#include <QObject>

class zchxRegistorChecker : public QObject
{
    Q_OBJECT
public:
    explicit zchxRegistorChecker(QObject *parent = 0);
    bool     startCheck(const QString& key = QString());
    QString     getMachineCode() const;
    void        removeKey() {removeRegedit(mRegKey);}

private:

    QString     getCPUId() const;
    QString     getDiskId() const;
    void    writeRegedit(QString key,QString val);
    QString    getRegedit(QString key);
    void    removeRegedit(QString key);

signals:

public slots:

private:
    QString     mRegOrg;
    QString     mRegApp;
    QString     mRegKey;
};

#endif // ZCHXREGISTORCHECKER_H
