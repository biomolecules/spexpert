#include "exptasklist.h"

/*!
   \namespace ExpTaskListTraits
   \brief Contains ExpTaskList traits
 */

/*!
   \enum ExpTaskListTraits::TaskType
   \brief This scoped enum contains the list of tasks for faster runtime
   identification.

   \sa TaskItem

   \var ExpTaskListTraits::None
       \brief none
   \var ExpTaskListTraits::ExpList
       \brief ExpTaskList
   \var ExpTaskListTraits::WaitList
       \brief WaitTaskList
   \var ExpTaskListTraits::WinSpecStart
       \brief WinSpecTasks::Start
   \var ExpTaskListTraits::StageControlRun
       \brief StageTasks::Run
   \var ExpTaskListTraits::StageControlToLimit
       \brief StageTasks::GoToLim
   \var ExpTaskListTraits::StageControlSendToPos
       \brief StageTasks::SendToPos
   \var ExpTaskListTraits::StageControlSetLimit
       \brief StageTasks::SetLim
   \var ExpTaskListTraits::StageControlSwitchPower
       \brief StageTasks::SwitchPower
   \var ExpTaskListTraits::StartWaiting
       \brief StartWaitingTask
   \var ExpTaskListTraits::WaitExp
       \brief WaitExpTask
   \var ExpTaskListTraits::Waiting
       \brief WaitingTask
 */

/*!
   \struct ExpTaskListTraits::TaskItem
   \brief Container for ExpTask attaching to it its type for faster runtime
   identification.

   \sa TaskType

   \var ExpTaskListTraits::TaskItem::task
       \brief the ExpTask
   \var ExpTaskListTraits::TaskItem::taskType
       \brief the TaskType

   \fn ExpTaskListTraits::TaskItem::TaskItem(TaskType tt, ExpTask * t)
       \brief Constructs the TaskItem stuct.
       \param tt type of the task
       \param t a pointer to the task
 */

/*!
   \class ExpTaskList
   \brief ExpTaskList
 */

/*!
   \brief Creates ExpTaskList object.
   \param parent A parent QObjet in Qt's ownership system.
 */
ExpTaskList::ExpTaskList(QObject *parent) :
    ExpTask(parent)
{
    blRunning = false;
    blStop = false;
}

/*!
   \brief Appends task to the task list.
   \param tsk appended task.
 */
void ExpTaskList::addTask(ExpTaskListTraits::TaskItem tsk)
{
    if (tsk.task->currTimesExec() > 0) {
        tasks.append(tsk);
    }
}

/*!
   \fn ExpTaskList::running() const
   \brief This method returns true if the ExpTaskList is executing its content.
   \return true if ExpTaskList is running.

   \sa start(), stop()
 */

/*!
   \brief This method returns true if the ExpTaskList is stopped by the stop()
   method.
   \return true if ExpTaskList is stopped.
 */
bool ExpTaskList::stopStatus() const
{
    return blStop;
}

/*!
   \brief This method returns true if ExpTaskList does not contain any
   ExpTaskListTraits::TaskItem.
   \return true if ExpTaskList is empty.
 */
bool ExpTaskList::isEmpty() const
{
    return tasks.isEmpty();
}

/*!
   \brief This method returns the current running task.
   \return current running task
 */
ExpTaskListTraits::TaskItem ExpTaskList::getCurrTask() const
{
    if (currTasks.isEmpty())
        return ExpTaskListTraits::TaskItem(ExpTaskListTraits::TaskType::None);
    else
        return currTasks.first();
}

/*!
   \fn ExpTaskList::getTasks() const
   \brief This method returns the reference to the list containing all stored
   tasks.
   \return list containing all stored tasks.
 */

/*!
   \fn ExpTaskList::stopTask()
   \brief This signal is emited form the stop() method and is connected in the
   nextTask() method to the ExpTask::stop() slot of the current running
   ExpTask.
 */

/*!
   \brief This method starts execution of the ExpTask object contained in the
   ExpTaskList.

   At first, this method controlls if the ExpTaskList is not already running.
   If yes, it skips the request, it the other case, it sets private variables
   indicating ExpTaskList state and calls nexTask() method.

   \sa stop(), taskFinished(), taskListFinished()
 */
