#include <QStringBuilder>

#include <biomolecules/sprelay/core/k8090.h>

#include "appcore.h"
#include "centralwidget.h"
#include "mainwindow.h"
#include "winspec.h"
#include "stagecontrol.h"
#include "neslab.h"
//#include "waittask_old.h"
#include "appstate.h"
#include "plotproxy.h"

#include "exptasks.h"
#include "waittasks.h"
#include "waittasklist.h"
#include "relay.h"
#include "timespan.h"

#include <QTimer>
#include <QStatusBar>
#include <QMessageBox>
#include <QLabel>
#include <QThread>

#include <QDir>

//AppCore::AppCore(QObject *parent) :
//    AppCore(parent, nullptr)
//{
//}

AppCore::AppCore(CentralWidget *centralWidget, QStatusBar* sb, MainWindow *mainWindow, QObject *parent) :
    QObject(parent), centralWidget_(centralWidget), mainStatusBar(sb), mainWindow_(mainWindow)
{
    qDebug() << "AppCore::AppCore";
    appState_ = new AppState(this, this);
    appState_->winSpec()->showWinSpecWin();
    neslabThread = new QThread(this);
    appState_->neslab()->moveToThread(neslabThread);
    connect(neslabThread, &QThread::finished, appState_->neslab(), &Neslab::deleteLater);
    neslabThread->start();

    taskScheduler = new ExpTaskList(this);
    initStage = nullptr;
    goToPosStage = nullptr;

    stageAtMeas = false;

    messageStatusBarLabel = new QLabel("Ready");
    messageStatusBarLabel->setStyleSheet("color: green");
    winSpecStatusBarLabel = new QLabel("Winspec: Ready");
    winSpecStatusBarLabel->setStyleSheet("color: green");
    waitingStatusBarLabel = new QLabel("0 s");
    waitingStatusBarLabel->hide();
    temperatureStatusBarLabel = new QLabel("0 °C");
    temperatureStatusBarLabel->hide();
    stagePosStatusBarLabel = new QLabel("stagepos: 0");
    stagePosStatusBarLabel->hide();

    gratingPosStatusBarLabel = new QLabel("stage: 0");
    gratingPosStatusBarLabel->hide();

    mainStatusBar->addWidget(messageStatusBarLabel);
    winSpecStatusBarLabel->setStyleSheet("color: green");
    mainStatusBar->addPermanentWidget(gratingPosStatusBarLabel);
    mainStatusBar->addPermanentWidget(stagePosStatusBarLabel);
    mainStatusBar->addPermanentWidget(waitingStatusBarLabel);
    mainStatusBar->addPermanentWidget(temperatureStatusBarLabel);
    mainStatusBar->addPermanentWidget(winSpecStatusBarLabel);

//    appState_->addExpParams(new WinSpecTasks::Params(1, 1, 2, QString(R"(D:\users\Lumik\Desktop\Temp\WinSpecTest\temp.spe)"), -30, 5, 3, 1));
//    appState_->addCalParams(new WinSpecTasks::Params(0.2, 10, 1, QString(R"(D:\users\Lumik\Desktop\Temp\WinSpecTest\ktemp.spe)"), -30, 5, 3, 1));
//    appState->addExpParams(new WinSpecTasks::Params(5, 3, 2, QString(R"(E:\Lumik\Test\temp.spe)"), -30, 5, 3, 1));
//    appState->addCalParams(new WinSpecTasks::Params(1, 10, 1, QString(R"(E:\Lumik\Test\ktemp.spe)"), -30, 5, 3, 1));
    appState_->setWinSpecState(AppStateTraits::WinSpecState::Ready);
    appState_->setTState(AppStateTraits::TState::None);

//    appState_->setTemperatures(NeslabTasks::Temperatures(20, 5, 3));

    connect(appState_, &AppState::lastFrameChangedSignal, this, &AppCore::updatePlot);
    connect(appState_, &AppState::spectrumChangedSignal, this, &AppCore::updatePlot);
    connect(appState_, &AppState::measTAdded, this, &AppCore::updateMeasPlot);
    connect(appState_, &AppState::readTAdded, this, &AppCore::updateReadPlot);
    connect(appState_, &AppState::plotTypeChanged, this, &AppCore::plotTypeChanged);
    connect(taskScheduler, &ExpTaskList::finished, this, &AppCore::onTaskSchedulerFinished);
    connect(appState_->neslab(), &Neslab::readSetpointFinished, this, &AppCore::onReadTSetpoint);

    appTimer = new QTimer(this);

    connect(appTimer, &QTimer::timeout, this, &AppCore::updateApp);
    appTimer->start(300);
}

AppState *AppCore::appState()
{
    return appState_;
}

MainWindow *AppCore::mainWindow()
{
    return mainWindow_;
}

CentralWidget *AppCore::centralWidget()
{
    return centralWidget_;
}

