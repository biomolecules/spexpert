#include "waittasklist.h"
#include "waittask.h"
#include "waittasks.h"

#include <QMutex>
#include <QThread>
#include <QTimer>

/*!
   \namespace WaitTaskListTraits
   \brief Namespace containing WaitTaskList's enums and helper structures
 */

/*!
   \enum WaitTaskListTraits::WaitFor
   \brief This scoped enum contains the list of tasks for faster runtime
   identification.

   \sa TaskItem

   \var WaitTaskListTraits::None
       \brief None
   \var WaitTaskListTraits::Delay
       \brief DelayWaitTask
   \var WaitTaskListTraits::WinSpec
       \brief WinSpecWaitTask
   \var WaitTaskListTraits::Motor
       \brief StageControlWaitTask
   \var WaitTaskListTraits::Spectrograph
       \brief for future use
   \var WaitTaskListTraits::Other1
       \brief for future use
   \var WaitTaskListTraits::Other2
       \brief for future use
   \var WaitTaskListTraits::Other3
       \brief for future use
   \var WaitTaskListTraits::Other4
       \brief for future use

   \fn WaitTaskListTraits::operator|(WaitTaskListTraits::WaitFor a, WaitTaskListTraits::WaitFor b)
       \brief logical OR operator

   \fn WaitTaskListTraits::operator&(WaitTaskListTraits::WaitFor a, WaitTaskListTraits::WaitFor b)
       \brief logical AND operator

   \fn WaitTaskListTraits::operator~(WaitTaskListTraits::WaitFor a)
       \brief logical negation operator

   \fn WaitTaskListTraits::operator|=(WaitTaskListTraits::WaitFor& a, WaitTaskListTraits::WaitFor b)
       \brief logical assignment OR operator

   \fn WaitTaskListTraits::operator&=(WaitTaskListTraits::WaitFor& a, WaitTaskListTraits::WaitFor b)
       \brief logical assignment AND operator
 */

/*!
   \struct WaitTaskListTraits::TaskItem
   \brief Container for WaitTask attaching to it its type for faster runtime
   identification.

   \sa TaskType

   \var WaitTaskListTraits::TaskItem::task
       \brief the WaitTask
   \var WaitTaskListTraits::TaskItem::waitFor
       \brief the WaitFor

   \fn WaitTaskListTraits::TaskItem::TaskItem(WaitFor wf, WaitTask * wt)
       \brief Constructs the TaskItem stuct.
       \param wf type of the task
       \param wt a pointer to the task
 */

/*!
   \class WaitExpTask

   \brief This class contains the WaitTask in the WaitTaskListTraits::TaskItem,
   which will be submited to the list of tasks which will be waited for, when
   added to the WaitTaskList. Each WaitTask should be handled by some
   WaitingTask to have some effect, so there shouldn't be any WaitExpTask added
   to the WaitTaskList without WaitingTask added somewhere after it.
 */

/*!
   \brief Constructs WaitExpTask object.
 */
WaitExpTask::WaitExpTask(const WaitTaskListTraits::TaskItem & waitTask, QObject *parent) :
    ExpTask(parent), waitTaskItem(waitTask)
{
}

/*!
   \brief Sets delayed destruction of the task also for the contained WaitTask.
   Viz ExpTask::setDelayedDelete()
 */
void WaitExpTask::setDelayedDelete(bool blDelayedDelete)
{
    waitTaskItem.task->setDelayedDelete(blDelayedDelete);
    ExpTask::setDelayedDelete(blDelayedDelete);
}

/*!
   \fn WaitExpTask::getWaitTaskItem()
   \brief Returns contained WaitTaskListTraits::TaskItem. This method is used
   in WaitTaskList::addTask(), where the contained WaitTask obtaines its id.
 */

// signals
/*!
   \fn WaitExpTask::startWaitTask(unsigned int id)
   \brief This signal is emited from the start() method, and in WaitTaskList is
   connected to the WaitTaskList::onStartWaitTask() which submits contained
   WaitTask to the WaitWorker.
 */

/*!
   \brief emits startWaitTask() signal, to start waiting for this task and
   finished() signal to finish this task. The waiting for the task is then
   handled by the WaitingTask.
 */
void WaitExpTask::start()
{
    emit startWaitTask(waitTaskItem.task->id());
    emit finished();
}

