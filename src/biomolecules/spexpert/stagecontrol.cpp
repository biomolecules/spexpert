#include "stagecontrol.h"

#ifndef VIRTUALSTAGECONTROL
#include "usmcvb_com.h"
#endif // VIRTUALSTAGECONTROL

#include <QString>
#include <QMutex>
#include <QMutexLocker>

StageControl::StageControl(QObject *parent) :
    QObject(parent)
{
#ifndef VIRTUALSTAGECONTROL
    usmcvb = new USMCVB_COM::USMCVB_COM;
#else // VIRTUALSTAGECONTROL
    usmcvb = new VirtualStageControl;
#endif // VIRTUALSTAGECONTROL
    blConnected_ = false;
    blInitialized_ = false;
    stageRange_ = defaultStageRange;
    iLowerLim_ = 0;
    iUpperLim_ = 0;
    iCurPos_ = 0;
    iMeasPos_ = 0;
    iCalPos_ = 0;
    mutex_ = new QMutex;
}

bool StageControl::motorRunning() const
{
    bool blMotorRunning = false;
    if (blConnected_)
    {
        QMutexLocker locker(mutex_);
        usmcvb->motorRunning(blMotorRunning);
    }
    return blMotorRunning;
}

bool StageControl::connect(QString &strErrMsg)
{
    QMutexLocker locker(mutex_);
    blConnected_ = false;
    bool blConnected;
    int ok = usmcvb->connect_2(strErrMsg, blConnected);
    if (!ok)
    {
        blConnected_ = true;
        return true;
    }
    else
    {
        blConnected_ = false;
        return false;
    }
}

bool StageControl::powerOn()
{
    bool isPoweredOff;
    QMutexLocker locker(mutex_);
    usmcvb->isPoweredOff(isPoweredOff);
    if (isPoweredOff)
        return !usmcvb->powerOn();
    else
        return false;
}

bool StageControl::powerOff()
{
    bool isPoweredOff;
    QMutexLocker locker(mutex_);
    usmcvb->isPoweredOff(isPoweredOff);
    if (!isPoweredOff)
        return !usmcvb->powerOff();
    else
        return false;
}

bool StageControl::run(int dest, StageControlTraits::ReferenceType ref)
{
    bool blErr = true;
    switch (ref)
    {
    case StageControlTraits::ReferenceType::Absolute :
        blErr = run(dest);
        break;
    case StageControlTraits::ReferenceType::LowerLimit :
        blErr = run(dest + iLowerLim_);
        break;
    case StageControlTraits::ReferenceType::UpperLimit :
        blErr = run(iUpperLim_ - dest);
        break;
    }
    return !blErr;
}

bool StageControl::stop()
{
    QMutexLocker locker(mutex_);
    return !usmcvb->stopMotion();
}

bool StageControl::goToLowerLim()
{
    QMutexLocker locker(mutex_);
    int iCurPos;
    bool blErr = usmcvb->curPos(iCurPos);
    if (blErr)
        return !blErr;

    return !usmcvb->run(iCurPos - graterThanLimit);
}

bool StageControl::goToUpperLim()
{
    QMutexLocker locker(mutex_);
    int iCurPos;
    bool blErr = usmcvb->curPos(iCurPos);
    if (blErr)
        return !blErr;

    return !usmcvb->run(iCurPos + graterThanLimit);
}

bool StageControl::goToMeas()
{
    int pos;
    QMutexLocker locker(mutex_);
    switch (measPosRef)
    {
    case StageControlTraits::ReferenceType::Absolute :
        pos = iMeasPos_;
        break;
    case StageControlTraits::ReferenceType::LowerLimit :
        pos = iLowerLim_ + iMeasPos_;
        break;
    case StageControlTraits::ReferenceType::UpperLimit :
        pos = iUpperLim_ - iMeasPos_;
        break;
    }
    return !usmcvb->run(pos);
//    return !usmcvb->run(iMeasPos_);
}

