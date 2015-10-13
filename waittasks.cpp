#include "waittasks.h"
#include "appstate.h"
#include "winspec.h"
#include "waittasklist.h"
#include "stagecontrol.h"
#include "timespan.h"

#include <QDateTime>

DelayWaitTask::DelayWaitTask(AppState *pappState, const TimeSpan *delay, QObject *parent) :
    WaitTask(parent), pappState_(pappState)
{
    delay_ = new TimeSpan(*delay);
    finishTime_ = new QDateTime;
}

bool DelayWaitTask::running()
{
    return (*finishTime_ > QDateTime::currentDateTime());
}

void DelayWaitTask::start()
{
    *finishTime_ = pappState_->getWaitingStartedTime().addMSecs(delay_->toMSec());
    WaitTask::start();
}

void DelayWaitTask::stop()
{
    pappState_->removeWaitingState(WaitTaskListTraits::WaitFor::Delay);
    WaitTask::stop();
}

void DelayWaitTask::finish()
{
    WaitTask::finish();
}

DelayWaitTask::~DelayWaitTask()
{
    delete delay_;
    delete finishTime_;
}


WinSpecWaitTask::WinSpecWaitTask(AppState *pappState, QObject *parent) :
    WaitTask(parent), pappState_(pappState), pwinSpec_(pappState->winSpec())
{
    hasSpectrum_ = false;
    connect(this, &WinSpecWaitTask::lastFrameChanged, pappState_, &AppState::setLastFrameChanged);
    connect(this, &WinSpecWaitTask::spectrumChanged, pappState_, &AppState::setSpectrumChanged);
}

bool WinSpecWaitTask::running()
{
   if (pwinSpec_->running())
   {
       if (pappState_->plotStyle() == AppStateTraits::PlotStyle::Spectra) {
           int currFrame;
           int iXX;
           int iYY;
           pwinSpec_->getLastFrame(pappState_->getLastFrame(), &currFrame, &iXX, &iYY);
           pappState_->setCurrExpParams(pwinSpec_->getAccum(), currFrame, AppStateTraits::WinSpecState::Running);
           emit lastFrameChanged(true);
       }
       return true;
   }
   else
       return false;
}

void WinSpecWaitTask::start()
{
    if (pwinSpec_->running())
    {
        hasSpectrum_ = true;
        pappState_->setCurrExpParams(0, 0, AppStateTraits::WinSpecState::Running);
        pappState_->setPlotType(AppStateTraits::PlotType::Frame);
    }
    WaitTask::start();
}

void WinSpecWaitTask::stop()
{
    pwinSpec_->stop();
    WaitTask::stop();
}

void WinSpecWaitTask::finish()
{
    if (hasSpectrum_ && pappState_->plotStyle() == AppStateTraits::PlotStyle::Spectra) {
        int iFrames;
        int iX;
        int iY;
        pwinSpec_->getSpectrum(pappState_->getSpectrum(), &iFrames, &iX, &iY);
        pappState_->setPlotType(AppStateTraits::PlotType::Spectrum);
        pappState_->makePlotSpectrum();
        emit spectrumChanged(true);
        pappState_->setCurrExpParams(0, 0, AppStateTraits::WinSpecState::Ready);
        hasSpectrum_ = false;
    } else {
        pappState_->setWinSpecState(AppStateTraits::WinSpecState::Ready);
    }
    WaitTask::finish();
}

WinSpecWaitTask::~WinSpecWaitTask()
{
}


StageControlWaitTask::StageControlWaitTask(AppState *pappState, QObject *parent) :
    WaitTask(parent), pappState_(pappState), pstageControl_(pappState->stageControl())
{
    setInitialDelay(200);
}

bool StageControlWaitTask::running()
{
    if (pstageControl_->motorRunning())
    {
        pappState_->setCurrStageState(pstageControl_->curPos());
        return true;
    }
    else
        return false;
}

void StageControlWaitTask::start()
{
    pappState_->addWaitingState(WaitTaskListTraits::WaitFor::Motor);
    WaitTask::start();
}

void StageControlWaitTask::stop()
{
    pstageControl_->stop();
    pstageControl_->powerOff();
    WaitTask::stop();
}

void StageControlWaitTask::finish()
{
    pappState_->removeWaitingState(WaitTaskListTraits::WaitFor::Motor);
    WaitTask::finish();
}

StageControlWaitTask::~StageControlWaitTask()
{
}



GratingWaitTask::GratingWaitTask(AppState *appState, QObject *parent) :
    WaitTask(parent), appState_(appState)
{
    finishTime_ = new QDateTime;
}

bool GratingWaitTask::running()
{
    return (*finishTime_ > QDateTime::currentDateTime());
}

void GratingWaitTask::start()
{
    appState_->addWaitingState(WaitTaskListTraits::WaitFor::Grating);
    *finishTime_ = QDateTime::currentDateTime().addSecs(5);
    WaitTask::start();
}

void GratingWaitTask::stop()
{
    WaitTask::stop();
}

void GratingWaitTask::finish()
{
    qDebug() << "GratingWaitTask::finish()";
    appState_->removeWaitingState(WaitTaskListTraits::WaitFor::Grating);
    WaitTask::finish();
}

GratingWaitTask::~GratingWaitTask()
{
    delete finishTime_;
}

