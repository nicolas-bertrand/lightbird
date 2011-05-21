#ifndef TIMER_H
# define TIMER_H

# include <QString>
# include <QThread>

class ApiTimers;

class Timer : public QThread
{
    Q_OBJECT

public :
    Timer(QString id, QString name, unsigned timeout, ApiTimers &apiTimers);
    ~Timer();
    Timer(const Timer &timer);
    Timer &operator=(const Timer &timer);

    void        run();
    unsigned    getTimeout();
    void        setTimeout(unsigned int timeout);
    void        stop();

private slots:
    void        _timeout();

private:
    Timer();

    QString     id;
    QString     name;
    unsigned    timeout;
    ApiTimers   &apiTimers;
    bool        stopped;
};

#endif // TIMER_H