void AppCore::reportParams()
{
    AppStateTraits::InitWinSpecParams *params = appState()->initWinSpecParams();
    int sign = ((params->tExp.endT == params->tExp.startT) ?
                    0 :
                    (params->tExp.endT > params->tExp.startT) ?
                        1 : -1);
    int numTMeas = sign * int((params->tExp.endT - params->tExp.startT) /
            params->tExp.stepT) + 1;

    int n;
    if (params->extRan.extendedRange) {
        n = params->expe.size();
    } else {
        n = 1;
    }
    if (params->tExp.tExp && params->batch.batchExp) {
        WinSpecTasks::Params *winSpecParams;
        WinSpecTasks::Params *kwinSpecParams;
        NeslabTasks::Temperatures *temperatures;
        double expo;
        int acc;
        int frm;
        QString fn;
        if (params->tExp.loop && numTMeas > 1) {
            for (int ii = 0; ii < 2 * params->batch.numSpectra; ii = ii + 2) {
                for (int jj = 0; jj < numTMeas; ++jj) {
                    for (int kk = 0; kk < n; ++kk) {
                        winSpecParams = appState()->expParamsAt(ii + kk * n * 2);
                        kwinSpecParams = appState()->calParamsAt(ii + kk * n * 2);
                        temperatures = appState()->temperaturesTakeAt(ii + kk * n * 2);
                        winSpecParams->takeOne(&expo, &acc, &frm, fn);
                        qDebug() << "AppCore::startExperiment(): filename" << fn;
                        qDebug() << "AppCore::startExperiment(): expo" << expo << ", acc" << acc << ", frm" << frm << ", grPos" << winSpecParams->grPos();
                        if (params->cal.at(kk).autoCal) {
                            kwinSpecParams->takeOne(&expo, &acc, &frm, fn);
                            qDebug() << "AppCore::startExperiment(): kfilename" << fn;
                            qDebug() << "AppCore::startExperiment(): expo" << expo << ", acc" << acc << ", frm" << frm << ", grPos" << kwinSpecParams->grPos();
                            qDebug() << "AppCore::startExperiment(): enableLampSwitch "
                                << params->cal.at(kk).enableLampSwitch;
                        }
                        qDebug() << "AppCore::startExperiment(): T =" << temperatures->takeT();
                    }
                }
                for (int jj = 0; jj < numTMeas - 1; ++jj) {
                    for (int kk = 0; kk < n; ++kk) {
                        winSpecParams = appState()->expParamsAt(ii + kk * n * 2 + 1);
                        kwinSpecParams = appState()->calParamsAt(ii + kk * n * 2 + 1);
                        temperatures = appState()->temperaturesTakeAt(ii + kk * n * 2 + 1);
                        winSpecParams->takeOne(&expo, &acc, &frm, fn);
                        qDebug() << "AppCore::startExperiment(): filename" << fn;
                        qDebug() << "AppCore::startExperiment(): expo" << expo << ", acc" << acc << ", frm" << frm << ", grPos" << winSpecParams->grPos();
                        if (params->cal.at(kk).autoCal) {
                            kwinSpecParams->takeOne(&expo, &acc, &frm, fn);
                            qDebug() << "AppCore::startExperiment(): kfilename" << fn;
                            qDebug() << "AppCore::startExperiment(): expo" << expo << ", acc" << acc << ", frm" << frm << ", grPos" << kwinSpecParams->grPos();
                            qDebug() << "AppCore::startExperiment(): enableLampSwitch "
                                << params->cal.at(kk).enableLampSwitch;
                        }
                        qDebug() << "AppCore::startExperiment(): T =" << temperatures->takeT();
                    }
                }
            }
        } else {
            for (int ii = 0; ii < params->batch.numSpectra; ++ii) {
                for (int jj = 0; jj < numTMeas; ++jj) {
                    for (int kk = 0; kk < n; ++kk) {
                        winSpecParams = appState()->expParamsAt(ii + kk * n);
                        kwinSpecParams = appState()->calParamsAt(ii + kk * n);
                        temperatures = appState()->temperaturesTakeAt(ii + kk * n);
                        winSpecParams->takeOne(&expo, &acc, &frm, fn);
                        qDebug() << "AppCore::startExperiment(): filename" << fn;
                        qDebug() << "AppCore::startExperiment(): expo" << expo << ", acc" << acc << ", frm" << frm << ", grPos" << winSpecParams->grPos();
                        if (params->cal.at(kk).autoCal) {
                            kwinSpecParams->takeOne(&expo, &acc, &frm, fn);
                            qDebug() << "AppCore::startExperiment(): kfilename" << fn;
                            qDebug() << "AppCore::startExperiment(): expo" << expo << ", acc" << acc << ", frm" << frm << ", grPos" << kwinSpecParams->grPos();
                            qDebug() << "AppCore::startExperiment(): enableLampSwitch "
                                << params->cal.at(kk).enableLampSwitch;
                        }
                        qDebug() << "AppCore::startExperiment(): T =" << temperatures->takeT();
                    }
                }
            }
        }
    } else if (params->batch.batchExp) {
        WinSpecTasks::Params *winSpecParams;
        WinSpecTasks::Params *kwinSpecParams;
        double expo;
        int acc;
        int frm;
        QString fn;
        for (int jj = 0; jj < params->batch.numSpectra; ++jj) {
            for (int kk = 0; kk < n; ++kk) {
                winSpecParams = appState()->expParamsAt(kk);
                kwinSpecParams = appState()->calParamsAt(kk);
                winSpecParams->takeOne(&expo, &acc, &frm, fn);
                qDebug() << "AppCore::startExperiment(): filename" << fn;
                qDebug() << "AppCore::startExperiment(): expo" << expo << ", acc" << acc << ", frm" << frm << ", grPos" << winSpecParams->grPos();
                if (params->cal.at(kk).autoCal) {
                    kwinSpecParams->takeOne(&expo, &acc, &frm, fn);
                    qDebug() << "AppCore::startExperiment(): kfilename" << fn;
                    qDebug() << "AppCore::startExperiment(): expo" << expo << ", acc" << acc << ", frm" << frm << ", grPos" << kwinSpecParams->grPos();
                    qDebug() << "AppCore::startExperiment(): enableLampSwitch "
                        << params->cal.at(kk).enableLampSwitch;
                }
            }
        }
    } else if (params->tExp.tExp) {
        qDebug() << "AppCore::reportParams(): tExp";
        WinSpecTasks::Params *winSpecParams;
        WinSpecTasks::Params *kwinSpecParams;
        NeslabTasks::Temperatures *temperatures;
        double expo;
        int acc;
        int frm;
        QString fn;
        if (params->tExp.loop) {
            for (int jj = 0; jj < numTMeas; ++jj) {
                for (int kk = 0; kk < n; ++kk) {
                    winSpecParams = appState()->expParamsAt(2 * kk);
                    kwinSpecParams = appState()->calParamsAt(2 * kk);
                    temperatures = appState()->temperaturesTakeAt(2 * kk);
                    winSpecParams->takeOne(&expo, &acc, &frm, fn);
                    qDebug() << "AppCore::startExperiment(): filename" << fn;
                    qDebug() << "AppCore::startExperiment(): expo" << expo << ", acc" << acc << ", frm" << frm << ", grPos" << winSpecParams->grPos();
                    if (params->cal.at(kk).autoCal) {
                        kwinSpecParams->takeOne(&expo, &acc, &frm, fn);
                        qDebug() << "AppCore::startExperiment(): kfilename" << fn;
                        qDebug() << "AppCore::startExperiment(): expo" << expo << ", acc" << acc << ", frm" << frm << ", grPos" << kwinSpecParams->grPos();
                        qDebug() << "AppCore::startExperiment(): enableLampSwitch "
                            << params->cal.at(kk).enableLampSwitch;
                    }
                    qDebug() << "AppCore::startExperiment(): T =" << temperatures->takeT();
                }
            }
            for (int jj = 0; jj < numTMeas - 1; ++jj) {
                for (int kk = 0; kk < n; ++kk) {
                    winSpecParams = appState()->expParamsAt(2 * kk + 1);
                    kwinSpecParams = appState()->calParamsAt(2 * kk + 1);
                    temperatures = appState()->temperaturesTakeAt(2 * kk + 1);
                    winSpecParams->takeOne(&expo, &acc, &frm, fn);
                    qDebug() << "AppCore::startExperiment(): filename" << fn;
                    qDebug() << "AppCore::startExperiment(): expo" << expo << ", acc" << acc << ", frm" << frm << ", grPos" << winSpecParams->grPos();
                    if (params->cal.at(kk).autoCal) {
                        kwinSpecParams->takeOne(&expo, &acc, &frm, fn);
                        qDebug() << "AppCore::startExperiment(): kfilename" << fn;
                        qDebug() << "AppCore::startExperiment(): expo" << expo << ", acc" << acc << ", frm" << frm << ", grPos" << kwinSpecParams->grPos();
                        qDebug() << "AppCore::startExperiment(): enableLampSwitch "
                            << params->cal.at(kk).enableLampSwitch;
                    }
                    qDebug() << "AppCore::startExperiment(): T =" << temperatures->takeT();
                }
            }
        } else {
            for (int jj = 0; jj < numTMeas; ++jj) {
                for (int kk = 0; kk < n; ++kk) {
                    winSpecParams = appState()->expParamsAt(kk);
                    kwinSpecParams = appState()->calParamsAt(kk);
                    temperatures = appState()->temperaturesTakeAt(kk);
                    winSpecParams->takeOne(&expo, &acc, &frm, fn);
                    qDebug() << "AppCore::startExperiment(): filename" << fn;
                    qDebug() << "AppCore::startExperiment(): expo" << expo << ", acc" << acc << ", frm" << frm << ", grPos" << winSpecParams->grPos();
                    if (params->cal.at(kk).autoCal) {
                        kwinSpecParams->takeOne(&expo, &acc, &frm, fn);
                        qDebug() << "AppCore::startExperiment(): kfilename" << fn;
                        qDebug() << "AppCore::startExperiment(): expo" << expo << ", acc" << acc << ", frm" << frm << ", grPos" << winSpecParams->grPos();
                        qDebug() << "AppCore::startExperiment(): enableLampSwitch "
                            << params->cal.at(kk).enableLampSwitch;
                    }
                    qDebug() << "AppCore::startExperiment(): T =" << temperatures->takeT();
                }
            }
        }
    } else {
        WinSpecTasks::Params *winSpecParams;
        WinSpecTasks::Params *kwinSpecParams;
        double expo;
        int acc;
        int frm;
        QString fn;
        for (int kk = 0; kk < n; ++kk) {
            winSpecParams = appState()->expParamsAt(kk);
            kwinSpecParams = appState()->calParamsAt(kk);
            winSpecParams->takeOne(&expo, &acc, &frm, fn);
            qDebug() << "AppCore::startExperiment(): filename" << fn;
            qDebug() << "AppCore::startExperiment(): expo" << expo << ", acc" << acc << ", frm" << frm << ", grPos" << winSpecParams->grPos();
            if (params->cal.at(kk).autoCal) {
                kwinSpecParams->takeOne(&expo, &acc, &frm, fn);
                qDebug() << "AppCore::startExperiment(): kfilename" << fn;
                qDebug() << "AppCore::startExperiment(): expo" << expo << ", acc" << acc << ", frm" << frm << ", grPos" << winSpecParams->grPos();
                qDebug() << "AppCore::startExperiment(): enableLampSwitch "
                    << params->cal.at(kk).enableLampSwitch;
            }
        }
    }

    experimentFinished();
}

