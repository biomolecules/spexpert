#ifndef EXPTASKS_H
#define EXPTASKS_H

#include "exptask.h"
#include "waittasklist.h"

// forward declarations
class AppState;
class WinSpec;
class StageControl;
class Neslab;
class AppCore;
class MeasurementLog;
class TimeSpan;
namespace StageControlTraits
{
enum class ReferenceType;
enum class InitType;
enum class LimType;
enum class PosType;
}

class StartWaitingTask : public ExpTask
{
    Q_OBJECT
public:
    StartWaitingTask(AppState * pappState, const TimeSpan *delay, QObject *parent = 0);
    ~StartWaitingTask();
    virtual void start();

signals:

public slots:

private:
    AppState * pappState_;
    TimeSpan *delay_;
};

class FinishWaitingTask : public ExpTask
{
    Q_OBJECT
public:
    explicit FinishWaitingTask(AppState * pappState, QObject *parent = 0);
    virtual void start();

private:
    AppState *pappState_;
};

namespace WinSpecTasks
{

class Params
{
public:
    explicit Params(double dblExposure = 1.0, int iAccums = 1, int iFrames = 1, const QString &strFileName = "",
                  int firstFileNameNumber = 0, int stepFileNameCounter = 1, int numMeas = 1, int numDigits = 1, bool forceNum = false, int grPos = 0) :
        vecExposures_(1, dblExposure), vecAccums_(1, iAccums), vecFrames_(1, iFrames), vecFileNames_(1, strFileName), forceNum_(forceNum), grPos_(grPos)
    { initializeParams(firstFileNameNumber, stepFileNameCounter, numMeas, numDigits); }

    Params(const QVector<double> & vecExposures, const QVector<int> & vecAccums,
                  const QVector<int> & vecFrames, const QVector<QString> &vecFileNames);

    // getters
    bool wrongSizes() { return wrongSizes_; }
    void takeOne(double * pexposure, int * paccums, int * pframes, QString &fileName);
    int getNumMeas();
    double getLastExposure() { return lastExposure_; }
    int getLastAccums() { return lastAccums_; }
    int getLastFrames() { return lastFrames_; }
    QString getLastFileName() { return lastFileName_; }
    int grPos();

private:
    int countNumDigits(int number);
    bool addCounter();
    int getCounter() { return counter_; }
    void addFileNameCounter() { ++fileNameCounter_; }
    int getFileNameCounter() { return fileNameCounter_; }
    void initializeParams(int firstFileNameNumber = 0, int stepFileNameCounter = 1, int numMeas = 1, int numDigits = 1);

    template <typename T>
    T takeControl(const QVector<T> & vec, const T & defVal);

    QString takeControl(const QVector<QString> &vec, bool dontCountVals);

    template <typename T>
    void correctVecSize(QVector<T> & vec, const T & defVal);

    QVector<double> vecExposures_;
    QVector<int> vecAccums_;
    QVector<int> vecFrames_;
    QVector<QString> vecFileNames_;

    bool forceNum_;
    int counter_;
    int fileNameCounter_;
    int firstFileNameNumber_;
    int stepFileNameCounter_;
    int numVals_;
    int numMeas_;
    int numDigits_;

    double defaultExposure_;
    int defaultAccums_;
    int defaultFrames_;
    QString defaultFileName_;

    double lastExposure_;
    int lastAccums_;
    int lastFrames_;
    QString lastFileName_;

    bool wrongSizes_;

