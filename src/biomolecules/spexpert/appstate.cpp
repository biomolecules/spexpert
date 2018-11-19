#include "appstate.h"

#include <biomolecules/sprelay/core/k8090.h>
#include <biomolecules/sprelay/core/k8090_defines.h>

#include "winspec.h"
#include "stagecontrol.h"
#include "neslabusmainwidget.h"
#include "neslab.h"
#include "lockableqvector.h"
#include "timespan.h"
#include "exptasks.h"
#include "relay.h"
#include <QDebug>
#include <QMutex>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QDateTime>
#include <limits>

#include <QDebug>

AppState::AppState(AppCore *appCore, QObject *parent)
  : QObject(parent),
    appCore_(appCore),
    k8090_{new biomolecules::sprelay::core::k8090::K8090{this}},
    relaySettings_{new biomolecules::spexpert::relay::Settings{
        biomolecules::sprelay::core::k8090::RelayID::One,  // calibration lamp_switch_id
        true                                               // calibration_lamp_switch_on
    }}
{
    qDebug() << "Startuji AppState...";

    winSpec_ = new WinSpec;
    stageControl_ = new StageControl;
    neslab_ = new Neslab;
    measurementLog_ = nullptr;

    lastFrame = new LockableFrame;
    blLastFrameChanged = false;
    mutexLastFrameChanged = new QMutex;

    spectrum = new LockableSpectrum;
    blSpectrumChanged = false;
    mutexSpectrumChanged = new QMutex;

    plotSpectrum = new LockablePlotSpectrum;

    plotType = AppStateTraits::PlotType::None;
    mutexPlotType = new QMutex;

    plotStyle_ = AppStateTraits::PlotStyle::Temperatures;
    plotStyleMutex = new QMutex;

    lastExpParamsMutex = new QMutex;

    lastGrPosMutex = new QMutex;

    currAccum = 0;
    currFrame = 0;
    currExpParamMutex = new QMutex;

    tStateMutex = new QMutex;

    currStagePos_ = 0;
    stageStateMutex = new QMutex;

    measStartedTime = new QDateTime;
    measStartedMutex = new QMutex;

    measFinishTime = new QDateTime;
    measFinishMutex = new QMutex;

    waitStartedTime = new QDateTime;
    waitStartedMutex = new QMutex;

    waitFinishTime = new QDateTime;
    waitFinishMutex = new QMutex;

    waitingState_ = WaitTaskListTraits::WaitFor::None;
    waitingStateMutex_ = new QMutex;

    xSpectrumShift = 50;
    ySpectrumShift = 1000;

    expParamListMutex_ = new QMutex;
    calParamListMutex_ = new QMutex;

    stageParams_ = new StageControlTraits::Params;

    stageParams_->measPos = 1000;
    stageParams_->measRefType = StageControlTraits::ReferenceType::LowerLimit;
    stageParams_->calPos = 1000;
    stageParams_->calRefType = StageControlTraits::ReferenceType::UpperLimit;

    autoReadTSettings_ = new NeslabusWidgets::AutoReadT::Settings;

    maxReadT = 1000;
    startMeasT_ = new QDateTime;
    readTfreq = 1;
    readTcounter = 0;
}

void AppState::makePlotSpectrum()
{
    plotSpectrum->shiftFromSpectrumSafe(*spectrum, xSpectrumShift, ySpectrumShift);
}

void AppState::startMeasT()
{
    qDebug() << "AppState::startMeasT()";
    setPlotStyle(AppStateTraits::PlotStyle::Temperatures);
    qDebug() << "AppState::startMeasT(): 1";
    *startMeasT_ = QDateTime::currentDateTime();
    readTfreq = 1;
    readTcounter = 0;
    readTs_.clear();
    tReadTs_.clear();
    measTs_.clear();
    tMeasTs_.clear();
    tYMax_ = std::numeric_limits<double>::min();
    tYMin_ = std::numeric_limits<double>::max();
    qDebug() << "AppState::startMeasT(): finished";
}

void AppState::addExpParams(WinSpecTasks::Params *winSpecParams)
{
    QMutexLocker locker(expParamListMutex_);
    expParamList_.append(winSpecParams);
}

