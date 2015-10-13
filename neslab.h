#ifndef SERIALPORT_H
#define SERIALPORT_H

#define VIRTUALSERIALPORT

#include <QObject>

// forward declarations
class QTimer;
class QMutex;

#ifdef VIRTUALSERIALPORT
class VirtualSerialPort;
class QDateTime;
#include <random>
#else // VIRTUALSERIALPORT
class QSerialPort;
#endif // VIRTUALSERIALPORT

namespace NeslabTraits
{
enum class Command {
    SetOnOff,
    ReadStatus,
    ReadAcknowledge,
    SetLowTemperatureLimit,
    ReadLowTemperatureLimit,
    SetHighTemperatureLimit,
    ReadHighTemperatureLimit,
    SetHeatProportionalBand,
    ReadHeatProportionalBand,
    SetHeatIntegral,
    ReadHeatIntegral,
    SetHeatDerivative,
    ReadHeatDerivative,
    SetCoolProportionalBand,
    ReadCoolProportionalBand,
    SetCoolIntegral,
    ReadCoolIntegral,
    SetCoolDerivative,
    ReadCoolDerivative,
    ReadSetpoint,
    SetSetpoint,
    ReadExternalSensor,
    ReadInternalTemperature,
    None
};

inline Command& operator++(Command &cmd) { cmd = static_cast<Command>(static_cast<int>(cmd) + 1); return cmd; }
inline Command operator++(Command &cmd, int) { Command tmp(cmd); cmd++; return tmp; }

enum struct PowerOnOff
{
    d1_unitOnOff,
    d2_sensorEnabled,
    d3_faultsEnabled,
    d4_mute,
    d5_autoRestart,
    d6_01precDegCEnable,
    d7_fullRangeCoolEnable,
    d8_serialCommEnable
};

struct PowerOnOffParm
{
    PowerOnOffParm() { erase(); }
    unsigned char & operator[](PowerOnOff d) { return bytes_[static_cast<int>(d)]; }
    unsigned char & operator[](int d) { return bytes_[d]; }
    const unsigned char & operator[](PowerOnOff d) const { return bytes_[static_cast<int>(d)]; }
    const unsigned char & operator[](int d) const { return bytes_[d]; }
    void erase() { for (int ii = 0; ii < 8; ++ii) bytes_[ii] = 2; }
    const unsigned char * bytes() { return bytes_; }
private:
    unsigned char bytes_[8];
};

namespace BathStatusTraits
{
    enum struct d1 : unsigned char
    {
        None             = 0,
        RTD1OpenFault    = 1 << 7,
        RTD1ShortedFault = 1 << 6,
        RTD1Open         = 1 << 5,
        RTD1Shorted      = 1 << 4,
        RTD3OpenFault    = 1 << 3,
        RTD3ShortedFault = 1 << 2,
        RTD3Open         = 1 << 1,
        RTD3Shorted      = 1 << 0
    };

    enum struct d2 : unsigned char
    {
        None             = 0,
        RTD2OpenFault    = 1 << 7,
        RTD2ShortedFault = 1 << 6,
        RTD2OpenWarn     = 1 << 5,
        RTD2ShortedWarn  = 1 << 4,
        RTD2Open         = 1 << 3,
        RTD2Shorted      = 1 << 2,
        RefrigHighTemp   = 1 << 1,
        HTCFault         = 1 << 0
    };

    enum struct d3 : unsigned char
    {
        None               = 0,
        HighFixedTempFault = 1 << 7,
        LowFixedTempFault  = 1 << 6,
        HighTempFault      = 1 << 5,
        LowTempFault       = 1 << 4,
        LowLevelFault      = 1 << 3,
        HighTempWarn       = 1 << 2,
        LowTempWarn        = 1 << 1,
        LowLevelWarn       = 1 << 0
    };

    enum struct d4 : unsigned char
    {
        None         = 0,
        BuzzerOn     = 1 << 7,
        AlarmMuted   = 1 << 6,
        UnitFaulted  = 1 << 5,
        UnitStopping = 1 << 4,
        UnitOn       = 1 << 3,
        PumpOn       = 1 << 2,
        CompressorOn = 1 << 1,
        HeaterOn     = 1 << 0
    };

