#ifndef EXPTASK_H
#define EXPTASK_H

#include <QObject>

class ExpTask : public QObject
{
    Q_OBJECT

public:
    explicit ExpTask(QObject *parent = 0);
    virtual ~ExpTask() {}
    virtual void onceExecuted();
    virtual void restartTimesExec();

    // getters
    int currTimesExec() const;
    int timesExec() const;
    bool delayedDelete() const;

    // setters
    virtual void setTimesExec(int timesExecuted);
    virtual void setDelayedDelete(bool blDelayedDelete);

signals:
    void finished();
    void taskFailed();

protected:

public slots:
    virtual void start() = 0;
    virtual void stop();

private:
    int timesExec_;
    int currTimesExec_;
    bool blDelayedDelete_;

};

#endif // EXPTASK_H
