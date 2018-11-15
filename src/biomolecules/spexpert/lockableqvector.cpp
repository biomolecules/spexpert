#include "lockableqvector.h"
#include <limits>

bool LockableFrame::autoResizeX()
{
    if (x.size() != y.size())
    {
        x.resize(y.size());
        return true;
    }
    else
    {
        return false;
    }
}

bool LockableFrame::autoResizeXSafe()
{
    QMutexLocker lockerY(&mutexY);
    QMutexLocker lockerX(&mutexX);
    if (x.size() != y.size())
    {
        x.resize(y.size());
        lockerY.unlock();
        lockerX.unlock();
        return true;
    }
    else
    {
        lockerY.unlock();
        lockerX.unlock();
        return false;
    }
}

void LockableFrame::autoGenerateX()
{
    if (x.size() != y.size())
    {
        x.resize(y.size());
    }
    FrameIterator it;
    int ii;
    for (ii = 1, it = x.begin(); it != x.end(); ++ii, ++it)
        *it = ii;
}

void LockableFrame::autoGenerateXSafe()
{
    QMutexLocker lockerY(&mutexY);
    QMutexLocker lockerX(&mutexX);
    if (x.size() != y.size())
    {
        x.resize(y.size());
    }
    FrameIterator it;
    int ii;
    for (ii = 0, it = x.begin(); it != x.end(); ++ii, ++it)
        *it = ii;
}


bool LockableSpectrum::autoResizeX()
{
    if (y.size() < 1 && x.size() > 0)
    {
        x.resize(0);
        return true;
    }
    else if (x.size() != y.first().size())
    {
        x.resize(y.first().size());
        return true;
    }
    else
    {
        return false;
    }
}

bool LockableSpectrum::autoResizeXSafe()
{
    QMutexLocker lockerY(&mutexY);
    QMutexLocker lockerX(&mutexX);
    if (y.size() < 1 && x.size() > 0)
    {
        x.resize(0);
        return true;
    }
    else if (x.size() != y.first().size())
    {
        x.resize(y.first().size());
        lockerY.unlock();
        lockerX.unlock();
        return true;
    }
    else
    {
        lockerY.unlock();
        lockerX.unlock();
        return false;
    }
}

void LockableSpectrum::autoGenerateX()
{
    if (y.size() < 1 && x.size() > 0)
    {
        x.resize(0);
        return;
    }
    else if (x.size() != y.first().size())
    {
        x.resize(y.first().size());
    }
    FrameIterator it;
    int ii;
    for (ii = 0, it = x.begin(); it != x.end(); ++ii, ++it)
        *it = ii;
}

void LockableSpectrum::autoGenerateXSafe()
{
    QMutexLocker lockerY(&mutexY);
    QMutexLocker lockerX(&mutexX);
    if (y.size() < 1 && x.size() > 0)
    {
        x.resize(0);
        return;
    }
    else if (x.size() != y.first().size())
    {
        x.resize(y.first().size());
    }
    FrameIterator it;
    int ii;
    for (ii = 0, it = x.begin(); it != x.end(); ++ii, ++it)
        *it = ii;
}


LockablePlotSpectrum::LockablePlotSpectrum()
{
    for (int ii = 0; ii < 4; ++ii)
        lims[ii] = 0;
}

bool LockablePlotSpectrum::autoResizeX()
{
    if (y.size() < 1 && x.size() > 0)
    {
        x.resize(0);
        return true;
    }
    else if (x.size() != y.size() || x.size() < 1 || x.first().size() != y.first().size())
    {
        x.resize(y.size());
        for (SpectrumIterator it = x.begin(); it != x.end(); ++it)
        {
            it->resize(y.first().size());
        }
        return true;
    }
    else
    {
        return false;
    }
}

bool LockablePlotSpectrum::autoResizeXSafe()
{
    QMutexLocker lockerY(&mutexY);
    QMutexLocker lockerX(&mutexX);
    if (y.size() < 1 && x.size() > 0)
    {
        x.resize(0);
        return true;
    }
    else if (x.size() != y.size() || x.size() < 1 || x.first().size() != y.first().size())
    {
        x.resize(y.size());
        for (SpectrumIterator it = x.begin(); it != x.end(); ++it)
        {
            it->resize(y.first().size());
        }
        return true;
    }
    else
    {
        return false;
    }
}

void LockablePlotSpectrum::autoGenerateX()
{
    if (y.size() < 1 && x.size() > 0)
    {
        x.resize(0);
        return;
    }
    else if (x.size() != y.size() || x.size() < 1 || x.first().size() != y.first().size())
    {
        XSpectrumIterator itS;
        XScaleIterator itF;
        int ii;
        x.resize(y.size());
        for (itS = x.begin(); itS != x.end(); ++itS)
        {
            itS->resize(y.first().size());
            for (ii = 0, itF = itS->begin(); itF != itS->end(); ++ii, ++itF)
                *itF = ii;
        }
    }
}