bool StageControl::goToCal()
{
    int pos;
    QMutexLocker locker(mutex_);
    switch (calPosRef)
    {
    case StageControlTraits::ReferenceType::Absolute :
        pos = iCalPos_;
        break;
    case StageControlTraits::ReferenceType::LowerLimit :
        pos = iLowerLim_ + iCalPos_;
        break;
    case StageControlTraits::ReferenceType::UpperLimit :
        pos = iUpperLim_ - iCalPos_;
        break;
    }
    return !usmcvb->run(pos);
//    return !usmcvb->run(iCalPos_);
}

void StageControl::setStageRange(int val)
{
    QMutexLocker locker(mutex_);
    stageRange_ = val;
}

void StageControl::setLowerLim(int val)
{
    QMutexLocker locker(mutex_);
    if (iLowerLim_ != val) {
        iLowerLim_ = val;
        locker.unlock();
        emit lowerLimChanged(val);
    }
}

void StageControl::setUpperLim(int val)
{
    QMutexLocker locker(mutex_);
    if (iUpperLim_ != val) {
        iUpperLim_ = val;
        locker.unlock();
        emit upperLimChanged(val);
    }
}

void StageControl::setMeasPos(int pos, StageControlTraits::ReferenceType ref)
{
    QMutexLocker locker(mutex_);
    measPosRef = ref;
    iMeasPos_ = pos;
//    switch (ref)
//    {
//    case StageControlTraits::ReferenceType::Absolute :
//        iMeasPos_ = pos;
//        break;
//    case StageControlTraits::ReferenceType::LowerLimit :
//        iMeasPos_ = iLowerLim_ + pos;
//        break;
//    case StageControlTraits::ReferenceType::UpperLimit :
//        iMeasPos_ = iUpperLim_ - pos;
//        break;
//    }
}

void StageControl::setCalPos(int pos, StageControlTraits::ReferenceType ref)
{
    QMutexLocker locker(mutex_);
    calPosRef = ref;
    iCalPos_ = pos;
//    switch (ref)
//    {
//    case StageControlTraits::ReferenceType::Absolute :
//        iCalPos_ = pos;
//        break;
//    case StageControlTraits::ReferenceType::LowerLimit :
//        iCalPos_ = iLowerLim_ + pos;
//        break;
//    case StageControlTraits::ReferenceType::UpperLimit :
//        iCalPos_ = iUpperLim_ - pos;
//        break;
//    }
}

void StageControl::setInitialized()
{
    QMutexLocker locker(mutex_);
    blInitialized_ = true;
}

bool StageControl::initialized() const
{
    QMutexLocker locker(mutex_);
    return blInitialized_;
}

bool StageControl::connected() const
{
    QMutexLocker locker(mutex_);
    return blConnected_;
}

int StageControl::stageRange() const
{
    QMutexLocker locker(mutex_);
    return stageRange_;
}

int StageControl::lowerLim(StageControlTraits::ReferenceType ref) const
{
    QMutexLocker locker(mutex_);
    switch (ref)
    {
    case StageControlTraits::ReferenceType::Absolute :
        return iLowerLim_;
        break;
    case StageControlTraits::ReferenceType::LowerLimit :
        return 0;
        break;
    case StageControlTraits::ReferenceType::UpperLimit :
        return iUpperLim_ - iLowerLim_;
        break;
    default :
        return iLowerLim_;
    }
}

int StageControl::upperLim(StageControlTraits::ReferenceType ref) const
{
    QMutexLocker locker(mutex_);
    switch (ref)
    {
    case StageControlTraits::ReferenceType::Absolute :
        return iUpperLim_;
        break;
    case StageControlTraits::ReferenceType::LowerLimit :
        return iUpperLim_ - iLowerLim_;
        break;
    case StageControlTraits::ReferenceType::UpperLimit :
        return 0;
    default :
        return iUpperLim_;
    }
}

int StageControl::curPos(StageControlTraits::ReferenceType ref)
{
    QMutexLocker locker(mutex_);
    int cp;
    bool cpchanged = false;
    usmcvb->curPos(cp);
    if (cp != iCurPos_) {
        cpchanged = true;
        iCurPos_ = cp;
    }
    int rv;
    switch (ref)
    {
    case StageControlTraits::ReferenceType::Absolute :
        rv = iCurPos_;
        break;
    case StageControlTraits::ReferenceType::LowerLimit :
        rv = iCurPos_ - iLowerLim_;
        break;
    case StageControlTraits::ReferenceType::UpperLimit :
        rv = iUpperLim_ - iCurPos_;
        break;
    default :
        rv = iCurPos_;
    }
    locker.unlock();
    if (cpchanged) {
        emit currPosChanged(cp);
    }
    return rv;
}

