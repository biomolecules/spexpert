#ifndef APPCORE_H
#define APPCORE_H

#include <QObject>
#include <QDebug>

// forward declarations
class ExpTaskList;
class WinSpec;
class StageControl;
class Neslab;
class AppState;
class CentralWidget;
class MainWindow;
class QStatusBar;
class QLabel;
class QTimer;
class QThread;
class MeasurementLog;
class WaitTaskList;

namespace WinSpecTasks
{
class Params;
}

namespace AppStateTraits
{
class InitWinSpecParams;
}

namespace StageTasks
{
class Initialize;
class GoToPos;
}

class AppCore : public QObject
{
    Q_OBJECT

public:
    AppCore(CentralWidget *centralWidget, QStatusBar *sb, MainWindow *mainWindow, QObject *parent = 0);
    virtual ~AppCore();

    // getters
    AppState *appState();
    MainWindow *mainWindow();
    CentralWidget *centralWidget();

    void reportParams();


signals:
    void experimentStarted();
    void experimentFinished();
    void stageMovementStarted();
    void stageMovementFinished();
    void initStageFinished();
    void startReadingTemperature();
    void finishReadingTemperature();

public slots:
    void startExperiment();
    void stopExperiment();
    void onGoToMeas();
    void onGoToCal();
    void onInitStage();
    void stopStage();
    void onInitStageFinished();
    void stageIsAtPos();
    void updatePlot();
    void updateMeasPlot();
    void updateReadPlot();
    void plotTypeChanged();
    void updateApp();
    void onTaskSchedulerFinished();
    void onReadTSetpoint(double t);
    void onReadExpT(double t);
    void onNeslabConnectionFailed();
    void onRelayConnectionFailed();

private:
    bool expAutoCal();
    bool expInitializeStage();
    bool expConnectNeslab();
    bool expConnectRelay();

    void buildExpParams();

    void buildInitialExpTaks(WaitTaskList *waitTaskList);
    void buildBodyExpTasks(WaitTaskList *waitTaskList);

    ExpTaskList *taskScheduler;
    QThread *neslabThread;
    StageTasks::Initialize *initStage;
    StageTasks::GoToPos *goToPosStage;
    AppState *appState_;
    CentralWidget *centralWidget_; // this is only pointer, remember not to delete it!!!
    QStatusBar *mainStatusBar;
    MainWindow *mainWindow_;
    QTimer * appTimer;
    MeasurementLog * measurementLog;

    QLabel *messageStatusBarLabel;
    QLabel *winSpecStatusBarLabel;
    QLabel *temperatureStatusBarLabel;
    QLabel *waitingStatusBarLabel;
    QLabel *stagePosStatusBarLabel;
    QLabel *gratingPosStatusBarLabel;

    int iPixelX;
    int iPixelY;
    double dblAcqExposure;
    int iAccums;
    int iFrames;
    double dblExposure;
    static const QString newDocName;
    QString strFileNameLong;

    bool stageAtMeas;
};

#endif // APPCORE_H
