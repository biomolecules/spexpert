#include "winspec.h"

#if defined(SPEXPERT_MOCK_WINSPEC)
#include "mock_winspec.h"
#else
#include "winspecvb.h"
#endif

#include "lockableqvector.h"

#include <QMutex>
#include <QMutexLocker>
#include <algorithm> // to provide copy on containers

WinSpec::WinSpec()
    :
#if defined(SPEXPERT_MOCK_WINSPEC)
      winSpec{new biomolecules::spexpert::core::MockWinSpec}
#else
      winSpec{new WinSpecVB::WinSpecVB}
#endif
{
    qDebug() << "Vytvarim WinSpec...";
    mutex = new QMutex;
}

bool WinSpec::start(double dblExpo_, int iAccums_, int iFrames_, const QString & strFileName_)
{
    QMutexLocker locker(mutex);
    return winSpec->Start_2(dblExpo_, iAccums_, iFrames_, strFileName_);
}

void WinSpec::stop()
{
    QMutexLocker locker(mutex);
    winSpec->StopRunning();
}

bool WinSpec::running()
{
    QMutexLocker locker(mutex);
    return winSpec->Running();
}

int WinSpec::getAccum()
{
    QMutexLocker locker(mutex);
    return winSpec->GetAccum();
}

int WinSpec::getFrame()
{
    QMutexLocker locker(mutex);
    return winSpec->GetFrame();
}

bool WinSpec::getSpectrum(LockableSpectrum &spectrum_, int *iFrames_, int *iX_, int *iY_)
{
    int iFr, iX, iY;
    bool blStat;
    QVariantList outspec;

    QMutexLocker locker(mutex);
    outspec = winSpec->GetSpectrum(iFr, iX, iY, blStat);
    locker.unlock();

    qDebug() << "GetSpectrum status: " << blStat;
    if (!blStat)
        return blStat;
    if (outspec.size() != iFr * iX * iY)
        return false;

    QMutexLocker specLocker(spectrum_.toMutexY());
    spectrum_.toYQVector().resize(iFr);
    {
        FrameIterator fI;
        QVariantList::Iterator oI = outspec.begin();
        for (SpectrumIterator sI = spectrum_.toYQVector().begin(); sI != spectrum_.toYQVector().end(); ++sI)
        {
            sI->resize(iX);
            for (fI = sI->begin(); fI != sI->end(); ++fI)
            {
                *fI = std::accumulate(oI, oI + iY, 0, [] (const QVariant & a, const QVariant & b) ->double { return a.toInt() + b.toInt(); } );
                oI += iY;
            }
        }
    }
    QMutexLocker xLocker(spectrum_.toMutexX());
    if (spectrum_.toXQVector().size() != spectrum_.toYQVector().first().size())
        spectrum_.autoGenerateX();
    specLocker.unlock();
    xLocker.unlock();
    //emit spectrumChanged(true);
    *iFrames_ = iFr;
    *iX_ = iX;
    *iY_ = iY;
    return true;
}

bool WinSpec::getLastFrame(LockableFrame &lastFrame_, int *iFrame_, int *iX_, int *iY_)
{
    int iFr, iX, iY;
    bool blStat;
    QVariantList outFram;
    QMutexLocker locker(mutex);
    outFram = winSpec->GetLastFrame(iFr, iX, iY, blStat);
    locker.unlock();
//    qDebug() << "WinSpec::getLastFrame(): status: " << blStat;
    if (!blStat)
        return blStat;
    if (outFram.size() != iX * iY)
        return false;

    QMutexLocker framLocker(lastFrame_.toMutexY());
    lastFrame_.toYQVector().resize(iX);
    {
        QVariantList::Iterator oI = outFram.begin();
        for (FrameIterator fI = lastFrame_.toYQVector().begin(); fI != lastFrame_.toYQVector().end(); ++fI)
        {
            *fI = std::accumulate(oI, oI + iY, 0, [] (const QVariant & a, const QVariant & b) ->double { return a.toInt() + b.toInt(); } );
            oI += iY;
        }
    }
    QMutexLocker xLocker(lastFrame_.toMutexX());
    if (lastFrame_.toXQVector().size() != lastFrame_.toYQVector().size())
        lastFrame_.autoGenerateX();
    framLocker.unlock();
    xLocker.unlock();
//    qDebug() << "Vytvoril jsem LockableFrame pro frame: " << iFr;
    //emit lastFrameChanged(true);
    *iFrame_ = iFr;
    *iX_ = iX;
    *iY_ = iY;
    return true;

}

void WinSpec::getAcqParams(double *dblExposure_, int *iAccums_, int *iFrames_)
{
    double dblExposure;
    int iAccums, iFrames;
    QMutexLocker locker(mutex);
    winSpec->GetWinSpecAcqParams(dblExposure, iAccums, iFrames);
    locker.unlock();
    *dblExposure_ = dblExposure;
    *iAccums_ = iAccums;
    *iFrames_ = iFrames;
}

void WinSpec::getAcqParams(double *dblExposure_, int *iAccums_)
{
    double dblExposure;
    int iAccums;
    QMutexLocker locker(mutex);
    winSpec->GetWinSpecAcqParams_2(dblExposure, iAccums);
    locker.unlock();
    *dblExposure_ = dblExposure;
    *iAccums_ = iAccums;
}

void WinSpec::setAcqParams(double dblExposure_, int iAccums_, int iFrames_)
{
    QMutexLocker locker(mutex);
    winSpec->SetWinSpecAcqParams(dblExposure_, iAccums_, iFrames_);
}

void WinSpec::setAcqParams(double dblExposure_, int iAccums_)
{
    QMutexLocker locker(mutex);
    winSpec->SetWinSpecAcqParams_2(dblExposure_, iAccums_);
}

void WinSpec::getFilePath(QString &strFileName_)
{
    QMutexLocker locker(mutex);
    winSpec->GetFilePath(strFileName_);
}

void WinSpec::setFilePath(const QString &strFileName_)
{
    QMutexLocker locker(mutex);
    winSpec->SetFilePath(strFileName_);
}

bool WinSpec::save()
{
    QMutexLocker locker(mutex);
    return winSpec->Save();
}

bool WinSpec::saveAs(const QString &strFileName_)
{
    QMutexLocker locker(mutex);
    return winSpec->SaveAs(strFileName_);
}

bool WinSpec::activateWindow()
{
    QMutexLocker locker(mutex);
    return winSpec->ActivateWindow();
}

void WinSpec::closeWindow()
{
    QMutexLocker locker(mutex);
    winSpec->CloseWindow();
}

void WinSpec::closeAllWindows()
{
    QMutexLocker locker(mutex);
    winSpec->CloseAllWindows();
}

bool WinSpec::quitWinSpec()
{
    QMutexLocker locker(mutex);
    return winSpec->QuitWinSpec();
}

bool WinSpec::showWinSpecWin()
{
    QMutexLocker locker(mutex);
    return winSpec->ShowWinSpecWin();
}

bool WinSpec::winSpecConnectionFailed()
{
    QMutexLocker locker(mutex);
    return winSpec->WinSpecConnectionFailed();
}

WinSpec::~WinSpec()
{
    qDebug() << "Rusim WinSpec...";
    delete mutex;
}