    enum struct d5 : unsigned char
    {
        None            = 0,
        RTD2Controlling = 1 << 7,
        HeatLEDFlashing = 1 << 6,
        HeatLEDOn       = 1 << 5,
        CoolLEDFlashing = 1 << 4,
        CoolLEDOn       = 1 << 3,
        Other1          = 1 << 2,
        Other2          = 1 << 1,
        Other3          = 1 << 0
    };

inline constexpr BathStatusTraits::d1 operator|(BathStatusTraits::d1 a, BathStatusTraits::d1 b) {
    return static_cast<BathStatusTraits::d1>(static_cast<unsigned char>(a) | static_cast<unsigned char>(b)); }
inline constexpr BathStatusTraits::d1 operator&(BathStatusTraits::d1 a, BathStatusTraits::d1 b) {
    return static_cast<BathStatusTraits::d1>(static_cast<unsigned char>(a) & static_cast<unsigned char>(b)); }
inline BathStatusTraits::d1 operator~(BathStatusTraits::d1 a) { return static_cast<BathStatusTraits::d1>(~static_cast<unsigned char>(a)); }
inline BathStatusTraits::d1& operator|=(BathStatusTraits::d1& a, BathStatusTraits::d1 b) { return a = (a | b); }
inline BathStatusTraits::d1& operator&=(BathStatusTraits::d1& a, BathStatusTraits::d1 b) { return a = (a & b); }

inline constexpr BathStatusTraits::d2 operator|(BathStatusTraits::d2 a, BathStatusTraits::d2 b) {
    return static_cast<BathStatusTraits::d2>(static_cast<unsigned char>(a) | static_cast<unsigned char>(b)); }
inline constexpr BathStatusTraits::d2 operator&(BathStatusTraits::d2 a, BathStatusTraits::d2 b) {
    return static_cast<BathStatusTraits::d2>(static_cast<unsigned char>(a) & static_cast<unsigned char>(b)); }
inline BathStatusTraits::d2 operator~(BathStatusTraits::d2 a) { return static_cast<BathStatusTraits::d2>(~static_cast<unsigned char>(a)); }
inline BathStatusTraits::d2& operator|=(BathStatusTraits::d2& a, BathStatusTraits::d2 b) { return a = (a | b); }
inline BathStatusTraits::d2& operator&=(BathStatusTraits::d2& a, BathStatusTraits::d2 b) { return a = (a & b); }

inline constexpr BathStatusTraits::d3 operator|(BathStatusTraits::d3 a, BathStatusTraits::d3 b) {
    return static_cast<BathStatusTraits::d3>(static_cast<unsigned char>(a) | static_cast<unsigned char>(b)); }
inline constexpr BathStatusTraits::d3 operator&(BathStatusTraits::d3 a, BathStatusTraits::d3 b) {
    return static_cast<BathStatusTraits::d3>(static_cast<unsigned char>(a) & static_cast<unsigned char>(b)); }
inline BathStatusTraits::d3 operator~(BathStatusTraits::d3 a) { return static_cast<BathStatusTraits::d3>(~static_cast<unsigned char>(a)); }
inline BathStatusTraits::d3& operator|=(BathStatusTraits::d3& a, BathStatusTraits::d3 b) { return a = (a | b); }
inline BathStatusTraits::d3& operator&=(BathStatusTraits::d3& a, BathStatusTraits::d3 b) { return a = (a & b); }

inline constexpr BathStatusTraits::d4 operator|(BathStatusTraits::d4 a, BathStatusTraits::d4 b) {
    return static_cast<BathStatusTraits::d4>(static_cast<unsigned char>(a) | static_cast<unsigned char>(b)); }
inline constexpr BathStatusTraits::d4 operator&(BathStatusTraits::d4 a, BathStatusTraits::d4 b) {
    return static_cast<BathStatusTraits::d4>(static_cast<unsigned char>(a) & static_cast<unsigned char>(b)); }
inline BathStatusTraits::d4 operator~(BathStatusTraits::d4 a) { return static_cast<BathStatusTraits::d4>(~static_cast<unsigned char>(a)); }
inline BathStatusTraits::d4& operator|=(BathStatusTraits::d4& a, BathStatusTraits::d4 b) { return a = (a | b); }
inline BathStatusTraits::d4& operator&=(BathStatusTraits::d4& a, BathStatusTraits::d4 b) { return a = (a & b); }

inline constexpr BathStatusTraits::d5 operator|(BathStatusTraits::d5 a, BathStatusTraits::d5 b) {
    return static_cast<BathStatusTraits::d5>(static_cast<unsigned char>(a) | static_cast<unsigned char>(b)); }
inline constexpr BathStatusTraits::d5 operator&(BathStatusTraits::d5 a, BathStatusTraits::d5 b) {
    return static_cast<BathStatusTraits::d5>(static_cast<unsigned char>(a) & static_cast<unsigned char>(b)); }
inline BathStatusTraits::d5 operator~(BathStatusTraits::d5 a) { return static_cast<BathStatusTraits::d5>(~static_cast<unsigned char>(a)); }
inline BathStatusTraits::d5& operator|=(BathStatusTraits::d5& a, BathStatusTraits::d5 b) { return a = (a | b); }
inline BathStatusTraits::d5& operator&=(BathStatusTraits::d5& a, BathStatusTraits::d5 b) { return a = (a & b); }
}

struct BathStatus
{
    BathStatus()
    {
        d1 = BathStatusTraits::d1::None;
        d2 = BathStatusTraits::d2::None;
        d3 = BathStatusTraits::d3::None;
        d4 = BathStatusTraits::d4::None;
        d5 = BathStatusTraits::d5::None;
    }
    BathStatusTraits::d1 d1;
    BathStatusTraits::d2 d2;
    BathStatusTraits::d3 d3;
    BathStatusTraits::d4 d4;
    BathStatusTraits::d5 d5;
};

struct ComPortParams
{
    QString portName;
    QString description;
    QString manufacturer;
};

}

