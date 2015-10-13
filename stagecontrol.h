#ifndef STAGECONTROL_H
#define STAGECONTROL_H

#define VIRTUALSTAGECONTROL

#include <QObject>

// forwar declarations
class QString;
class QMutex;
#ifdef VIRTUALSTAGECONTROL
class VirtualStageControl;

#else // VIRTUALSTAGECONTROL
namespace USMCVB_COM
{
class USMCVB_COM;
}
#endif // VIRTUALSTEGECONTROL

namespace StageControlTraits
{
enum struct ReferenceType {
    Absolute   = 0,
    LowerLimit = 1,
    UpperLimit = 2
};

enum struct InitType {
    None       = 0,
    LowerLimit = 1 << 0,
    UpperLimit = 1 << 1
};

enum struct LimType {
    LowerLimit = 0,
    UpperLimit = 1
};

enum struct PosType {
    Measurement = 0,
    Calibration = 1
};

inline constexpr InitType operator|(InitType a, InitType b) {
    return static_cast<InitType>(static_cast<unsigned char>(a) | static_cast<unsigned char>(b)); }

inline constexpr InitType operator&(InitType a, InitType b) {
    return static_cast<InitType>(static_cast<unsigned char>(a) & static_cast<unsigned char>(b)); }

inline InitType operator~(InitType a) { return static_cast<InitType>(~static_cast<unsigned char>(a)); }

inline InitType& operator|=(InitType& a, InitType b) { return a = (a | b); }

inline InitType& operator&=(InitType& a, InitType b) { return a = (a & b); }

struct Params
{
    int currPos;
    int upperLim;
    int lowerLim;
    ReferenceType currPosRefType;
    InitType initType;
    int range;
    int measPos;
    ReferenceType measRefType;
    InitType measInitType;
    int calPos;
    ReferenceType calRefType;
    InitType calInitType;
};

} // namespace StageControlTraits

class StageControl : public QObject
{
    Q_OBJECT

public:
    static const int graterThanLimit = 1000000;
    static const int defaultStageRange = 138000;

    StageControl(QObject *parent = 0);
    ~StageControl();

    bool motorRunning() const;
    bool connect(QString & strErrMsg);
    bool powerOn();
    bool powerOff();
    bool run(int dest, StageControlTraits::ReferenceType ref = StageControlTraits::ReferenceType::Absolute);
    bool stop();
    bool goToLowerLim();
    bool goToUpperLim();
    bool goToMeas();
    bool goToCal();

    // setters
    void setStageRange(int val);
    void setLowerLim() { setLowerLim(curPos()); }
    void setLowerLim(int val);
    void setUpperLim() { setUpperLim(curPos()); }
    void setUpperLim(int val);
    void setMeasPos(int pos, StageControlTraits::ReferenceType ref = StageControlTraits::ReferenceType::Absolute);
    void setCalPos(int pos, StageControlTraits::ReferenceType ref = StageControlTraits::ReferenceType::Absolute);
    void setInitialized();

    // getters
    bool initialized() const;
    bool connected() const;
    int stageRange() const;
    int lowerLim(StageControlTraits::ReferenceType ref = StageControlTraits::ReferenceType::Absolute) const;
    int upperLim(StageControlTraits::ReferenceType ref = StageControlTraits::ReferenceType::Absolute) const;
    int curPos(StageControlTraits::ReferenceType ref = StageControlTraits::ReferenceType::Absolute);

signals:
    void lowerLimChanged(int pos);
    void upperLimChanged(int lim);
    void currPosChanged(int lim);

private:

#ifndef VIRTUALSTAGECONTROL
    USMCVB_COM::USMCVB_COM * usmcvb;
#else // VIRTUALSTAGECONTROL
    VirtualStageControl * usmcvb;
#endif // VIRTUALSTAGECONTROL

    bool blConnected_;
    bool blInitialized_;
    int stageRange_;
    int iLowerLim_;
    int iUpperLim_;
    int iCurPos_;

    int iMeasPos_;
    StageControlTraits::ReferenceType measPosRef;
    int iCalPos_;
    StageControlTraits::ReferenceType calPosRef;

    QMutex * mutex_;
};

// usmcvb controller simulation
#ifdef VIRTUALSTAGECONTROL

#include <QObject>

// forward declarations
class QDateTime;
class QTimer;

class VirtualStageControl : public QObject
{
    Q_OBJECT

public:
    VirtualStageControl(QObject *parent = 0);
    virtual ~VirtualStageControl();
    int connect_2(QString & strErrMsg, bool &blConnected);
    int motorRunning(bool & blMotorRunning);
    int isPoweredOff(bool & blPoweredOff);
    int powerOn();
    int powerOff();
    int run(int dest);
    int stopMotion();
    int curPos(int & iCurPos);

signals:
    void stopTimer();

private slots:
    void movementFinished();
    void onStopTimer();

private:
    static const int speed_ = 10; // steps per millisecond
    static const int iLowerLim_ = 0;
    static const int iUpperLim_ = 150000;
    bool blConnected_;
    bool blPoweredOff_;
    bool blRunning_;
    int direction_;
    bool blStop_;

    int iStartPos_;
    int iDest_;
    QDateTime * startTime_;
    QTimer * timer_;
};

#endif // VIRTUALSTAGECONTROL

#endif // STAGECONTROL_H