/*!
   \brief stops adding of the WaitTask to the WaitWorker. Do not use it.
 */
void WaitExpTask::stop()
{
    ExpTask::stop();
}

/*!
   \brief WaitExpTask::~WaitExpTask
 */
WaitExpTask::~WaitExpTask()
{
}

/*!
   \var WaitExpTask::waitTaskItem
       \brief contained WaitTaskItem
 */

/*!
   \class WaitingTask

   \brief Handles waiting for all WaitExpTask objects previously added to the
   WaitTaskList.
 */

/*!
   \brief Constructs WaitingTask object.
   \param parent Parent QObject in Qt's ownership system.
 */
WaitingTask::WaitingTask(QObject *parent) :
    ExpTask(parent)
{
}

/*!
   \brief This method resets the number of planned executions of the
   WaitingTask resetting also the WaitTask objects which will be waited for.
 */
void WaitingTask::restartTimesExec()
{
    currIds_ = ids_;
    ExpTask::restartTimesExec();
}

/*!
   \fn WaitingTask::setIds(const QSet<unsigned int> & ids)
   \brief This method sets the ids of WaitTask objects which will be waited
   for. It is used in WaitTaskList::addWaitTask() method which is called from
   the WaitTaskList::addTask() method.
   \param ids list of ids of tasks which will be waited for.
 */

// signals
/*!
   \fn WaitingTask::waitTaskFinished(unsigned int id)
   \brief This signal is emited from the onWaitTaskFinished() method and
   connected to the WaitTaskList::onWaitTaskFinished() in the
   WaitTaskList::addWaitTask() method which is called from the
   WaitTaskList::addTask() method.
   \param id of finished WaitTask
 */


// slots
/*!
   \brief This slot is connected to all WaitTask object, which was added
   before the addition of this WaitingTask and not already handled by any
   preceeding WaitingTask. It emits waitTaskFinished() signal and if all
   WaitTask objects which are connected to this WaitingTask have been already
   executed, it emits finished() signal.
 */
void WaitingTask::onWaitTaskFinished(unsigned int id)
{
    currIds_.remove(id);
    emit waitTaskFinished(id);
    if (currIds_.isEmpty())
    {
        emit finished();
    }
}

/*!
   \brief This method starts the waiting for all connected WaitTask objects.
   See onWaitTaskFinished() slot.
 */
void WaitingTask::start()
{
    if (currIds_.isEmpty())
    {
        emit finished();
    }
}

/*!
   \brief Stops the waiting.
 */
void WaitingTask::stop()
{
    ExpTask::stop();
}

/*!
   \brief Destoryes WaitingTask object.
 */
WaitingTask::~WaitingTask()
{
}

/*!
   \var WaitingTask::currIds_
       \brief Ids of WaitTask object, which are or will be currently waited
       for.
   \var WaitingTask::ids_
       \brief Ids of all WaitTask objects which are connected to this
       WaitingTask
 */

