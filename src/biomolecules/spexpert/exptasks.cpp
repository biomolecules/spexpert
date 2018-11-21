#include <QStringBuilder>

#include <biomolecules/sprelay/core/k8090.h>

#include "exptasks.h"
#include "waittasks.h"
#include "appstate.h"
#include "winspec.h"
#include "stagecontrol.h"
#include "neslab.h"
#include "appcore.h"
#include "mainwindow.h"
#include "timespan.h"
#include "relay.h"
#include <QFileInfo>
#include <QDir>

#include <QMessageBox>

#include <QTimer>

#include <QDebug>

StartWaitingTask::StartWaitingTask(AppState * pappState, const TimeSpan *delay, QObject *parent) :
    ExpTask(parent), pappState_(pappState)
{
    delay_ = new TimeSpan(*delay);
}

void StartWaitingTask::start()
{
    pappState_->waitingStartedTime();
    pappState_->setWaitingFinishTime(*delay_);
    pappState_->addWaitingState(WaitTaskListTraits::WaitFor::Delay);
    emit finished();
}

StartWaitingTask::~StartWaitingTask()
{
    delete delay_;
}

FinishWaitingTask::FinishWaitingTask(AppState *pappState, QObject *parent) :
    ExpTask(parent), pappState_(pappState)
{
}

void FinishWaitingTask::start()
{
    pappState_->removeWaitingState(WaitTaskListTraits::WaitFor::Delay);
    emit finished();
}

WinSpecTasks::Params::Params(const QVector<double> &vecExposures, const QVector<int> &vecAccums,
                                           const QVector<int> &vecFrames, const QVector<QString> &vecFileNames)
    : vecExposures_(vecExposures), vecAccums_(vecAccums), vecFrames_(vecFrames), vecFileNames_(vecFileNames)
{
//    initializeParams(firstFileNameNumber, stepFileNameCounter, numMeas, numDigits);
    forceNum_ = false;
    const int numSizes = 4;
    int arSizes[numSizes];
    arSizes[0] = vecExposures_.size();
    arSizes[1] = vecAccums_.size();
    arSizes[2] = vecFrames_.size();
    arSizes[3] = vecFileNames_.size();
    for (int ii = 0; ii < numSizes; ++ii)
    {
        if (arSizes[ii] > 1)
        {
            if (numVals_ < 2)
            {
                numVals_ = arSizes[ii];
            }
            else if (arSizes[ii] != numVals_)
            {
                wrongSizes_ = true;
                qDebug() << ii << "WinSpecTasks::Params::Params(): Parametry nemaji stejnou velikost";
                if (arSizes[ii] < numVals_)
                    numVals_ = arSizes[ii];
            }
        }
        else if (arSizes[ii] < 1)
            wrongSizes_ = true;
    }
    qDebug() << "WinSpecTasks::Params::Params(): numVals =" << numVals_;
    numMeas_ = numVals_;
    if (wrongSizes())
    {
        correctVecSize(vecExposures_, defaultExposure_);
        correctVecSize(vecAccums_, defaultAccums_);
        correctVecSize(vecFrames_, defaultFrames_);
        correctVecSize(vecFileNames_, defaultFileName_);
    }
}

int WinSpecTasks::Params::countNumDigits(int number)
{
    int digits = 0;
    if (number < 0) digits = 1; // remove this line if '-' counts as a digit
    while (number) {
        number /= 10;
        digits++;
    }
    return digits;
}

bool WinSpecTasks::Params::addCounter()
{
    if (counter_ < numVals_ - 1 && counter_ < numMeas_ - 1) {
        ++counter_;
        return true;
    } else {
        return false;
    }
}

void WinSpecTasks::Params::initializeParams(int firstFileNameNumber, int stepFileNameCounter, int numMeas, int numDigits)
{
    defaultExposure_ = 1;
    defaultAccums_ = 1;
    defaultFrames_ = 1;
    defaultFileName_ = "";
    counter_ = -1;
    fileNameCounter_ = -1;
    firstFileNameNumber_ = firstFileNameNumber;
    stepFileNameCounter_ = stepFileNameCounter;
    wrongSizes_ = false;
    numVals_ = 1;
    numMeas_ = numMeas;

    int numDigits1 = countNumDigits(firstFileNameNumber_);
    int numDigits2 = countNumDigits((numMeas_ - 1) * stepFileNameCounter_ + firstFileNameNumber_);;

    if (numDigits1 > numDigits2)
        numDigits_ = numDigits1;
    else
        numDigits_ = numDigits2;


    if (numDigits_ > numDigits)
    {
        qDebug() << "WinSpecTasks::Params::initializeParams(): numDigits je male, musim ho zvisit na" << numDigits_;
    }
    else {
        numDigits_ = numDigits;
    }

}

void WinSpecTasks::Params::takeOne(double *pexposure, int *paccums, int *pframes, QString &fileName)
{
    bool dontCountMeas = addCounter();
    if ((vecFileNames_.size() == 1 && numMeas_ > 1) || forceNum_)
        dontCountMeas = false;
    *pexposure = lastExposure_ = takeControl(vecExposures_, defaultExposure_);
    *paccums = lastAccums_ = takeControl(vecAccums_, defaultAccums_);
    *pframes = lastFrames_ = takeControl(vecFrames_, defaultFrames_);
    fileName = lastFileName_ = takeControl(vecFileNames_, dontCountMeas);
}

int WinSpecTasks::Params::getNumMeas()
{
    return numMeas_;
}

int WinSpecTasks::Params::grPos()
{
    return grPos_;
}

template <typename T>
T WinSpecTasks::Params::takeControl(const QVector<T> &vec, const T &defVal)
{
    T out;
    if (vec.size() > 1)
        out = vec.at(getCounter());
    else
        out = vec.at(0);

    if (out <= 0)
        out = defVal;

    return out;
}

QString WinSpecTasks::Params::takeControl(const QVector<QString> &vec, bool dontCountVals)
{
    QString out;
    if (vec.size() > 1)
        out = vec.at(counter_);
    else
        out = vec.at(0);

    if (!dontCountVals)
    {
        addFileNameCounter();

        int numDigits = countNumDigits(getFileNameCounter());
        if (numDigits < numDigits_) {
            numDigits = numDigits_;
        } else {
            qDebug() << "WinSpecTasks::Params::takeControl(): numDigits je moc male, zvysuji ho z" << numDigits_ << "na" << numDigits;
        }

        QFileInfo outFile(out);
        out = QDir::toNativeSeparators(outFile.path()) % QDir::separator()
                % outFile.completeBaseName() % QString("%1")
                .arg(firstFileNameNumber_ + stepFileNameCounter_ * getFileNameCounter(),
                     numDigits, 10, QChar('0')) % ".spe";
    }

    return out;
}

template <typename T>
void WinSpecTasks::Params::correctVecSize(QVector<T> &vec, const T & defVal)
{
    if (vec.size() > 1 && vec.size() != numVals_)
        vec.resize(numVals_);
    else if (vec.size() < 1)
        vec.append(defVal);
}


WinSpecTasks::Start::Start(AppState *pappState_, double dblExposure_,
                         int iAccums_, int iFrames_, const QString & strFileName_, QObject *parent) :
    ExpTask(parent), pwinSpec(pappState_->winSpec()), pappState(pappState_), pwinSpecParams(nullptr),
    dblExposure(dblExposure_), iAccums(iAccums_), iFrames(iFrames_), strFileName(strFileName_),
    autoGetParams(false), cal_(false)
{
}

WinSpecTasks::Start::Start(AppState *pappState_, Params *pwinSpecParams_, QObject *parent) :
    ExpTask(parent), pwinSpec(pappState_->winSpec()), pappState(pappState_), pwinSpecParams(pwinSpecParams_),
    dblExposure(1), iAccums(1), iFrames(1), strFileName(""), autoGetParams(false), cal_(false)
{
}

WinSpecTasks::Start::Start(AppState *pappState_, bool cal, QObject *parent) :
    ExpTask(parent), pwinSpec(pappState_->winSpec()), pappState(pappState_), pwinSpecParams(nullptr),
    dblExposure(1), iAccums(1), iFrames(1), strFileName(""), autoGetParams(true), cal_(cal)
{
}

void WinSpecTasks::Start::expSetup(double dblExposure_, int iAccums_, int iFrames_, const QString &strFileName_)
{
    dblExposure = dblExposure_;
    iFrames = iFrames_;
    iAccums = iAccums_;
    strFileName = strFileName_;
}

void WinSpecTasks::Start::setFileName(const QString &strFileName_)
{
    strFileName = strFileName_;
}

void WinSpecTasks::Start::start()
{

    if (pappState->winSpecState() == AppStateTraits::WinSpecState::Running)
    {
        qDebug() << "WinSpecTasks::Start::start(): AppState::WinSpecState::Running";
        emit finished();
    }
    else
    {
        if (pwinSpec->running())
        {
            qDebug() << "WinSpecTasks::Start::start(): Pozor, WinSpec bezi, cekam, nez ho zastavite!";
            QTimer::singleShot(300, this, SLOT(start()));
//            emit failed();
//            emit finished();
            return;
        }
        if (autoGetParams) {
            if (cal_) {
                pwinSpecParams = pappState->calParamsAt(pappState->getCurrExpNumber());
            } else {
                pwinSpecParams = pappState->expParamsAt(pappState->getCurrExpNumber());
            }
        }
        if (pwinSpecParams) {
            pwinSpecParams->takeOne(&dblExposure, &iAccums, &iFrames, strFileName);
        }
        pappState->setLastExpParams(dblExposure, iAccums, iFrames, strFileName);
        pwinSpec->start(dblExposure, iAccums, iFrames, strFileName);
        pappState->measurementStartedTime();
        emit finished();
    }
}

void WinSpecTasks::Start::stop()
{
    pwinSpec->stop();
    ExpTask::stop();
}

WinSpecTasks::Start::~Start()
{
}


WinSpecTasks::LogLastExpParams::LogLastExpParams(AppState *appState, QObject *parent) :
    ExpTask(parent), appState_(appState), measurementLog_(appState->measurementLog())
{
}

void WinSpecTasks::LogLastExpParams::start()
{
    double expo;
    int acc;
    int frm;
    QString fn;
    appState_->lastExpParams(&expo, &acc, &frm, &fn);
    measurementLog_->setExpParams(expo, acc, frm, fn);
    measurementLog_->setSaveGrPos(appState_->lastGrPos());

    emit finished();
}

WinSpecTasks::AddExpNumber::AddExpNumber(AppState *appState, int shift, QObject *parent) :
    ExpTask(parent), appState_(appState), shift_(shift)
{
}

void WinSpecTasks::AddExpNumber::start()
{
    qDebug() << "WinSpecTasks::AddExpNumber::start(): setting current expNumber to" << appState_->getCurrExpNumber() + shift_;
    appState_->setCurrExpNumber(appState_->getCurrExpNumber() + shift_);
    emit finished();
}