void AppState::clearExpParams()
{
    QMutexLocker locker(expParamListMutex_);
    while (!expParamList_.isEmpty()) {
        delete expParamList_.takeFirst();
    }
}

WinSpecTasks::Params *AppState::expParamsAt(int ind)
{
//    qDebug() << "AppState::expParamsTakeAt(): ind" << ind;
    QMutexLocker locker(expParamListMutex_);
    if (ind >=0 && ind < expParamList_.size()) {
        return expParamList_.at(ind);
    }
    else {
        qDebug() << "AppState::expParamsTakeAt(): returning nullptr";
        return nullptr;
    }
}

void AppState::addCalParams(WinSpecTasks::Params *winSpecParams)
{
    QMutexLocker locker(calParamListMutex_);
    calParamList_.append(winSpecParams);
}

void AppState::clearCalParams()
{
    QMutexLocker locker(calParamListMutex_);
    while (!calParamList_.isEmpty()) {
        delete calParamList_.takeFirst();
    }
}

WinSpecTasks::Params *AppState::calParamsAt(int ind)
{
    QMutexLocker locker(calParamListMutex_);
    if (ind >=0 && ind < calParamList_.size()) {
        return calParamList_.at(ind);
    }
    else {
        qDebug() << "AppState::calParamsTakeAt(): returning nullptr";
        return nullptr;
    }
}

void AppState::addTemperatures(NeslabTasks::Temperatures *temperatures)
{
    temperatureList_.append((temperatures));
}

void AppState::clearTemperatures()
{
    while (!temperatureList_.isEmpty()) {
        delete temperatureList_.takeFirst();
    }
}

NeslabTasks::Temperatures *AppState::temperaturesTakeAt(int ind)
{
    if (ind >= 0 && ind < temperatureList_.size()) {
        return temperatureList_.at(ind);
    } else {
        qDebug() << "AppState::temperaturesTakeAt(): returning nullptr";
        return nullptr;
    }
}

void AppState::addReadT(double t)
{
    if (readTs_.size() < maxReadT) {
        if (++readTcounter == readTfreq) {
//            qDebug() << "AppState::addReadT(): normal" << (QDateTime::currentDateTime().toMSecsSinceEpoch() - startMeasT_->toMSecsSinceEpoch()) / 1000.0 << ":" << t;

            if (t > tYMax_) {
                tYMax_ = t;
            }
            if (t < tYMin_) {
                tYMin_ = t;
            }
            readTs_.append(t);
            tReadTs_.append((QDateTime::currentDateTime().toMSecsSinceEpoch() - startMeasT_->toMSecsSinceEpoch()) / 1000.0);
            readTcounter = 0;
            emit readTAdded();
        }
    } else {
        qDebug() << "AppState::addReadT(): reshaping";
        QVector<double> tempTs;
        QVector<double> tTempTs;
        for (int ii = 0; ii < readTs_.size(); ii += 2) {
            tempTs.append(readTs_.at(ii));
            tTempTs.append(tReadTs_.at(ii));
        }
        readTs_ = tempTs;
        tReadTs_ = tTempTs;
        readTfreq *= 2;
    }
}

void AppState::clearReadTs()
{
    readTs_.clear();
    tReadTs_.clear();
    readTfreq = 1;
    readTcounter = 0;
}

const QVector<double> &AppState::readTs()
{
    return readTs_;
}

const QVector<double> &AppState::tReadTs()
{
    return tReadTs_;
}

void AppState::addMeasT(double t)
{
    qDebug() << "AppState::addMeasT():" << (QDateTime::currentDateTime().toMSecsSinceEpoch() - startMeasT_->toMSecsSinceEpoch()) / 1000.0 << ":" << t;
//    if (t > tYMax_) {
//        tYMax_ = t;
//    }
//    if (t < tYMin_) {
//        tYMin_ = t;
//    }
    measTs_.append(t);
    tMeasTs_.append((QDateTime::currentDateTime().toMSecsSinceEpoch() - startMeasT_->toMSecsSinceEpoch()) / 1000.0);
    emit measTAdded();
}

void AppState::clearMeasTs()
{
    measTs_.clear();
    tMeasTs_.clear();
}