class Neslab : public QObject
{
    Q_OBJECT
#ifdef VIRTUALSERIALPORT
    friend class VirtualSerialPort;
#endif // VIRTUALSERIALPORT

public:
    explicit Neslab(QObject *parent = 0);
    virtual ~Neslab();

    const QString & comPortName();
    void setComPortName(const QString &name);
    static QList<NeslabTraits::ComPortParams> availablePorts();
    bool isConnected() const;
    bool isTurnedOn(bool *state) const;
    bool getBathStatus(NeslabTraits::BathStatus *status) const;
    NeslabTraits::PowerOnOffParm getOnOffParms() const;
    bool isExternalSensorUsed(bool *state) const;

signals:
    void sendingCommand();
    void gotResponse();
    void connected();
    void portOpeningFailed();
    void disconnected();
    void connectionFailed();
    void setOnOffFinished();
    void readStatusFinished(QString msg);
    void statusUpdated();
    void turnedOn();
    void turnedOff();
    void readAcknowledgeFinished(unsigned char v1, unsigned char v2);
    void setLowTemperetareLimitFinished();
    void readLowTemperatureLimitFinished(double T, bool enhancedPrec);
    void setHighTemperatureLimitFinished();
    void readHighTemperatureLimitFinished(double T, bool enhancedPrec);
    void setHeatProportionalBandFinished();
    void readHeatProportionalBandFinished(double P, bool enhancedPrec);
    void setHeatIntegralFinished();
    void readHeatIntegralFinished(double I, bool enhancedPrec);
    void setHeatDerivativeFinished();
    void readHeatDerivativeFinished(double D, bool enhancedPrec);
    void setCoolProportionalBandFinished();
    void readCoolProportionalBandFinished(double P, bool enhancedPrec);
    void setCoolIntegralFinished();
    void readCoolIntegralFinished(double I, bool enhancedPrec);
    void setCoolDerivativeFinished();
    void readCoolDerivativeFinished(double D, bool enhancedPrec);
    void readSetpointFinished(double T, bool enhancedPrec);
    void setSetpointFinished();
    void readExternalSensorFinished(double T, bool enhancedPrec);
    void readInternalTemperatureFinished(double T, bool enhancedPrec);
    void connectNeslabInThread();
    void sendToSerial(const unsigned char *buffer, int n);
    void startCmdTimer();

public slots:
    void restoreDefaultSettingsCommand();
    void connectNeslab();
    void setOnOffCommand(const NeslabTraits::PowerOnOffParm &parm);
    void turnOn();
    void turnOff();
    void isTurnedOn();
    void readStatusCommand();
    void updateStatusCommand();
    void readAcknowledgeCommand();
    void setLowTemperatureLimitCommand(double T);
    void readLowTemperatureLimitCommand();
    void setHighTemperatureLimitCommand(double T);
    void readHighTemperatureLimitCommand();
    void setHeatProportionalBandCommand(double P);
    void readHeatProportionalBandCommand();
    void setHeatIntegralCommand(double I);
    void readHeatIntegralCommand();
    void setHeatDerivativeCommand(double D);
    void readHeatDerivativeCommand();
    void setCoolProportionalBandCommand(double P);
    void readCoolProportionalBandCommand();
    void setCoolIntegralCommand(double I);
    void readCoolIntegralCommand();
    void setCoolDerivativeCommand(double D);
    void readCoolDerivativeCommand();
    void readSetpointCommand();
    void setSetpointCommand(double T);
    void readTemperatureCommand();

private slots:
    void onReadyData();
    void onWaitingFinished();
    void onNoResponse();
    void onStartCmdTimer();
    void onConnectNeslabInThread();
    void onSendToSerial(const unsigned char *buffer, int n);

private:

