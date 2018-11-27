#ifndef WINSPEC_H
#define WINSPEC_H

#include <memory>

#include <QVector>

#include <QDebug>

// forward declarations
class QMutex;
class LockableSpectrum;
class LockableFrame;

#if defined(SPEXPERT_MOCK_WINSPEC)
namespace biomolecules {
namespace spexpert {
namespace core {
class MockWinSpec;
}
}
}
#else
namespace WinSpecVB
{
    class WinSpecVB;
}
#endif
//template<typename T>
//class LockableQVector;



class WinSpec
{

public:
    WinSpec();
     WinSpec(const WinSpec&) = delete;
     WinSpec& operator=(const WinSpec&) = delete;

    bool start(double dblExpo_, int iAccums_, int iFrames_, const QString & strFileName_);
    bool startFocus(double dblExpo_, int iAccums_, const QString & strFileName_);
    void stop();
    bool running();
    int getAccum();
    int getFrame();
    bool getSpectrum(LockableSpectrum & spectrum_, int * iFrames_, int * iX_, int * iY_);
    bool getLastFrame(LockableFrame &lastFrame_, int * iFrame_, int * iX_, int * iY_);
    void getAcqParams(double * dblExposure_, int * iAccums_, int * iFrames_);
    void getAcqParams(double * dblExposure_, int * iAccums_);
    void setAcqParams(double dblExposure_, int iAccums_, int iFrames_);
    void setAcqParams(double dblExposure_, int iAccums_);
    void getFilePath(QString & strFileName_);
    void setFilePath(const QString & strFileName_);
    bool save();
    bool saveAs(const QString & strFileName_);
    bool activateWindow();
    void closeWindow();
    void closeAllWindows();
    bool quitWinSpec();
    bool showWinSpecWin();
    bool winSpecConnectionFailed();
    virtual ~WinSpec();

public slots:

private:
    QMutex *mutex;
#if defined(SPEXPERT_MOCK_WINSPEC)
    std::unique_ptr<biomolecules::spexpert::core::MockWinSpec> winSpec;
#else
    std::unique_ptr<WinSpecVB::WinSpecVB> winSpec;
#endif
};

#endif // WINSPEC_H