const QVector<double> &AppState::measTs()
{
    return measTs_;
}

const QVector<double> &AppState::tMeasTs()
{
    return tMeasTs_;
}

double AppState::tYMax()
{
    return tYMax_;
}

double AppState::tYMin()
{
    return tYMin_;
}

void AppState::newMeasurementLog(const QString &logFile, bool saveTs, bool saveGrPosisitions)
{
    measurementLog_ = new MeasurementLog(logFile, saveTs, saveGrPosisitions, this);
}

void AppState::clearMeasurementLog()
{
    delete measurementLog_;
    measurementLog_ = nullptr;
}

AppCore *AppState::appCore()
{
    return appCore_;
}

WinSpec *AppState::winSpec()
{
    return winSpec_;
}

StageControl *AppState::stageControl()
{
    return stageControl_;
}

Neslab *AppState::neslab()
{
    return neslab_;
}

biomolecules::sprelay::core::k8090::K8090* AppState::k8090()
{
    return k8090_;
}

MeasurementLog *AppState::measurementLog()
{
    return measurementLog_;
}

void AppState::lastExpParams(double *expo, int *acc, int *frm, QString *fn)
{
    QMutexLocker locker(lastExpParamsMutex);
    *expo = lastExp_;
    *acc = lastAcc_;
    *frm = lastFrm_;
    *fn = lastFN_;
}

int AppState::lastGrPos()
{
    QMutexLocker locker(lastGrPosMutex);
    return lastGrPos_;
}

AppStateTraits::InitWinSpecParams *AppState::initWinSpecParams()
{
    return &initWinSpecParams_;
}

LockableSpectrum &AppState::getSpectrum()
{
    return *spectrum;
}

const LockableSpectrum &AppState::getConstSpectrum() const
{
    return *spectrum;
}

LockablePlotSpectrum &AppState::getPlotSpectrum()
{
    return *plotSpectrum;
}

const LockablePlotSpectrum &AppState::getConstPlotSpectrum() const
{
    return *plotSpectrum;
}

//LockableFrame &AppState::getLastFrame()
//{
//    return *lastFrame;
//}

//const LockableFrame &AppState::getConstLastFrame() const
//{
//    return *lastFrame;
//}

AppStateTraits::PlotType AppState::getPlotType() const
{
    AppStateTraits::PlotType plotType_;
    QMutexLocker locker(mutexPlotType);
    plotType_ = plotType;
    locker.unlock();
    return plotType_;
}

AppStateTraits::PlotStyle AppState::plotStyle() const
{
    QMutexLocker locker(plotStyleMutex);
    return plotStyle_;
}

bool AppState::lastFrameChanged() const
{
    bool blLastFrameChanged_;
    QMutexLocker locker(mutexLastFrameChanged);
    blLastFrameChanged_ = blLastFrameChanged;
    locker.unlock();
    return blLastFrameChanged_;
}

bool AppState::spectrumChanged() const
{
    bool blSpectrumChanged_;
    QMutexLocker locker(mutexSpectrumChanged);
    blSpectrumChanged_ = blSpectrumChanged;
    locker.unlock();
    return blSpectrumChanged_;
}

void AppState::addWaitingState(WaitTaskListTraits::WaitFor ws)
{
    QMutexLocker locker(waitingStateMutex_);
    waitingState_ |= ws;
}

void AppState::removeWaitingState(WaitTaskListTraits::WaitFor ws)
{
    QMutexLocker locker(waitingStateMutex_);
    waitingState_ &= ~ws;
}

int AppState::getCurrAccum() const
{
    QMutexLocker locker(currExpParamMutex);
    return currAccum;
}

int AppState::getCurrFrame() const
{
    QMutexLocker locker(currExpParamMutex);
    return currFrame;
}

int AppState::getCurrExpNumber() const
{
    QMutexLocker locker(currExpParamMutex);
    return currExpNumber;
}

int AppState::getCurrStagePos() const
{
    QMutexLocker locker(stageStateMutex);
    return currStagePos_;
}

TimeSpan AppState::getRemainingWait() const
{
    return getWaitingFinishTime() - QDateTime::currentDateTime();
}