void ExpTaskList::start()
{
    if (blRunning)
    {
    }
    else
    {
        blFailed = false;
        blStop = false;
        blRunning = true;
        currTasks = tasks;
        nextTask();
    }
}

/*!
   \brief This methods stops execution of the ExpTaskList and current running
   ExpTask through the emitting the stopTask() signal.

   After the current running ExpTask is finished, all the contained ExpTask
   objects are deleted and all the ExpTaskListTraits::TaskItem structs are
   removed from the list and the taskListFinished() method is called, which
   emits taskFailed() and finished() signals.
 */
void ExpTaskList::stop()
{
    if (!blStop)
    {
        blStop = true;
        emit stopTask();
    }
}

/*!
   \brief This method is executed after the ExpTaskList finishes execution
   of all contained ExpTask objects.

   It emits taskFailed() signal if this is a consequence of stop() method,
   while running() status is still true. Then it emits finished() signal,
   where the running() status is already false.
 */
void ExpTaskList::taskListFinished()
{
    blRunning = false;
    if (blStop || blFailed) {
        emit taskFailed();
    }
    emit finished();
}

/*!
   \brief Executes a next ExpTask. The ExpTaskListTraits::TaskItem objects
   which contain ExpTask objects are saved in ExpTaskList::currTasks, where
   first element is that, which will by next executed. If there is no task in
   list, taskListFinished() method is called. The ExpTask's ExpTask::finished()
   signal is connected to the taskFinished() slot and stopTask() signal is
   connected to the ExpTask's ExpTask::stop() slot. If the current ExpTask has
   not ExpTask::currTimesExec() greater than zero, the taskFinished() method is
   called.
 */
void ExpTaskList::nextTask()
{
    // controll if there is next task to be executed
    if (currTasks.isEmpty())
    {
        taskListFinished();
        return;
    }
    // resets the number of executions to initial value, repeating calling of
    // the task is handled by taskFinished() method.
    currTasks.first().task->restartTimesExec();

    // control if the task may be deleted later or directly after the execution.
    if (delayedDelete() || currTimesExec() > 1)
        currTasks.first().task->setDelayedDelete(true);
    else
        currTasks.first().task->setDelayedDelete(false);

    // control, if any executions of the task are planned.
    if (currTasks.first().task->currTimesExec() > 0)
    {
        connect(currTasks.first().task, &ExpTask::taskFailed, this, &ExpTaskList::onFailed);
        connect(currTasks.first().task, &ExpTask::finished, this, &ExpTaskList::taskFinished);
        connect(this, &ExpTaskList::stopTask, currTasks.first().task, &ExpTask::stop);
        currTasks.first().task->start();
    }
    else {
        // no execution
        taskFinished();
    }
}

/*!
   \brief This method is connected to the ExpTask::finished() signal of the
   current task and handles repeating of its execution until the intended
   number of executions is reached.

   At first, the method controlls if the task was stopped by the
   stop method and if so, it cleans up containing tasks and executes
   taskListFinished().

   Then it decreases ExpTask's execution counter by the ExpTask::onceExecuted()
   method and executes it once more if the counter is styl greater than one,
   else it disconnects the the ExpTask::finished() signal from this method and
   ExpTaskList::stopTask() signla from ExpTask::stop() method. If the task
   need to be deleted later, it calls ExpTask::restartTimesExec(), else
   it deletes task and removes it from tasks list. Then it calls nextTask().
 */
void ExpTaskList::taskFinished()
{
    // it the ExpTaskList was stopped by the stop() method, the contained tasks
    // are cleaned up
    if (blStop || blFailed)
    {
        QList<ExpTaskListTraits::TaskItem>::Iterator it;
        for (it = tasks.begin(); it != tasks.end(); ++it)
        {
            delete it->task;
        }
        tasks.clear();
        currTasks.clear();
        taskListFinished();
        return;
    }

    // decreases executions counter
    currTasks.first().task->onceExecuted();

    // if more executions are required
    if (currTasks.first().task->currTimesExec() > 0)
    {
        currTasks.first().task->start();
    }
    else
    {
        // disconnects already proccessed task.
        disconnect(currTasks.first().task, &ExpTask::finished, this, &ExpTaskList::taskFinished);
        disconnect(this, &ExpTaskList::stopTask, currTasks.first().task, &ExpTask::stop);

        // if not deleted immediatelly after execution
        if (currTimesExec() > 1 || delayedDelete())
            currTasks.first().task->restartTimesExec();
        else {
            // deletes task
            delete tasks.first().task;
            tasks.pop_front();
        }
        // removes task from actual task list
        currTasks.pop_front();

        // invokes new task
        nextTask();
    }
}

