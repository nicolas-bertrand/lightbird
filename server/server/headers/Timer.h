#ifndef TIMER_H
# define TIMER_H

# include <QThread>

class Timer : public QThread
{
    Q_OBJECT

public :
    Timer(QString id, QString name, unsigned timeout, QObject *parent = 0);
    ~Timer();
    Timer(const Timer &timer);
    Timer &operator=(const Timer &timer);

    void            run();
    unsigned int    getTimeout();
    void            setTimeout(unsigned int timeout);
    void            stop();

private slots:
    void    _timeout();

private:
    Timer();

    QString     id;
    unsigned    timeout;
    QString     name;
    QObject     *parent;
    bool        stopped;
};

#endif // TIMER_H