StageControl::~StageControl()
{
    delete usmcvb;
    delete mutex_;
}

#ifdef VIRTUALSTAGECONTROL

#include <QTimer>
#include <QDateTime>
#include <cmath>

VirtualStageControl::VirtualStageControl(QObject *parent) :
    QObject(parent)
{
//    qDebug() << "VirtualStageControl::VirtualStageControl(): Vytavrim VirtualStageControl";
    blStop_ = false;
    blRunning_ = false;
    blPoweredOff_ = true;
    iStartPos_ = 20000;
    startTime_ = new QDateTime;
    timer_ = new QTimer;
    connect(this, &VirtualStageControl::stopTimer, this, &VirtualStageControl::onStopTimer);
    connect(timer_, &QTimer::timeout, this, &VirtualStageControl::movementFinished);
}

int VirtualStageControl::connect_2(QString &strErrMsg, bool &blConnected)
{
    strErrMsg = "";
    blConnected_ = true;
    blConnected = blConnected_;
    return 0;
}

int VirtualStageControl::motorRunning(bool &blMotorRunning)
{
    blMotorRunning = blRunning_;
    if (blConnected_)
        return 0;
    else
        return -1;
}

int VirtualStageControl::isPoweredOff(bool &blPoweredOff)
{
    blPoweredOff = blPoweredOff_;
    if (blConnected_)
        return 0;
    else return -1;
}

int VirtualStageControl::powerOn()
{
    blPoweredOff_ = false;
    if (blConnected_)
        return 0;
    else
        return -1;
}

int VirtualStageControl::powerOff()
{
    blPoweredOff_ = true;
    if (blConnected_)
        return 0;
    else
        return -1;
}

int VirtualStageControl::run(int dest)
{
    if (blConnected_ && !blPoweredOff_)
    {
        iDest_ = dest;
        *startTime_ = QDateTime::currentDateTime();
        blStop_ = false;
        curPos(iStartPos_);
        if (dest > iStartPos_)
            direction_ = 1;
        else
            direction_ = -1;
        timer_->setInterval(double(direction_ * (dest - iStartPos_)) / speed_);
        timer_->setSingleShot(true);
        blRunning_ = true;
        timer_->start();
        return 0;
    }
    else
        return -1;
}

int VirtualStageControl::stopMotion()
{
    if (blConnected_ && blRunning_) {
        emit stopTimer();
        blStop_ = true;
        movementFinished();
        return 0;
    }
    else
        return -1;
}

int VirtualStageControl::curPos(int &iCurPos)
{
    if (blConnected_) {
        if (blRunning_) {
            iCurPos = iStartPos_ + direction_ *
                    (QDateTime::currentDateTime().toMSecsSinceEpoch() - startTime_->toMSecsSinceEpoch()) * speed_;
            if (iCurPos < iLowerLim_) {
                iCurPos = iLowerLim_;
                iDest_ = iCurPos;
                if (!blStop_)
                    stopMotion();
            }
            else if(iCurPos > iUpperLim_) {
                iCurPos = iUpperLim_;
                iDest_ = iCurPos;
                if (!blStop_)
                    stopMotion();
            }
        }
        else {
            iCurPos = iStartPos_;
        }
        return 0;
    }
    else
        return -1;
}

void VirtualStageControl::movementFinished()
{
    if (blStop_) {
        curPos(iStartPos_);
        blStop_ = false;
    }
    else {
        iStartPos_ = iDest_;
    }
    blRunning_ = false;
}

void VirtualStageControl::onStopTimer()
{
    timer_->stop();
}

VirtualStageControl::~VirtualStageControl()
{
//    qDebug() << "VirtualStageControl::~VirtualStageControl(): Rusim VirtualStageControl";
    delete startTime_;
    delete timer_;
}

#endif