/*!
   \class WaitTaskList
   \brief This class provides interface for executing tasks which waits for
   some process to be finished.

   It behaves like ExpTaskList but for cases when
   tasks with special meaning is added: WaitExpTask, WaitingTask, ExpTaskList,
   ForkJoinTask.

   When WaitExpTask is added, the contained WaitTask is extracted by the
   WaitExpTask::getWaitTaskItem() method and unique id is attributed to this
   WaitTask and WaitExpTask::startWaitTask() signal is connected to the
   onStartWaitTask() slot.

   When WaitingTask is added, all the previously added WaitTask objects, which
   have not been handled by any WaitingTask are connected to this WaitingTask
   by adding their ids by WaitingTask::setIds() method and connecting their
   WaitTask::waitTaskFinished() to the WaitingTask::onWaitTaskFinished() slot.
   The WaitingTask's WaitingTask::waitTaskFinished() signal is connected to the
   WaitTaskList::onWaitTaskFinished() which handles clean up of WaitTask
   object of no further use. The added WaitTask is also set as handled by the
   WaitTask::setHandled() method.

   When ExpTaskList is added, all the contained ExpTask objects are extracted
   from it and submited recursively to the same process of ExpTask adding as
   described above.

   When ForkJoinTask is added, all the contained ExpTask objects from all the
   contained threads are extracted from it and submited recursively to the same
   process of ExpTask adding as described above. The threads is walked through
   succesively from the thread no. 0. This means, that all so far unhandled
   WaitTask objects will be handled by the first encounteder Waiting task in
   the thread with lowest number.

   When, the start() slot is invoked, the underlying QThread and ExpTaskList is
   started. When the WaitExpTask comes to execution, the
   WaitTaskList::onStartWaitTask() slot is executed, where the inner WaitTask
   connection to some WaitingTask is controlled and if so, the WaitTask is
   submited to the WaitWorker by emiting WaitTaskList::startWaitTaskInWorker()
   signal.

   WaitWorker waits WaitTask's WaitTask::initialDelay() and then runs
   WaitTask::start() slot. Then it periodically controlls WaitTask::running().
   If it returns false, it executes WaitTask::finish() slot. This slot emits
   WaitTask::waitTaskFinished() signal which is connected to the
   WaitingTask::onWaitTaskFinished() slot in which the WaitTask is removed from
   the list of tasks, the WaitingTask is waiting for and then the WaitingTask
   emits WaitingTask::waitTaskFinished() signal which is connected to the
   WaitTaskList::onWaitTaskFinished() slot. In this slot, it is decided,
   if the WaitTask should be deleted instantly or the deletion will be delayed,
   see ExpTask::delayedDelete() and WaitTask::delayedDelete().

   When the WaitingTask comes to execution, its execution will not be finished
   until the all WaitTask's connected to this WaitingTask is not finished.

   When the ForkJoinTask comes to execution, its execution will not be finished
   until all threads is not finished (this is a same behaviour of ForkJoinTask
   as if it is only in the ExpTaskList).

   When all ExpTasks are executed, the ExpTaskList's taskListFinished() method
   is reimplemented to delete the contained thread is deleted (only if the
   delayedDelete() is false and curTimesExec() is 1 or less).

   The execution can be stopped by the stop() slot.
 */

/*!
   \brief Creates WaitTaskList object.
   \param parent Parent in Qt's ownership system.
 */
WaitTaskList::WaitTaskList(QObject *parent) :
    ExpTaskList(parent)
{
    thread = nullptr;
    blQuit_ = false;

    // variables, which are used when the execution is interupted to wait for
    // ending both the contained ExpTaskList and WaitWorker.
    blTaskListFinished_ = false;
    blWaitWorkerFinished_ = false;
}

/*!
   \brief Adds task to the list and calls the addWaitTask() method, which
   treats the special cases connected to the handling of WaitTask objects.
 */
void WaitTaskList::addTask(ExpTaskListTraits::TaskItem tsk)
{
    if (tsk.task->currTimesExec() > 0) {
        addWaitTask(tsk);
        ExpTaskList::addTask(tsk);
    }
}

/*!
    \fn WaitTaskList::quit()
        \brief returns true if setQuit() was set to true.
    \fn WaitTaskList::taskListIsFinished()
        \brief returns true if setTaskListFinished() was set to true.
    \fn WaitTaskList::waitWorkerIsFinished()
        \brief returns true if setWaitWorkerFinished() was set to true.
    \fn WaitTaskList::setQuit(bool blQuit)
        \brief setQuit()
    \fn WaitTaskList::setTaskListFinished(bool blTaskListFinished)
        \brief setTaskListFinished()
    \fn WaitTaskList::setWaitWorkerFinished(bool blWaitWorkerFinished)
        \brief setWaitWorkerFinished()
 */

// signals
/*!
    \fn WaitTaskList::startWaitTaskInWorker(WaitTaskListTraits::TaskItem waitTaskItem)
        \brief Emitted from the onStartWaitTask() method and connected to the
        WaitTaskList::waitWorker's WaitWorker::addWaitTask() slot.
    \fn WaitTaskList::stopWorker()
        \brief Emitted from the stop() slot and from the destructor, to stop
        WaitWorker. It is connected to the WaitWorker's WaitWorker::stop()
        slot.
 */

/*!
   \brief Starts execution of the ExpTask objects in WaitTaskList and builds
   the new thread, which can execute all WaitTask's which are wrapped in the
   WaitExpTask objects inserted in the WaitTaskList. The execution can be
   stopped by the stop() method. See addTask() for more details.
 */