WinSpecTasks::ExpList::ExpList(AppState *appState, bool cal, int expNumber, QObject *parent) :
    ExpTaskList(parent)
{
    ExpTaskListTraits::TaskItem taskItem;
    WaitTaskListTraits::TaskItem waitTaskItem;

    if (cal && appState->initWinSpecParams()->cal.at(expNumber).enableLampSwitch) {
        biomolecules::sprelay::core::k8090::CommandID commandId;
        if (appState->relaySettings()->calibration_lamp_switch_on) {
            commandId = biomolecules::sprelay::core::k8090::CommandID::RelayOn;
        } else {
            commandId = biomolecules::sprelay::core::k8090::CommandID::RelayOff;
        }
        taskItem.task = new RelayTasks::SwitchRelay{
            appState->k8090(),
            commandId,
            appState->relaySettings()->calibration_lamp_switch_id};
        taskItem.taskType = ExpTaskListTraits::TaskType::SwitchRelay;
        addTask(taskItem);

        WaitTaskListTraits::TaskItem waitTaskItem;
        TimeSpan calibration_lamp_switch_delay;
        calibration_lamp_switch_delay
            .fromMSec(appState->relaySettings()->calibration_lamp_switch_delay_msec);
        waitTaskItem.task = new DelayWaitTask(
            appState, &calibration_lamp_switch_delay, this);
        waitTaskItem.waitFor = WaitTaskListTraits::WaitFor::Lamp;
        taskItem.task = new WaitExpTask(waitTaskItem, this);
        taskItem.taskType = ExpTaskListTraits::TaskType::WaitExp;
        addTask(taskItem);

        taskItem.task = new WaitingTask(this);
        taskItem.taskType = ExpTaskListTraits::TaskType::Waiting;
        addTask(taskItem);
    }

    taskItem.task = new WinSpecTasks::Start(appState, cal, this);
    taskItem.taskType = ExpTaskListTraits::TaskType::WinSpecStart;
    addTask(taskItem);

    if (!cal) {
        if (appState->initWinSpecParams()->tExp.tExp) {
            taskItem.task = new NeslabTasks::ReadT(appState, this);
            taskItem.taskType = ExpTaskListTraits::TaskType::NesalbReadT;
            addTask(taskItem);
        }

        taskItem.task = new WinSpecTasks::LogLastExpParams(appState, this);
        taskItem.taskType = ExpTaskListTraits::TaskType::WinSpecLogLastExpParams;
        addTask(taskItem);

        taskItem.task = new SaveExpLog(appState->measurementLog(), this);
        taskItem.taskType = ExpTaskListTraits::TaskType::SaveLog;
        addTask(taskItem);
    }

    waitTaskItem.task  = new WinSpecWaitTask(appState, appState);
    waitTaskItem.waitFor = WaitTaskListTraits::WaitFor::WinSpec;
    taskItem.task =  new WaitExpTask(waitTaskItem, this);
    taskItem.taskType = ExpTaskListTraits::TaskType::WaitExp;
    addTask(taskItem);

    taskItem.task = new WaitingTask(this);
    taskItem.taskType = ExpTaskListTraits::TaskType::Waiting;
    addTask(taskItem);

    if (cal && appState->initWinSpecParams()->cal.at(expNumber).enableLampSwitch) {
        biomolecules::sprelay::core::k8090::CommandID commandId;
        if (appState->relaySettings()->calibration_lamp_switch_on) {
            commandId = biomolecules::sprelay::core::k8090::CommandID::RelayOff;
        } else {
            commandId = biomolecules::sprelay::core::k8090::CommandID::RelayOn;
        }
        taskItem.task = new RelayTasks::SwitchRelay{
            appState->k8090(),
            commandId,
            appState->relaySettings()->calibration_lamp_switch_id};
        taskItem.taskType = ExpTaskListTraits::TaskType::SwitchRelay;
        addTask(taskItem);
    }
}

StageTasks::Run::Run(AppState *pappState, int dest,
                     StageControlTraits::ReferenceType refType, QObject *parent) :
    ExpTask(parent), pstageControl_(pappState->stageControl()), pappState_(pappState), dest_(dest), refType_(refType)
{
}

void StageTasks::Run::start()
{
    if (!pstageControl_->connected())
    {
        QMessageBox::critical(pappState_->appCore()->mainWindow(),
                              tr("Stage is not connected!"),
                              tr("Stage failed to start movement because it is not connected."),
                              QMessageBox::Ok);
        emit taskFailed();
        emit finished();
        return;
    }
    if (pstageControl_->motorRunning())
    {
        QMessageBox::critical(pappState_->appCore()->mainWindow(),
                              tr("Stage is already running!"),
                              tr("Stage failed to start movement because it is already running."),
                              QMessageBox::Ok);
        emit taskFailed();
        emit finished();
        return;
    }
    pstageControl_->run(dest_, refType_);
    emit finished();
}

void StageTasks::Run::stop()
{
    ExpTask::stop();
}

StageTasks::Run::~Run()
{
}

StageTasks::GoToLim::GoToLim(AppState *pappState, StageControlTraits::LimType limType,
                             QObject *parent) :
    ExpTask(parent), pstageControl_(pappState->stageControl()), pappState_(pappState), limType_(limType)
{
}

void StageTasks::GoToLim::start()
{
    if (!pstageControl_->connected())
    {
        QMessageBox::critical(pappState_->appCore()->mainWindow(),
                              tr("Stage is not connected!"),
                              tr("Stage failed to start movement because it is not connected."),
                              QMessageBox::Ok);
        emit taskFailed();
        emit finished();
        return;
    }
    if (pstageControl_->motorRunning())
    {
        QMessageBox::critical(pappState_->appCore()->mainWindow(),
                              tr("Stage is already running!"),
                              tr("Stage failed to start movement because it is already running."),
                              QMessageBox::Ok);
        emit taskFailed();
        emit finished();
        return;
    }
    switch (limType_)
    {
    case StageControlTraits::LimType::LowerLimit :
        pstageControl_->goToLowerLim();
        break;
    case StageControlTraits::LimType::UpperLimit :
        pstageControl_->goToUpperLim();
        break;
    }
    emit finished();
}

void StageTasks::GoToLim::stop()
{
    ExpTask::stop();
}

StageTasks::GoToLim::~GoToLim()
{
}

StageTasks::SendToPos::SendToPos(AppState *pappState, StageControlTraits::PosType posType,
                             QObject *parent) :
    ExpTask(parent), pstageControl_(pappState->stageControl()), pappState_(pappState), posType_(posType)
{
}

void StageTasks::SendToPos::start()
{
    if (!pstageControl_->connected())
    {
        QMessageBox::critical(pappState_->appCore()->mainWindow(),
                              tr("Stage is not connected!"),
                              tr("Stage failed to start movement because it is not connected."),
                              QMessageBox::Ok);
        emit taskFailed();
        emit finished();
        return;
    }
    if (pstageControl_->motorRunning())
    {
        QMessageBox::critical(pappState_->appCore()->mainWindow(),
                              tr("Stage is already running!"),
                              tr("Stage failed to start movement because it is already running."),
                              QMessageBox::Ok);
        emit taskFailed();
        emit finished();
        return;
    }
    switch (posType_)
    {
    case StageControlTraits::PosType::Measurement :
        pstageControl_->goToMeas();
        break;
    case StageControlTraits::PosType::Calibration :
        pstageControl_->goToCal();
        break;
    }
    emit finished();
}

void StageTasks::SendToPos::stop()
{
    ExpTask::stop();
}

StageTasks::SendToPos::~SendToPos()
{
}

StageTasks::SwitchPower::SwitchPower(AppState *pappState, StageTasks::SwitchPower::Power power, QObject *parent) :
    ExpTask(parent), pstageControl_(pappState->stageControl()), pappState_(pappState), power_(power)
{
}

void StageTasks::SwitchPower::start()
{
    if (!pstageControl_->connected())
    {
        QMessageBox::critical(pappState_->appCore()->mainWindow(),
                              tr("Stage is not connected"),
                              tr("Stage failed to start movement because it is not connected"),
                              QMessageBox::Ok);
        emit taskFailed();
        emit finished();
        return;
    }
    switch (power_)
    {
    case Power::On :
        pstageControl_->powerOn();
        break;
    case Power::Off :
        pstageControl_->powerOff();
    }

    emit finished();
}

void StageTasks::SwitchPower::stop()
{
    ExpTask::stop();
}

StageTasks::SwitchPower::~SwitchPower()
{
}

StageTasks::SetLim::SetLim(AppState *pappState,
                           StageControlTraits::LimType limType, int limVal,
                           QObject *parent) :
    ExpTask(parent), pstageControl_(pappState->stageControl()),
    pappState_(pappState), limType_(limType), limVal_(limVal)
{
}

void StageTasks::SetLim::start()
{
    switch (limType_)
    {
    case StageControlTraits::LimType::LowerLimit :
        if (limVal_) {
            limVal_ = pstageControl_->upperLim() - limVal_;
            pstageControl_->setLowerLim(limVal_);
        } else {
            pstageControl_->setLowerLim();
        }
        break;
    case StageControlTraits::LimType::UpperLimit :
        if (limVal_) {
            limVal_ = pstageControl_->lowerLim() + limVal_;
            pstageControl_->setUpperLim(limVal_);
        } else {
            pstageControl_->setUpperLim();
        }
    }
    emit finished();
}

void StageTasks::SetLim::stop()
{
    ExpTask::stop();
}

StageTasks::SetLim::~SetLim()
{

}

StageTasks::Initialize::Initialize(AppState *pappState, StageControlTraits::InitType initType, int limVal, QObject *parent) :
    WaitTaskList(parent), appState_(pappState), pstageControl_(pappState->stageControl()), initType_(initType)
{
    ExpTaskListTraits::TaskItem taskItem;
    WaitTaskListTraits::TaskItem waitTaskItem;

    TimeSpan timeSpan;
    taskItem.task = new StartWaitingTask(pappState, &timeSpan, this);
    taskItem.taskType = ExpTaskListTraits::TaskType::StartWaiting;
    addTask(taskItem);

    taskItem.task = new StageTasks::SwitchPower(pappState, StageTasks::SwitchPower::Power::On, this);
    taskItem.taskType = ExpTaskListTraits::TaskType::StageControlSwitchPower;
    addTask(taskItem);

    if (static_cast<bool>(initType == StageControlTraits::InitType::None)) {
        initType = StageControlTraits::InitType::LowerLimit;
    }
    if (static_cast<bool>(initType & StageControlTraits::InitType::LowerLimit)) {
        taskItem.task = new StageTasks::GoToLim(pappState, StageControlTraits::LimType::LowerLimit, this);
        taskItem.taskType = ExpTaskListTraits::TaskType::StageControlToLimit;
        addTask(taskItem);

        waitTaskItem.task = new StageControlWaitTask(pappState, pappState);
        waitTaskItem.waitFor = WaitTaskListTraits::WaitFor::Motor;
        taskItem.task = new WaitExpTask(waitTaskItem, this);
        taskItem.taskType = ExpTaskListTraits::TaskType::WaitExp;
        addTask(taskItem);

        taskItem.task  = new WaitingTask(this);
        taskItem.taskType = ExpTaskListTraits::TaskType::Waiting;
        addTask(taskItem);

        taskItem.task = new StageTasks::SetLim(pappState, StageControlTraits::LimType::LowerLimit, limVal, this);
        taskItem.taskType = ExpTaskListTraits::TaskType::StageControlSetLimit;
        addTask(taskItem);
    }
    if (static_cast<bool>(initType & StageControlTraits::InitType::UpperLimit)) {
        taskItem.task = new StageTasks::GoToLim(pappState, StageControlTraits::LimType::UpperLimit, this);
        taskItem.taskType = ExpTaskListTraits::TaskType::StageControlToLimit;
        addTask(taskItem);

        waitTaskItem.task = new StageControlWaitTask(pappState, pappState);
        waitTaskItem.waitFor = WaitTaskListTraits::WaitFor::Motor;
        taskItem.task = new WaitExpTask(waitTaskItem, this);
        taskItem.taskType = ExpTaskListTraits::TaskType::WaitExp;
        addTask(taskItem);

        taskItem.task  = new WaitingTask(this);
        taskItem.taskType = ExpTaskListTraits::TaskType::Waiting;
        addTask(taskItem);

        taskItem.task = new StageTasks::SetLim(pappState, StageControlTraits::LimType::UpperLimit, limVal, this);
        taskItem.taskType = ExpTaskListTraits::TaskType::StageControlSetLimit;
        addTask(taskItem);
    }
    taskItem.task = new StageTasks::SwitchPower(pappState, StageTasks::SwitchPower::Power::Off, this);
    taskItem.taskType = ExpTaskListTraits::TaskType::StageControlSwitchPower;
    addTask(taskItem);

    taskItem.task = new FinishWaitingTask(pappState, this);
    taskItem.taskType = ExpTaskListTraits::TaskType::StartWaiting;
    addTask(taskItem);
}

