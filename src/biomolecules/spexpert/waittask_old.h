#ifndef WAITTASK_OLD_H
#define WAITTASK_OLD_H

#include "exptask.h"
#include "timespan.h"

// forward declarations
class QThread;
class QMutex;
class WinSpec;
class QTimer;
class WaitWorker_old;
class AppState;

class LockableFrame;
class LockableSpectrum;
//template<typename T>
//class LockableQVector;



class WaitTask_old : public ExpTask
{
    Q_OBJECT

private:
    friend class WaitWorker_old;
public:
    enum struct WaitFor : unsigned char
    {
        None         = 0,
        Delay        = 1 << 0,
        WinSpec      = 1 << 1,
        Motor        = 1 << 2,
        Spectrograph = 1 << 3,
        Other1       = 1 << 4,
        Other2       = 1 << 5,
        Other3       = 1 << 6,
        Other4       = 1 << 7
    };

    struct DelayType
    {
        TimeSpan delay;  // delay in milliseconds
        WaitFor waitFor;
        DelayType() : delay(0), waitFor(WaitFor::Delay) {}
        DelayType(WaitFor wf) : delay(0), waitFor(wf) {}
    };

    WaitTask_old(WinSpec *pwinSpec_, DelayType delay_, AppState * pappState_, QObject *parent = 0);
    virtual ~WaitTask_old();

signals:
    void operate(WaitTask_old::DelayType delay_);

public slots:
    virtual void start();
    virtual void stop();
    void waitingFinished();

private slots:
    void reportState(int currAccum, int currFrm);

private:
    QMutex *mutex;
    WinSpec *pwinSpec;
    DelayType delay;
    QThread *thread;
    WaitWorker_old *waitWorker;
    AppState * pappState;
};

inline constexpr WaitTask_old::WaitFor operator|(WaitTask_old::WaitFor a, WaitTask_old::WaitFor b) {
    return static_cast<WaitTask_old::WaitFor>(static_cast<unsigned char>(a) | static_cast<unsigned char>(b)); }

inline constexpr WaitTask_old::WaitFor operator&(WaitTask_old::WaitFor a, WaitTask_old::WaitFor b) {
    return static_cast<WaitTask_old::WaitFor>(static_cast<unsigned char>(a) & static_cast<unsigned char>(b)); }

inline WaitTask_old::WaitFor operator~(WaitTask_old::WaitFor a) { return static_cast<WaitTask_old::WaitFor>(~static_cast<unsigned char>(a)); }

inline WaitTask_old::WaitFor& operator|=(WaitTask_old::WaitFor& a, WaitTask_old::WaitFor b) { return a = (a | b); }

inline WaitTask_old::WaitFor& operator&=(WaitTask_old::WaitFor& a, WaitTask_old::WaitFor b) { return a = (a & b); }

class WaitWorker_old : public QObject
{
    Q_OBJECT

public:
    WaitWorker_old(WinSpec *pwinSpec, AppState *pappState, QObject *parent = 0);
    virtual ~WaitWorker_old();
    void setQuit(bool blQuit);
    bool quit();

public slots:
    void start(WaitTask_old::DelayType delay);
    void winSpecRunning();
    void delayWaiting();

private slots:
    void taskFinishedSlot(WaitTask_old::WaitFor finishedTask_);

signals:
    void finished();
    void taskFinished(WaitTask_old::WaitFor finishedTask_);
    void expStateChanged(int currAccum, int currFrm);
    void lastFrameChanged(bool blLastFrameChanged_);
    void spectrumChanged(bool blSpectrumChanged_);

private:
    WinSpec *pwinSpec_;
    WaitTask_old::DelayType delay_;
    WaitTask_old::DelayType currDelay_;
    WaitTask_old::DelayType backupDelay_;
    AppState *pappState_;
    QMutex *mutex_;
    bool blQuit_;
    QTimer *timer_;
    bool blWaitWorkerRunning_;
    QMutex *mutexWaitWorkerRunning_;

    QDateTime *finishTime_;
    QMutex *mutexStartTime_;
};

Q_DECLARE_METATYPE(WaitTask_old::DelayType)

#endif // WAITTASK_OLD_H