AppStateTraits::WinSpecState AppState::winSpecState() const
{
    QMutexLocker locker(currExpParamMutex);
    return enumWinSpecState;
}

AppStateTraits::TState AppState::tState() const
{
    QMutexLocker locker(tStateMutex);
    return tState_;
}

WaitTaskListTraits::WaitFor AppState::waitingState() const
{
    QMutexLocker locker(waitingStateMutex_);
    return waitingState_;
}

QDateTime AppState::getMeasurementStartedTime() const
{
    QMutexLocker locker(measStartedMutex);
    return *measStartedTime;
}

QDateTime AppState::getMeasurementFinishTime() const
{
    QMutexLocker locker(measFinishMutex);
    return *measFinishTime;
}

QDateTime AppState::getWaitingStartedTime() const
{
    QMutexLocker locker(waitStartedMutex);
    return *waitStartedTime;
}

QDateTime AppState::getWaitingFinishTime() const
{
    QMutexLocker locker(waitFinishMutex);
    return *waitFinishTime;
}

StageControlTraits::Params *AppState::stageParams()
{
    return stageParams_;
}

biomolecules::spexpert::relay::Settings* AppState::relaySettings()
{
    return relaySettings_;
}

NeslabusWidgets::AutoReadT::Settings *AppState::autoReadTSettings() const
{
    return autoReadTSettings_;
}

double AppState::lastT() const
{
    return lastT_;
}

void AppState::setLastExpParams(double expo, int acc, int frm, const QString &fn)
{
    qDebug() << "AppState::setLastExpParams(): filename" << fn;
    qDebug() << "AppState::setLastExpParams(): expo" << expo << ", acc" << acc << ", frm" << frm;
    QMutexLocker locker(lastExpParamsMutex);
    lastExp_ = expo;
    lastAcc_ = acc;
    lastFrm_ = frm;
    lastFN_ = fn;
}

void AppState::setLastGrPos(int gp)
{
    QMutexLocker locker(lastGrPosMutex);
    lastGrPos_ = gp;
}

void AppState::setInitWinSpecParams(const AppStateTraits::InitWinSpecParams &wp)
{
    initWinSpecParams_ = wp;
}

void AppState::setPlotType(AppStateTraits::PlotType plotType_)
{
    QMutexLocker locker(mutexPlotType);
    if (plotType != plotType_)
    {
//        qDebug() << "Menim plot type z" <<
//                    ((plotType == AppStateTraits::PlotType::Frame) ? "frame" :
//                                                     ((plotType == AppStateTraits::PlotType::Spectrum) ? "spectrum" :
//                                                                                         "none"))
//                 << "na"
//                 << ((plotType_ == AppStateTraits::PlotType::Frame) ? "frame" :
//                                                      ((plotType_ == AppStateTraits::PlotType::Spectrum) ? "spectrum" :
//                                                                                           "none"));
        plotType = plotType_;
        locker.unlock();
        emit plotTypeChanged();
    }
}

void AppState::setPlotStyle(AppStateTraits::PlotStyle ps)
{
    QMutexLocker locker(plotStyleMutex);
    plotStyle_ = ps;
}

void AppState::setCurrExpParams(int currAccum_, int currFrame_, AppStateTraits::WinSpecState enumWinSpecState_)
{
    switch(enumWinSpecState_)
    {
    case AppStateTraits::WinSpecState::Ready :
//        qDebug() << "AppState::setCurrExpParams(): Nastavuji WinSpecState na Ready";
        break;
    case AppStateTraits::WinSpecState::Running :
//        qDebug() << "AppState::setCurrExpParams(): Nastavuji WinSpecState na Running";
        break;
//    default:
//        qDebug() << "AppState::setCurrExpParams(): Nastavuji WinSpecState na neco divneho";
    }

    QMutexLocker locker(currExpParamMutex);
    currAccum = currAccum_;
    currFrame = currFrame_;
    enumWinSpecState = enumWinSpecState_;
}

void AppState::setCurrStageState(int currPos)
{
    QMutexLocker locker(stageStateMutex);
    currStagePos_ = currPos;
}

void AppState::setCurrAccum(int currAccum_)
{
    QMutexLocker locker(currExpParamMutex);
    currAccum = currAccum_;
}