void WaitTaskList::start()
{
    if (!running()) {
        setTaskListFinished(false);
        setWaitWorkerFinished(false);
        if (thread)
        {
            ExpTaskList::start();
        }
        else
        {
            buildThread();
            ExpTaskList::start();
        }
    }
}

/*!
   \brief Stops the WaitTaskList execution.
 */
void WaitTaskList::stop()
{
    if (running()) {
        setQuit(true);
        if (thread) {
            emit stopWorker();
        }
        ExpTaskList::stop();
    }
}

/*!
   \brief This slot is connected to WaitExpTask::startWaitTask() signals of the
   all contained WaitExpTask objects and if the corresponding WaitTask is
   handled by some WaitingTask, it is added to the WaitWorker to be waited for.

   \param id Id of WaitTask in WaitTaskList's id system.
 */
void WaitTaskList::onStartWaitTask(unsigned int id)
{
    WaitTaskListTraits::TaskItem waitTaskItem = waitTasks.value(id);
//    if (waitTaskItem.waitFor == WaitTaskListTraits::WaitFor::Delay) {
//        qDebug() << "WaitTaskList::onStartWaitTask: delay id" << id;
//    }
//    if (waitTaskItem.waitFor == WaitTaskListTraits::WaitFor::WinSpec) {
//        qDebug() << "WaitTaskList::onStartWaitTask: WinSpec id" << id;
//    }
//    if (waitTaskItem.waitFor == WaitTaskListTraits::WaitFor::Motor) {
//        qDebug() << "WaitTaskList::onStartWaitTask: motor id" << id;
//    }

    if (waitTaskItem.task->handled()) {
        emit startWaitTaskInWorker(waitTaskItem);
    }
}

/*!
   \brief This slot is connected to the WaitingTask::waitTaskFinished() signal
   which is emited from the current running WaitingTask::onWaitTaskFinished()
   slot connected to the WaitTask::waitTaskFinished() signal of the currently
   executed WaitTask objects.
   \param id Id of WaitTask in WaitTaskList's id system.
 */
void WaitTaskList::onWaitTaskFinished(unsigned int id)
{
    WaitTaskListTraits::TaskItem waitTaskItem = waitTasks.value(id);
    if (currTimesExec() < 2 && !delayedDelete() && waitTaskItem.task && !waitTaskItem.task->delayedDelete()) {
        waitTasks.remove(id);
        delete waitTaskItem.task;
    }
}

/*!
   \brief This method is invoked only when the thread was quited by calling
   waitWorker's WaitWorker::stop() method in a reaction to the
   WaitWorker::quitFinished() signal.

   The WaitWorker::stop() method is called only after stop() is invoked and
   from the WaitTaskList::~WaitTaskList() destructor. It deletes thread and all
   the contained WaitTask objects.

   If this method is called earlier than the taskListFinished() (the
   taskListIsFinished() is false), the waitWorkerFinished() is set to true by
   the setWaitWorkerFinished(true) method. In the opposite case, the
   taskListFinished() method is called once more, to finish WaitTaskList
   finishing.
 */
void WaitTaskList::onWorkerQuitFinished()
{
    // it controls if WaitWorker was finished by the WaitTaskList::stop()
    // method, which sets quit() to true or by the destructor, which leaves
    // quit at false value.
    if (quit()) {
        if (thread) {
            thread->quit();
            thread->wait();
            delete thread;
            thread = nullptr;
        }
        for (QMap<unsigned int, WaitTaskListTraits::TaskItem>::iterator it = waitTasks.begin(); it != waitTasks.end(); ++it) {
            delete it->task;
        }
        waitTasks.clear();
        // this controls, if ExpTaskList has already finished.
        if (taskListIsFinished()) {
            setTaskListFinished(false);
            setQuit(false);
            ExpTaskList::taskListFinished();
        } else {
            setWaitWorkerFinished(true);
        }
    }
}