void StageTasks::Initialize::start()
{
    successful_ = false;
    QString strErrMsg;
    if (!pstageControl_->connected()) {
        if (pstageControl_->connect(strErrMsg)) {
            successful_ = true;
        } else {
            QMessageBox::critical(appState_->appCore()->mainWindow(),
                                  tr("Connection to the stage failed!"),
                                  tr("Connection to the stage failed:\n") + strErrMsg,
                                  QMessageBox::Ok);
        }
    } else
        successful_ = true;
    if (successful_) {
        WaitTaskList::start();
    }
}

void StageTasks::Initialize::stop()
{
    successful_ = false;
    pstageControl_->stop();
    pstageControl_->powerOff();
    appState_->removeWaitingState(WaitTaskListTraits::WaitFor::Delay | WaitTaskListTraits::WaitFor::Motor);
    WaitTaskList::stop();
}

void StageTasks::Initialize::taskListFinished()
{
    if (successful_)
        pstageControl_->setInitialized();
    if (static_cast<bool>(initType_ != StageControlTraits::InitType::None))
    {
        if (static_cast<bool>(~initType_ & StageControlTraits::InitType::LowerLimit)) {
            pstageControl_->setLowerLim(pstageControl_->upperLim() - pstageControl_->stageRange());
        }
        if (static_cast<bool>(~initType_ & StageControlTraits::InitType::UpperLimit)) {
            pstageControl_->setUpperLim(pstageControl_->lowerLim() + pstageControl_->stageRange());
        }
    }
    WaitTaskList::taskListFinished();
}

void StageTasks::Initialize::onFailed()
{
    successful_ = false;
    WaitTaskList::onFailed();
}

StageTasks::Initialize::~Initialize()
{
}

StageTasks::GoToLimWaitList::GoToLimWaitList(AppState *pappState, StageControlTraits::LimType limType, QObject *parent) :
    WaitTaskList(parent), appState_(pappState), stageControl_(pappState->stageControl())
{
    TimeSpan timeSpan;
    ExpTaskListTraits::TaskItem taskItem;
    WaitTaskListTraits::TaskItem waitTaskItem;
    taskItem.task = new StartWaitingTask(pappState, &timeSpan, this);
    taskItem.taskType = ExpTaskListTraits::TaskType::StartWaiting;
    addTask(taskItem);

    taskItem.task = new StageTasks::SwitchPower(pappState, StageTasks::SwitchPower::Power::On, this);
    taskItem.taskType = ExpTaskListTraits::TaskType::StageControlSwitchPower;
    addTask(taskItem);

    taskItem.task = new StageTasks::GoToLim(pappState, limType, this);
    taskItem.taskType = ExpTaskListTraits::TaskType::StageControlToLimit;
    addTask(taskItem);

    waitTaskItem.task = new StageControlWaitTask(pappState, pappState);
    waitTaskItem.waitFor = WaitTaskListTraits::WaitFor::Motor;
    taskItem.task = new WaitExpTask(waitTaskItem, this);
    taskItem.taskType = ExpTaskListTraits::TaskType::WaitExp;
    addTask(taskItem);

    taskItem.task  = new WaitingTask(this);
    taskItem.taskType = ExpTaskListTraits::TaskType::Waiting;
    addTask(taskItem);

    taskItem.task = new StageTasks::SetLim(pappState, limType, 0, this);
    taskItem.taskType = ExpTaskListTraits::TaskType::StageControlSetLimit;
    addTask(taskItem);

    taskItem.task = new StageTasks::SwitchPower(pappState, StageTasks::SwitchPower::Power::Off, this);
    taskItem.taskType = ExpTaskListTraits::TaskType::StageControlSwitchPower;
    addTask(taskItem);

    taskItem.task = new FinishWaitingTask(pappState, this);
    taskItem.taskType = ExpTaskListTraits::TaskType::StartWaiting;
    addTask(taskItem);
}

void StageTasks::GoToLimWaitList::start()
{
    QString strErrMsg;
    bool successful;
    if (!stageControl_->connected()) {
        if (stageControl_->connect(strErrMsg)) {
            successful = true;
        } else {
            QMessageBox::critical(appState_->appCore()->mainWindow(),
                                  tr("Connection to the stage failed!"),
                                  tr("Connection to the stage failed:\n") + strErrMsg,
                                  QMessageBox::Ok);
        }
    } else
        successful = true;
    if (successful) {
        WaitTaskList::start();
    }
}

void StageTasks::GoToLimWaitList::stop()
{
    stageControl_->stop();
    stageControl_->powerOff();
    appState_->removeWaitingState(WaitTaskListTraits::WaitFor::Delay | WaitTaskListTraits::WaitFor::Motor);
    WaitTaskList::stop();
}

StageTasks::GoToPos::GoToPos(AppState *pappState, StageControlTraits::PosType posType, QObject *parent) :
    WaitTaskList(parent), pstageControl_(pappState->stageControl())
{
    successful_ = false;
    QString strErrMsg;
    if (!pstageControl_->connected()) {
        if (pstageControl_->connect(strErrMsg)) {
            successful_ = true;
        } else {
            QMessageBox::critical(pappState->appCore()->mainWindow(),
                                  tr("Connection to the stage failed!"),
                                  tr("Connection to the stage failed:\n") + strErrMsg,
                                  QMessageBox::Ok);
        }
    }
    else
        successful_ = true;
    if (successful_)
    {
        ExpTaskListTraits::TaskItem taskItem;
        WaitTaskListTraits::TaskItem waitTaskItem;

        taskItem.task = new StageTasks::SwitchPower(pappState, StageTasks::SwitchPower::Power::On, this);
        taskItem.taskType = ExpTaskListTraits::TaskType::StageControlSwitchPower;
        addTask(taskItem);

        taskItem.task = new StageTasks::SendToPos(pappState, posType, this);
        taskItem.taskType = ExpTaskListTraits::TaskType::StageControlSendToPos;
        addTask(taskItem);

        waitTaskItem.task = new StageControlWaitTask(pappState, pappState);
        waitTaskItem.waitFor = WaitTaskListTraits::WaitFor::Motor;
        taskItem.task = new WaitExpTask(waitTaskItem, this);
        taskItem.taskType = ExpTaskListTraits::TaskType::WaitExp;
        addTask(taskItem);

        taskItem.task  = new WaitingTask(this);
        taskItem.taskType = ExpTaskListTraits::TaskType::Waiting;
        addTask(taskItem);

        taskItem.task = new StageTasks::SwitchPower(pappState, StageTasks::SwitchPower::Power::Off, this);
        taskItem.taskType = ExpTaskListTraits::TaskType::StageControlSwitchPower;
        addTask(taskItem);
    }
}

void StageTasks::GoToPos::stop()
{
    successful_ = false;
    pstageControl_->stop();
    pstageControl_->powerOff();
    WaitTaskList::stop();
}

StageTasks::GoToPos::~GoToPos()
{
}

StageTasks::GoToPosExpList::GoToPosExpList(AppState *appState, StageControlTraits::PosType posType, QObject *parent) :
    ExpTaskList(parent)
{
    ExpTaskListTraits::TaskItem taskItem;
    WaitTaskListTraits::TaskItem waitTaskItem;

    taskItem.task = new StageTasks::SwitchPower(appState, StageTasks::SwitchPower::Power::On, this);
    taskItem.taskType = ExpTaskListTraits::TaskType::StageControlSwitchPower;
    addTask(taskItem);

    taskItem.task = new StageTasks::SendToPos(appState, posType, this);
    taskItem.taskType = ExpTaskListTraits::TaskType::StageControlSendToPos;
    addTask(taskItem);

    waitTaskItem.task = new StageControlWaitTask(appState, appState);
    waitTaskItem.waitFor = WaitTaskListTraits::WaitFor::Motor;
    taskItem.task = new WaitExpTask(waitTaskItem, this);
    taskItem.taskType = ExpTaskListTraits::TaskType::WaitExp;
    addTask(taskItem);

    taskItem.task = new WaitingTask(this);
    taskItem.taskType = ExpTaskListTraits::TaskType::Waiting;
    addTask(taskItem);

    taskItem.task = new StageTasks::SwitchPower(appState, StageTasks::SwitchPower::Power::Off, this);
    taskItem.taskType = ExpTaskListTraits::TaskType::StageControlSwitchPower;
    addTask(taskItem);
}

NeslabTasks::Temperatures::Temperatures(double startT, double stepT, double endT) :
    startT_(startT), stepT_(stepT), endT_(endT), tCounter(0)
{
}

NeslabTasks::Temperatures::Temperatures(const QList<double> &TList)
{
    temperatures = TList;
}

double NeslabTasks::Temperatures::takeT()
{
    int sign = ((endT_ == startT_) ? 0 : ( (endT_ > startT_) ? 1 : -1) );
    int numMeas = sign * int((endT_ - startT_) / stepT_) + 1;

    if (!temperatures.isEmpty()) {
        if (temperatures.size() > 1) {
            return temperatures.takeFirst();
        } else {
            return temperatures.at(0);
        }
    } else {
        if (tCounter < numMeas) {
            return startT_ + sign * stepT_ * tCounter++;
        } else {
            return endT_;
        }
    }
}

NeslabTasks::SetT::SetT(AppState *appState, Temperatures *measT, QObject *parent) :
    ExpTask(parent), appState_(appState), neslab_(appState->neslab()),
    measT_(measT), useTemperatures(true), autoGetT(false), setT_(true)
{
}

NeslabTasks::SetT::SetT(AppState *appState, double t, QObject *parent) :
    ExpTask(parent), appState_(appState), neslab_(appState->neslab()),
    measT_(nullptr), useTemperatures(false), t_(t), autoGetT(false), setT_(true)
{
}