void AppCore::startExperiment()
{
    emit experimentStarted();

    if (expInitializeStage())
        return;

    if (expConnectNeslab())
        return;

    if (expConnectRelay())
        return;

    buildExpParams();

//    reportParams();
//    return;

    qDebug() << "Startuji experiment...";

    AppStateTraits::InitWinSpecParams *params = appState()->initWinSpecParams();
    messageStatusBarLabel->setText("Running");
    messageStatusBarLabel->setStyleSheet("color: red");
    if (appState_->initWinSpecParams()->tExp.tExp) {
        appState_->setTState(AppStateTraits::TState::Reading);
        temperatureStatusBarLabel->show();
        connect(appState_->neslab(), &Neslab::readExternalSensorFinished,
                appState_, &AppState::setT);
        connect(appState_->neslab(), &Neslab::readInternalTemperatureFinished,
                appState_, &AppState::setT);
        emit startReadingTemperature();
    }
    if (appState_->initWinSpecParams()->tExp.tExp) {
        appState_->startMeasT();
        centralWidget_->plotProxy->onPlotTemperatures();
    } else {
        appState_->setPlotStyle(AppStateTraits::PlotStyle::Spectra);
    }
    if (appState_->measurementLog()) {
        appState_->clearMeasurementLog();
    }
    appState_->newMeasurementLog(params->expe.at(0).directory % QDir::separator() % params->expe.at(0).fileNameBase % ".log",
                                 params->tExp.tExp, params->extRan.extendedRange);
    appState_->measurementLog()->saveHeader();
    appState_->setCurrExpNumber(0);
    appState_->waitingStartedTime(); // nastavim cas, od ktereho se odpocitava.

    ExpTaskListTraits::TaskItem taskItem;
    WaitTaskList *waitTaskList = new WaitTaskList(this);

    buildInitialExpTaks(waitTaskList);
    buildBodyExpTasks(waitTaskList);

    taskItem.task = waitTaskList;
    taskItem.taskType = ExpTaskListTraits::TaskType::WaitList;
    taskScheduler->addTask(taskItem);

    appState_->stageControl()->setMeasPos(appState_->stageParams()->measPos, appState_->stageParams()->measRefType);
    appState_->stageControl()->setCalPos(appState_->stageParams()->calPos, appState_->stageParams()->calRefType);

    taskScheduler->start();

//    winSpec->StopRunning();
//    winSpec->SaveAs(strFileNameLong);
//    winSpec->CloseWindow();
}

void AppCore::stopExperiment()
{
    if (taskScheduler->running()) {
        if (appState_->stageControl()->motorRunning()) {
            stageAtMeas = false;
        }
        taskScheduler->stop();
    } else {
        if (initStage) {
            disconnect(initStage, &StageTasks::Initialize::finished, this, &AppCore::startExperiment);
            emit experimentFinished();
            emit stageMovementFinished();
        }
        appState_->winSpec()->stop();
        appState_->stageControl()->stop();
    }
}

void AppCore::onGoToMeas()
{
    emit stageMovementStarted();

    StageControl *stageControl = appState_->stageControl();

    if (stageControl->initialized()) {
        if (stageControl->motorRunning()) {
            QMessageBox::critical(centralWidget_, tr("Stage is already running!"), tr("The stage is already running, stop it, please!"), QMessageBox::Ok);
            emit experimentFinished();
//        } else if (initStage || goToPosStage) {
//            QMessageBox::critical(centralWidget_, tr("Stage is controlled by another process!"), tr("The stage is controlled by another process, stop it, please!"), QMessageBox::Ok);
//            emit experimentFinished();
        } else {
            goToPosStage = new StageTasks::GoToPos(appState_, StageControlTraits::PosType::Measurement, this);
            connect(goToPosStage, &StageTasks::GoToPos::finished, this, &AppCore::stageIsAtPos);
            connect(centralWidget_, &CentralWidget::stopStage, goToPosStage, &StageTasks::GoToPos::stop);

            stageControl->setMeasPos(appState_->stageParams()->measPos, appState_->stageParams()->measRefType);

            goToPosStage->start();
        }
    } else {
        int ret = QMessageBox::question(centralWidget_, tr("Initialize stage?"), tr("Stage is not initialized yet. Do you want to initialize it?"),
                                        QMessageBox::Yes | QMessageBox::No);
        if (ret == QMessageBox::Yes) {
            if (stageControl->motorRunning()) {
                QMessageBox::critical(centralWidget_, tr("Stage is already running!"), tr("The stage is already running, stop it, please!"), QMessageBox::Ok);
                emit experimentFinished();
            } else if (initStage || goToPosStage) {
                QMessageBox::critical(centralWidget_, tr("Stage is controlled by another process!"), tr("The stage is controlled by another process, stop it, please!"), QMessageBox::Ok);
                emit experimentFinished();
            } else {
                initStage = new StageTasks::Initialize(appState_, appState_->stageParams()->initType, 0, this);
                connect(initStage, &StageTasks::Initialize::finished, this, &AppCore::onGoToMeas);
                connect(initStage, &StageTasks::Initialize::finished, this, &AppCore::onInitStageFinished);
                connect(centralWidget_, &CentralWidget::stopStage, initStage, &StageTasks::Initialize::stop);
                initStage->start();
            }
        }
    }
}