    bool statusUpdatedAfterOnOffCmd() const;
    void sendCommand();

    void sendSetOnOffCommand();
    void sendReadStatusCommand();
    void sendReadAcknowledgeCommand();
    void sendSetLowTemperatureLimitCommand();
    void sendReadLowTemperatureLimitCommand();
    void sendSetHighTemperatureLimitCommand();
    void sendReadHighTemperatureLimitCommand();
    void sendSetHeatProportionalBandCommand();
    void sendReadHeatProportionalBandCommand();
    void sendSetHeatIntegralCommand();
    void sendReadHeatIntegralCommand();
    void sendSetHeatDerivativeCommand();
    void sendReadHeatDerivativeCommand();
    void sendSetCoolProportionalBandCommand();
    void sendReadCoolProportionalBandCommand();
    void sendSetCoolIntegralCommand();
    void sendReadCoolIntegralCommand();
    void sendSetCoolDerivativeCommand();
    void sendReadCoolDerivativeCommand();
    void sendReadSetpointCommand();
    void sendSetSetpointCommand();
    void sendReadExternalSensorCommand();
    void sendReadInternalTemperatureCommand();

    void strSendToSerial(const QString &cmd);

    bool onSetOnOff(const unsigned char *buffer, int n);
    bool onReadStatus(const unsigned char *buffer, int n);
    bool onReadAcknowledge(const unsigned char *buffer, int n);
    bool onSetLowTemperatureLimit(const unsigned char *buffer, int n);
    bool onReadLowTemperatureLimit(const unsigned char *buffer, int n);
    bool onSetHighTemperatureLimit(const unsigned char *buffer, int n);
    bool onReadHighTemperatureLimit(const unsigned char *buffer, int n);
    bool onSetHeatProportionalBand(const unsigned char *buffer, int n);
    bool onReadHeatProportionalBand(const unsigned char *buffer, int n);
    bool onSetHeatIntegral(const unsigned char *buffer, int n);
    bool onReadHeatIntegral(const unsigned char *buffer, int n);
    bool onSetHeatDerivative(const unsigned char *buffer, int n);
    bool onReadHeatDerivative(const unsigned char *buffer, int n);
    bool onSetCoolProportionalBand(const unsigned char *buffer, int n);
    bool onReadCoolProportionalBand(const unsigned char *buffer, int n);
    bool onSetCoolIntegral(const unsigned char *buffer, int n);
    bool onReadCoolIntegral(const unsigned char *buffer, int n);
    bool onSetCoolDerivative(const unsigned char *buffer, int n);
    bool onReadCoolDerivative(const unsigned char *buffer, int n);
    bool onReadSetpoint(const unsigned char *buffer, int n);
    bool onSetSetpoint(const unsigned char *buffer, int n);
    bool onReadExternalSensor(const unsigned char *buffer, int n);
    bool onReadInternalTemperature(const unsigned char *buffer, int n);

    static void hexToByte(unsigned char **pbuffer, int *n, const QString &msg);
    static QString byteToHex(const unsigned char *buffer, int n);
    static QString checkSum(const QString &msg);
    static unsigned char checkSum(const unsigned char *bMsg, int n);
    static bool validateResponse(const QString &msg, NeslabTraits::Command cmd);
    static bool validateResponse(const unsigned char *bMsg, int n, NeslabTraits::Command cmd);

    static void fillCommandsArrays();

    static const QString defaultComPortName;
    QString comPortName_;

    bool commandBuffer[static_cast<int>(NeslabTraits::Command::None)];

    NeslabTraits::Command lastCommand;

    static unsigned char bCommands[static_cast<int>(NeslabTraits::Command::None)][4];
    static QString strCommands[static_cast<int>(NeslabTraits::Command::None)];

    static const QString commandBase;

    static const QString rExtSensorCmd;
    static const QString wSetPointCmd;
    static const QString rSetPointCmd;

