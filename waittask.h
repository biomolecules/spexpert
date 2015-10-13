#ifndef WAITTASK_H
#define WAITTASK_H

#include <QObject>

// forward declarations
class QMutex;

class WaitTask : public QObject
{
    Q_OBJECT

public:
    explicit WaitTask(QObject *parent = 0);
    virtual ~WaitTask();
    virtual bool running() = 0;

    // getters
    int initialDelay() const { return initDelay_; }
    bool delayedDelete() const;
    unsigned int id() const { return id_; }
    bool handled() const { return handled_;}


    // setters
    void setInitialDelay(int initDelay) { initDelay_ = initDelay; }
    void setDelayedDelete(bool blDelayedDelete);
    void setId(unsigned int uintId) { id_ = uintId; }
    void setHandled(bool blHandled) { handled_ = blHandled; }

signals:
    void waitTaskFinished(unsigned int uintId);
    void waitingFailed();

public slots:
    virtual void start() { }
    virtual void stop() { emit waitingFailed(); finish(); }
    virtual void finish() { emit waitTaskFinished(id()); }

private:
    unsigned int id_;

    int initDelay_; // initial delay in ms
    bool blDelayedDelete_;
    QMutex * delayedDeleteMutex;
    bool handled_;
};

#endif // WAITTASK_H
