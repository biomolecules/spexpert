#ifndef APPSTATE_H
#define APPSTATE_H

#include <QObject>
#include "waittasklist.h"
#include "timespan.h"

// forward declarations
class QMutex;
class QFile;
class QTextStream;
class QFileInfo;

class WinSpec;
class StageControl;
class Neslab;
class AppCore;
class MeasurementLog;
class LockableFrame;
class LockableSpectrum;
class LockablePlotSpectrum;
class TimeSpan;

class QDateTime;
namespace WinSpecTasks
{
class Params;
}
namespace StageControlTraits
{
class Params;
}
namespace NeslabTasks
{
class Temperatures;
}
namespace NeslabusWidgets
{
namespace AutoReadT
{
struct Settings;
}
}

//template<typename T>
//class LockableQVector;

namespace AppStateTraits
{
enum struct PlotStyle
{
    Spectra      = 0,
    Temperatures = 1
};

enum struct PlotType
{
    None = 0,
    Frame = 1,
    Spectrum = 2,
};

enum struct WinSpecState
{
    Ready = 0,
    Running = 1,
};

enum struct TState
{
    None = 0,
    Reading = 1
};

struct ExpWinSpecParams
{
    int grPos;
    QString fileNameBase;
    QString directory;
    double expo;
    int acc;
    int frm;
};

struct CalWinSpecParams
{
    bool autoCal;
    int eachMeas;
    double expo;
    int acc;
    int frm;
};

struct BatchExpParams
{
    bool batchExp;
    int numSpectra;
    int firstNum;
    int step;
    int numDigits;
    bool delayMeas;
    TimeSpan delay;
};

struct TExpParams
{
    bool tExp;
    double startT;
    double stepT;
    double endT;
    bool afterMeas;
    double afterMeasT;
    bool sameAsT;
    int firstNum;
    int step;
    int numDigits;
    bool initDelayMeas;
    TimeSpan initDelay;
    bool delayMeas;
    TimeSpan delay;
    bool loop;
    TimeSpan loopDelay;
};

struct ExtendedRangeParams
{
    bool extendedRange;
    bool autoFileNames;
    QString fileNameBase;
    QString directory;
};

struct InitWinSpecParams
{
    QList<ExpWinSpecParams> expe;
    QList<CalWinSpecParams> cal;
    BatchExpParams batch;
    TExpParams tExp;
    ExtendedRangeParams extRan;
};

}


class AppState : public QObject
{
    Q_OBJECT
public:
    explicit AppState(AppCore *appCore, QObject *parent = 0);
    virtual ~AppState();
    void makePlotSpectrum();

    void startMeasT();

    void addExpParams(WinSpecTasks::Params * winSpecParams);
    void clearExpParams();
    WinSpecTasks::Params *expParamsAt(int ind);
    void addCalParams(WinSpecTasks::Params * winSpecParams);
    void clearCalParams();
    WinSpecTasks::Params *calParamsAt(int ind);
    void addTemperatures(NeslabTasks::Temperatures *temperatures);
    void clearTemperatures();
    NeslabTasks::Temperatures *temperaturesTakeAt(int ind);
    void addReadT(double t);
    void clearReadTs();
    const QVector<double> & readTs();
    const QVector<double> & tReadTs();
    void addMeasT(double t);
    void clearMeasTs();
    const QVector<double> & measTs();
    const QVector<double> & tMeasTs();
    double tYMax();
    double tYMin();

    void newMeasurementLog(const QString & logFile, bool saveTs = false,
                           bool saveGrPosisitions = false);
    void clearMeasurementLog();

    // getters
    AppCore *appCore();
    WinSpec *winSpec();
    StageControl *stageControl();
    Neslab *neslab();
    MeasurementLog *measurementLog();
    void lastExpParams(double *expo, int *acc, int *frm, QString *fn);
    int lastGrPos();

    AppStateTraits::InitWinSpecParams *initWinSpecParams();

    LockableFrame & getLastFrame() { return * lastFrame; }
    const LockableFrame & getConstLastFrame() const { return * lastFrame; }

    LockableSpectrum & getSpectrum();
    const LockableSpectrum & getConstSpectrum() const;

    LockablePlotSpectrum & getPlotSpectrum();
    const LockablePlotSpectrum & getConstPlotSpectrum() const;

    AppStateTraits::PlotType getPlotType() const;
    AppStateTraits::PlotStyle plotStyle() const;
    bool lastFrameChanged() const;
    bool spectrumChanged() const;
    void addWaitingState(WaitTaskListTraits::WaitFor ws);
    void removeWaitingState(WaitTaskListTraits::WaitFor ws);
    int getCurrAccum() const;
    int getCurrFrame() const;
    int getCurrExpNumber() const;
    int getCurrStagePos() const;
    TimeSpan getRemainingWait() const;
    AppStateTraits::WinSpecState winSpecState() const;
    AppStateTraits::TState tState() const;
    WaitTaskListTraits::WaitFor waitingState() const;
    QDateTime getMeasurementStartedTime() const;
    QDateTime getMeasurementFinishTime() const;
    QDateTime getWaitingStartedTime() const;
    QDateTime getWaitingFinishTime() const;
    StageControlTraits::Params *stageParams();
    NeslabusWidgets::AutoReadT::Settings* autoReadTSettings() const;
    double lastT() const;

