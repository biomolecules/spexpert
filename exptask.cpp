#include "exptask.h"

/*!
   \class ExpTask
   \brief Abstract class, which forms the base for tasks usable with
   ExpTaskList and WaitTaskList.

   The task stores the number of repetitions of this task to be executed, which
   can be set by the setTimesExec() method. Each finished execution requires
   the onceExecuted() method to be called. Spare execution repetitions can be
   obtained by the currTimesExec() method and the counter of current spare
   executions can be reset to the initial value by the restartTimesExec()
   method.

   The ExpTask can be marked not to be deleted immediatelly after the execution
   counter reaches zero by the setDelayedDelete() method. This is useful if the
   task is part of some ExpTask chain (for example ExpTaskList) which is set to
   be executed more times, so the task is stored for the next execution of the
   whole list. If the task is marked to be deleted later can be obtained by the
   delayedDelete() method.
 */

/*!
   \brief Construct the ExpTask object.
   \param parent A parent QObject in Qt's ownership system.
 */
ExpTask::ExpTask(QObject *parent) :
    QObject(parent), timesExec_(1), currTimesExec_(timesExec_), blDelayedDelete_(false)
{
}

/*!
   \brief This method lowers execution counter ExpTask::currTimesExec_, which
   controls a number of repetitions the ExpTask was executed.

   This counter can be reset by restartTimesExec() method.

   \sa timesExec(), setTimesExec()
 */
void ExpTask::onceExecuted()
{
    --currTimesExec_;
}

/*!
   \brief This method resets the current execution number counter
   ExpTask::currTimesExec_ to the initial number of executions
   ExpTask::timesExec_.

   \sa setTimesExec(), timesExec(), onceExecuted()
 */
void ExpTask::restartTimesExec()
{
    currTimesExec_ = timesExec_;
}

/*!
   \brief This method gets current number of executions to be executed stored
   in the ExpTask::currTimesExec_ variable.
   \return the number of executions to be executed.

   \sa timesExec(), setTimesExec(), onceExecuted()
 */
int ExpTask::currTimesExec() const
{
    return currTimesExec_;
}

/*!
   \brief This method gets the initial number of repetitions stored in the
   ExpTask::timesExec_ variable.
   \return the initial number of executions.

   \sa setTimesExec(), restartTimesExec()
 */
int ExpTask::timesExec() const
{
    return timesExec_;
}

/*!
   \brief This method returns true if the ExpTask object could not be deleted
   after the number of executions currTimesExec() reaches zero.
   \return true if the destruction of the ExpTask object could be delayed.
   \sa setDelayedDelete(), onceExecuted()
 */
bool ExpTask::delayedDelete() const
{
    return blDelayedDelete_;
}

/*!
   \brief This method sets the initial number of repetitions of the ExpTask.
   \param timesExecuted initial number of repetitions.

   \sa timesExec(), restartTimesExec(), onceExecuted(), currTimesExec()
 */
void ExpTask::setTimesExec(int timesExecuted)
{
    if (timesExec_ != timesExecuted)
    {
        timesExec_ = timesExecuted;
    }
    restartTimesExec();
}

/*!
   \brief This method marks the ExpTask delayed delete. If it is set to true,
   the ExpTask object could not be dleted after the number of executions
   currTimesExec() reaches zero (typically because it is a part of the a chain
   of more exptasks which may be exetuted more times as whole).
   \param blDelayedDelete true if the destruction of the ExpTask object could
   be delayed.

   \sa delayedDelete()
 */
void ExpTask::setDelayedDelete(bool blDelayedDelete)
{
    if (blDelayedDelete_ != blDelayedDelete)
    {
        blDelayedDelete_ = blDelayedDelete;
    }
}

/*!
   \fn ExpTask::finished()
       \brief signal emited when the task execution is finished
   \fn ExpTask::taskFailed()
       \brief emited when task execution failed, for example when the execution
       is interupted by the execution of stop() slot.
   \fn ExpTask::start()
       \brief pure virtual method, need to be reimplemented
 */

/*!
   \brief stops the execution of the task emiting taskFailed() and finished()
   signals.
 */
void ExpTask::stop()
{
    emit taskFailed();
    emit finished();
}

/*!
   \fn ExpTask::~ExpTask()
   \brief empty destructor.
 */

/*!
   \var ExpTask::timesExec_
       \brief A variable which denotes how many times the task is executed when
       inserted in the ExpTaskList. This variable is set by the setTimesExec()
       and get by the timesExec()methods. The current remaining number of
       executions is stored in ExpTask::currTimesExec_, which can be reset
       to the ExpTask::timesExec_ by the restartTimesExec() method.
   \var ExpTask::currTimesExec_
       \brief A variable which denotes how many executions of the task remain
       when inserted in the ExpTaskList. It can be decremented by the
       onceExecuted method and reset to the ExpTask::timesExec_ value by the
       restartTimesExec() method. The value of this variable can be obtained by
       the currTimesExec() method.
   \var ExpTask::blDelayedDelete_
       \brief This variable indicates wheather the task should be deleted later
       (when it is part of some execution chain, which may be executed more
       than one times) or directly after the timesExec() reaches zero. It can
       be obtained by the delayedDelete() and set by setDelayedDelete() method.
 */