void AppState::setCurrFrame(int currFrame_)
{
    QMutexLocker locker(currExpParamMutex);
    currFrame = currFrame_;
}

void AppState::setCurrExpNumber(int currExpNumber_)
{
    QMutexLocker locker(currExpParamMutex);
    currExpNumber = currExpNumber_;
}

void AppState::setWinSpecState(AppStateTraits::WinSpecState enumWinSpecState_)
{
    switch(enumWinSpecState_)
    {
    case AppStateTraits::WinSpecState::Ready :
        break;
    case AppStateTraits::WinSpecState::Running :
        break;
    }

    QMutexLocker locker(currExpParamMutex);
    enumWinSpecState = enumWinSpecState_;
}

void AppState::setTState(AppStateTraits::TState ts)
{
    QMutexLocker locker(tStateMutex);
    tState_ = ts;
}

void AppState::setWaitingState(WaitTaskListTraits::WaitFor ws)
{
    QMutexLocker locker(waitingStateMutex_);
    waitingState_ = ws;
}

void AppState::measurementStartedTime()
{
    QMutexLocker locker(measStartedMutex);
    *measStartedTime = QDateTime::currentDateTime();
}

void AppState::setMeasurementFinishTime(const TimeSpan & delay)
{
    QMutexLocker locker(measFinishMutex);
    *measFinishTime = getWaitingStartedTime().addMSecs(delay.toMSec());
}

void AppState::waitingStartedTime()
{
    QMutexLocker locker(waitStartedMutex);
    *waitStartedTime = QDateTime::currentDateTime();
}

void AppState::setWaitingFinishTime(const TimeSpan & delay)
{
    QMutexLocker locker(waitFinishMutex);
    *waitFinishTime = getWaitingStartedTime().addMSecs(delay.toMSec());

}

void AppState::setAutoReadTSettings(const NeslabusWidgets::AutoReadT::Settings &s)
{
    *autoReadTSettings_ = s;
}

void AppState::setStageParamsToStage()
{
    stageControl_->setStageRange(stageParams_->range);
    stageControl_->setMeasPos(stageParams_->measPos, stageParams_->measRefType);
    stageControl_->setMeasPos(stageParams_->calPos, stageParams_->calRefType);
}

void AppState::setLastFrameChanged(bool blLastFrameChanged_)
{
//    qDebug() << "Nastavuji LastFrameChanged v AppState...";
    QMutexLocker locker(mutexLastFrameChanged);
    if (blLastFrameChanged != blLastFrameChanged_)
    {
        blLastFrameChanged = blLastFrameChanged_;
        locker.unlock();
        if (blLastFrameChanged_ && (plotType == AppStateTraits::PlotType::Frame))
        {
//            qDebug() << "emit AppState::lastFrameChangedSignal()";
            emit lastFrameChangedSignal();
        }
    }
}

void AppState::setSpectrumChanged(bool blSpectrumChanged_)
{
    QMutexLocker locker(mutexSpectrumChanged);
    if (blSpectrumChanged != blSpectrumChanged_)
    {
        blSpectrumChanged = blSpectrumChanged_;
        locker.unlock();
        if (blSpectrumChanged_ && (plotType == AppStateTraits::PlotType::Spectrum))
        {
            emit spectrumChangedSignal();
        }
    }
}

void AppState::setT(double t)
{
    lastT_ = t;
    if (plotStyle_ == AppStateTraits::PlotStyle::Temperatures) {
        addReadT(t);
    }
}

AppState::~AppState()
{
    delete lastFrame;
    delete mutexLastFrameChanged;

    delete spectrum;
    delete mutexSpectrumChanged;

    delete plotSpectrum;
    delete mutexPlotType;

    delete plotStyleMutex;

    delete lastExpParamsMutex;

    delete lastGrPosMutex;

    delete currExpParamMutex;

    delete tStateMutex;

    delete stageStateMutex;

    delete measStartedTime;
    delete measStartedMutex;

    delete measFinishTime;
    delete measFinishMutex;

    delete waitStartedTime;
    delete waitStartedMutex;

    delete waitFinishTime;
    delete waitFinishMutex;

    delete waitingStateMutex_;

    while (!expParamList_.isEmpty()) {
        delete expParamList_.takeFirst();
    }
    delete expParamListMutex_;

    while (!calParamList_.isEmpty()) {
        delete calParamList_.takeFirst();
    }
    delete calParamListMutex_;

    delete stageParams_;
    delete relaySettings_;

    delete autoReadTSettings_;

    delete startMeasT_;
}