/*!
   \brief This is reimplemented method of the ExpTaskList::taskListFinished().

   If this method is invoked after all tasks was finished, the contained thread
   is deleted and all remaining WaitTask objects, which have been not connected
   to some WaitingTask is discarded (only when no repetetion of the execution
   of this task is planned or the delayedDelete() is not set. See
   ExpTask::delayedDelete() for more details). Then the finishing process is
   ended by executing ExpTaskList::taskListFinished(), which emits the
   finished() signal.

   If this method is invoked in consequence of stop() method calling (quit() is
   true), it is controlled if the onWorkerQuitFinished() method has already
   been executed. If no, the taskListFinished() is set to true (by the
   setTaskListFinished() method) and the finishing process waits for the
   WaitWorker to finish. If the onWorkerQuitFinished() has already been
   executed, the finishing process is ended by executing
   ExpTaskList::taskListFinished(), which emits the finished() signal.

   \warning This means, that these two methods can't be called from different
   threads and must be called only in connection to some signal. Else you risk
   the deadlock.
 */
void WaitTaskList::taskListFinished()
{
    // controlls, if this is consequence to calling stop() method.
    if (quit()) {
        // controlls, if WaitWorker has already finished
        if (waitWorkerIsFinished()) {
            setWaitWorkerFinished(false);
            setQuit(false);
            ExpTaskList::taskListFinished();
        } else {
            setTaskListFinished(true);
        }
    } else {
        // if it was last run of this WaitWorker, everything is cleaned up
        if (currTimesExec() < 2 && !delayedDelete()) {
            if (thread) {
                thread->quit();
                thread->wait();
                delete thread;
                thread = nullptr;
            }
            if (!waitTasks.isEmpty()) {
                for (QMap<unsigned int, WaitTaskListTraits::TaskItem>::iterator it = waitTasks.begin(); it != waitTasks.end(); ++it) {
                    delete it->task;
                }
                waitTasks.clear();
            }
        }
        ExpTaskList::taskListFinished();
    }
}

/*!
   \brief This method is called from addTask() to controll, if new added task
   is not a case which requires special treatment (WaitExpTask, WaitingTask,
   ExpTaskList, ForkJoinTask). See description of the class for more details.

   \param tsk new task to be added
   \param currNewIds custom new ids list

   If you want your own new ids list, which is used for connecting new WaitTask
   objects to the WaitingTask, provide the optional parameter currNewIds. For
   example if you want to treat tasks from some list separately and do not want
   to connect external WaitTask object to WaitingTask object contained in this
   list

   Example of overriding of addWaitTask():
   \code
void WaitTaskList::addWaitTask(const ExpTaskListTraits::TaskItem &tsk, QSet<unsigned int> *currNewIds)
{
    if (!currNewIds) {
        currNewIds = &newIds;
    }
    if (ForkJoinTask * forkJoinTask = dynamic_cast<ForkJoinTask*>(tsk.task))
    {
        QList<ExpTaskListTraits::TaskItem> tasks;
        // if we define its own QSet with threadNewIds, the all waitTasks have
        // to be handled in its own thread.
        QSet<unsigned int> threadNewIds;
        for (unsigned int ii = 0; ii < forkJoinTask->getThreadsN(); ++ii) {
            tasks = forkJoinTask->getTasks(ii);
            for(QList<ExpTaskListTraits::TaskItem>::ConstIterator it = tasks.constBegin(); it != tasks.constEnd(); ++it)
                addWaitTask(*it, &threadNewIds);
            threadNewIds.clear();
        }
    } else {
        WaitTaskList::addWaitTask(tsk, currNewIds)
    }
}
   \endcode
 */
