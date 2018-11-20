#ifndef WAITTASKLIST_H
#define WAITTASKLIST_H

#include "exptask.h"
#include "exptasklist.h"
#include <QObject>
#include <QSet>
#include <QVector>
#include <QMap>

// forward declaration
class QThread;
class QMutex;
class QTimer;
class WaitWorker;
class WaitTask;

namespace WaitTaskListTraits
{
enum struct WaitFor : unsigned char
{
    None    = 0,
    Delay   = 1 << 0,
    WinSpec = 1 << 1,
    Motor   = 1 << 2,
    Grating = 1 << 3,
    Lamp    = 1 << 4,
    Other1  = 1 << 5,
    Other2  = 1 << 6,
    Other3  = 1 << 7
};

inline constexpr WaitFor operator|(WaitFor a, WaitFor b) {
    return static_cast<WaitFor>(static_cast<unsigned char>(a) | static_cast<unsigned char>(b)); }

inline constexpr WaitFor operator&(WaitFor a, WaitFor b) {
    return static_cast<WaitFor>(static_cast<unsigned char>(a) & static_cast<unsigned char>(b)); }

inline WaitFor operator~(WaitFor a) { return static_cast<WaitFor>(~static_cast<unsigned char>(a)); }

inline WaitFor& operator|=(WaitFor& a, WaitFor b) { return a = (a | b); }

inline WaitFor& operator&=(WaitFor& a, WaitFor b) { return a = (a & b); }

struct TaskItem
{
    WaitTask * task;
    WaitFor waitFor;

    TaskItem(WaitFor wf = WaitFor::None, WaitTask * wt = nullptr) : task(wt), waitFor(wf) {}
};

}

class WaitExpTask : public ExpTask
{
    Q_OBJECT

public:
    explicit WaitExpTask(const WaitTaskListTraits::TaskItem &waitTask, QObject *parent = 0);
    virtual ~WaitExpTask();
    virtual void setDelayedDelete(bool blDelayedDelete);

    // getters
    WaitTaskListTraits::TaskItem getWaitTaskItem() { return waitTaskItem; }

signals:
    void startWaitTask(unsigned int id);

public slots:
    virtual void start();
    virtual void stop();


private:
    WaitTaskListTraits::TaskItem waitTaskItem;
};

class WaitingTask : public ExpTask
{
    Q_OBJECT

public:
    explicit WaitingTask(QObject *parent = 0);
    virtual ~WaitingTask();

    virtual void restartTimesExec();
    void setIds(const QSet<unsigned int> & ids) { ids_ = ids; currIds_ = ids_; }

signals:
    void waitTaskFinished(unsigned int id);

public slots:
    void onWaitTaskFinished(unsigned int id);
    virtual void start();
    virtual void stop();

private:
    QSet<unsigned int> currIds_;
    QSet<unsigned int> ids_;
};

class WaitTaskList : public ExpTaskList
{
    Q_OBJECT
public:
    explicit WaitTaskList(QObject *parent = 0);
    ~WaitTaskList();
    virtual void addTask(ExpTaskListTraits::TaskItem tsk);

    // getters
    bool quit() { return blQuit_; }
    bool taskListIsFinished() { return blTaskListFinished_; }
    bool waitWorkerIsFinished() { return blWaitWorkerFinished_; }

    // setters
    void setQuit(bool blQuit) { blQuit_ = blQuit; }
    void setTaskListFinished(bool blTaskListFinished) { blTaskListFinished_ = blTaskListFinished; }
    void setWaitWorkerFinished(bool blWaitWorkerFinished) { blWaitWorkerFinished_ = blWaitWorkerFinished; }

signals:
    void startWaitTaskInWorker(WaitTaskListTraits::TaskItem waitTaskItem);
    void stopWorker();

public slots:
    virtual void start();
    virtual void stop();
    void onStartWaitTask(unsigned int id);
    void onWaitTaskFinished(unsigned int id);
    void onWorkerQuitFinished();

protected:
    virtual void taskListFinished();
    void addWaitTask(const ExpTaskListTraits::TaskItem &tsk, QSet<unsigned int> *currNewIds = 0);

private:
    unsigned int getId();
    void buildThread();

    QThread *thread;
    WaitWorker *waitWorker;
    QMap<unsigned int, WaitTaskListTraits::TaskItem> waitTasks;
    QSet<unsigned int> newIds;
    bool blQuit_;
    bool blTaskListFinished_;
    bool blWaitWorkerFinished_;
};

class WaitWorker : public QObject
{
    Q_OBJECT

public:
    WaitWorker(QObject *parent = 0);
    virtual ~WaitWorker();

signals:
    void quitFinished();

public slots:
    void stop();
    void addWaitTask(const WaitTaskListTraits::TaskItem &wt);

private slots:
    void addDelayedWaitTask(const WaitTaskListTraits::TaskItem &wt);
    void waitWorkerLoop();

private:
    void quitLoop();

    QTimer *timer;
    QMap<unsigned int, WaitTaskListTraits::TaskItem> waitTasks_;
    QList<unsigned int> markedForDelete;

    int refreshRate_;
};

class DelayedStart : public QObject
{
    Q_OBJECT

public:
    DelayedStart(const WaitTaskListTraits::TaskItem & waitTaskItem, QObject * parent = 0);
    ~DelayedStart();

signals:
    void addTask(WaitTaskListTraits::TaskItem waitTaskItem);

public slots:
    void delayedAddWaitTask();

private:
    WaitTaskListTraits::TaskItem waitTaskItem_;
};

Q_DECLARE_METATYPE(WaitTaskListTraits::TaskItem)

#endif // WAITTASKLIST_H