    static const QString rStatusCmd;
    static const QString rAcknowledgeCmd;
    static const QString rInternalTCmd;
    static const QString rLowTLimCmd;
    static const QString rHighTLimCmd;
    static const QString rHeatProportionalBandCmd;
    static const QString rHeatIntegraCmd;
    static const QString rHeatDerivativeCmd;
    static const QString rCoolProportionalBandCmd;
    static const QString rCoolIntegraCmd;
    static const QString rCoolDerivativeCmd;

    static const QString wLowTLimCmd;
    static const QString wHigTLimCmd;
    static const QString wHeatProportionalBandCmd;
    static const QString wHeatIntegraCmd;
    static const QString wHeatDerivativeCmd;
    static const QString wCoolProportionalBandCmd;
    static const QString wCoolIntegraCmd;
    static const QString wCoolDerivativeCmd;

    static const QString wOnOffCmd;

    static const QString badCommandCmd;
    static const QString badCheckSumCmd;

    double wSetpointT_;
    double wLowTLim_;
    double wHighTLim_;
    double wHeatP_;
    double wHeatI_;
    double wHeatD_;
    double wCoolP_;
    double wCoolI_;
    double wCoolD_;
    NeslabTraits::PowerOnOffParm wOnOffParm_;
    NeslabTraits::PowerOnOffParm rOnOffParm_;
    NeslabTraits::BathStatus bathStatus_;
    bool enhancedBathPrecision_;

    bool connected_;
    bool connecting_;
    bool switchingOn_;
    bool switchingOff_;
    bool readStatusSilently_;
    bool statusUpdatedAfterOnOffCmd_;

    static const int waitForResponse;
    static const int cmdDelay;
    static const int initTrialCount;
    static const int maxNumber;
    static const int negativeNumbersLimit;
    static const double defaultHeatP;
    static const double defaultHeatI;
    static const double defaultHeatD;
    static const double defaultCoolP;
    static const double defaultCoolI;
    static const double defaultCoolD;
    QTimer *waitingTimer;
    bool waitingForNextCommand_;

    int trialCount;
    QTimer *cmdTimer;

    class MutexManager
    {
    public:
        MutexManager();
        ~MutexManager();
        QMutex * getMutex() const { return mutex; }

    private:
        QMutex *mutex;
    };

    static MutexManager serialPortInfoMutexManager;

    QMutex * mutex;

#ifdef VIRTUALSERIALPORT
    VirtualSerialPort *serialPort;
#else // VIRTUALSERIALPORT
    QSerialPort *serialPort;
#endif // VIRTUALSERIALPORT
};

#ifdef VIRTUALSERIALPORT

class VirtualSerialPort : public QObject
{
    Q_OBJECT

public:
    enum OpenModeFlag {
        NotOpen = 0x0000,
        ReadOnly = 0x0001,
        WriteOnly = 0x0002,
        ReadWrite = ReadOnly | WriteOnly,
        Append = 0x0004,
        Truncate = 0x0008,
        Text = 0x0010,
        Unbuffered = 0x0020
    };
    Q_DECLARE_FLAGS(OpenMode, OpenModeFlag)

    enum Direction  {
        Input = 1,
        Output = 2,
        AllDirections = Input | Output
    };
    Q_DECLARE_FLAGS(Directions, Direction)

    enum BaudRate {
        Baud1200 = 1200,
        Baud2400 = 2400,
        Baud4800 = 4800,
        Baud9600 = 9600,
        Baud19200 = 19200,
        Baud38400 = 38400,
        Baud57600 = 57600,
        Baud115200 = 115200,
        UnknownBaud = -1
    };

    enum DataBits {
        Data5 = 5,
        Data6 = 6,
        Data7 = 7,
        Data8 = 8,
        UnknownDataBits = -1
    };

    enum Parity {
        NoParity = 0,
        EvenParity = 2,
        OddParity = 3,
        SpaceParity = 4,
        MarkParity = 5,
        UnknownParity = -1
    };

    enum StopBits {
        OneStop = 1,
        OneAndHalfStop = 3,
        TwoStop = 2,
        UnknownStopBits = -1
    };

    enum FlowControl {
        NoFlowControl,
        HardwareControl,
        SoftwareControl,
        UnknownFlowControl = -1
    };

    enum PinoutSignal {
        NoSignal = 0x00,
        TransmittedDataSignal = 0x01,
        ReceivedDataSignal = 0x02,
        DataTerminalReadySignal = 0x04,
        DataCarrierDetectSignal = 0x08,
        DataSetReadySignal = 0x10,
        RingIndicatorSignal = 0x20,
        RequestToSendSignal = 0x40,
        ClearToSendSignal = 0x80,
        SecondaryTransmittedDataSignal = 0x100,
        SecondaryReceivedDataSignal = 0x200
    };
    Q_DECLARE_FLAGS(PinoutSignals, PinoutSignal)