void AppCore::onGoToCal()
{
    emit stageMovementStarted();

    StageControl *stageControl = appState_->stageControl();

    if (stageControl->initialized()) {
        if (stageControl->motorRunning()) {
            QMessageBox::critical(centralWidget_, tr("Stage is already running!"), tr("The stage is already running, stop it, please!"), QMessageBox::Ok);
            emit experimentFinished();
//        } else if (initStage || goToPosStage) {
//            QMessageBox::critical(centralWidget_, tr("Stage is controlled by another process!"), tr("The stage is controlled by another process, stop it, please!"), QMessageBox::Ok);
//            emit experimentFinished();
        } else {
            goToPosStage = new StageTasks::GoToPos(appState_, StageControlTraits::PosType::Calibration, this);
            connect(goToPosStage, &StageTasks::GoToPos::finished, this, &AppCore::stageIsAtPos);
            connect(centralWidget_, &CentralWidget::stopStage, goToPosStage, &StageTasks::GoToPos::stop);

            stageControl->setCalPos(appState_->stageParams()->calPos, appState_->stageParams()->calRefType);

            goToPosStage->start();
        }
    } else {
        int ret = QMessageBox::question(centralWidget_, tr("Initialize stage?"), tr("Stage is not initialized yet. Do you want to initialize it?"),
                                        QMessageBox::Yes | QMessageBox::No);
        if (ret == QMessageBox::Yes) {
            if (stageControl->motorRunning()) {
                QMessageBox::critical(centralWidget_, tr("Stage is already running!"), tr("The stage is already running, stop it, please!"), QMessageBox::Ok);
                emit experimentFinished();
            } else if (initStage || goToPosStage) {
                QMessageBox::critical(centralWidget_, tr("Stage is controlled by another process!"), tr("The stage is controlled by another process, stop it, please!"), QMessageBox::Ok);
                emit experimentFinished();
            } else {
                initStage = new StageTasks::Initialize(appState_, appState_->stageParams()->initType, 0, this);
                connect(initStage, &StageTasks::Initialize::finished, this, &AppCore::onGoToMeas);
                connect(initStage, &StageTasks::Initialize::finished, this, &AppCore::onInitStageFinished);
                connect(centralWidget_, &CentralWidget::stopStage, initStage, &StageTasks::Initialize::stop);
                initStage->start();
            }
        }
    }
}

void AppCore::onInitStage()
{
    qDebug() << "AppCore::onInitStage()";
    if (!initStage) {
        initStage = new StageTasks::Initialize(appState_, appState_->stageParams()->initType, 0, this);
        connect(initStage, &StageTasks::Initialize::finished, this, &AppCore::onInitStageFinished);
        qDebug() << "AppCore::onInitStage(): starting initialization";
        initStage->start();
    }
}

void AppCore::stopStage()
{
    if (appState_->stageControl()->motorRunning()) {
        stageAtMeas = false;
        appState_->stageControl()->stop();
        appState_->stageControl()->powerOff();
    }
}

void AppCore::onInitStageFinished()
{
    qDebug() << "AppCore::onInitStageFinished()";
    delete initStage;
    initStage = nullptr;
    emit initStageFinished();
}

void AppCore::stageIsAtPos()
{
    delete goToPosStage;
    goToPosStage = nullptr;
    emit stageMovementFinished();
}

void AppCore::updatePlot()
{
    centralWidget_->plotProxy->updatePlot(appState_);
}

void AppCore::updateMeasPlot()
{
    centralWidget_->plotProxy->onUpdateMeasT(appState_);
}

void AppCore::updateReadPlot()
{
    centralWidget_->plotProxy->onUpdateReadT(appState_);
}

void AppCore::plotTypeChanged()
{
    centralWidget_->plotProxy->plotTypeChanged(appState_);
}

void AppCore::updateApp()
{
    QString status;
    status.append(tr("WinSpec: "));
    switch (appState_->winSpecState())
    {
    case AppStateTraits::WinSpecState::Ready :
        status.append(tr("Ready"));
        winSpecStatusBarLabel->setText("Winspec: Ready");
        winSpecStatusBarLabel->setStyleSheet("color: green");
        break;
    case AppStateTraits::WinSpecState::Running :
        status.append(tr("Running, Accum %1/%2, Frame %3/%4")
                      .arg(appState_->getCurrAccum())
                      .arg(appState_->expParamsAt(appState_->getCurrExpNumber())->getLastAccums())
                      .arg(appState_->getCurrFrame())
                      .arg(appState_->expParamsAt(appState_->getCurrExpNumber())->getLastFrames()));
        winSpecStatusBarLabel->setText(status);
        winSpecStatusBarLabel->setStyleSheet("color: red");
        break;
//    case AppState::WinSpecState::Waiting :
//        status.append(tr("Waiting: %1").arg(appState->getRemainingWait().toString()));
    }
    WaitTaskListTraits::WaitFor ws = appState_->waitingState();
    if (ws != WaitTaskListTraits::WaitFor::None) {
//        status.append(tr(" : waiting %1").arg(appState->getRemainingWait().toString()));
        if (static_cast<bool>(ws & WaitTaskListTraits::WaitFor::Delay))
        {
            TimeSpan rw = appState_->getRemainingWait();
            if (rw.toMSec() > 0) {
                waitingStatusBarLabel->setStyleSheet("color: green");
            } else {
                waitingStatusBarLabel->setStyleSheet("color: red");
            }
            waitingStatusBarLabel->setText(tr("waiting %1").arg(rw.toString()));
            waitingStatusBarLabel->show();
        } else {
            waitingStatusBarLabel->hide();
        }
        if (static_cast<bool>(ws & WaitTaskListTraits::WaitFor::Motor))
        {
            stagePosStatusBarLabel->setText(tr("stage: %1").arg(appState_->getCurrStagePos()));
            stagePosStatusBarLabel->show();
        } else {
            stagePosStatusBarLabel->hide();
        }
        if (static_cast<bool>(ws & WaitTaskListTraits::WaitFor::Grating))
        {
            gratingPosStatusBarLabel->setText(tr("grating: %1").arg(0));
            gratingPosStatusBarLabel->show();
        } else {
            gratingPosStatusBarLabel->hide();
        }
    } else {
        waitingStatusBarLabel->hide();
        stagePosStatusBarLabel->hide();
        gratingPosStatusBarLabel->hide();
    }
    if (appState_->tState() == AppStateTraits::TState::Reading) {
        temperatureStatusBarLabel -> setText(tr("%1 °C").arg(appState_->lastT(), 0, 'f', 2));
    }
    if (static_cast<bool>(ws & WaitTaskListTraits::WaitFor::WinSpec))
        status.append(tr(" : !!!WinSpec bezi, zastavte ho!!!"));
//    mainStatusBar->clearMessage();
    //    mainStatusBar->showMessage(status);
}

void AppCore::onTaskSchedulerFinished()
{
    qDebug() << "AppCore::onTaskSchedulerFinished()";
    appState_->setTState(AppStateTraits::TState::None);
    disconnect(appState_->neslab(), &Neslab::readExternalSensorFinished,
            appState_, &AppState::setT);
    disconnect(appState_->neslab(), &Neslab::readInternalTemperatureFinished,
            appState_, &AppState::setT);
    emit finishReadingTemperature();
    temperatureStatusBarLabel->hide();
    messageStatusBarLabel->setText("Ready");
    messageStatusBarLabel->setStyleSheet("color: green");
    appState_->clearMeasurementLog();
    appState_->removeWaitingState(~WaitTaskListTraits::WaitFor::None);
    emit experimentFinished();
}