    // setters
    void setLastExpParams(double expo, int acc, int frm, const QString &fn);
    void setLastGrPos(int gp);
    void setInitWinSpecParams(const AppStateTraits::InitWinSpecParams &wp);
    void setPlotType(AppStateTraits::PlotType plotType_);
    void setPlotStyle(AppStateTraits::PlotStyle ps);
    void setCurrExpParams(int currAccum_, int currFrame_, AppStateTraits::WinSpecState enumWinSpecState_);
    void setCurrStageState(int currPos);
    void setCurrAccum(int currAccum_);
    void setCurrFrame(int currFrame_);
    void setCurrExpNumber(int currExpNumber_);
    void setWinSpecState(AppStateTraits::WinSpecState enumWinSpecState_);
    void setTState(AppStateTraits::TState ts);
    void setWaitingState(WaitTaskListTraits::WaitFor ws);
    void measurementStartedTime();
    void setMeasurementFinishTime(const TimeSpan &delay);
    void waitingStartedTime();
    void setWaitingFinishTime(const TimeSpan &delay);
    void setAutoReadTSettings(const NeslabusWidgets::AutoReadT::Settings &s);
    void setStageParamsToStage();

signals:
    void plotTypeChanged();
    void lastFrameChangedSignal();
    void spectrumChangedSignal();
    void readTAdded();
    void measTAdded();

public slots:
    void setLastFrameChanged(bool blLastFrameChanged_);
    void setSpectrumChanged(bool blSpectrumChanged_);
    void setT(double t);

private:
    AppCore *appCore_;
    WinSpec *winSpec_;
    StageControl *stageControl_;
    Neslab *neslab_;
    MeasurementLog *measurementLog_;

    double xSpectrumShift; // shift in xscale between frames, when spectrum is ploted.
    double ySpectrumShift; // shift in yscale between frames, when spectrum is ploted.

    LockableFrame * lastFrame;
    bool blLastFrameChanged;
    mutable QMutex * mutexLastFrameChanged;

    LockableSpectrum * spectrum;
    bool blSpectrumChanged;
    mutable QMutex * mutexSpectrumChanged;

    LockablePlotSpectrum * plotSpectrum;

    AppStateTraits::PlotType plotType;
    mutable QMutex * mutexPlotType;

    double lastExp_;
    int lastAcc_;
    int lastFrm_;
    QString lastFN_;
    QMutex * lastExpParamsMutex;

    int lastGrPos_;
    QMutex *lastGrPosMutex;

    int currAccum;
    int currFrame;
    int currExpNumber;
    AppStateTraits::WinSpecState enumWinSpecState;
    QMutex * currExpParamMutex;

    AppStateTraits::TState tState_;
    QMutex * tStateMutex;

    double lastT_;

    int currStagePos_;
    QMutex *stageStateMutex;

    QDateTime * measStartedTime;
    QMutex * measStartedMutex;

    QDateTime * measFinishTime;
    QMutex * measFinishMutex;

    QDateTime * waitStartedTime;
    QMutex * waitStartedMutex;

    QDateTime * waitFinishTime;
    QMutex * waitFinishMutex;

    WaitTaskListTraits::WaitFor waitingState_;
    QMutex *waitingStateMutex_;

    QList<WinSpecTasks::Params*> expParamList_;
    QMutex *expParamListMutex_;

    QList<WinSpecTasks::Params*> calParamList_;
    QMutex *calParamListMutex_;

    QList<NeslabTasks::Temperatures*> temperatureList_;

    StageControlTraits::Params* stageParams_;

    AppStateTraits::InitWinSpecParams initWinSpecParams_;

    NeslabusWidgets::AutoReadT::Settings *autoReadTSettings_;

    AppStateTraits::PlotStyle plotStyle_;
    QMutex *plotStyleMutex;

    QVector<double> readTs_;
    QVector<double> tReadTs_;
    QVector<double> measTs_;
    QVector<double> tMeasTs_;
    int maxReadT;
    int readTfreq;
    int readTcounter;
    QDateTime *startMeasT_;
    double tYMax_;
    double tYMin_;
};

class MeasurementLog : public QObject
{
    Q_OBJECT
public:
    explicit MeasurementLog(const QString & logFile, bool saveTs = false,
                            bool saveGrPosisitions = false,
                            QObject * parent = 0);
    ~MeasurementLog();

    void saveHeader();
    void saveMeasurement();

public slots:
    void setExpParams(double expo, int acc, int frm, const QString &measFile);
    void setSaveT(double t);
    void setSaveGrPos(double grPos);

private:
    void saveToFile();

    bool saveMeas_;
    QFile *logFile_;
    QTextStream *logTextStream;
    double exp_;
    int acc_;
    int frm_;
    QFileInfo *measFile_;
    QDateTime *dateTime_;
    bool saveExpParams_;
    double t_;
    bool saveTs_;
    bool saveT_;
    double grPos_;
    bool saveGrPositions_;
    bool saveGrPos_;
};

#endif // APPSTATE_H
