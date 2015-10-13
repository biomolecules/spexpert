#ifndef WAITTASKS_H
#define WAITTASKS_H

#include "waittask.h"

// forward declarations
class AppState;
class QDateTime;
class WinSpec;
class StageControl;
class TimeSpan;

class DelayWaitTask : public WaitTask
{
    Q_OBJECT

public:
    explicit DelayWaitTask(AppState * pappState, const TimeSpan *delay, QObject * parent = 0);
    virtual ~DelayWaitTask();
    virtual bool running();

public slots:
    virtual void start();
    virtual void stop();
    virtual void finish();

private:
    AppState *pappState_;
    TimeSpan *delay_;
    QDateTime *finishTime_;
};

class WinSpecWaitTask : public WaitTask
{
    Q_OBJECT

public:
    WinSpecWaitTask(AppState * pappState, QObject * parent = 0);
    virtual ~WinSpecWaitTask();
    virtual bool running();

signals:
    void spectrumChanged(bool blSpectrumChanged);
    void lastFrameChanged(bool blLastFrameChanged);

public slots:
    virtual void start();
    virtual void stop();
    virtual void finish();

private:
    AppState * pappState_;
    WinSpec * pwinSpec_;
    bool hasSpectrum_;
};

class StageControlWaitTask : public WaitTask
{
    Q_OBJECT

public:
    StageControlWaitTask(AppState *pappState, QObject *parent = 0);
    virtual ~StageControlWaitTask();
    virtual bool running();

public slots:
    virtual void start();
    virtual void stop();
    virtual void finish();

private:
    AppState *pappState_;
    StageControl *pstageControl_;
};

class GratingWaitTask : public WaitTask
{
    Q_OBJECT

public:
    explicit GratingWaitTask(AppState * appState, QObject * parent = 0);
    virtual ~GratingWaitTask();
    virtual bool running();

public slots:
    virtual void start();
    virtual void stop();
    virtual void finish();

private:
    AppState *appState_;
    QDateTime *finishTime_;
};

#endif // WAITTASKS_H
