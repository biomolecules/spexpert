#ifndef NESLABUSMAINWIDGET_H
#define NESLABUSMAINWIDGET_H

#include <QWidget>
#include <QDialog>

// forward declarations
class QPushButton;
class QLabel;
class QLineEdit;
class QCheckBox;
class QDoubleSpinBox;
class QSpinBox;
class QComboBox;
class QDialogButtonBox;
class QGroupBox;
class QStatusBar;
class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QTimer;
class QFile;
class QTextStream;
class QThread;
class Neslab;
namespace NeslabTraits
{
struct BathStatus;
struct PowerOnOffParm;
}

namespace NeslabusWidgets
{
class MainWidget;
namespace AutoReadT
{
struct Settings;
}
}

namespace NeslabusWidgets {

class MainDialogWindow : public QDialog
{
    Q_OBJECT
public:
    MainDialogWindow(Neslab *ns, QString *comPortName,
                     AutoReadT::Settings *autoReadTSettings,
                     QWidget *parent = 0);

signals:
    void autoTStarted();
    void autoTFinished();

public slots:
    void onStartAutoT();
    void onFinishAutoT();
    void onAutoTStarted();
    void onAutoTFinished();

protected:
    virtual void hideEvent(QHideEvent *event);
    virtual void showEvent(QShowEvent *event);

private:
    MainWidget *mainWidget;
    QDialogButtonBox *dialogButtonBox;
    QHBoxLayout *dialogButtonBoxHLayout;
    QStatusBar *statusBar;
    QVBoxLayout *mainVLayout;
};

class MainWidget : public QWidget
{
    Q_OBJECT
public:
    static const int initAutoReadRepetitionTime;
    static const bool initAutoSaveTemperatures;
    static const int initAutoSaveRepetitionTime;

    explicit MainWidget(QWidget *parent = 0);
    explicit MainWidget(Neslab *ns, QString *comPortName,
                        AutoReadT::Settings *autoReadTSettings,
                        QWidget *parent = 0);
    virtual ~MainWidget();

    void initMainWidget(Neslab *ns);

    QLabel *getCommandStatusLabel();

signals:
    void autoTStarted();
    void autoTFinished();

public slots:
    void onHidded();
    void onShown();
    void onStartAutoT();
    void onFinishAutoT();
    void onAdvancedSettingsCheckStateChanged();
    void onNoSerialPortAvailable();
    void onSendingCommand();
    void onGotResponse();

    void onConnectButtonClicked();
    void onTurnOnButtonClicked();
    void onSetSetpointButtonClicked();
    void onSetSetpointSpinBoxEditingFinished();
    void onAutoTemperatureCheckStateChanged();
    void onAutoReadTimerTimeout();
    void onAutoTSettingsButtonClicked();
    void onReadTemperatureButtonClicked();
    void onSetLowTemperatureLimitButtonClicked();
    void onSetLowTemperatureLimitSpinBoxEditingFinished();
    void onSetHighTemperatureLimitButtonClicked();
    void onSetHighTemperatureLimitSpinBoxEditingFinished();
    void onSetHeatProportionalBandButtonClicked();
    void onSetHeatProportionalBandSpinBoxEditingFinished();
    void onSetHeatIntegralButtonClicked();
    void onSetHeatIntegralSpinBoxEditingFinished();
    void onSetHeatDerivativeButtonClicked();
    void onSetHeatDerivativeSpinBoxEditingFinished();
    void onSetCoolProportionalBandButtonClicked();
    void onSetCoolProportionalBandSpinBoxEditingFinished();
    void onSetCoolIntegralButtonClicked();
    void onSetCoolIntegralSpinBoxEditingFinished();
    void onSetCoolDerivativeButtonClicked();
    void onSetCoolDerivativeSpinBoxEditingFinished();
    void onReadStatusButtonClicked();
    void onBathSettingsStateChanged();
    void onRefreshPortsButtonClicked();
    void onPortsComboBoxCurrentIndexChanged(const QString &portName);
    void onRefreshButtonClicked();
    void onDefaultValuesButtonClicked();