void WaitTaskList::addWaitTask(const ExpTaskListTraits::TaskItem &tsk, QSet<unsigned int> *currNewIds)
{
    if (!currNewIds) {
        currNewIds = &newIds;
    }
    if (WaitExpTask * waitExpTask = dynamic_cast<WaitExpTask*>(tsk.task))
    {
        unsigned int id = getId();
        WaitTaskListTraits::TaskItem waitTaskItem = waitExpTask->getWaitTaskItem();
        waitTaskItem.task->setId(id);
        waitTasks.insert(id,waitTaskItem);
        currNewIds->insert(id);

        connect(waitExpTask, &WaitExpTask::startWaitTask, this, &WaitTaskList::onStartWaitTask);
    }

    if (WaitingTask * waitingTask = dynamic_cast<WaitingTask*>(tsk.task))
    {
        connect(waitingTask, &WaitingTask::waitTaskFinished, this, &WaitTaskList::onWaitTaskFinished);
        waitingTask->setIds(*currNewIds);
        for (QSet<unsigned int>::ConstIterator it = currNewIds->begin(); it != currNewIds->end(); ++it)
        {
            connect(waitTasks.value(*it).task, &WaitTask::waitTaskFinished, waitingTask, &WaitingTask::onWaitTaskFinished);
            waitTasks.value(*it).task->setHandled(true);
        }
        currNewIds->clear();
    }

    {
        ExpTaskList * expTaskList;
        WaitTaskList * waitTaskList;
        if ((expTaskList = dynamic_cast<ExpTaskList*>(tsk.task)) &&
                !(waitTaskList = dynamic_cast<WaitTaskList*>(tsk.task))) {
            QList<ExpTaskListTraits::TaskItem> tasks = expTaskList->getTasks();
            for(QList<ExpTaskListTraits::TaskItem>::ConstIterator it = tasks.constBegin(); it != tasks.constEnd(); ++it)
                addWaitTask(*it);
        }
    }

    if (ForkJoinTask * forkJoinTask = dynamic_cast<ForkJoinTask*>(tsk.task))
    {
        QList<ExpTaskListTraits::TaskItem> tasks;
        // if we define its own QSet with threadNewIds, the all waitTasks have
        // to be handled in its own thread. It is not necessary for the
        // functionality.
//        QSet<unsigned int> threadNewIds;
        for (unsigned int ii = 0; ii < forkJoinTask->getThreadsN(); ++ii) {
            tasks = forkJoinTask->getTasks(ii);
            for(QList<ExpTaskListTraits::TaskItem>::ConstIterator it = tasks.constBegin(); it != tasks.constEnd(); ++it)
                addWaitTask(*it);
//                addWaitTask(*it, &threadNewIds);
//            threadNewIds.clear();
        }
    }
}

/*!
   \brief Gets unused id for WaitTaskList's WaitTask id system.

   \sa WaitTask::id(), WaitingTask::setIds()
 */
unsigned int WaitTaskList::getId()
{
    unsigned int ii = 0;
    while (waitTasks.contains(ii))
        ++ii;
    return ii;
}

/*!
   \brief Constructs a new QThread and connects the WaitTaskList::stopWorker()
   signal to the WaitWorker::stop(), WaitTaskList::startWaitTaskInWorker()
   signal to the WaitWorker::addWaitTask() and finally
   WaitWorker::quitFinished() signal to the
   WaitTaskList::onWorkerQuitFinished() slot. Then it starts thread.
 */
void WaitTaskList::buildThread()
{
    // creating new thread
    thread = new QThread(this);
    // creating new worker and moving it to the thread
    waitWorker = new WaitWorker;
    waitWorker->moveToThread(thread);
    // the thread object is in the WaitTaskList's thread, whereas the
    // waitWorker is in the thread inside the QThread thread object, so it is
    // not safe to destroy it from WaitTaskList's thread and threrefore the
    // Qt's queued connection is used (it is chosen automatically if the signal
    // is emitted from the different thread than is the owner of the slot) to
    // the waitWorker's deleteLater() method.
    connect(thread, &QThread::finished, waitWorker, &QObject::deleteLater);

    // signals, which controlls the execution of the waitWorker
    connect(this, &WaitTaskList::stopWorker, waitWorker, &WaitWorker::stop);
    connect(waitWorker, &WaitWorker::quitFinished, this, &WaitTaskList::onWorkerQuitFinished);
    connect(this, &WaitTaskList::startWaitTaskInWorker, waitWorker, &WaitWorker::addWaitTask);

    // starting thread's event loop
    thread->start();
    setWaitWorkerFinished(false);
}

/*!
   \brief Destroyes WaitTaskList.
 */
WaitTaskList::~WaitTaskList()
{
    waitTasks.clear();
    if (thread)
    {
        emit stopWorker();
        thread->quit();
        thread->wait();
    }
    for (QMap<unsigned int, WaitTaskListTraits::TaskItem>::iterator it = waitTasks.begin(); it != waitTasks.end(); ++it)
    {
        delete it->task;
    }
}

