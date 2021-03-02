#ifndef ZCHXVALIDATIONTHREAD_H
#define ZCHXVALIDATIONTHREAD_H

#include <QThread>

class zchxValidationThread : public QThread
{
    Q_OBJECT
public:
    explicit zchxValidationThread(QObject *parent = 0);
protected:
    void run();

signals:
    void     signalAppExpired();
public slots:
};

#endif // ZCHXVALIDATIONTHREAD_H
