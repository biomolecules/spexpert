#ifndef EXPTASKLIST_H
#define EXPTASKLIST_H

#include "exptask.h"
#include <QList>
#include <QVector>

namespace ExpTaskListTraits
{

enum class TaskType {
    None,
    ExpList,
    ForkJoin,
    WaitList,
    WinSpecStart,
    WinSpecLogLastExpParams,
    WinSpecAddExpNumber,
    StageControlRun,
    StageControlToLimit,
    StageControlSendToPos,
    StageControlSetLimit,
    StageControlSwitchPower,
    StageControlGoToPosExpList,
    StartWaiting,
    FinishWaiting,
    WaitExp,
    Waiting,
    NeslabSetT,
    NesalbReadT,
    GratingSendToPos,
    SaveLog,
    WinSpecExpList,
    WholeExtExpList,
    TExpList,
    ExtTExpList,
    WholeTExpList,
    BatchExpList,
    WholeBatchExpList
};

struct TaskItem
{
    ExpTask * task;
    TaskType taskType;

    TaskItem(TaskType tt = TaskType::None, ExpTask * t = nullptr) : task(t), taskType(tt) {}
};

} // ExpTaskListTraits namespace

class ExpTaskList : public ExpTask
{
    Q_OBJECT
public:
    explicit ExpTaskList(QObject *parent = 0);
    virtual ~ExpTaskList();
    virtual void addTask(ExpTaskListTraits::TaskItem tsk);
    inline bool running() const { return blRunning; }
    bool stopStatus() const;
    bool isEmpty() const;

    // getters
    ExpTaskListTraits::TaskItem getCurrTask() const;
    const QList<ExpTaskListTraits::TaskItem> & getTasks() const { return tasks; }


signals:
    void stopTask();

public slots:
    virtual void start();
    virtual void stop();

protected:
    virtual void taskListFinished();
    virtual void nextTask();

protected slots:
    virtual void taskFinished();
    virtual void onFailed();

private:
    QList<ExpTaskListTraits::TaskItem> tasks;
    QList<ExpTaskListTraits::TaskItem> currTasks;
    bool blRunning;
    bool blStop;
    bool blFailed;
};

class ForkJoinTask : public ExpTask
{
    Q_OBJECT

public:
    explicit ForkJoinTask(unsigned int threadsN = 2, QObject *parent = 0);
    virtual ~ForkJoinTask();
    virtual void onceExecuted();

    void addTask(ExpTaskListTraits::TaskItem tsk, unsigned int threadno);

    // getters
    unsigned int getThreadsN() const { return threadsN_; }

    // setters
    virtual void setTimesExec(int timesExecuted);
    virtual void setDelayedDelete(bool blDelayedDelete);

public slots:
    virtual void start();
    virtual void stop();

protected slots:
    void threadFinished();
    void onFailed();

protected:
    friend class WaitTaskList;
    const QList<ExpTaskListTraits::TaskItem> & getTasks(unsigned int threadno) const;

private:
    QVector<ExpTaskList* > expTaskLists;
//    QList<ExpTaskList* > runningExpTaskLists;
    unsigned int threadsN_;
    unsigned int runningThreads_;
    bool stopped;
    bool allThreadsStopped;
    bool allThreadsFinished;
    bool running;
    bool failed;
};

#endif // EXPTASKLIST_H