void LockablePlotSpectrum::autoGenerateXSafe()
{
    QMutexLocker lockerY(&mutexY);
    QMutexLocker lockerX(&mutexX);
    if (y.size() < 1 && x.size() > 0)
    {
        x.resize(0);
        return;
    }
    else if (x.size() != y.size() || x.size() < 1 || x.first().size() != y.first().size())
    {
        XSpectrumIterator itS;
        XScaleIterator itF;
        int ii;
        x.resize(y.size());
        for (itS = x.begin(); itS != x.end(); ++itS)
        {
            itS->resize(y.first().size());
            for (ii = 0, itF = itS->begin(); itF != itS->end(); ++ii, ++itF)
                *itF = ii;
        }
    }
}

void LockablePlotSpectrum::shiftFromSpectrum(const LockableSpectrum & spectrum, double xShift, double yShift)
{
    int N = spectrum.toConstYQVector().size(); // number of spectra
    int M = spectrum.toConstXQVector().size(); // number of spectral points
    x.resize(N);
    y.resize(N);
    SpectrumIterator itS1;
    ConstSpectrumIterator itS2;
    FrameIterator itF1;
    ConstFrameIterator itF2;
    XSpectrumIterator itXS;
    XScaleIterator itX1;
    ConstXScaleIterator itX2;
    int ii;
    for (ii = 0, itS1 = y.begin(), itS2 = spectrum.toConstYQVector().begin(), itXS = x.begin(); itS1 != y.end(); ++ii, ++itS1, ++itS2, ++itXS)
    {
        itS1->resize(M);
        itXS->resize(M);
        for (itF1 = itS1->begin(), itF2 = itS2->begin(), itX1 = itXS->begin(), itX2 = spectrum.toConstXQVector().begin();
             itF1 != itS1->end(); ++itF1, ++itF2, ++itX1, ++itX2)
        {
            *itF1 = *itF2 + ii * yShift;
            *itX1 = *itX2 + ii * xShift;
        }
    }
}

void LockablePlotSpectrum::shiftFromSpectrumSafe(LockableSpectrum &spectrum, double xShift, double yShift)
{
    QMutexLocker lockerY1(&mutexY);
    QMutexLocker lockerX1(&mutexX);
    QMutexLocker lockerX2(spectrum.toMutexX());
    QMutexLocker lockerY2(spectrum.toMutexY());
    int N = spectrum.toYQVector().size(); // number of spectra
    int M = spectrum.toXQVector().size(); // number of spectral points
    x.resize(N);
    y.resize(N);
    SpectrumIterator itS1, itS2;
    FrameIterator itF1, itF2;
    XSpectrumIterator itXS;
    XScaleIterator itX1, itX2;
    int ii;
    if (spectrum.toYQVector().size() < 1 || spectrum.toXQVector().size() < 1)
        return;
    QMutexLocker lockerLims(&mutexLims);
    lims[0] = std::numeric_limits<double>::max();
    lims[1] = std::numeric_limits<double>::min();
    lims[2] = std::numeric_limits<double>::max();
    lims[3] = std::numeric_limits<double>::min();
    for (ii = 0, itS1 = y.begin(), itS2 = spectrum.toYQVector().begin(), itXS = x.begin(); itS1 != y.end(); ++ii, ++itS1, ++itS2, ++itXS)
    {
        itS1->resize(M);
        itXS->resize(M);
        for (itF1 = itS1->begin(), itF2 = itS2->begin(), itX1 = itXS->begin(), itX2 = spectrum.toXQVector().begin();
             itF1 != itS1->end(); ++itF1, ++itF2, ++itX1, ++itX2)
        {
            *itF1 = *itF2 + ii * yShift;
            *itX1 = *itX2 + ii * xShift;
            if (*itX1 < lims[0])
                lims[0] = *itX1;
            if (*itX1 > lims[1])
                lims[1] = *itX1;
            if (*itF1 < lims[2])
                lims[2] = *itF1;
            if (*itF1 > lims[3])
                lims[3] = *itF1;
        }
    }
}

const double *LockablePlotSpectrum::getLims() const
{
    return lims;
}

void LockablePlotSpectrum::getLimsSafe(double lims_[])
{
    QMutexLocker locker(&mutexLims);
    for (int ii = 0; ii < 4; ++ii)
        lims_[ii] = lims[ii];
}

void LockablePlotSpectrum::setLims(const double lims_[])
{
    for (int ii = 0; ii < 4; ++ii)
        lims[ii] = lims_[ii];
}

void LockablePlotSpectrum::setLimsSafe(const double lims_[])
{
    QMutexLocker locker(&mutexLims);
    for (int ii = 0; ii < 4; ++ii)
        lims[ii] = lims_[ii];
}