/*!
   \var WaitTaskList::thread
       \brief The QThread object, which contains thread in which the WaitWorker
       object runs.

       \sa buildThread()
   \var WaitTaskList::waitWorker
       \brief The WaitWorker object, which controlls wheather the contained
       WaitTask's is running.
   \var WaitTaskList::waitTasks
       \brief List which contains all the added WaitTask objects and their ids.
   \var WaitTaskList::newIds
       \brief List containing ids of WaitTask objects which have not aready
       been handle by some WaitingTask.
   \var WaitTaskList::blQuit_
       \brief Indicitas, if the execution of WaitTaskList was stopped by the
       stop() method.
   \var WaitTaskList::blTaskListFinished_
       \brief Indicates, if the ExpTaskList object inside the WaitTaskList has
       already finished when the stop() method was invoked.
   \var WaitTaskList::blWaitWorkerFinished_
       \brief Indicates, if the waitWorker has already finished when the stop()
       method was invoked.
 */

/*!
   \class WaitWorker
   \brief This class is intended for the internal use in the WaitTaskList for
   controlling if the processes controlled by WaitTask objects is running. It
   is executed in new thread.

   When the new WaitTask is added to the list of controlled tasks by the
   addWaitTask() method, it is checked if the task has some
   WaitTask::initialDelay(). If yes, the new DelayedStart object is created and
   the task is inserted inside it. The DelayedStart::addTask() signal is
   connected to the addDelayedWaitTask() method and then the
   QTimer::singleShot() method is connected to the
   DelayedStart::delayedAddWaitTask() slot, which emits DelayedStart::addTask()
   signal connected to addDelayedWaitTask() slot. If there is no
   WaitTask::initialDelay(), the addDelayedWaitTask() slot is invoked directly
   from this method.

   addDelayedWaitTask() controlls if any WaitTask is already beeing checked. If
   no, the internal QTimer is started with repetition rate of
   WaitWorker::refreshRate_ ms, the signal QTimer::timeout() of which is
   connected to the waitWorkerLoop() slot, and the WaitTask is inserted into
   the interanl list and its WaitTask::start() is invoked.

   In the waitWorkerLoop(), all WaitTask objects from the list is tested, if
   their WaitTask::running() are true and the WaitTask::finish() method is
   invoked for those which have already finished. The finished WaitTask objects
   are also removed from the internal list. If there is no WaitTask in the
   list, the internal QTimer is stopped.

   The waitWorkerLoop() execution can be stopped by invoking the stop method,
   which calls the quitLoop() method, which stops all the running task by the
   WaitTask::stop() method executin and then it removes all the added WaitTask
   objects from the internal list. At the end, it stops the internal QTimer and
   emits quitFinished() signal, which is connected to the
   WaitTaskList::onWorkerQuitFinished() slot.
 */

/*!
   \brief Construts the WaitWorker object.
   \param parent Parent QObject in the Qt's ownership system.
 */
WaitWorker::WaitWorker(QObject *parent) :
    QObject(parent)
{
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &WaitWorker::waitWorkerLoop);
    refreshRate_ = 300;
}

// signals
/*!
   \fn WaitWorker::quitFinished()
       \brief This signal is emited only from the quitLoop() method.
 */

/*!
   \brief Stops the waitWorkerLoop() execution by calling the quitLoop()
   method.
 */
void WaitWorker::stop()
{
    quitLoop();
}

/*!
   \brief Adds WaitTask in the internal list. It controlls if the WaitTask does
   not have some WaitTask::initialDelay(). See description of WaitTask for
   more details.

   \sa addDelayedWaitTask, DelayedStart
 */
void WaitWorker::addWaitTask(const WaitTaskListTraits::TaskItem & wt)
{
    if (wt.task->initialDelay() > 0)
    {
        DelayedStart * ds = new DelayedStart(wt, this);
        connect(ds, &DelayedStart::addTask, this, &WaitWorker::addDelayedWaitTask);
        QTimer::singleShot(wt.task->initialDelay(), ds, SLOT(delayedAddWaitTask()));
    }
    else
    {
        addDelayedWaitTask(wt);
    }
}

/*!
   \brief This method is called from the addWaitTask() method after the
   WaitTask::initialDelay() delay elapses and adds the WaitTask in the internal
   list and invokes its WaitTask::start() method.
   \param wt
 */