/*!
   \brief Slot connected to the current executing ExpTask::taskFailed() signal.
   It sets blFailed to true, which ensures stopping execution of the
   ExpTaskList and emitting taskFailed() signal.
 */
void ExpTaskList::onFailed()
{
    blFailed = true;
}

/*!
   \brief Destroyes ExpTaskList, deletes all ExpTaskListTraits::TaskItem
   objects from the task list.
 */
ExpTaskList::~ExpTaskList()
{
    QList<ExpTaskListTraits::TaskItem>::Iterator it;
    for (it = tasks.begin(); it != tasks.end(); ++it)
    {
        delete it->task;
    }
}

/*!
    \var ExpTaskList::tasks
        \brief Private variable containing all tasks to be executed.
    \var ExpTaskList::currTasks
        \brief Private varibale containing current task list. It do not store
        tasks which were already executed in current run even though they will
        be executed in next run of the list.
    \var ExpTaskList::blRunning
        \brief true if ExpTaskList is running
    \var ExpTaskList::blStop
        \brief true if stop() method is stopping or has stopped the ExpTaskList
        execution.
    \var ExpTaskList::blFailed
        \brief set to true if currently executed ExpTask emits
        ExpTask::taskFailed() signal, which causes emiting taskFailed() signal
        from this ExpTaskList.
 */

/*!
   \class ForkJoinTask
   \brief This class creates inside multiple ExpTaskList objects and runs each
   of them in parallel when started, waiting for them all to finish.
 */

/*!
   \brief Constructs ForkJoinTask object.
   \param threadsN The number of required threads
   \param parent A parent in Qt's ownership system.
 */
ForkJoinTask::ForkJoinTask(unsigned int threadsN, QObject *parent) :
    ExpTask(parent)
{
    running = false;
    if (threadsN > 0) {
        threadsN_ = threadsN;
        expTaskLists.resize(threadsN_);
        for (unsigned int ii = 0; ii < threadsN_; ++ii) {
            expTaskLists[ii] = new ExpTaskList(this);
            connect(expTaskLists[ii], &ExpTaskList::taskFailed, this, &ForkJoinTask::onFailed);
            connect(expTaskLists[ii], &ExpTaskList::finished, this, &ForkJoinTask::threadFinished);
            expTaskLists[ii]->setDelayedDelete(delayedDelete());
        }
    } else {
        threadsN_ = 1;
        expTaskLists.resize(threadsN_);
        expTaskLists[0] = new ExpTaskList(this);
        connect(expTaskLists[0], &ExpTaskList::taskFailed, this, &ForkJoinTask::onFailed);
        connect(expTaskLists[0], &ExpTaskList::finished, this, &ForkJoinTask::threadFinished);
        expTaskLists[0]->setDelayedDelete(delayedDelete());
    }
}

/*!
   \brief Propagates information about the finished execution to the all
   contained threads. See also ExpTask::onceExecuted().
 */
void ForkJoinTask::onceExecuted()
{
    if (currTimesExec() == 2 && !delayedDelete()) {
        for (unsigned int ii = 0; ii < threadsN_; ++ii) {
            expTaskLists[ii]->setDelayedDelete(false);
        }
    }
    ExpTask::onceExecuted();
}

/*!
   \brief Adds task to the particular thread by the ExpTaskList::addTask()
   method.
   \param tsk added task
   \param threadno thread number
 */
void ForkJoinTask::addTask(ExpTaskListTraits::TaskItem tsk, unsigned int threadno)
{
    if (threadno < threadsN_) {
        expTaskLists.at(threadno)->addTask(tsk);
    }
}

/*!
   \brief Propagates the desired number of executions to the all contained
   threads. See also ExpTask::setTimesExec()
 */
void ForkJoinTask::setTimesExec(int timesExecuted)
{
    if (timesExecuted > 1) {
        for (unsigned int ii = 0; ii < threadsN_; ++ii) {
            expTaskLists[ii]->setDelayedDelete(true);
        }
    }
    ExpTask::setTimesExec(timesExecuted);
}