NeslabTasks::SetT::SetT(AppState *appState, QObject *parent) :
    ExpTask(parent), appState_(appState), neslab_(appState->neslab()),
    measT_(nullptr), useTemperatures(true), autoGetT(true), shiftT_(false), setT_(true)
{
}

NeslabTasks::SetT::SetT(AppState *appState, bool shiftT, int shift, bool setT,
                        QObject *parent) :
    ExpTask(parent), appState_(appState), neslab_(appState->neslab()),
    measT_(nullptr), useTemperatures(true), autoGetT(true), shiftT_(shiftT),
    shift_(shift), setT_(setT)
{
}

void NeslabTasks::SetT::start()
{
    if (useTemperatures) {
        if (autoGetT) {
            if (shiftT_) {
                measT_ = appState_->temperaturesTakeAt(
                            appState_->getCurrExpNumber() + shift_);
            } else {
            measT_ = appState_->temperaturesTakeAt(
                        appState_->getCurrExpNumber());
            }
        }
        int T = measT_->takeT();
        if (setT_) {
            qDebug() << "NeslabTasks::SetT::start(): setting setpoint" << T;
            neslab_->setSetpointCommand(T);
        } else {
            qDebug() << "NeslabTasks::SetT::start(): skipping setpoint" << T;
        }
    } else {
        qDebug() << "NeslabTasks::SetT::start(): setting setpoint" << t_;
        neslab_->setSetpointCommand(t_);
    }
    neslab_->readSetpointCommand();
    emit finished();
}


NeslabTasks::ReadT::ReadT(AppState *appState, QObject *parent) :
    ExpTask(parent), neslab_(appState->neslab()), appCore_(appState->appCore())
{
}

void NeslabTasks::ReadT::start()
{
    connect(neslab_, &Neslab::readExternalSensorFinished, appCore_, &AppCore::onReadExpT);
    connect(neslab_, &Neslab::readInternalTemperatureFinished, appCore_, &AppCore::onReadExpT);
    neslab_->readTemperatureCommand();
    emit finished();
}

RelayTasks::SwitchRelay::SwitchRelay(
    biomolecules::sprelay::core::k8090::K8090 *k8090,
    biomolecules::sprelay::core::k8090::CommandID commandId,
    biomolecules::sprelay::core::k8090::RelayID relayId,
    QObject *parent)
    : ExpTask{parent}, k8090_{k8090}, commandId_{commandId}, relayId_{relayId}
{}

void RelayTasks::SwitchRelay::start()
{
    if (commandId_ == biomolecules::sprelay::core::k8090::CommandID::RelayOn) {
        qDebug() << "RelayTasks::SwitchRelay::start(): Relay on!";
        k8090_->switchRelayOn(relayId_);
    } else if (commandId_ == biomolecules::sprelay::core::k8090::CommandID::RelayOff) {
        qDebug() << "RelayTasks::SwitchRelay::start(): Relay off!";
        k8090_->switchRelayOff(relayId_);
    }
    emit finished();
}

GratingTasks::SendToPos::SendToPos(AppState *appState, WinSpecTasks::Params *wsp, QObject *parent) :
    ExpTask(parent), appState_(appState), winSpecParams_(wsp), useWinSpecParams(true), autoGetPos(false)
{
}

GratingTasks::SendToPos::SendToPos(AppState *appState, int pos, QObject *parent) :
    ExpTask(parent), appState_(appState), useWinSpecParams(false), autoGetPos(false), pos_(pos)
{
}

GratingTasks::SendToPos::SendToPos(AppState *appState, bool shiftGr, int shiftGrExpNum, QObject *parent) :
    ExpTask(parent), appState_(appState), useWinSpecParams(true), autoGetPos(true), shiftGr_(shiftGr),
    shiftGrExpNum_(shiftGrExpNum)
{
}

void GratingTasks::SendToPos::start()
{
    if (useWinSpecParams) {
        if (autoGetPos) {
            if (shiftGr_) {
                pos_ = appState_->expParamsAt(appState_->getCurrExpNumber() + shiftGrExpNum_)->grPos();
            } else {
                pos_ = appState_->expParamsAt(appState_->getCurrExpNumber())->grPos();
            }
        } else {
            pos_ = winSpecParams_->grPos();
        }
    }
    qDebug() << "GratingTasks::SendToPos::start(): sending to" << pos_ << ", expNumber" << appState_->getCurrExpNumber() + shiftGrExpNum_;
    appState_->setLastGrPos(pos_);
    emit finished();
}

SaveExpLog::SaveExpLog(MeasurementLog *measurementLog, QObject *parent) :
    ExpTask(parent), measurementLog_(measurementLog)
{
}

void SaveExpLog::start()
{
    measurementLog_->saveMeasurement();
    emit finished();
}

WholeExtExpList::WholeExtExpList(AppState *appState, QObject *parent) :
    ExpTaskList(parent)
{
    AppStateTraits::InitWinSpecParams *params = appState->initWinSpecParams();

    ExpTaskListTraits::TaskItem taskItem;
    WaitTaskListTraits::TaskItem waitTaskItem;

    for (int ii = 0; ii < params->expe.size(); ++ii) {
        taskItem.task = new WinSpecTasks::ExpList(appState, false, ii, this);
        taskItem.taskType = ExpTaskListTraits::TaskType::WinSpecExpList;
        addTask(taskItem);

        if (params->cal.at(ii).autoCal) {
            TimeSpan timeSpan;
            taskItem.task = new StartWaitingTask(appState, &timeSpan, this);
            taskItem.taskType = ExpTaskListTraits::TaskType::StartWaiting;
            addTask(taskItem);

            taskItem.task = new StageTasks::GoToPosExpList(appState, StageControlTraits::PosType::Calibration, this);
            taskItem.taskType = ExpTaskListTraits::TaskType::StageControlGoToPosExpList;
            addTask(taskItem);

            taskItem.task = new WinSpecTasks::ExpList(appState, true, ii, this);
            taskItem.taskType = ExpTaskListTraits::TaskType::WinSpecExpList;
            addTask(taskItem);

            ForkJoinTask *fj = new ForkJoinTask(2, this);

            taskItem.task = new StageTasks::GoToPosExpList(appState, StageControlTraits::PosType::Measurement, this);
            taskItem.taskType = ExpTaskListTraits::TaskType::StageControlGoToPosExpList;
            fj->addTask(taskItem, 0);

            if (ii == params->expe.size() - 1) {
                taskItem.task = new GratingTasks::SendToPos(appState, params->expe.at(0).grPos, this);
            } else {
                taskItem.task = new GratingTasks::SendToPos(appState, true, 1, this);
            }
            taskItem.taskType = ExpTaskListTraits::TaskType::GratingSendToPos;
            fj->addTask(taskItem, 1);

            waitTaskItem.task = new GratingWaitTask(appState, appState);
            waitTaskItem.waitFor = WaitTaskListTraits::WaitFor::Grating;
            taskItem.task = new WaitExpTask(waitTaskItem, this);
            taskItem.taskType = ExpTaskListTraits::TaskType::WaitExp;
            fj->addTask(taskItem, 1);

            taskItem.task = new WaitingTask(this);
            taskItem.taskType = ExpTaskListTraits::TaskType::Waiting;
            fj->addTask(taskItem, 1);

            taskItem.task = fj;
            taskItem.taskType = ExpTaskListTraits::TaskType::ForkJoin;
            addTask(taskItem);
            // ForkJoinTask filling end

            taskItem.task = new FinishWaitingTask(appState, this);
            taskItem.taskType = ExpTaskListTraits::TaskType::StartWaiting;
            addTask(taskItem);

        } else {
            TimeSpan timeSpan;
            taskItem.task = new StartWaitingTask(appState, &timeSpan, this);
            taskItem.taskType = ExpTaskListTraits::TaskType::StartWaiting;
            addTask(taskItem);

            if (ii == params->expe.size() - 1) {
                taskItem.task = new GratingTasks::SendToPos(appState, params->expe.at(0).grPos, this);
            } else {
                taskItem.task = new GratingTasks::SendToPos(appState, true, 1, this);
            }
            taskItem.taskType = ExpTaskListTraits::TaskType::GratingSendToPos;
            addTask(taskItem);

            waitTaskItem.task = new GratingWaitTask(appState, appState);
            waitTaskItem.waitFor = WaitTaskListTraits::WaitFor::Grating;
            taskItem.task = new WaitExpTask(waitTaskItem, this);
            taskItem.taskType = ExpTaskListTraits::TaskType::WaitExp;
            addTask(taskItem);

            taskItem.task = new WaitingTask(this);
            taskItem.taskType = ExpTaskListTraits::TaskType::Waiting;
            addTask(taskItem);

            taskItem.task = new FinishWaitingTask(appState, this);
            taskItem.taskType = ExpTaskListTraits::TaskType::StartWaiting;
            addTask(taskItem);
        }
        taskItem.task = new WinSpecTasks::AddExpNumber(
                    appState, 1, this);
        taskItem.taskType = ExpTaskListTraits::TaskType::WinSpecAddExpNumber;
        addTask(taskItem);
    }
}

TExpList::TExpList(AppState *appState, TimeSpan *delay, QObject *parent) :
    TExpList(appState, delay, 0, 0, true, 0, parent)
{
}