    int grPos_;
};

class Start : public ExpTask
{
    Q_OBJECT

public:
    Start(AppState *pappState_, double dblExposure_,
                     int iAccums_, int iFrames_, const QString &strFileName_, QObject *parent = 0);
    Start(AppState *pappState_, Params *pwinSpecParams_, QObject *parent = 0);
    Start(AppState *pappState_, bool cal, QObject *parent = 0);
    virtual ~Start();
    void expSetup(double dblExposure_, int iAccums_, int iFrames_, const QString & strFileName_);
    void setFileName(const QString & strFileName_);

signals:

public slots:
    virtual void start();
    virtual void stop();

private:
    WinSpec *pwinSpec;
    AppState *pappState;
    WinSpecTasks::Params *pwinSpecParams;
    double dblExposure;
    int iAccums;
    int iFrames;
    QString strFileName;
    bool autoGetParams;
    bool cal_;
};

class LogLastExpParams : public ExpTask
{
    Q_OBJECT

public:
    LogLastExpParams(AppState *appState, QObject * parent = 0);

public slots:
    virtual void start();

private:
    AppState *appState_;
    MeasurementLog *measurementLog_;
};

class AddExpNumber : public ExpTask
{
    Q_OBJECT

public:
    explicit AddExpNumber(AppState *appState, int shift = 1, QObject *parent = 0);

public slots:
    virtual void start();

private:
    AppState *appState_;
    int shift_;
};

class ExpList : public ExpTaskList
{
    Q_OBJECT

public:
    ExpList(AppState *appState, bool cal, QObject *parent = 0);
};

} // namespace WinSpecTasks

namespace StageTasks
{

class Run : public ExpTask
{
    Q_OBJECT

public:
    Run(AppState *pappState, int dest, StageControlTraits::ReferenceType refType, QObject *parent = 0);
    virtual ~Run();

public slots:
    virtual void start();
    virtual void stop();

private:
    StageControl *pstageControl_;
    AppState *pappState_;
    int dest_;
    StageControlTraits::ReferenceType refType_;
};

class GoToLim : public ExpTask
{
    Q_OBJECT

public:
    GoToLim(AppState *pappState, StageControlTraits::LimType limType, QObject *parent = 0);
    virtual ~GoToLim();

public slots:
    virtual void start();
    virtual void stop();

private:
    StageControl *pstageControl_;
    AppState *pappState_;
    StageControlTraits::LimType limType_;
};

class SendToPos : public ExpTask
{
    Q_OBJECT

public:
    SendToPos(AppState *pappState, StageControlTraits::PosType posType, QObject *parent = 0);
    virtual ~SendToPos();

public slots:
    virtual void start();
    virtual void stop();

private:
    StageControl *pstageControl_;
    AppState *pappState_;
    StageControlTraits::PosType posType_;
};

class SwitchPower : public ExpTask
{
    Q_OBJECT

public:

    enum struct Power {
        On,
        Off
    };

    SwitchPower(AppState *pappState, Power power, QObject *parent);
    virtual ~SwitchPower();

public slots:
    virtual void start();
    virtual void stop();

private:
    StageControl *pstageControl_;
    AppState *pappState_;
    Power power_;
};

class SetLim : public ExpTask
{
    Q_OBJECT

public:
    SetLim(AppState *pappState, StageControlTraits::LimType limType, int limVal = 0, QObject *parent = 0);
    virtual ~SetLim();

public slots:
    virtual void start();
    virtual void stop();

private:
    StageControl *pstageControl_;
    AppState *pappState_;
    StageControlTraits::LimType limType_;
    int limVal_;
};

class Initialize : public WaitTaskList
{
    Q_OBJECT

public:
    Initialize(AppState *pappState, StageControlTraits::InitType initType, int limVal = 0, QObject *parent = 0);
    virtual ~Initialize();

public slots:
    virtual void start();
    virtual void stop();

protected:
    virtual void taskListFinished();

protected slots:
    virtual void onFailed();

private:
    AppState *appState_;
    StageControl *pstageControl_;
    bool successful_;
    StageControlTraits::InitType initType_;
};

class GoToLimWaitList : public WaitTaskList
{
    Q_OBJECT

public:
    GoToLimWaitList(AppState *pappState, StageControlTraits::LimType limType, QObject *parent = 0);

public slots:
    virtual void start();
    virtual void stop();

private:
    AppState *appState_;
    StageControl *stageControl_;
};

class GoToPos : public WaitTaskList
{
    Q_OBJECT

public:
    GoToPos(AppState *pappState, StageControlTraits::PosType posType, QObject *parent = 0);
    virtual ~GoToPos();

public slots:
    virtual void stop();

private:
    StageControl *pstageControl_;
    bool successful_;
};

class GoToPosExpList : public ExpTaskList
{
    Q_OBJECT

public:
    GoToPosExpList(AppState *appState, StageControlTraits::PosType posType, QObject *parent = 0);
};

}