void AppCore::onReadTSetpoint(double t)
{
    qDebug() << "AppCore::onReadTSetpoint(): setpoint =" << t;
}

void AppCore::onReadExpT(double t)
{
    disconnect(appState_->neslab(), &Neslab::readExternalSensorFinished, this, &AppCore::onReadExpT);
    disconnect(appState_->neslab(), &Neslab::readInternalTemperatureFinished, this, &AppCore::onReadExpT);
    if (appState_->plotStyle() == AppStateTraits::PlotStyle::Temperatures) {
        appState_->addMeasT(t);
    }
    if (appState_->measurementLog()) {
        appState_->measurementLog()->setSaveT(t);
    }
}

void AppCore::onNeslabConnectionFailed()
{
    disconnect(appState_->neslab(), &Neslab::connected,
            this, &AppCore::startExperiment);
    disconnect(appState_->neslab(), &Neslab::connectionFailed,
            this, &AppCore::startExperiment);
    emit experimentFinished();
    QMessageBox::critical(centralWidget_, tr("Connection failed!"), tr("Neslab thermostated bath connection failed!"), QMessageBox::Ok);
}

void AppCore::onRelayConnectionFailed()
{
    disconnect(appState_->k8090(), &biomolecules::sprelay::core::k8090::K8090::connected,
            this, &AppCore::startExperiment);
    disconnect(appState_->k8090(), &biomolecules::sprelay::core::k8090::K8090::connectionFailed,
            this, &AppCore::startExperiment);
    emit experimentFinished();
    QMessageBox::critical(centralWidget_, tr("Connection failed!"), tr("Relay connection failed!"), QMessageBox::Ok);
}

bool AppCore::expAutoCal()
{
    if (appState()->initWinSpecParams()->extRan.extendedRange) {
        for (int ii = 0; ii < appState()->initWinSpecParams()->cal.size(); ++ii) {
            if (appState()->initWinSpecParams()->cal.at(ii).autoCal) {
                return true;
            }
        }
    } else {
        if (appState()->initWinSpecParams()->cal.at(0).autoCal) {
            return true;
        }
    }
    return false;
}

bool AppCore::expInitializeStage()
{
    if (expAutoCal()) {
        StageControl *stageControl = appState_->stageControl();
        if (!stageControl->initialized()) {
            int ret = QMessageBox::question(centralWidget_, tr("Initialize stage?"), tr("Even though the stage is not initialized yet, the measurement witch automatic calibration is scheduled. Do you want to initialize it?"),
                                            QMessageBox::Yes | QMessageBox::No);
            if (ret == QMessageBox::Yes) {
                if (stageControl->motorRunning()) {
                    QMessageBox::critical(centralWidget_, tr("Stage is already running!"), tr("The stage is already running, stop it, please!"), QMessageBox::Ok);
                    emit experimentFinished();
                } else if (initStage || goToPosStage) {
                    QMessageBox::critical(centralWidget_, tr("Stage is controlled by another process!"), tr("The stage is controlled by another process, stop it, please!"), QMessageBox::Ok);
                    emit experimentFinished();
                } else {
                    initStage = new StageTasks::Initialize(appState_, appState_->stageParams()->initType, 0, this);
                    connect(initStage, &StageTasks::Initialize::finished, this, &AppCore::startExperiment);
                    connect(initStage, &StageTasks::Initialize::finished, this, &AppCore::onInitStageFinished);
                    connect(centralWidget_, &CentralWidget::stopExperiment, initStage, &StageTasks::Initialize::stop);
                    connect(centralWidget_, &CentralWidget::stopStage, initStage, &StageTasks::Initialize::stop);
                    initStage->start();
                }
            } else {
                experimentFinished();
            }
            return true;
        }
    }
    return false;
}

bool AppCore::expConnectNeslab()
{
    Neslab* neslab = appState_->neslab();
    disconnect(neslab, &Neslab::connected,
            this, &AppCore::startExperiment);
    disconnect(neslab, &Neslab::connectionFailed,
            this, &AppCore::startExperiment);
    if (appState()->initWinSpecParams()->tExp.tExp) {
        if (!neslab->isConnected()) {
            int ret = QMessageBox::question(centralWidget_, tr("Connect Neslab?"), tr("Even though the Neslab thermostated bath is not connected yet, the temperature measurement is scheduled. Do you want to connect it?"),
                                            QMessageBox::Yes | QMessageBox::No);
            if (ret == QMessageBox::Yes) {
                connect(neslab, &Neslab::connected,
                        this, &AppCore::startExperiment);
                connect(neslab, &Neslab::connectionFailed,
                        this, &AppCore::onNeslabConnectionFailed);
                neslab->connectNeslab();
                return true;
            } else {
                appState()->initWinSpecParams()->tExp.tExp = false;
                emit experimentFinished();
                return true;
            }
        }
    }
    return false;
}

bool AppCore::expConnectRelay()
{
    biomolecules::sprelay::core::k8090::K8090* k8090 = appState_->k8090();
    disconnect(k8090, &biomolecules::sprelay::core::k8090::K8090::connected,
            this, &AppCore::startExperiment);
    disconnect(k8090, &biomolecules::sprelay::core::k8090::K8090::connectionFailed,
            this, &AppCore::startExperiment);
    if (expAutoCal()) {
        if (!k8090->isConnected()) {
            int ret = QMessageBox::question(centralWidget_, tr("Connect Relay?"),
                tr("Even though the Relay is not connected yet the measurement which requires relay (automatic "
                   "calibration switch) is scheduled. Do you want to connect it?"),
                                            QMessageBox::Yes | QMessageBox::No);
            if (ret == QMessageBox::Yes) {
                connect(k8090, &biomolecules::sprelay::core::k8090::K8090::connected,
                        this, &AppCore::startExperiment);
                connect(k8090, &biomolecules::sprelay::core::k8090::K8090::connectionFailed,
                        this, &AppCore::onRelayConnectionFailed);
                k8090->connectK8090();
                return true;
            } else {
                emit experimentFinished();
                return true;
            }
        }
    }
    return false;
}