    void onConnected();
    void onDisconnected();
    void onNotConnected();
    void onPortOpeningFailed();
    void onConnectionFailed();
    void onReadAcknowledgeFinished(unsigned char v1, unsigned char v2);
    void onTurnedOnOff();
    void onReadSetpointFinished(double T, bool enhancedPrec);
    void onReadExternalSensorFinished(double T, bool enhancedPrec);
    void onReadInternalTemperatureFinished(double T, bool enhancedPrec);
    void onSaveTemperature(double T, bool enhancedPrec);
    void onReadLowTemperatureLimitFinished(double T, bool enhancedPrec);
    void onReadHighTemperatureLimitFinished(double T, bool enhancedPrec);
    void onReadHeatProportionalBandFinished(double P);
    void onReadHeatIntegralFinished(double I);
    void onReadHeatDerivativeFinished(double D);
    void onReadCoolProportionalBandFinished(double P);
    void onReadCoolIntegralFinished(double I);
    void onReadCoolDerivativeFinished(double D);
    void onReadStatusFinished(QString msg);
    void onBathSettingsUpdated();
    void onStatusUpdated();

private slots:
    void switchPowerConfirmed();

private:
    void startTemperatureMeasurement(const QString &directory, const QString &filename);

    QHBoxLayout *horizontalLayout;

    QCheckBox *advancedSettingsCheck;

    QLabel *versionLabel;
    QHBoxLayout *connectLayout;
    QPushButton *connectButton;
    QPushButton *connectIndicator;

    QHBoxLayout *powerLayout;
    QPushButton *turnOnButton;
    QPushButton *turnOnIndicator;

    QGroupBox *setpointGroupBox;
    QVBoxLayout *setpointLayout;
    QHBoxLayout *setSetpointLayout;
    QDoubleSpinBox *setSetpointSpinBox;
    QPushButton *setSetpointButton;
    QLabel *setpointLabel;

    QGroupBox *temperatureGroupBox;
    QHBoxLayout *autoTHLayout;
    QHBoxLayout *readTemperatureLayout;
    QVBoxLayout *temperatureLayout;
    QCheckBox *autoTemperatureCheck;
    QPushButton *autoTSettingsButton;
    QPushButton *readTemperatureButton;
    QLabel *temperatureLabel;

    QWidget *buttonsWidget;
    QVBoxLayout *buttonsLayout;

    QWidget *advancedSettingsWidget;

    QGroupBox *lowTemperatureLimitGroupBox;
    QGridLayout *lowTemperatureLimitGridLayout;
    QDoubleSpinBox *setLowTemperatureLimitSpinBox;
    QPushButton *setLowTemperatureLimitButton;
    QLabel *lowTemperatureLimitLabel;

    QGroupBox *highTemperatureLimitGroupBox;
    QGridLayout *highTemperatureLimitGridLayout;
    QDoubleSpinBox *setHighTemperatureLimitSpinBox;
    QPushButton *setHighTemperatureLimitButton;
    QLabel *highTemperatureLimitLabel;

    QGroupBox *heatProportionalBandGroupBox;
    QGridLayout *heatProportionalBandGridLayout;
    QDoubleSpinBox *setHeatProportionalBandSpinBox;
    QPushButton *setHeatProportionalBandButton;
    QLabel *heatProportionalBandLabel;

    QGroupBox *heatIntegralGroupBox;
    QGridLayout *heatIntegralGridLayout;
    QDoubleSpinBox *setHeatIntegralSpinBox;
    QPushButton *setHeatIntegralButton;
    QLabel *heatIntegralLabel;

    QGroupBox *heatDerivativeGroupBox;
    QGridLayout *heatDerivativeGridLayout;
    QDoubleSpinBox *setHeatDerivativeSpinBox;
    QPushButton *setHeatDerivativeButton;
    QLabel *heatDerivativeLabel;

    QGroupBox *coolProportionalBandGroupBox;
    QGridLayout *coolProportionalBandGridLayout;
    QDoubleSpinBox *setCoolProportionalBandSpinBox;
    QPushButton *setCoolProportionalBandButton;
    QLabel *coolProportionalBandLabel;

    QGroupBox *coolIntegralGroupBox;
    QGridLayout *coolIntegralGridLayout;
    QDoubleSpinBox *setCoolIntegralSpinBox;
    QPushButton *setCoolIntegralButton;
    QLabel *coolIntegralLabel;