/*!
   \brief Propagates the delayedDelete() attribute to all the contained
   threads. See also ExpTask::setDelayedDelete().
 */
void ForkJoinTask::setDelayedDelete(bool blDelayedDelete)
{
    if (delayedDelete() != blDelayedDelete) {
        for (unsigned int ii = 0; ii < threadsN_; ++ii) {
            expTaskLists[ii]->setDelayedDelete(blDelayedDelete);
        }
        ExpTask::setDelayedDelete(blDelayedDelete);
    }
}

/*!
   \fn ForkJoinTask::getThreadsN()
   \brief Gets number of threads.
 */

/*!
   \brief Starts all contained threads (ExpTaskList objects).
 */
void ForkJoinTask::start()
{
    if (!running) {
        running = true;
        stopped = false;
        allThreadsFinished = false;
        allThreadsStopped = false;
        runningThreads_ = threadsN_;
//        runningExpTaskLists.clear();
        for (unsigned int ii = 0; ii < threadsN_; ++ii) {
//            runningExpTaskLists.append(expTaskLists[ii]);
            expTaskLists[ii]->start();
        }
    }
}

/*!
   \brief Stops all contained threads (ExpTaskList objects).
 */
void ForkJoinTask::stop()
{
    stopped = true;
    for (unsigned int ii = 0; ii < threadsN_; ++ii) {
        expTaskLists[ii]->stop();
    }
    if (allThreadsFinished) {
        running = false;
        emit taskFailed();
        emit finished();
    } else {
        allThreadsStopped = true;
    }
}

/*!
   \brief The ExpTaskList::finished() signals of the all contained threads
   are connected to this slot. It decrements the ForkJoinTask::runningThreads_
   counter and when all the threads are finished, it emits finished() signal.
 */
void ForkJoinTask::threadFinished()
{
//    ExpTaskList *expTaskList = static_cast<ExpTaskList *>(QObject::sender());
//    if (expTaskList) {
//        runningExpTaskLists.removeOne(expTaskList);
//    }
    if (--runningThreads_ < 1) {
        if (stopped && !allThreadsStopped) {
            allThreadsFinished = true;
        } else {
            running = false;
            if (stopped) {
                emit taskFailed();
            }
            emit finished();
        }
    }
}

void ForkJoinTask::onFailed()
{
    if (!stopped) {
        stop();
    }
}

/*!
   \brief Gets all tasks from the particular thread (ExpTaskList objects).
 */
const QList<ExpTaskListTraits::TaskItem> &ForkJoinTask::getTasks(unsigned int threadno) const
{
    if (threadno < threadsN_) {
        return expTaskLists.at(threadno)->getTasks();
    } else {
        return expTaskLists.at(0)->getTasks();
    }
}

/*!
   \brief Destroyes ForkJoinTask object.
 */
ForkJoinTask::~ForkJoinTask()
{
    for (unsigned int ii = 0; ii < threadsN_; ++ii) {
        delete expTaskLists[ii];
    }
}

/*!
   \var ForkJoinTask::expTaskLists
       \brief contained threads (ExpTaskList objects).
   \var ForkJoinTask::threadsN_
       \brief a number of contained threads (ExpTaskList objects).
   \var ForkJoinTask::runningThreads_
       \brief a number of currently running threads. It is set in start()
       method to the threadsN_ number and decremented in the threadFinished()
       slot.
   \var ForkJoinTask::stopped
       \brief true, if the stop slot was invoked to signal to threadFinished()
       slot, that it may wait for stopping all the contained threads before
       emitting finished() signal earlier before the threads, which were
       already finished is stopped too.
   \var ForkJoinTask::allThreadsStopped
       \brief true, if all threads have been already stopped, but not finished
       yet. See also ForkJoinTask::stopped.
   \var ForkJoinTask::allThreadsFinished
       \brief true, if all threads have already finished but not all were
       stopped. It can happen, when the thread finished before the stop method
       was called. See also ForkJoinTask::stoped.
   \var ForkJoinTask::running
       \brief true, if ForkJoinTask is running. When it is running, it can't
       be started once more. It protects internal boolean indicators from not
       to be flawed.
 */