TExpList::TExpList(AppState *appState, TimeSpan *delay, int expNumber, int shiftT, bool setT, int shiftGr, QObject *parent) :
    ExpTaskList(parent)
{
    AppStateTraits::InitWinSpecParams *params = appState->initWinSpecParams();

    ExpTaskListTraits::TaskItem taskItem;
    WaitTaskListTraits::TaskItem waitTaskItem;
    ForkJoinTask * forkJoinTask;

    taskItem.task = new WinSpecTasks::ExpList(appState, false, expNumber, this);
    taskItem.taskType = ExpTaskListTraits::TaskType::WinSpecExpList;
    addTask(taskItem);
    if (params->tExp.delayMeas) {
        taskItem.task = new StartWaitingTask(appState, delay, this);
        taskItem.taskType = ExpTaskListTraits::TaskType::StartWaiting;
        addTask(taskItem);

        taskItem.task = new NeslabTasks::SetT(appState, true, shiftT, setT, this);
        taskItem.taskType = ExpTaskListTraits::TaskType::NeslabSetT;
        addTask(taskItem);

        if (params->cal.at(expNumber).autoCal) {
            forkJoinTask = new ForkJoinTask(2, this);

            // thread no. 0
            waitTaskItem.task = new DelayWaitTask(appState, delay, appState);
            waitTaskItem.waitFor = WaitTaskListTraits::WaitFor::Delay;
            taskItem.task = new WaitExpTask(waitTaskItem, this);
            taskItem.taskType = ExpTaskListTraits::TaskType::WaitExp;
            forkJoinTask->addTask(taskItem, 0);

            taskItem.task = new WaitingTask(this);
            taskItem.taskType = ExpTaskListTraits::TaskType::Waiting;
            forkJoinTask->addTask(taskItem, 0);

            // thread no. 1
            taskItem.task = new StageTasks::GoToPosExpList(appState, StageControlTraits::PosType::Calibration, this);
            taskItem.taskType = ExpTaskListTraits::TaskType::StageControlGoToPosExpList;
            forkJoinTask->addTask(taskItem, 1);

            taskItem.task = new WinSpecTasks::ExpList(appState, true, expNumber, this);
            taskItem.taskType = ExpTaskListTraits::TaskType::WinSpecExpList;
            forkJoinTask->addTask(taskItem, 1);

            if (params->extRan.extendedRange && params->expe.size() > 1) {
                ForkJoinTask *fj = new ForkJoinTask(2, this);

                taskItem.task = new StageTasks::GoToPosExpList(appState, StageControlTraits::PosType::Measurement, this);
                taskItem.taskType = ExpTaskListTraits::TaskType::StageControlGoToPosExpList;
                fj->addTask(taskItem, 0);

                taskItem.task = new GratingTasks::SendToPos(appState, true, shiftGr, this);
                taskItem.taskType = ExpTaskListTraits::TaskType::GratingSendToPos;
                fj->addTask(taskItem, 1);

                waitTaskItem.task = new GratingWaitTask(appState, appState);
                waitTaskItem.waitFor = WaitTaskListTraits::WaitFor::Grating;
                taskItem.task = new WaitExpTask(waitTaskItem, this);
                taskItem.taskType = ExpTaskListTraits::TaskType::WaitExp;
                fj->addTask(taskItem, 1);

                taskItem.task = new WaitingTask(this);
                taskItem.taskType = ExpTaskListTraits::TaskType::Waiting;
                fj->addTask(taskItem, 1);

                taskItem.task = fj;
                taskItem.taskType = ExpTaskListTraits::TaskType::ForkJoin;
                forkJoinTask->addTask(taskItem, 1);
            } else {
                taskItem.task = new StageTasks::GoToPosExpList(appState, StageControlTraits::PosType::Measurement, this);
                taskItem.taskType = ExpTaskListTraits::TaskType::StageControlGoToPosExpList;
                forkJoinTask->addTask(taskItem, 1);
            }
            // ForkJoinTask filling end

            taskItem.task = forkJoinTask;
            taskItem.taskType = ExpTaskListTraits::TaskType::ForkJoin;
            addTask(taskItem);
        } else {
            if (params->extRan.extendedRange && params->expe.size() > 1) {
                taskItem.task = new GratingTasks::SendToPos(appState, true, shiftGr, this);
                taskItem.taskType = ExpTaskListTraits::TaskType::GratingSendToPos;
                addTask(taskItem);

                waitTaskItem.task = new GratingWaitTask(appState, appState);
                waitTaskItem.waitFor = WaitTaskListTraits::WaitFor::Grating;
                taskItem.task = new WaitExpTask(waitTaskItem, this);
                taskItem.taskType = ExpTaskListTraits::TaskType::WaitExp;
                addTask(taskItem);
            }

            waitTaskItem.task = new DelayWaitTask(appState, delay, appState);
            waitTaskItem.waitFor = WaitTaskListTraits::WaitFor::Delay;
            taskItem.task = new WaitExpTask(waitTaskItem, this);
            taskItem.taskType = ExpTaskListTraits::TaskType::WaitExp;
            addTask(taskItem);

            taskItem.task = new WaitingTask(this);
            taskItem.taskType = ExpTaskListTraits::TaskType::Waiting;
            addTask(taskItem);
        }

        taskItem.task = new FinishWaitingTask(appState, this);
        taskItem.taskType = ExpTaskListTraits::TaskType::FinishWaiting;
        addTask(taskItem);
    } else if (params->cal.at(expNumber).autoCal) {
        TimeSpan timeSpan;
        taskItem.task = new StartWaitingTask(appState, &timeSpan, this);
        taskItem.taskType = ExpTaskListTraits::TaskType::StartWaiting;
        addTask(taskItem);

        taskItem.task = new NeslabTasks::SetT(appState, true, shiftT, setT, this);
        taskItem.taskType = ExpTaskListTraits::TaskType::NeslabSetT;
        addTask(taskItem);

        taskItem.task = new StageTasks::GoToPosExpList(appState, StageControlTraits::PosType::Calibration, this);
        taskItem.taskType = ExpTaskListTraits::TaskType::StageControlGoToPosExpList;
        addTask(taskItem);

        taskItem.task = new WinSpecTasks::ExpList(appState, true, expNumber, this);
        taskItem.taskType = ExpTaskListTraits::TaskType::WinSpecExpList;
        addTask(taskItem);

        if (params->extRan.extendedRange && params->expe.size() > 1) {
            ForkJoinTask *fj = new ForkJoinTask(2, this);

            taskItem.task = new StageTasks::GoToPosExpList(appState, StageControlTraits::PosType::Measurement, this);
            taskItem.taskType = ExpTaskListTraits::TaskType::StageControlGoToPosExpList;
            fj->addTask(taskItem, 0);

            taskItem.task = new GratingTasks::SendToPos(appState, true, shiftGr, this);
            taskItem.taskType = ExpTaskListTraits::TaskType::GratingSendToPos;
            fj->addTask(taskItem, 1);

            waitTaskItem.task = new GratingWaitTask(appState, appState);
            waitTaskItem.waitFor = WaitTaskListTraits::WaitFor::Grating;
            taskItem.task = new WaitExpTask(waitTaskItem, this);
            taskItem.taskType = ExpTaskListTraits::TaskType::WaitExp;
            fj->addTask(taskItem, 1);

            taskItem.task = new WaitingTask(this);
            taskItem.taskType = ExpTaskListTraits::TaskType::Waiting;
            fj->addTask(taskItem, 1);

            taskItem.task = fj;
            taskItem.taskType = ExpTaskListTraits::TaskType::ForkJoin;
            addTask(taskItem);
        } else {
            taskItem.task = new StageTasks::GoToPosExpList(appState, StageControlTraits::PosType::Measurement, this);
            taskItem.taskType = ExpTaskListTraits::TaskType::StageControlGoToPosExpList;
            addTask(taskItem);
        }

        taskItem.task = new FinishWaitingTask(appState, this);
        taskItem.taskType = ExpTaskListTraits::TaskType::FinishWaiting;
        addTask(taskItem);
    } else if (params->extRan.extendedRange && params->expe.size() > 1) {
        TimeSpan timeSpan;
        taskItem.task = new StartWaitingTask(appState, &timeSpan, this);
        taskItem.taskType = ExpTaskListTraits::TaskType::StartWaiting;
        addTask(taskItem);

        taskItem.task = new GratingTasks::SendToPos(appState, shiftGr, this);
        taskItem.taskType = ExpTaskListTraits::TaskType::GratingSendToPos;
        addTask(taskItem);

        waitTaskItem.task = new GratingWaitTask(appState, appState);
        waitTaskItem.waitFor = WaitTaskListTraits::WaitFor::Grating;
        taskItem.task = new WaitExpTask(waitTaskItem, this);
        taskItem.taskType = ExpTaskListTraits::TaskType::WaitExp;
        addTask(taskItem);

        taskItem.task = new WaitingTask(this);
        taskItem.taskType = ExpTaskListTraits::TaskType::Waiting;
        addTask(taskItem);

        taskItem.task = new FinishWaitingTask(appState, this);
        taskItem.taskType = ExpTaskListTraits::TaskType::FinishWaiting;
        addTask(taskItem);
    }
}

ExtTExpList::ExtTExpList(AppState *appState, TimeSpan *delay, int shiftT,
                         bool last, QObject *parent) :
    ExpTaskList(parent)
{
    AppStateTraits::InitWinSpecParams *params = appState->initWinSpecParams();

    ExpTaskListTraits::TaskItem taskItem;

    int n;
    if (params->batch.batchExp && params->tExp.tExp) {
        n = params->batch.numSpectra;
    } else {
        n = 1;
    }
    if (params->tExp.loop) {
        n = 2 * n;
    }

    for (int ii = 0; ii < params->expe.size() - 1; ++ii) {
        taskItem.task = new TExpList(appState, delay, ii, n, false, n, this);
        taskItem.taskType = ExpTaskListTraits::TaskType::TExpList;
        addTask(taskItem);

        taskItem.task = new WinSpecTasks::AddExpNumber(
                    appState, n, this);
        taskItem.taskType = ExpTaskListTraits::TaskType::WinSpecAddExpNumber;
        addTask(taskItem);
    }
    if (!last) {
        taskItem.task = new TExpList(appState, delay, params->expe.size() - 1, - (params->expe.size() - 1) * n + shiftT, true,
                                     - (params->expe.size() - 1) * n + shiftT, this);
        taskItem.taskType = ExpTaskListTraits::TaskType::TExpList;
        addTask(taskItem);

        taskItem.task = new WinSpecTasks::AddExpNumber(
                    appState, - (params->expe.size() - 1) * n, this);
        taskItem.taskType = ExpTaskListTraits::TaskType::WinSpecAddExpNumber;
        addTask(taskItem);
    }
}