void AppCore::buildExpParams()
{
    appState()->clearExpParams();
    appState()->clearCalParams();
    appState()->clearTemperatures();

    AppStateTraits::InitWinSpecParams *params = appState_->initWinSpecParams();

    int sign = ((params->tExp.endT == params->tExp.startT) ?
                    0 :
                    (params->tExp.endT > params->tExp.startT) ?
                        1 : -1);
    int numTMeas = sign * int((params->tExp.endT - params->tExp.startT) /
                              params->tExp.stepT) + 1;
    QString fnb1;
    QString kfnb1;
    QString fnb2;
    QString kfnb2;
    QString fn1;
    QString kfn1;
    QString fn2;
    QString kfn2;
    QString grstr("%1_");
    QString batchstr("%2_");
    int start1 = 0;
    int start2 = 0;
    int step1 = 0;
    int step2 = 0;
    int numSpc1 = 1;
    int numSpc2 = 1;
    int grPos =0;
    int numDgt = 0;
    int n = 1;
    int m = 1;
    if (params->extRan.extendedRange) {
        n = params->expe.size();
    }
    if (params->tExp.tExp) {
        if (params->batch.batchExp) {
            m = params->batch.numSpectra;
        }
        numSpc1 = numTMeas;
        if (params->tExp.loop && numTMeas > 1) {
            numSpc2 = numTMeas - 1;
        }
        if (params->tExp.sameAsT) {
            start1 = params->tExp.startT;
            step1 = sign * params->tExp.stepT;
            if (params->tExp.loop && numTMeas > 1) {
                start2 = params->tExp.startT + sign * (numTMeas - 2) *
                        params->tExp.stepT;
                step2 = - sign * params->tExp.stepT;
            }
            numDgt = 3;
        } else {
            start1 = params->tExp.firstNum;
            step1 = params->tExp.step;
            if (params->tExp.loop && numTMeas > 1) {
                start2 = params->tExp.firstNum + numTMeas *
                        params->tExp.step;
                step2 = params->tExp.step;
            }
            numDgt = params->tExp.numDigits;
        }
    } else if (params->batch.batchExp) {
        start1 = params->batch.firstNum;
        step1 = params->batch.step;
        numSpc1 = params->batch.numSpectra;
        numDgt = params->batch.numDigits;
    }
    for (int jj = 0; jj < n; ++jj) {
        grPos = params->expe.at(jj).grPos;
        if (params->extRan.extendedRange && params->extRan.autoFileNames) {
            if (params->tExp.tExp || params->batch.batchExp) {
                fnb1 = params->extRan.directory % QDir::separator() %
                        params->extRan.fileNameBase % grstr
                        .arg(jj, 1, 10, QChar('0'));
                kfnb1 = params->extRan.directory % QDir::separator() % "k" %
                        params->extRan.fileNameBase % grstr
                        .arg(jj, 1, 10, QChar('0'));
                if (params->tExp.loop && numTMeas > 1) {
                    fnb2 = params->extRan.directory % QDir::separator() %
                            params->extRan.fileNameBase % grstr
                            .arg(jj, 1, 10, QChar('0'));
                    kfnb2 = params->extRan.directory % QDir::separator() % "k" %
                            params->extRan.fileNameBase % grstr
                            .arg(jj, 1, 10, QChar('0'));
                }
            } else {
                fnb1 = params->extRan.directory % QDir::separator() %
                        params->extRan.fileNameBase % QString("%1")
                        .arg(jj, 1, 10, QChar('0'));
                kfnb1 = params->extRan.directory % QDir::separator() % "k" %
                        params->extRan.fileNameBase % QString("%1")
                        .arg(jj, 1, 10, QChar('0'));
                if (params->tExp.loop && numTMeas > 1) {
                    fnb2 = params->extRan.directory % QDir::separator() %
                            params->extRan.fileNameBase % QString("%1")
                            .arg(jj, 1, 10, QChar('0'));
                    kfnb2 = params->extRan.directory % QDir::separator() % "k" %
                            params->extRan.fileNameBase % QString("%1")
                            .arg(jj, 1, 10, QChar('0'));
                }
            }
        } else {
            fnb1 = params->expe.at(jj).directory % QDir::separator() % params->expe.at(jj).fileNameBase;
            kfnb1 = params->expe.at(jj).directory % QDir::separator() % "k" % params->expe.at(jj).fileNameBase;
            if (params->tExp.loop && numTMeas > 1) {
                fnb2 = params->expe.at(jj).directory % QDir::separator() % params->expe.at(jj).fileNameBase;
                kfnb2 = params->expe.at(jj).directory % QDir::separator() % "k" % params->expe.at(jj).fileNameBase;
            }
        }
        if (params->tExp.tExp) {
            if (params->batch.batchExp) {
                fnb1.append(batchstr);
                kfnb1.append(batchstr);
                if (params->tExp.loop && numTMeas > 1) {
                    fnb2.append(batchstr);
                    kfnb2.append(batchstr);
                }
            }
            if (params->tExp.loop && numTMeas > 1 && params->tExp.sameAsT) {
                fnb1.append("a");
                kfnb1.append("a");
                fnb2.append("b");
                kfnb2.append("b");
            }
        }
        for (int ii = 0; ii < m; ++ii) {
            fn1 = fnb1;
            kfn1 = kfnb1;
            fn2 = fnb2;
            kfn2 = kfnb2;
            if (params->tExp.tExp && params->batch.batchExp) {
                qDebug() << "AppCore::buildExpParams():" << ii;
                fn1 = fn1.arg(params->batch.firstNum + ii * params->batch.step,
                        params->batch.numDigits, 10, QChar('0'));
                kfn1 = kfn1.arg(params->batch.firstNum + ii * params->batch.step,
                         params->batch.numDigits, 10, QChar('0'));
                if (params->tExp.loop && numTMeas > 1) {
                    fn2 = fn2.arg(params->batch.firstNum + ii * params->batch.step,
                            params->batch.numDigits, 10, QChar('0'));
                    kfn2 = kfn2.arg(params->batch.firstNum + ii * params->batch.step,
                             params->batch.numDigits, 10, QChar('0'));
                }
            }
            appState_->addExpParams(
                        new WinSpecTasks::Params(
                                            params->expe.at(jj).expo,
                                            params->expe.at(jj).acc,
                                            params->expe.at(jj).frm,
                                            fn1, start1, step1, numSpc1, numDgt, false, grPos));
            if (params->cal.at(jj).autoCal) {
                appState_->addCalParams(
                            new WinSpecTasks::Params(
                                                params->cal.at(jj).expo,
                                                params->cal.at(jj).acc,
                                                params->cal.at(jj).frm,
                                                kfn1, start1, step1, numSpc1, numDgt, false, grPos));
            } else {
                appState_->addCalParams(new WinSpecTasks::Params());
            }
            if (params->tExp.tExp && params->tExp.loop && numTMeas > 1) {
                appState_->addExpParams(
                            new WinSpecTasks::Params(
                                                params->expe.at(jj).expo,
                                                params->expe.at(jj).acc,
                                                params->expe.at(jj).frm,
                                                fn2, start2, step2, numSpc2, numDgt, true, grPos));
                if (params->cal.at(jj).autoCal) {
                    appState_->addCalParams(
                                new WinSpecTasks::Params(
                                                    params->cal.at(jj).expo,
                                                    params->cal.at(jj).acc,
                                                    params->cal.at(jj).frm,
                                                    kfn2, start2, step2, numSpc2, numDgt, true, grPos));
                } else {
                    appState_->addCalParams(new WinSpecTasks::Params());
                }
            }
            if (params->tExp.tExp) {
                appState_->addTemperatures(
                            new NeslabTasks::Temperatures(
                                params->tExp.startT, params->tExp.stepT,
                                params->tExp.endT));
                if (params->tExp.loop) {
                    appState_->addTemperatures(
                                new NeslabTasks::Temperatures(
                                    params->tExp.startT + sign * (numTMeas - 2) *
                                    params->tExp.stepT, params->tExp.stepT,
                                    params->tExp.startT));
                }
            }
        }
    }
}