MeasurementLog::MeasurementLog(const QString &logFile, bool saveTs,
                               bool saveGrPosisitions, QObject *parent) :
    QObject(parent), saveMeas_(false), saveExpParams_(false), saveTs_(saveTs),
    saveT_(false), saveGrPositions_(saveGrPosisitions), saveGrPos_(false)
{
    logFile_ = new QFile(logFile);
    logTextStream = nullptr;
    measFile_ = new QFileInfo;
    dateTime_ = new QDateTime;
}

void MeasurementLog::saveHeader()
{
    if (!logFile_->open(QIODevice::WriteOnly | QIODevice::Text))
        return;
    logTextStream = new QTextStream(logFile_);
    *logTextStream << tr("Measurement on Spex Raman spectrometer.\n%1\n"
                         "%2 %3 %4 %5 %6")
                      .arg(QDateTime::currentDateTime().toString(tr("dd.MM.yyyy' 'hh:mm:ss")))
                      .arg("time", 19).arg("filename", 20)
                      .arg("expo", 10).arg("acc", 5)
                      .arg("frm", 5);
    if (saveTs_) {
        *logTextStream << tr(" %1").arg("T(°C)", 5);
    }
    if (saveGrPositions_) {
        *logTextStream << tr(" %1").arg("gr. pos.", 10);
    }
    *logTextStream << "\n";
    logTextStream->flush();
    logFile_->close();
}

void MeasurementLog::saveMeasurement()
{
    if (saveExpParams_ && ((saveTs_ && saveT_) || !saveTs_) &&
            ((saveGrPositions_ && saveGrPos_) || !saveGrPositions_)) {
        saveToFile();
    } else {
        saveMeas_ = true;
    }
}

void MeasurementLog::setExpParams(double expo, int acc, int frm, const QString &measFile)
{
    exp_ = expo;
    acc_ = acc;
    frm_ = frm;
    measFile_->setFile(measFile);
    if (saveMeas_ && ((saveTs_ && saveT_) || !saveTs_) &&
            ((saveGrPositions_ && saveGrPos_) || !saveGrPositions_)) {
        saveToFile();
    } else {
        saveExpParams_ = true;
    }
}

void MeasurementLog::setSaveT(double t)
{
    t_ = t;
    if (saveMeas_ && saveExpParams_ && saveTs_ &&
            ((saveGrPositions_ && saveGrPos_) || !saveGrPositions_)) {
        saveToFile();
    } else {
        saveT_ = true;
    }
}

void MeasurementLog::setSaveGrPos(double grPos)
{
    grPos_ = grPos;
    if (saveMeas_ && saveExpParams_ && ((saveTs_ && saveT_) || !saveTs_) &&
            saveGrPositions_) {
        saveToFile();
    } else {
        saveGrPos_ = true;
    }
}

void MeasurementLog::saveToFile()
{
    if (!logFile_->open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append))
        return;
    *logTextStream << tr("%1 %2 %3 %4 %5")
                      .arg(QDateTime::currentDateTime().toString(tr("dd.MM.yyyy' 'hh:mm:ss")))
                      .arg(measFile_->fileName(), 20)
                      .arg(exp_, 10, 'f', 3).arg(acc_, 5).arg(frm_, 5);
    saveExpParams_ = false;
    if (saveTs_) {
        *logTextStream << tr(" %1").arg(t_, 5);
        saveT_ = false;
    }
    if (saveGrPositions_) {
        *logTextStream << tr(" %1").arg(grPos_, 10);
        saveGrPos_ = false;
    }
    *logTextStream << "\n";
    logTextStream->flush();
    logFile_->close();
    saveMeas_ = false;
}

MeasurementLog::~MeasurementLog()
{
    delete logTextStream;
    delete logFile_;
    delete measFile_;
    delete dateTime_;
}