WholeTExpList::WholeTExpList(AppState *appState, QObject *parent) :
    ExpTaskList(parent)
{
    AppStateTraits::InitWinSpecParams *params = appState->initWinSpecParams();
    int sign = ((params->tExp.endT == params->tExp.startT) ?
                    0 :
                    (params->tExp.endT > params->tExp.startT) ?
                        1 : -1);
    int numTMeas = sign * int((params->tExp.endT - params->tExp.startT) /
            params->tExp.stepT) + 1;


    ExpTaskListTraits::TaskItem taskItem;
    WaitTaskListTraits::TaskItem waitTaskItem;
    TExpList *tExpList;
    ExtTExpList *extTExpList;

    if (numTMeas > 1) {
        if (params->extRan.extendedRange) {
            if (params->tExp.loop) {
                extTExpList = new ExtTExpList(appState, &params->tExp.delay, 0, false, this);
                extTExpList->setTimesExec(numTMeas - 1);
                taskItem.task = extTExpList;
                taskItem.taskType = ExpTaskListTraits::TaskType::ExtTExpList;
                addTask(taskItem);

                taskItem.task =  new ExtTExpList(appState, &params->tExp.loopDelay, 1, false, this);
                taskItem.taskType = ExpTaskListTraits::TaskType::ExtTExpList;
                addTask(taskItem);

                taskItem.task = new WinSpecTasks::AddExpNumber(appState, 1, this);
                taskItem.taskType =
                        ExpTaskListTraits::TaskType::WinSpecAddExpNumber;
                addTask(taskItem);

                extTExpList = new ExtTExpList(appState, &params->tExp.loopDelay, 0, false, this);
                extTExpList->setTimesExec(numTMeas - 2);
                taskItem.task = extTExpList;
                taskItem.taskType = ExpTaskListTraits::TaskType::ExtTExpList;
                addTask(taskItem);

                // the last task is may be without delay, so the last task is only
                // spectrum measurement, see next taskItem.
                if (params->expe.size() > 1) {
                    taskItem.task = new ExtTExpList(appState, &params->tExp.loopDelay, 0, true, this);
                    taskItem.taskType = ExpTaskListTraits::TaskType::WinSpecExpList;
                    addTask(taskItem);
                }
            } else {
                extTExpList =
                        new ExtTExpList(appState, &params->tExp.delay, 0, false, this);
                extTExpList->setTimesExec(numTMeas - 1);
                taskItem.task = extTExpList;
                taskItem.taskType = ExpTaskListTraits::TaskType::ExtTExpList;
                addTask(taskItem);

                // the last task is may be without delay, so the last task is only
                // spectrum measurement, see next taskItem.
                if (params->expe.size() > 1) {
                    taskItem.task = new ExtTExpList(appState, &params->tExp.delay, 0, true, this);
                    taskItem.taskType = ExpTaskListTraits::TaskType::ExtTExpList;
                    addTask(taskItem);
                }
            }
            taskItem.task = new WinSpecTasks::ExpList(appState, false, params->expe.size() - 1, this);
            taskItem.taskType = ExpTaskListTraits::TaskType::WinSpecExpList;
            addTask(taskItem);
        } else {
            if (params->tExp.loop) {
                tExpList = new TExpList(appState, &params->tExp.delay, 0, 0, true, 0, this);
                tExpList->setTimesExec(numTMeas - 1);
                taskItem.task = tExpList;
                taskItem.taskType = ExpTaskListTraits::TaskType::TExpList;
                addTask(taskItem);

                taskItem.task =  new TExpList(appState, &params->tExp.loopDelay, 0, 1, true, 0, this);
                taskItem.taskType = ExpTaskListTraits::TaskType::TExpList;
                addTask(taskItem);

                taskItem.task = new WinSpecTasks::AddExpNumber(appState, 1, this);
                taskItem.taskType =
                        ExpTaskListTraits::TaskType::WinSpecAddExpNumber;
                addTask(taskItem);

                tExpList = new TExpList(appState, &params->tExp.loopDelay, this);
                tExpList->setTimesExec(numTMeas - 2);
                taskItem.task = tExpList;
                taskItem.taskType = ExpTaskListTraits::TaskType::TExpList;
                addTask(taskItem);
            } else {
                tExpList =
                        new TExpList(appState, &params->tExp.delay, this);
                tExpList->setTimesExec(numTMeas - 1);
                taskItem.task = tExpList;
                taskItem.taskType = ExpTaskListTraits::TaskType::TExpList;
                addTask(taskItem);
            }
            // the last task is may be without delay, so the last task is only
            // spectrum measurement, see next taskItem.
            taskItem.task = new WinSpecTasks::ExpList(appState, false, 0, this);
            taskItem.taskType = ExpTaskListTraits::TaskType::WinSpecExpList;
            addTask(taskItem);
        }
    }

    if (params->tExp.afterMeas) {
        taskItem.task = new NeslabTasks::SetT(appState, params->tExp.afterMeasT, this);
        taskItem.taskType = ExpTaskListTraits::TaskType::NeslabSetT;
        addTask(taskItem);
    }
    int lastExpNumber;
    if (params->extRan.extendedRange) {
        lastExpNumber = params->expe.size() - 1;
    } else {
        lastExpNumber = 0;
    }
    if (params->cal.at(lastExpNumber).autoCal) {
        TimeSpan timeSpan;
        taskItem.task = new StartWaitingTask(appState, &timeSpan, this);
        taskItem.taskType = ExpTaskListTraits::TaskType::StartWaiting;
        addTask(taskItem);

        taskItem.task = new StageTasks::GoToPosExpList(appState, StageControlTraits::PosType::Calibration, this);
        taskItem.taskType = ExpTaskListTraits::TaskType::StageControlGoToPosExpList;
        addTask(taskItem);

        taskItem.task = new WinSpecTasks::ExpList(appState, true, lastExpNumber, this);
        taskItem.taskType = ExpTaskListTraits::TaskType::WinSpecExpList;
        addTask(taskItem);

        if (params->extRan.extendedRange) {
            ForkJoinTask *fj = new ForkJoinTask(2, this);

            taskItem.task = new StageTasks::GoToPosExpList(appState, StageControlTraits::PosType::Measurement, this);
            taskItem.taskType = ExpTaskListTraits::TaskType::StageControlGoToPosExpList;
            fj->addTask(taskItem, 0);

            taskItem.task = new GratingTasks::SendToPos(appState, params->expe.at(0).grPos, this);
            taskItem.taskType = ExpTaskListTraits::TaskType::GratingSendToPos;
            fj->addTask(taskItem, 1);

            waitTaskItem.task = new GratingWaitTask(appState, appState);
            waitTaskItem.waitFor = WaitTaskListTraits::WaitFor::Grating;
            taskItem.task = new WaitExpTask(waitTaskItem, this);
            taskItem.taskType = ExpTaskListTraits::TaskType::WaitExp;
            fj->addTask(taskItem, 1);

            taskItem.task = new WaitingTask(this);
            taskItem.taskType = ExpTaskListTraits::TaskType::Waiting;
            fj->addTask(taskItem, 1);

            taskItem.task = fj;
            taskItem.taskType = ExpTaskListTraits::TaskType::ForkJoin;
            addTask(taskItem);
        } else {
            taskItem.task = new StageTasks::GoToPosExpList(appState, StageControlTraits::PosType::Measurement, this);
            taskItem.taskType = ExpTaskListTraits::TaskType::StageControlGoToPosExpList;
            addTask(taskItem);
        }

        taskItem.task = new FinishWaitingTask(appState, this);
        taskItem.taskType = ExpTaskListTraits::TaskType::FinishWaiting;
        addTask(taskItem);
    } else if (params->extRan.extendedRange && params->expe.size() > 1) {
        TimeSpan timeSpan;
        taskItem.task = new StartWaitingTask(appState, &timeSpan, this);
        taskItem.taskType = ExpTaskListTraits::TaskType::StartWaiting;
        addTask(taskItem);

        taskItem.task = new GratingTasks::SendToPos(appState, params->expe.at(0).grPos, this);
        taskItem.taskType = ExpTaskListTraits::TaskType::GratingSendToPos;
        addTask(taskItem);

        waitTaskItem.task = new GratingWaitTask(appState, appState);
        waitTaskItem.waitFor = WaitTaskListTraits::WaitFor::Grating;
        taskItem.task = new WaitExpTask(waitTaskItem, this);
        taskItem.taskType = ExpTaskListTraits::TaskType::WaitExp;
        addTask(taskItem);

        taskItem.task = new WaitingTask(this);
        taskItem.taskType = ExpTaskListTraits::TaskType::Waiting;
        addTask(taskItem);

        taskItem.task = new FinishWaitingTask(appState, this);
        taskItem.taskType = ExpTaskListTraits::TaskType::FinishWaiting;
        addTask(taskItem);
    }
}

