#include "waittask.h"

#include <QMutex>
#include <QMutexLocker>

/*!
   \class WaitTask
   \brief Abstract class which forms the base for task usable with
   WaitTaskList. The running() method need to be reimplemented.

   The object of this class is added wrapped in the
   WaitTaskListTratis::TaskItem to the WaitExpTask and when the WaitExpTask is
   added to the WaitTaskList, it is submited to the list of WaitTask objects.
   Then, when WaitingTask is added, the WaitTask's waitTaskFinished() signal is
   connected to the WaitingTask's WaitingTask::onWaitTaskFinished() slot. The,
   if the WaitExpTask is started in the WaitTaskList, the WaitWorker controlls
   if initialDelay() is greater than zero and if so, it waits at first the
   initial delay, then it calls WaitTask's start() method and then periodically
   controlls the running() method and when it returns false, the WaitTask's
   finish() method is called which should emit the waitTaskFinished() signal
   at the end.
 */

/*!
   \brief Constructs WaitTask object.
   \param parent Parent in Qt's ownership system.
 */
WaitTask::WaitTask(QObject *parent) :
    QObject(parent), id_(0), initDelay_(0)
{
    delayedDeleteMutex = new QMutex;
    blDelayedDelete_ = false;

}

/*!
   \fn WaitTask::running()
   \brief Virtual method, which need to be reimplemented. If this method
   returns false, the waiting in WaitTaskList is ended.
 */

/*!
   \fn WaitTask::initialDelay() const
   \brief Returns initial delay. See the class description for more details.
 */

/*!
   \brief This method returns true if the deletion of the WaitTask may be
   delayed. See setDelayedDelete() for more details.
 */
bool WaitTask::delayedDelete() const
{
    QMutexLocker locker(delayedDeleteMutex);
    return blDelayedDelete_;
}

/*!
   \fn WaitTask::id() const
   \brief Gets WaitTask's id. This id is retreived in WaitTaskList's id system.
 */

/*!
   \fn WaitTask::handled() const
   \brief True, if WaitTask is handled by some WaitingTask in WaitTaskList.
 */

/*!
   \fn WaitTask::setInitialDelay(int initDelay)
   \brief Sets initial delay in ms before the waiting in WaitTaskList begins.
 */

/*!
   \brief This method sets the if the WaitTask will be deleted immediatelly
   after it is finished or not. The delayed deletion is necessary in the case
   when the WaitingTask which waits for this WaitTask will be executed
   repetitively. For the details see ExpTask::setDelayedDelete()
 */
void WaitTask::setDelayedDelete(bool blDelayedDelete)
{
    QMutexLocker locker(delayedDeleteMutex);
    blDelayedDelete_ = blDelayedDelete;
}

/*!
   \fn WaitTask::setId(unsigned int uintId)
   \brief Sets the WaitTask's id. This methods is used by the WaitTaskList
   to manage contained WaitTask objects.
 */

/*!
   \fn WaitTask::setHandled(bool blHandled)
   \brief Sets if the WaitTask is handled by some WaitingTask or not. If it is
   not handled in the time it may be executed in WaitTaskList, the execution
   of this task is skipped.
 */

// signals
/*!
   \fn WaitTask::waitTaskFinished(unsigned int uintId)
       \brief This signal need to be emited after the execution of the WaitTask
       is finished (succesfully or not) to stop waiting for this WaitTask.
       It is emited from WaitTask::finish() method.
   \fn WaitTask::waitingFailed()
       \brief This signal is emited when the waiting for this wait task is
       stopped from the WaitTask::stop() method.
 */

// public slots
/*!
   \fn WaitTask::start()
       \brief Contains commands which is executed at the beginning of the
       waiting.
   \fn WaitTask::stop()
       \brief Contains commands which is executed when the waiting is stopped.
       It emits waitingFailed() signal and calls finish() method.
    \fn WaitTask::finish()
       \brief Contains commands which is executed when the waiting is finished
       and emits waitTaskFinished() signal.
 */

// destuctor
/*!
   \brief Destroyes the WaitTask.
 */
WaitTask::~WaitTask()
{
    delete delayedDeleteMutex;
}

// private variables
/*!
   \var WaitTask::id_
       \brief id of the WaitTask, which was retreived from the WaitTaskList
       id system.
   \var WaitTask::initDelay_
       \brief initial delay in ms which is waited before the waiting for
       WaitTask is proceeded.
   \var WaitTask::blDelayedDelete_
       \brief true if the WaitTask should not be deleted immediatelly after it
       finishes.
   \var WaitTask::delayedDeleteMutex
       \brief mutex which manages thread safety.
   \var WaitTask::handled_
       \brief true if the WaitTask is handled by some WaitingTask in
       WaitTaskList.
*/