    explicit VirtualSerialPort(QObject *parent);
    virtual ~VirtualSerialPort();
    void setPortName(const QString &comPortName);
    void setBaudRate(qint32 baudRate, Directions directions = AllDirections); //QSerialPort::Baud9600);
    void setDataBits(DataBits dataBits); //QSerialPort::Data8);
    void setParity(Parity parity); //QSerialPort::NoParity);
    void setStopBits(StopBits stopBits); //QSerialPort::OneStop);
    void setFlowControl(FlowControl flowControl); //QSerialPort::NoFlowControl);

    bool isOpen() const { return opened_; }
    bool open(OpenMode mode) { opened_ = true; openMode_ = mode; return true; }
    void close();

    QByteArray readAll();
    qint64 write(char *data, qint64 len);

signals:
    void readyRead();

public slots:

private slots:
    void onCmdWaitFinished();

private:
    bool onPowerOnOff(const unsigned char *buffer, int n);
    bool onReadStatus(const unsigned char *buffer);

    bool onReadAcknowledge(const unsigned char *buffer);
    bool onSetLowTemperatureLimit(const unsigned char *buffer, int n);
    bool onReadLowTemperatureLimit(const unsigned char *buffer);
    bool onSetHighTemperatureLimit(const unsigned char *buffer, int n);
    bool onReadHighTemperatureLimit(const unsigned char *buffer);
    bool onSetHeatProportionalBand(const unsigned char *buffer, int n);
    bool onReadHeatProportionalBand(const unsigned char *buffer);
    bool onSetHeatIntegral(const unsigned char *buffer, int n);
    bool onReadHeatIntegral(const unsigned char *buffer);
    bool onSetHeatDerivative(const unsigned char *buffer, int n);
    bool onReadHeatDerivative(const unsigned char *buffer);
    bool onSetCoolProportionalBand(const unsigned char *buffer, int n);
    bool onReadCoolProportionalBand(const unsigned char *buffer);
    bool onSetCoolIntegral(const unsigned char *buffer, int n);
    bool onReadCoolIntegral(const unsigned char *buffer);
    bool onSetCoolDerivative(const unsigned char *buffer, int n);
    bool onReadCoolDerivative(const unsigned char *buffer);

    bool onReadSetpoint(const unsigned char *buffer);
    bool onSetSetpoint(const unsigned char *buffer, int n);
    bool onReadExternalSensor(const unsigned char *buffer);
    bool onReadInternalTemperature(const unsigned char *buffer);

    void onBadCheckSum(const unsigned char *buffer, int n);
    void onBadCommand(const unsigned char *buffer, int n);


    static bool validateResponse(const QString &msg);
    static bool validateResponse(const unsigned char *bMsg, int n);

    void fillCommandsArrays();

    static unsigned char bCommands[static_cast<int>(NeslabTraits::Command::None)][4];
    static QString strCommands[static_cast<int>(NeslabTraits::Command::None)];
    static const QString commandBase;
    static const double speed_;

    static const int cmdWait;
    bool cmdWaiting_;
    QTimer * cmdTimer_;
    unsigned char * cmd_;
    int n_;
    QDateTime * setTCmdTime_;
    int currExtT_;
    int currIntT_;
    int setTCmdT_;
    int destT_;
    int direction_;
    int lowTLim_;
    int highTLim_;
    int heatP_; // 0.1  - 99.9
    int heatI_; // 0.00 -  9.99
    int heatD_; // 0.0  -  5.0
    int coolP_; // 0.1  - 99.9
    int coolI_; // 0.00 -  9.99
    int coolD_; // 0.0  -  5.0
    NeslabTraits::BathStatus bathStatus;
    NeslabTraits::PowerOnOffParm powerOnOffParm_;
    bool faultsEnabled_;
    bool autorestart_;
    bool enhancedBathPrecision_;
    bool serialCommEnabled_;

    bool opened_;
    OpenMode openMode_;
    QString portName_;
    qint32 baudRate_;
    Directions baudRateDirections_;
    DataBits dataBits_;
    Parity parity_;
    StopBits stopBits_;
    FlowControl flowControl_;

    static std::default_random_engine engine;
};
#endif // VIRTUALSERIALPORT

#endif // SERIALPORT_H