void WaitWorker::addDelayedWaitTask(const WaitTaskListTraits::TaskItem &wt)
{
    if (wt.task->initialDelay() > 0) {
        DelayedStart * ds = static_cast<DelayedStart *>(QObject::sender());
        delete ds;
    }
    if (waitTasks_.isEmpty()) {
        timer->start(refreshRate_);
    }
    waitTasks_.insert(wt.task->id(), wt);
    wt.task->start();
}

/*!
   \brief This method controlls all the WaitTask objects from the internal
   list, wheather thei are running. If not, it executes their
   WaitTask::finish() method and removes them from the internal list.
 */
void WaitWorker::waitWorkerLoop()
{
    for (QMap<unsigned int, WaitTaskListTraits::TaskItem>::ConstIterator it = waitTasks_.constBegin(); it != waitTasks_.constEnd(); ++it) {
        if (!it->task->running()) {
            markedForDelete.append(it->task->id());
            it->task->finish();
        }
    }
    if (!markedForDelete.isEmpty()) {
        for (QList<unsigned int>::ConstIterator it = markedForDelete.begin(); it != markedForDelete.end(); ++it)
            waitTasks_.remove(*it);
        markedForDelete.clear();
    }
    if (waitTasks_.isEmpty()) {
        timer->stop();
    }
}

/*!
   \brief This method is invoked by the stop() method to WaitTask::stop() all
   the contained WaitTask objects and to clear the list of the all contained
   WaitTask objeccts. At the end, it emits quitFinished() signal.
 */
void WaitWorker::quitLoop()
{
    for (QMap<unsigned int, WaitTaskListTraits::TaskItem>::ConstIterator it = waitTasks_.constBegin(); it != waitTasks_.constEnd(); ++it)
    {
        it->task->stop();
    }
    waitTasks_.clear();
    timer->stop();
    emit quitFinished();
}

/*!
   \brief Destroys WaitWorker.
 */
WaitWorker::~WaitWorker()
{
}

/*!
   \var WaitWorker::timer
       \brief Internal timer, the QTimer::timeout() method of which is
       connected to the waitWorkerLoop() slot and its repetition rate is set
       to the WaitWorker::refreshRate_ ms.
   \var WaitWorker::waitTasks_
       \brief Internal list of all contained WaitTask objects.
   \var WaitWorker::markedForDelete
       \brief This variable enables using the iterators for deleting objects
       from QMap class, because QMap do not preserves iterator positions after
       any modification. This variable is used only in the waitWorkerLoop, but
       because the loop is rapidly recurently invoked it is created globaly to
       save computer resources.
   \var WaitWorker::refreshRate_
       \brief The repetition rate in ms of WaitWorker::timer, which determines
       how often the contained WaitTask objects are controlled, if they are
       WaitTask::running()
 */

/*!
   \class DelayedStart
   \brief This class is helper class, which wraps the WaitTask object if it has
   some WaitTask::initialDelay() in the WaitWorker::delayedAddWaitTask()
   method. Its method is invoked by a QTimer after the initialy delay elapses.
   The delayedAddWaitTask() emits addTask() signal, which is connected to the
   WaitWorker::addDelayedWaitTask() method and then it destroyes itself.
 */

/*!
   \brief Constructs the DelayedStart object.
   \param waitTaskItem The WaitTask, which may be delayed.
   \param parent A parent in the Qt's ownership system.
 */
DelayedStart::DelayedStart(const WaitTaskListTraits::TaskItem &waitTaskItem, QObject *parent) :
    QObject(parent), waitTaskItem_(waitTaskItem)
{
}

// signals
/*!
   \fn DelayedStart::addTask(WaitTaskListTraits::TaskItem waitTaskItem)
   \brief This signal is emited from the delayedAddWaitTask() method and
   connected to the WaitWorker::addDelayedWaitTask() slot.
*/


// slots
/*!
   \brief This slot is invoked by the QTimer from the WaitWorker::addWaitTask()
   method. It emits addTask() signal and deletes the DelayedStart object.
 */
void DelayedStart::delayedAddWaitTask()
{
    emit addTask(waitTaskItem_);
//    delete this;
}

/*!
   \brief Destroyes DelayedStart objects.
 */
DelayedStart::~DelayedStart()
{
}

/*!
   \var DelayedStart::waitTaskItem_
       \brief contained WaitTask.
 */
