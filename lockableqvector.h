#ifndef LOCABLEQVECTOR_H
#define LOCABLEQVECTOR_H

#include <QVector>
#include <QMutex>

template<typename T>
struct LockableQVector
{
    QVector<T> vector;
    mutable QMutex mutex;
    QVector<T> & toQVector() { return vector; }
    const QVector<T> & toConstQVector() const { return vector; }
    QMutex * toMutex() const { return &mutex; }
    const QMutex * toConstMutex() const { return &mutex; }
};

template<typename T1, typename T2>
struct LockableSpectrumBase
{
    mutable QMutex mutexX;
    mutable QMutex mutexY;
    QVector<T1> x;
    QVector<T2> y;
    QMutex * toMutexX() const { return &mutexX; }
    const QMutex * toConstMutexX() const { return &mutexX; }
    QMutex * toMutexY() const { return &mutexY; }
    const QMutex * toConstMutexY() const { return &mutexY; }
    QVector<T1> & toXQVector() { return x; }
    const QVector<T1> & toConstXQVector() const { return x; }
    QVector<T2> & toYQVector() { return y; }
    const QVector<T2> & toConstYQVector() const { return y; }
};

struct LockableFrame : public LockableSpectrumBase<double, double>
{
    bool autoResizeX();
    bool autoResizeXSafe();
    void autoGenerateX();
    void autoGenerateXSafe();
};

struct LockableSpectrum : public LockableSpectrumBase<double, QVector<double> >
{
    bool autoResizeX();
    bool autoResizeXSafe();
    void autoGenerateX();
    void autoGenerateXSafe();
};

struct LockablePlotSpectrum : public LockableSpectrumBase<QVector<double>, QVector<double> >
{
    LockablePlotSpectrum();
    bool autoResizeX();
    bool autoResizeXSafe();
    void autoGenerateX();
    void autoGenerateXSafe();
    void shiftFromSpectrum(const LockableSpectrum &spectrum, double xShift, double yShift);
    void shiftFromSpectrumSafe(LockableSpectrum &spectrum, double xShift, double yShift);
    const double * getLims() const;
    void getLimsSafe(double lims_[]);
    void setLims(const double lims_[]);
    void setLimsSafe(const double lims_[]);
    QMutex * toMutexLims() const { return &mutexLims; }
    const QMutex * toConstMutexLims() const { return &mutexLims; }
    mutable QMutex mutexLims;
private:
    double lims[4];
};


typedef QVector<QVector<double> >::Iterator SpectrumIterator;
typedef QVector<QVector<double> >::ConstIterator ConstSpectrumIterator;
typedef QVector<double>::Iterator FrameIterator;
typedef QVector<double>::ConstIterator ConstFrameIterator;

typedef QVector<double>::Iterator XScaleIterator;
typedef QVector<double>::ConstIterator ConstXScaleIterator;
typedef QVector<QVector<double> >::Iterator XSpectrumIterator;
typedef QVector<QVector<double> >::ConstIterator ConstXSpectrumIterator;

#endif // LOCABLEQVECTOR_H