BatchExpList::BatchExpList(AppState *appState, QObject *parent) :
    ExpTaskList(parent)
{
    AppStateTraits::InitWinSpecParams *params = appState->initWinSpecParams();

    ExpTaskListTraits::TaskItem taskItem;
    WaitTaskListTraits::TaskItem waitTaskItem;
    ForkJoinTask * forkJoinTask;

    if (params->tExp.tExp) {
        TExpList *tExpList;
        int sign = ((params->tExp.endT == params->tExp.startT) ?
                        0 :
                        (params->tExp.endT > params->tExp.startT) ?
                            1 : -1);
        int numTMeas = sign * int((params->tExp.endT - params->tExp.startT) /
                params->tExp.stepT) + 1;
        if (numTMeas > 1) {
            if (params->extRan.extendedRange) {
                ExtTExpList *extTExpList;
                if (params->tExp.loop) {
                    extTExpList = new ExtTExpList(appState, &params->tExp.delay, 0, false, this);
                    extTExpList->setTimesExec(numTMeas - 1);
                    taskItem.task = extTExpList;
                    taskItem.taskType = ExpTaskListTraits::TaskType::ExtTExpList;
                    addTask(taskItem);

                    taskItem.task =  new ExtTExpList(appState, &params->tExp.loopDelay, 1, false, this);
                    taskItem.taskType = ExpTaskListTraits::TaskType::ExtTExpList;
                    addTask(taskItem);

                    taskItem.task = new WinSpecTasks::AddExpNumber(appState, 1, this);
                    taskItem.taskType =
                            ExpTaskListTraits::TaskType::WinSpecAddExpNumber;
                    addTask(taskItem);

                    extTExpList = new ExtTExpList(appState, &params->tExp.loopDelay, 0, false, this);
                    extTExpList->setTimesExec(numTMeas - 2);
                    taskItem.task = extTExpList;
                    taskItem.taskType = ExpTaskListTraits::TaskType::ExtTExpList;
                    addTask(taskItem);

                    // the last task is may be without delay, so the last task is only
                    // spectrum measurement, see next taskItem.
                    if (params->expe.size() > 1) {
                        taskItem.task = new ExtTExpList(appState, &params->tExp.loopDelay, 0, true, this);
                        taskItem.taskType = ExpTaskListTraits::TaskType::WinSpecExpList;
                        addTask(taskItem);
                    }
                } else {
                    extTExpList =
                            new ExtTExpList(appState, &params->tExp.delay, 0, false, this);
                    extTExpList->setTimesExec(numTMeas - 1);
                    taskItem.task = extTExpList;
                    taskItem.taskType = ExpTaskListTraits::TaskType::ExtTExpList;
                    addTask(taskItem);

                    // the last task is may be without delay, so the last task is only
                    // spectrum measurement, see next taskItem.
                    if (params->expe.size() > 1) {
                        taskItem.task = new ExtTExpList(appState, &params->tExp.delay, 0, true, this);
                        taskItem.taskType = ExpTaskListTraits::TaskType::ExtTExpList;
                        addTask(taskItem);
                    }
                }
                taskItem.task = new WinSpecTasks::ExpList(appState, false, params->expe.size() - 1, this);
                taskItem.taskType = ExpTaskListTraits::TaskType::WinSpecExpList;
                addTask(taskItem);
            } else {
                if (params->tExp.loop) {
                    tExpList = new TExpList(appState, &params->tExp.delay, 0, 0, true, 0, this);
                    tExpList->setTimesExec(numTMeas - 1);
                    taskItem.task = tExpList;
                    taskItem.taskType = ExpTaskListTraits::TaskType::TExpList;
                    addTask(taskItem);

                    taskItem.task =  new TExpList(appState, &params->tExp.loopDelay, 0, 1, true, 0, this);
                    taskItem.taskType = ExpTaskListTraits::TaskType::TExpList;
                    addTask(taskItem);

                    taskItem.task = new WinSpecTasks::AddExpNumber(appState, 1, this);
                    taskItem.taskType =
                            ExpTaskListTraits::TaskType::WinSpecAddExpNumber;
                    addTask(taskItem);

                    tExpList = new TExpList(appState, &params->tExp.loopDelay, this);
                    tExpList->setTimesExec(numTMeas - 2);
                    taskItem.task = tExpList;
                    taskItem.taskType = ExpTaskListTraits::TaskType::TExpList;
                    addTask(taskItem);
                } else {
                    tExpList =
                            new TExpList(appState, &params->tExp.delay, this);
                    tExpList->setTimesExec(numTMeas - 1);
                    taskItem.task = tExpList;
                    taskItem.taskType = ExpTaskListTraits::TaskType::TExpList;
                    addTask(taskItem);
                }
                // the last task is may be without delay, so the last task is only
                // spectrum measurement, see next taskItem.
                taskItem.task = new WinSpecTasks::ExpList(appState, false, 0, this);
                taskItem.taskType = ExpTaskListTraits::TaskType::WinSpecExpList;
                addTask(taskItem);
            }
        }
    } else {  // TODO(lumik): Bug? What about extended range?
        taskItem.task = new WinSpecTasks::ExpList(appState, false, 0, this);
        taskItem.taskType = ExpTaskListTraits::TaskType::WinSpecExpList;
        addTask(taskItem);
    }

    int lastExpNumber;
    int n;
    if (params->extRan.extendedRange) {
        lastExpNumber = params->expe.size() - 1;
        if (params->batch.batchExp && params->tExp.tExp) {
            n = params->batch.numSpectra;
        } else {
            n = 1;
        }
        if (params->tExp.tExp && params->tExp.loop) {
            n = 2 * n;
        }
    } else {
        lastExpNumber = 0;
    }

    if (params->batch.delayMeas) {
        taskItem.task = new StartWaitingTask(appState, &params->batch.delay, this);
        taskItem.taskType = ExpTaskListTraits::TaskType::StartWaiting;
        addTask(taskItem);

        if (params->cal.at(lastExpNumber).autoCal) {
            forkJoinTask = new ForkJoinTask(2, this);

            // thread no. 0
            waitTaskItem.task = new DelayWaitTask(appState, &params->batch.delay, appState);
            waitTaskItem.waitFor = WaitTaskListTraits::WaitFor::Delay;
            taskItem.task = new WaitExpTask(waitTaskItem, this);
            taskItem.taskType = ExpTaskListTraits::TaskType::WaitExp;
            forkJoinTask->addTask(taskItem, 0);

            taskItem.task = new WaitingTask(this);
            taskItem.taskType = ExpTaskListTraits::TaskType::Waiting;
            forkJoinTask->addTask(taskItem, 0);

            // thread no. 1
            if (params->tExp.tExp) {
                if (params->extRan.extendedRange) {
                    taskItem.task = new NeslabTasks::SetT(
                                appState,
                                true, - (params->expe.size() - 1) * n + 1,
                                true, this);
                    taskItem.taskType = ExpTaskListTraits::TaskType::NeslabSetT;
                    forkJoinTask->addTask(taskItem, 1);
                } else {
                    taskItem.task = new NeslabTasks::SetT(appState, true, 1, true, this);
                    taskItem.taskType = ExpTaskListTraits::TaskType::NeslabSetT;
                    forkJoinTask->addTask(taskItem, 1);
                }
            }

            taskItem.task = new StageTasks::GoToPosExpList(appState, StageControlTraits::PosType::Calibration, this);
            taskItem.taskType = ExpTaskListTraits::TaskType::StageControlGoToPosExpList;
            forkJoinTask->addTask(taskItem, 1);

            taskItem.task = new WinSpecTasks::ExpList(appState, true, lastExpNumber, this);
            taskItem.taskType = ExpTaskListTraits::TaskType::WinSpecExpList;
            forkJoinTask->addTask(taskItem, 1);

            if (params->extRan.extendedRange) {
                ForkJoinTask *fj = new ForkJoinTask(2, this);

                taskItem.task = new StageTasks::GoToPosExpList(appState, StageControlTraits::PosType::Measurement, this);
                taskItem.taskType = ExpTaskListTraits::TaskType::StageControlGoToPosExpList;
                fj->addTask(taskItem, 0);

                if (params->tExp.tExp) {
                    taskItem.task = new GratingTasks::SendToPos(
                                appState, true,
                                - (params->expe.size() - 1) * n + 1, this);
                    taskItem.taskType = ExpTaskListTraits::TaskType::GratingSendToPos;
                    fj->addTask(taskItem, 1);
                } else {
                    taskItem.task = new GratingTasks::SendToPos(
                                appState, true,
                                - (params->expe.size() - 1) * n, this);
                    taskItem.taskType = ExpTaskListTraits::TaskType::GratingSendToPos;
                    fj->addTask(taskItem, 1);
                }

                waitTaskItem.task = new GratingWaitTask(appState, appState);
                waitTaskItem.waitFor = WaitTaskListTraits::WaitFor::Grating;
                taskItem.task = new WaitExpTask(waitTaskItem, this);
                taskItem.taskType = ExpTaskListTraits::TaskType::WaitExp;
                fj->addTask(taskItem, 1);

                taskItem.task = new WaitingTask(this);
                taskItem.taskType = ExpTaskListTraits::TaskType::Waiting;
                fj->addTask(taskItem, 1);

                taskItem.task = fj;
                taskItem.taskType = ExpTaskListTraits::TaskType::ForkJoin;
                forkJoinTask->addTask(taskItem, 1);
            } else {
                taskItem.task = new StageTasks::GoToPosExpList(appState, StageControlTraits::PosType::Measurement, this);
                taskItem.taskType = ExpTaskListTraits::TaskType::StageControlGoToPosExpList;
                forkJoinTask->addTask(taskItem, 1);
            }
            // ForkJoinTask filling end

            taskItem.task = forkJoinTask;
            taskItem.taskType = ExpTaskListTraits::TaskType::ForkJoin;
            addTask(taskItem);
        } else {
            if (params->tExp.tExp) {
                if (params->extRan.extendedRange) {
                    taskItem.task = new NeslabTasks::SetT(
                                appState,
                                true, - (params->expe.size() - 1) * n + 1,
                                true, this);
                    taskItem.taskType = ExpTaskListTraits::TaskType::NeslabSetT;
                    addTask(taskItem);
                } else {
                    taskItem.task = new NeslabTasks::SetT(appState, true, 1, this);
                    taskItem.taskType = ExpTaskListTraits::TaskType::NeslabSetT;
                    addTask(taskItem);
                }
            }

            if (params->extRan.extendedRange) {
                if (params->tExp.tExp) {
                    taskItem.task = new GratingTasks::SendToPos(
                                appState, true,
                                - (params->expe.size() - 1) * n + 1, this);
                    taskItem.taskType = ExpTaskListTraits::TaskType::GratingSendToPos;
                    addTask(taskItem);
                } else {
                    taskItem.task = new GratingTasks::SendToPos(
                                appState, true,
                                - (params->expe.size() - 1) * n, this);
                    taskItem.taskType = ExpTaskListTraits::TaskType::GratingSendToPos;
                    addTask(taskItem);
                }

                waitTaskItem.task = new GratingWaitTask(appState, appState);
                waitTaskItem.waitFor = WaitTaskListTraits::WaitFor::Grating;
                taskItem.task = new WaitExpTask(waitTaskItem, this);
                taskItem.taskType = ExpTaskListTraits::TaskType::WaitExp;
                addTask(taskItem);
            }

            waitTaskItem.task = new DelayWaitTask(appState, &params->batch.delay, appState);
            waitTaskItem.waitFor = WaitTaskListTraits::WaitFor::Delay;
            taskItem.task = new WaitExpTask(waitTaskItem, this);
            taskItem.taskType = ExpTaskListTraits::TaskType::WaitExp;
            addTask(taskItem);

            taskItem.task = new WaitingTask(this);
            taskItem.taskType = ExpTaskListTraits::TaskType::Waiting;
            addTask(taskItem);
        }

        taskItem.task = new FinishWaitingTask(appState, this);
        taskItem.taskType = ExpTaskListTraits::TaskType::FinishWaiting;
        addTask(taskItem);
    } else if (params->cal.at(0).autoCal) {
        TimeSpan timeSpan;
        taskItem.task = new StartWaitingTask(appState, &timeSpan, this);
        taskItem.taskType = ExpTaskListTraits::TaskType::StartWaiting;
        addTask(taskItem);

        if (params->tExp.tExp) {
            if (params->extRan.extendedRange) {
                taskItem.task = new NeslabTasks::SetT(
                            appState,
                            true, - (params->expe.size() - 1) * n + 1,
                            true, this);
                taskItem.taskType = ExpTaskListTraits::TaskType::NeslabSetT;
                addTask(taskItem);
            } else {
                taskItem.task = new NeslabTasks::SetT(appState, true, 1, this);
                taskItem.taskType = ExpTaskListTraits::TaskType::NeslabSetT;
                addTask(taskItem);
            }
        }

        taskItem.task = new StageTasks::GoToPosExpList(appState, StageControlTraits::PosType::Calibration, this);
        taskItem.taskType = ExpTaskListTraits::TaskType::StageControlGoToPosExpList;
        addTask(taskItem);

        taskItem.task = new WinSpecTasks::ExpList(appState, true, lastExpNumber, this);
        taskItem.taskType = ExpTaskListTraits::TaskType::WinSpecExpList;
        addTask(taskItem);
        if (params->extRan.extendedRange) {
            ForkJoinTask *fj = new ForkJoinTask(2, this);

            taskItem.task = new StageTasks::GoToPosExpList(appState, StageControlTraits::PosType::Measurement, this);
            taskItem.taskType = ExpTaskListTraits::TaskType::StageControlGoToPosExpList;
            fj->addTask(taskItem, 0);

            if (params->tExp.tExp) {
                taskItem.task = new GratingTasks::SendToPos(
                            appState, true,
                            - (params->expe.size() - 1) * n + 1, this);
                taskItem.taskType = ExpTaskListTraits::TaskType::GratingSendToPos;
                fj->addTask(taskItem, 1);
            } else {
                taskItem.task = new GratingTasks::SendToPos(
                            appState, true,
                            - (params->expe.size() - 1) * n, this);
                taskItem.taskType = ExpTaskListTraits::TaskType::GratingSendToPos;
                fj->addTask(taskItem, 1);
            }

            waitTaskItem.task = new GratingWaitTask(appState, appState);
            waitTaskItem.waitFor = WaitTaskListTraits::WaitFor::Grating;
            taskItem.task = new WaitExpTask(waitTaskItem, this);
            taskItem.taskType = ExpTaskListTraits::TaskType::WaitExp;
            fj->addTask(taskItem, 1);

            taskItem.task = new WaitingTask(this);
            taskItem.taskType = ExpTaskListTraits::TaskType::Waiting;
            fj->addTask(taskItem, 1);

            taskItem.task = fj;
            taskItem.taskType = ExpTaskListTraits::TaskType::ForkJoin;
            addTask(taskItem);
        } else {
            taskItem.task = new StageTasks::GoToPosExpList(appState, StageControlTraits::PosType::Measurement, this);
            taskItem.taskType = ExpTaskListTraits::TaskType::StageControlGoToPosExpList;
            addTask(taskItem);
        }

        taskItem.task = new FinishWaitingTask(appState, this);
        taskItem.taskType = ExpTaskListTraits::TaskType::FinishWaiting;
        addTask(taskItem);
    } else if (params->extRan.extendedRange && params->expe.size() > 1) {
        TimeSpan timeSpan;
        taskItem.task = new StartWaitingTask(appState, &timeSpan, this);
        taskItem.taskType = ExpTaskListTraits::TaskType::StartWaiting;
        addTask(taskItem);

        if (params->tExp.tExp) {
            taskItem.task = new GratingTasks::SendToPos(
                        appState, true,
                        - (params->expe.size() - 1) * n + 1, this);
            taskItem.taskType = ExpTaskListTraits::TaskType::GratingSendToPos;
            addTask(taskItem);
        } else {
            taskItem.task = new GratingTasks::SendToPos(
                        appState, true,
                        - (params->expe.size() - 1) * n, this);
            taskItem.taskType = ExpTaskListTraits::TaskType::GratingSendToPos;
            addTask(taskItem);
        }

        waitTaskItem.task = new GratingWaitTask(appState, appState);
        waitTaskItem.waitFor = WaitTaskListTraits::WaitFor::Grating;
        taskItem.task = new WaitExpTask(waitTaskItem, this);
        taskItem.taskType = ExpTaskListTraits::TaskType::WaitExp;
        addTask(taskItem);

        taskItem.task = new WaitingTask(this);
        taskItem.taskType = ExpTaskListTraits::TaskType::Waiting;
        addTask(taskItem);

        taskItem.task = new FinishWaitingTask(appState, this);
        taskItem.taskType = ExpTaskListTraits::TaskType::FinishWaiting;
        addTask(taskItem);
    }

    if (params->extRan.extendedRange) {
        taskItem.task = new WinSpecTasks::AddExpNumber(
                    appState, - (params->expe.size() - 1) * n, this);
        taskItem.taskType = ExpTaskListTraits::TaskType::WinSpecAddExpNumber;
        addTask(taskItem);
    }

    if (params->tExp.tExp) {
        taskItem.task = new WinSpecTasks::AddExpNumber(appState, 1, this);
        taskItem.taskType = ExpTaskListTraits::TaskType::StageControlGoToPosExpList;
        addTask(taskItem);
    }
}