void AppCore::buildInitialExpTaks(WaitTaskList *waitTaskList)
{
    AppStateTraits::InitWinSpecParams *params = appState()->initWinSpecParams();
    ExpTaskListTraits::TaskItem taskItem;
    WaitTaskListTraits::TaskItem waitTaskItem;
    if (params->tExp.tExp) {
        WaitTaskListTraits::TaskItem waitTaskItem;
        ForkJoinTask * forkJoinTask;
        if (params->cal.at(0).autoCal && params->tExp.initDelayMeas) {
            taskItem.task = new StartWaitingTask(
                        appState_, &params->tExp.initDelay, this);
            taskItem.taskType = ExpTaskListTraits::TaskType::StartWaiting;
            waitTaskList->addTask(taskItem);

            forkJoinTask = new ForkJoinTask(2, this);

            // thread no. 0
            if (params->extRan.extendedRange) {
                taskItem.task = new GratingTasks::SendToPos(appState_, true, params->expe.at(0).grPos, this);
                taskItem.taskType = ExpTaskListTraits::TaskType::GratingSendToPos;
                forkJoinTask->addTask(taskItem, 0);

                waitTaskItem.task = new GratingWaitTask(appState_, this);
                waitTaskItem.waitFor = WaitTaskListTraits::WaitFor::Grating;
                taskItem.task = new WaitExpTask(waitTaskItem, this);
                taskItem.taskType = ExpTaskListTraits::TaskType::WaitExp;
                forkJoinTask->addTask(taskItem, 0);
            }

            waitTaskItem.task = new DelayWaitTask(
                        appState_, &params->tExp.initDelay, this);
            waitTaskItem.waitFor = WaitTaskListTraits::WaitFor::Delay;
            taskItem.task = new WaitExpTask(waitTaskItem, this);
            taskItem.taskType = ExpTaskListTraits::TaskType::WaitExp;
            forkJoinTask->addTask(taskItem, 0);

            taskItem.task = new WaitingTask(this);
            taskItem.taskType = ExpTaskListTraits::TaskType::Waiting;
            forkJoinTask->addTask(taskItem, 0);

            // thread no. 1;
            taskItem.task = new NeslabTasks::SetT(appState_, this);
            taskItem.taskType = ExpTaskListTraits::TaskType::NeslabSetT;
            forkJoinTask->addTask(taskItem, 1);

            taskItem.task = new StageTasks::GoToPosExpList(
                        appState_, StageControlTraits::PosType::Measurement,
                        this);
            taskItem.taskType =
                    ExpTaskListTraits::TaskType::StageControlSendToPos;
            forkJoinTask->addTask(taskItem, 1);

            taskItem.task = forkJoinTask;
            taskItem.taskType = ExpTaskListTraits::TaskType::ForkJoin;
            waitTaskList->addTask(taskItem);

            taskItem.task = new FinishWaitingTask(appState_, this);
            taskItem.taskType = ExpTaskListTraits::TaskType::FinishWaiting;
            waitTaskList->addTask(taskItem);
        } else if (params->cal.at(0).autoCal) {
            TimeSpan timeSpan;
            taskItem.task = new StartWaitingTask(appState_, &timeSpan, this);
            taskItem.taskType = ExpTaskListTraits::TaskType::StartWaiting;
            waitTaskList->addTask(taskItem);

            taskItem.task = new NeslabTasks::SetT(appState_, this);
            taskItem.taskType = ExpTaskListTraits::TaskType::NeslabSetT;
            waitTaskList->addTask(taskItem);

            if (params->extRan.extendedRange) {
                ForkJoinTask *fj = new ForkJoinTask(2, this);

                taskItem.task = new StageTasks::GoToPosExpList(appState_, StageControlTraits::PosType::Measurement, this);
                taskItem.taskType = ExpTaskListTraits::TaskType::StageControlGoToPosExpList;
                fj->addTask(taskItem, 0);

                taskItem.task = new GratingTasks::SendToPos(appState_, true, params->expe.at(0).grPos, this);
                taskItem.taskType = ExpTaskListTraits::TaskType::GratingSendToPos;
                fj->addTask(taskItem, 1);

                waitTaskItem.task = new GratingWaitTask(appState_, this);
                waitTaskItem.waitFor = WaitTaskListTraits::WaitFor::Grating;
                taskItem.task = new WaitExpTask(waitTaskItem, this);
                taskItem.taskType = ExpTaskListTraits::TaskType::WaitExp;
                fj->addTask(taskItem, 1);

                taskItem.task = new WaitingTask(this);
                taskItem.taskType = ExpTaskListTraits::TaskType::Waiting;
                fj->addTask(taskItem, 1);

                taskItem.task = fj;
                taskItem.taskType = ExpTaskListTraits::TaskType::ForkJoin;
                waitTaskList->addTask(taskItem);
            } else {
                taskItem.task = new StageTasks::GoToPosExpList(
                            appState_, StageControlTraits::PosType::Measurement,
                            this);
                taskItem.taskType =
                        ExpTaskListTraits::TaskType::StageControlSendToPos;
                waitTaskList->addTask(taskItem);
            }

            taskItem.task = new FinishWaitingTask(appState_, this);
            taskItem.taskType = ExpTaskListTraits::TaskType::FinishWaiting;
            waitTaskList->addTask(taskItem);
        } else if (params->tExp.initDelayMeas) {
            taskItem.task = new StartWaitingTask(
                        appState_, &params->tExp.initDelay, this);
            taskItem.taskType = ExpTaskListTraits::TaskType::StartWaiting;
            waitTaskList->addTask(taskItem);

            if (params->extRan.extendedRange) {
                taskItem.task = new GratingTasks::SendToPos(appState_, true, params->expe.at(0).grPos, this);
                taskItem.taskType = ExpTaskListTraits::TaskType::GratingSendToPos;
                waitTaskList->addTask(taskItem);

                waitTaskItem.task = new GratingWaitTask(appState_, this);
                waitTaskItem.waitFor = WaitTaskListTraits::WaitFor::Grating;
                taskItem.task = new WaitExpTask(waitTaskItem, this);
                taskItem.taskType = ExpTaskListTraits::TaskType::WaitExp;
                waitTaskList->addTask(taskItem);
            }

            waitTaskItem.task = new DelayWaitTask(
                        appState_, &params->tExp.initDelay, this);
            waitTaskItem.waitFor = WaitTaskListTraits::WaitFor::Delay;
            taskItem.task = new WaitExpTask(waitTaskItem, this);
            taskItem.taskType = ExpTaskListTraits::TaskType::WaitExp;
            waitTaskList->addTask(taskItem);

            taskItem.task = new NeslabTasks::SetT(appState_, this);
            taskItem.taskType = ExpTaskListTraits::TaskType::NeslabSetT;
            waitTaskList->addTask(taskItem);

            taskItem.task = new WaitingTask(this);
            taskItem.taskType = ExpTaskListTraits::TaskType::Waiting;
            waitTaskList->addTask(taskItem);

            taskItem.task = new FinishWaitingTask(appState_, this);
            taskItem.taskType = ExpTaskListTraits::TaskType::FinishWaiting;
            waitTaskList->addTask(taskItem);
        } else {
            taskItem.task = new NeslabTasks::SetT(appState_, this);
            taskItem.taskType = ExpTaskListTraits::TaskType::NeslabSetT;
            waitTaskList->addTask(taskItem);

            if (params->extRan.extendedRange) {
                TimeSpan timeSpan;
                taskItem.task = new StartWaitingTask(appState_, &timeSpan, this);
                taskItem.taskType = ExpTaskListTraits::TaskType::StartWaiting;
                waitTaskList->addTask(taskItem);

                taskItem.task = new GratingTasks::SendToPos(appState_, true, params->expe.at(0).grPos, this);
                taskItem.taskType = ExpTaskListTraits::TaskType::GratingSendToPos;
                waitTaskList->addTask(taskItem);

                waitTaskItem.task = new GratingWaitTask(appState_, this);
                waitTaskItem.waitFor = WaitTaskListTraits::WaitFor::Grating;
                taskItem.task = new WaitExpTask(waitTaskItem, this);
                taskItem.taskType = ExpTaskListTraits::TaskType::WaitExp;
                waitTaskList->addTask(taskItem);

                taskItem.task = new WaitingTask(this);
                taskItem.taskType = ExpTaskListTraits::TaskType::Waiting;
                waitTaskList->addTask(taskItem);

                taskItem.task = new FinishWaitingTask(appState_, this);
                taskItem.taskType = ExpTaskListTraits::TaskType::FinishWaiting;
                waitTaskList->addTask(taskItem);
            }
        }
    } else if (params->cal.at(0).autoCal) {
        TimeSpan timeSpan;
        taskItem.task = new StartWaitingTask(appState_, &timeSpan, this);
        taskItem.taskType = ExpTaskListTraits::TaskType::StartWaiting;
        waitTaskList->addTask(taskItem);

        if (params->extRan.extendedRange) {
            ForkJoinTask *fj = new ForkJoinTask(2, this);

            taskItem.task = new StageTasks::GoToPosExpList(appState_, StageControlTraits::PosType::Measurement, this);
            taskItem.taskType = ExpTaskListTraits::TaskType::StageControlGoToPosExpList;
            fj->addTask(taskItem, 0);

            taskItem.task = new GratingTasks::SendToPos(appState_, true, params->expe.at(0).grPos, this);
            taskItem.taskType = ExpTaskListTraits::TaskType::GratingSendToPos;
            fj->addTask(taskItem, 1);

            waitTaskItem.task = new GratingWaitTask(appState_, this);
            waitTaskItem.waitFor = WaitTaskListTraits::WaitFor::Grating;
            taskItem.task = new WaitExpTask(waitTaskItem, this);
            taskItem.taskType = ExpTaskListTraits::TaskType::WaitExp;
            fj->addTask(taskItem, 1);

            taskItem.task = new WaitingTask(this);
            taskItem.taskType = ExpTaskListTraits::TaskType::Waiting;
            fj->addTask(taskItem, 1);

            taskItem.task = fj;
            taskItem.taskType = ExpTaskListTraits::TaskType::ForkJoin;
            waitTaskList->addTask(taskItem);
        } else {
            taskItem.task = new StageTasks::GoToPosExpList(
                        appState_, StageControlTraits::PosType::Measurement,
                        this);
            taskItem.taskType =
                    ExpTaskListTraits::TaskType::StageControlSendToPos;
            waitTaskList->addTask(taskItem);
        }

        taskItem.task = new FinishWaitingTask(appState_, this);
        taskItem.taskType = ExpTaskListTraits::TaskType::FinishWaiting;
        waitTaskList->addTask(taskItem);
    } else if (params->extRan.extendedRange) {
        TimeSpan timeSpan;
        taskItem.task = new StartWaitingTask(appState_, &timeSpan, this);
        taskItem.taskType = ExpTaskListTraits::TaskType::StartWaiting;
        waitTaskList->addTask(taskItem);

        taskItem.task = new GratingTasks::SendToPos(appState_, true, params->expe.at(0).grPos, this);
        taskItem.taskType = ExpTaskListTraits::TaskType::GratingSendToPos;
        waitTaskList->addTask(taskItem);

        waitTaskItem.task = new GratingWaitTask(appState_, this);
        waitTaskItem.waitFor = WaitTaskListTraits::WaitFor::Grating;
        taskItem.task = new WaitExpTask(waitTaskItem, this);
        taskItem.taskType = ExpTaskListTraits::TaskType::WaitExp;
        waitTaskList->addTask(taskItem);

        taskItem.task = new WaitingTask(this);
        taskItem.taskType = ExpTaskListTraits::TaskType::Waiting;
        waitTaskList->addTask(taskItem);

        taskItem.task = new FinishWaitingTask(appState_, this);
        taskItem.taskType = ExpTaskListTraits::TaskType::FinishWaiting;
        waitTaskList->addTask(taskItem);
    }
}