    QGroupBox *coolDerivativeGroupBox;
    QGridLayout *coolDerivativeGridLayout;
    QDoubleSpinBox *setCoolDerivativeSpinBox;
    QPushButton *setCoolDerivativeButton;
    QLabel *coolDerivativeLabel;

    QPushButton *readStatusButton;
    QPushButton *refreshPortsButton;
    QHBoxLayout *readStatusRefreshPortsHLayout;
    QLabel *portsLabel;
    QComboBox *portsComboBox;
    QHBoxLayout *portsHLayout;

    QGroupBox *bathSettingsGroupBox;
    QVBoxLayout *bathSettingsLayout;
    QCheckBox *enableSensorCheck;
    QCheckBox *enableFaultsCheck;
    QCheckBox *muteCheck;
    QCheckBox *autoRestartCheck;
    QCheckBox *enhancedPrecisionCheck;
    QCheckBox *fullRangeCoolCheck;
    QCheckBox *serialCommEnableCheck;

    QPushButton *defaultValuesButton;
    QPushButton *refreshButton;
    QHBoxLayout *defaultValuesRefreshHLayout;

    QGridLayout *advancedSettingsGridLayout;
    QVBoxLayout *advancedSettingsVLayout1;
    QVBoxLayout *advancedSettingsVLayout2;
    QVBoxLayout *advancedSettingsVLayout3;
    QHBoxLayout *advancedSettingsHLayout;

    QLabel *commandStatusLabel;

    QThread * thread;

    Neslab *neslab;

    QString *comPortName_;

    double currT_;
    QTimer *autoReadTimer;
    double setpoint_;
    double setSetpoint_;
    double lowTLim_;
    double highTLim_;
    double heatP_;
    double heatI_;
    double heatD_;
    double coolP_;
    double coolI_;
    double coolD_;
    NeslabTraits::BathStatus *bathStatus_;
    NeslabTraits::PowerOnOffParm *powerOnOffParm_;

    bool turnedOn_;
    bool connected_;
    bool connecting_;
    bool *connectCommandsUpdate_;
    bool sensorEnabled_;
//    bool faultsEnabled_;
    bool muted_;
//    bool autoRestart_;
    bool enhancedPrecision_;
    bool fullRangeCool_;
//    bool serialCommEnabled_;

    bool refreshingPortsComboBoxContent;

    AutoReadT::Settings * autoReadTSettings_;
    bool extAutoReadTSettings;
    QFile * temperaturesFile;
    QTextStream * temperaturesTextStream;
    QDateTime * startMeasTime;
    int autosaveTcounter;
    bool autoReadingT;
    bool rewriteChecked;
    bool extAutoReadT;
    bool winIsVisible;
};

namespace AutoReadT
{

struct Settings {
    bool autoReadT;
    int repetitionTime;
    bool saveTemperatures;
    int saveRepetitionTime;
    QString directory;
    QString fileName;
};

class Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog(QWidget * parent = 0);
    explicit Dialog(const Settings &rv, QWidget * parent = 0);
    virtual ~Dialog();
    virtual void keyPressEvent(QKeyEvent *e);
    const Settings & getValues();

signals:

public slots:
    virtual void accept();
    void onFilenameEditEditingFinished();
    void onFilenameButtonClicked();
    void onSaveTemperaturesGroupBoxClicked();

private:
    QLabel *repetitionTimeLabel;
    QDoubleSpinBox *repetitionTimeSpinBox;
    QHBoxLayout *repetitionTimeHLayout;
    QGroupBox *saveTemperaturesGroupBox;
    QLabel *saveRepetitionTimeLabel1;
    QLabel *saveRepetitionTimeLabel2;
    QSpinBox *saveRepetitionTimeSpinBox;
    QHBoxLayout *saveRepetitionTimeHLayout;
    QLabel *filenameLabel;
    QLineEdit *filenameEdit;
    QPushButton *filenameButton;
    QVBoxLayout *filenameVLayout;
    QHBoxLayout *filenameHLayout;
    QVBoxLayout *saveTemperaturesVLayout;
    QDialogButtonBox *buttonBox;
    QVBoxLayout *mainVLayout;
    Settings returnValues;
    bool saveTemperaturesChanged;
};

}

}

#endif // NESLABUSMAINWIDGET_H