WholeBatchExpList::WholeBatchExpList(AppState *appState, QObject *parent) :
    ExpTaskList(parent)
{
    AppStateTraits::InitWinSpecParams *params = appState->initWinSpecParams();

    ExpTaskListTraits::TaskItem taskItem;
    WaitTaskListTraits::TaskItem waitTaskItem;

    if (params->batch.numSpectra > 1) {
        BatchExpList *batchExpList =
                new BatchExpList(appState, this);
        batchExpList->setTimesExec(params->batch.numSpectra - 1);
        taskItem.task = batchExpList;
        taskItem.taskType = ExpTaskListTraits::TaskType::BatchExpList;
        addTask(taskItem);
    }

    int sign = ((params->tExp.endT == params->tExp.startT) ?
                    0 :
                    (params->tExp.endT > params->tExp.startT) ?
                        1 : -1);
    int numTMeas = sign * int((params->tExp.endT - params->tExp.startT) /
            params->tExp.stepT) + 1;


    if (params->tExp.tExp && numTMeas > 1) {
        TExpList *tExpList;
        if (params->extRan.extendedRange) {
            ExtTExpList *extTExpList;
            if (params->tExp.loop) {
                extTExpList = new ExtTExpList(appState, &params->tExp.delay, 0, false, this);
                extTExpList->setTimesExec(numTMeas - 1);
                taskItem.task = extTExpList;
                taskItem.taskType = ExpTaskListTraits::TaskType::ExtTExpList;
                addTask(taskItem);

                taskItem.task =  new ExtTExpList(appState, &params->tExp.loopDelay, 1, false, this);
                taskItem.taskType = ExpTaskListTraits::TaskType::ExtTExpList;
                addTask(taskItem);

                taskItem.task = new WinSpecTasks::AddExpNumber(appState, 1, this);
                taskItem.taskType =
                        ExpTaskListTraits::TaskType::WinSpecAddExpNumber;
                addTask(taskItem);

                extTExpList = new ExtTExpList(appState, &params->tExp.loopDelay, 0, false, this);
                extTExpList->setTimesExec(numTMeas - 2);
                taskItem.task = extTExpList;
                taskItem.taskType = ExpTaskListTraits::TaskType::ExtTExpList;
                addTask(taskItem);

                // the last task is may be without delay, so the last task is only
                // spectrum measurement, see next taskItem.
                if (params->expe.size() > 1) {
                    taskItem.task = new ExtTExpList(appState, &params->tExp.loopDelay, 0, true, this);
                    taskItem.taskType = ExpTaskListTraits::TaskType::WinSpecExpList;
                    addTask(taskItem);
                }
            } else {
                extTExpList =
                        new ExtTExpList(appState, &params->tExp.delay, 0, false, this);
                extTExpList->setTimesExec(numTMeas - 1);
                taskItem.task = extTExpList;
                taskItem.taskType = ExpTaskListTraits::TaskType::ExtTExpList;
                addTask(taskItem);

                // the last task is may be without delay, so the last task is only
                // spectrum measurement, see next taskItem.
                if (params->expe.size() > 1) {
                    taskItem.task = new ExtTExpList(appState, &params->tExp.delay, 0, true, this);
                    taskItem.taskType = ExpTaskListTraits::TaskType::ExtTExpList;
                    addTask(taskItem);
                }
            }
            taskItem.task = new WinSpecTasks::ExpList(appState, false, params->expe.size() - 1, this);
            taskItem.taskType = ExpTaskListTraits::TaskType::WinSpecExpList;
            addTask(taskItem);
        } else {
            if (params->tExp.loop) {
                tExpList = new TExpList(appState, &params->tExp.delay, 0, 0, true, 0, this);
                tExpList->setTimesExec(numTMeas - 1);
                taskItem.task = tExpList;
                taskItem.taskType = ExpTaskListTraits::TaskType::TExpList;
                addTask(taskItem);

                taskItem.task =  new TExpList(appState, &params->tExp.loopDelay, 0, 1, true, 0, this);
                taskItem.taskType = ExpTaskListTraits::TaskType::TExpList;
                addTask(taskItem);

                taskItem.task = new WinSpecTasks::AddExpNumber(appState, 1, this);
                taskItem.taskType =
                        ExpTaskListTraits::TaskType::WinSpecAddExpNumber;
                addTask(taskItem);

                tExpList = new TExpList(appState, &params->tExp.loopDelay, this);
                tExpList->setTimesExec(numTMeas - 2);
                taskItem.task = tExpList;
                taskItem.taskType = ExpTaskListTraits::TaskType::TExpList;
                addTask(taskItem);
            } else {
                tExpList =
                        new TExpList(appState, &params->tExp.delay, this);
                tExpList->setTimesExec(numTMeas - 1);
                taskItem.task = tExpList;
                taskItem.taskType = ExpTaskListTraits::TaskType::TExpList;
                addTask(taskItem);
            }
            // the last task is may be without delay, so the last task is only
            // spectrum measurement, see next taskItem.
            taskItem.task = new WinSpecTasks::ExpList(appState, false, 0, this);
            taskItem.taskType = ExpTaskListTraits::TaskType::WinSpecExpList;
            addTask(taskItem);
        }
    } else {  // TODO(lumik): Bug? What about extended range?
        taskItem.task = new WinSpecTasks::ExpList(appState, false, 0, this);
        taskItem.taskType = ExpTaskListTraits::TaskType::WinSpecExpList;
        addTask(taskItem);
    }
    if (params->tExp.tExp && params->tExp.afterMeas) {
        taskItem.task = new NeslabTasks::SetT(appState, params->tExp.afterMeasT, this);
        taskItem.taskType = ExpTaskListTraits::TaskType::NeslabSetT;
        addTask(taskItem);
    }
    int lastExpNumber;
    if (params->extRan.extendedRange) {
        lastExpNumber = params->expe.size() - 1;
    } else {
        lastExpNumber = 0;
    }
    if (params->cal.at(lastExpNumber).autoCal) {
        TimeSpan timeSpan;
        taskItem.task = new StartWaitingTask(appState, &timeSpan, this);
        taskItem.taskType = ExpTaskListTraits::TaskType::StartWaiting;
        addTask(taskItem);

        taskItem.task = new StageTasks::GoToPosExpList(appState, StageControlTraits::PosType::Calibration, this);
        taskItem.taskType = ExpTaskListTraits::TaskType::StageControlGoToPosExpList;
        addTask(taskItem);

        taskItem.task = new WinSpecTasks::ExpList(appState, true, lastExpNumber, this);
        taskItem.taskType = ExpTaskListTraits::TaskType::WinSpecExpList;
        addTask(taskItem);

        if (params->extRan.extendedRange) {
            ForkJoinTask *fj = new ForkJoinTask(2, this);

            taskItem.task = new StageTasks::GoToPosExpList(appState, StageControlTraits::PosType::Measurement, this);
            taskItem.taskType = ExpTaskListTraits::TaskType::StageControlGoToPosExpList;
            fj->addTask(taskItem, 0);

            taskItem.task = new GratingTasks::SendToPos(appState, params->expe.at(0).grPos, this);
            taskItem.taskType = ExpTaskListTraits::TaskType::GratingSendToPos;
            fj->addTask(taskItem, 1);

            waitTaskItem.task = new GratingWaitTask(appState, appState);
            waitTaskItem.waitFor = WaitTaskListTraits::WaitFor::Grating;
            taskItem.task = new WaitExpTask(waitTaskItem, this);
            taskItem.taskType = ExpTaskListTraits::TaskType::WaitExp;
            fj->addTask(taskItem, 1);

            taskItem.task = new WaitingTask(this);
            taskItem.taskType = ExpTaskListTraits::TaskType::Waiting;
            fj->addTask(taskItem, 1);

            taskItem.task = fj;
            taskItem.taskType = ExpTaskListTraits::TaskType::ForkJoin;
            addTask(taskItem);
        } else {
            taskItem.task = new StageTasks::GoToPosExpList(appState, StageControlTraits::PosType::Measurement, this);
            taskItem.taskType = ExpTaskListTraits::TaskType::StageControlGoToPosExpList;
            addTask(taskItem);
        }

        taskItem.task = new FinishWaitingTask(appState, this);
        taskItem.taskType = ExpTaskListTraits::TaskType::FinishWaiting;
        addTask(taskItem);
    } else if (params->extRan.extendedRange && params->expe.size() > 1) {
        TimeSpan timeSpan;
        taskItem.task = new StartWaitingTask(appState, &timeSpan, this);
        taskItem.taskType = ExpTaskListTraits::TaskType::StartWaiting;
        addTask(taskItem);

        taskItem.task = new GratingTasks::SendToPos(appState, params->expe.at(0).grPos, this);
        taskItem.taskType = ExpTaskListTraits::TaskType::GratingSendToPos;
        addTask(taskItem);

        waitTaskItem.task = new GratingWaitTask(appState, appState);
        waitTaskItem.waitFor = WaitTaskListTraits::WaitFor::Grating;
        taskItem.task = new WaitExpTask(waitTaskItem, this);
        taskItem.taskType = ExpTaskListTraits::TaskType::WaitExp;
        addTask(taskItem);

        taskItem.task = new WaitingTask(this);
        taskItem.taskType = ExpTaskListTraits::TaskType::Waiting;
        addTask(taskItem);

        taskItem.task = new FinishWaitingTask(appState, this);
        taskItem.taskType = ExpTaskListTraits::TaskType::FinishWaiting;
        addTask(taskItem);
    }
}