void AppCore::buildBodyExpTasks(WaitTaskList *waitTaskList)
{
    AppStateTraits::InitWinSpecParams *params = appState()->initWinSpecParams();

    ExpTaskListTraits::TaskItem taskItem;
    if (params->batch.batchExp) {
            taskItem.task = new WholeBatchExpList(appState_, this);
            taskItem.taskType = ExpTaskListTraits::TaskType::WholeBatchExpList;
            waitTaskList->addTask(taskItem);
    } else if (params->tExp.tExp) {
        taskItem.task = new WholeTExpList(appState_, this);
        taskItem.taskType = ExpTaskListTraits::TaskType::WholeTExpList;
        waitTaskList->addTask(taskItem);
    } else if (params->extRan.extendedRange){
        taskItem.task = new WholeExtExpList(appState_, this);
        taskItem.taskType = ExpTaskListTraits::TaskType::WholeExtExpList;
        waitTaskList->addTask(taskItem);
    } else {
        taskItem.task = new WinSpecTasks::ExpList(appState_, false, 0, this);
        taskItem.taskType = ExpTaskListTraits::TaskType::WinSpecExpList;
        waitTaskList->addTask(taskItem);
        if (params->cal.at(0).autoCal) {
            TimeSpan timeSpan;
            taskItem.task = new StartWaitingTask(appState_, &timeSpan, this);
            taskItem.taskType = ExpTaskListTraits::TaskType::StartWaiting;
            waitTaskList->addTask(taskItem);

            taskItem.task = new StageTasks::GoToPosExpList(appState_,
                        StageControlTraits::PosType::Calibration, this);
            taskItem.taskType =
                    ExpTaskListTraits::TaskType::StageControlGoToPosExpList;
            waitTaskList->addTask(taskItem);

            taskItem.task = new WinSpecTasks::ExpList(appState_, true, 0, this);
            taskItem.taskType = ExpTaskListTraits::TaskType::WinSpecExpList;
            waitTaskList->addTask(taskItem);

            taskItem.task = new StageTasks::GoToPosExpList(appState_,
                        StageControlTraits::PosType::Measurement, this);
            taskItem.taskType =
                    ExpTaskListTraits::TaskType::StageControlGoToPosExpList;
            waitTaskList->addTask(taskItem);

            taskItem.task = new FinishWaitingTask(appState_, this);
            taskItem.taskType = ExpTaskListTraits::TaskType::StartWaiting;
            waitTaskList->addTask(taskItem);
        }
    }
}

AppCore::~AppCore()
{
    qDebug() << "AppCore::~AppCore(): rusim AppCore";
    delete taskScheduler;
    delete initStage;
    delete goToPosStage;
    if (neslabThread) {
        neslabThread->quit();
        neslabThread->wait();
    }
}