namespace NeslabTasks {

class Temperatures
{
public:
    explicit Temperatures(double startT = 20.0, double stepT = 1.0, double endT = 20.0);
    explicit Temperatures(const QList<double> & TList);

    double takeT();

private:
    QList<double> temperatures;
    double startT_;
    double stepT_;
    double endT_;
    int tCounter;
};

class SetT : public ExpTask
{
    Q_OBJECT

public:
    SetT(AppState *appState, Temperatures *measT, QObject *parent = 0);
    SetT(AppState *appState, double t, QObject *parent = 0);
    explicit SetT(AppState *appState, QObject *parent = 0);
    SetT(AppState *appState, bool shiftT, int shift, bool setT, QObject *parent = 0);

public slots:
    virtual void start();

private:
    AppState *appState_;
    Neslab * neslab_;
    Temperatures *measT_;
    bool useTemperatures;
    double t_;
    bool autoGetT;
    bool shiftT_;
    double shift_;
    bool setT_;
};

class ReadT : public ExpTask
{
    Q_OBJECT

public:
    explicit ReadT(AppState * appState, QObject *parent = 0);

public slots:
    virtual void start();

private:
    Neslab * neslab_;
    AppCore * appCore_;
};

} // namespace NeslabTasks

namespace GratingTasks
{

class SendToPos : public ExpTask
{
    Q_OBJECT

public:
    SendToPos(AppState *appState, WinSpecTasks::Params *wsp, QObject *parent = 0);
    SendToPos(AppState *appState, int pos, QObject *parent = 0);
    SendToPos(AppState *appState, bool shiftGr, int shiftGrExpNum, QObject *parent = 0);

public slots:
    virtual void start();

private:
    AppState *appState_;
    WinSpecTasks::Params *winSpecParams_;
    bool useWinSpecParams;
    bool autoGetPos;
    double pos_;
    bool shiftGr_;
    int shiftGrExpNum_;
};} // namespace GratingTasks

class SaveExpLog : public ExpTask
{
    Q_OBJECT

public:
    explicit SaveExpLog(MeasurementLog *measurementLog, QObject *parent = 0);

public slots:
    virtual void start();

private:
    MeasurementLog *measurementLog_;
};

class WholeExtExpList : public ExpTaskList
{
    Q_OBJECT

public:
    explicit WholeExtExpList(AppState *appState, QObject *parent = 0);
};

class TExpList : public ExpTaskList
{
    Q_OBJECT

public:
    TExpList(AppState *appState, TimeSpan *delay, QObject *parent = 0);
    TExpList(AppState *appState, TimeSpan *delay, int expNumber, int shiftT, bool setT, int shiftGr, QObject *parent = 0);
};

class ExtTExpList : public ExpTaskList
{
    Q_OBJECT

public:
    ExtTExpList(AppState *appState, TimeSpan *delay, int shiftT, bool last, QObject *parent = 0);
};

class WholeTExpList : public ExpTaskList
{
    Q_OBJECT

public:
    WholeTExpList(AppState *appState, QObject *parent = 0);
};

class BatchExpList : public ExpTaskList
{
    Q_OBJECT

public:
    explicit BatchExpList(AppState *appState, QObject *parent = 0);
};

class WholeBatchExpList : public ExpTaskList
{
    Q_OBJECT

public:
    WholeBatchExpList(AppState *appState, QObject *parent = 0);
};

#endif // EXPTASKS_H
