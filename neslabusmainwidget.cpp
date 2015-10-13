#include "neslabusmainwidget.h"
#include "neslab.h"

#include <QStringBuilder>

#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QGroupBox>
#include <QLayout>
#include <QMessageBox>
#include <QStatusBar>

#include <QTimer>
#include <QKeyEvent>

#include <QDir>
#include <QFileInfo>
#include <QFileDialog>
#include <QTextStream>
#include <QDateTime>

#include <QSettings>
#include <QCoreApplication>

#include <QThread>

#include <QDebug>

using namespace NeslabusWidgets;

MainDialogWindow::MainDialogWindow(Neslab *ns, QString *comPortName, AutoReadT::Settings *autoReadTSettings, QWidget *parent) :
    QDialog(parent)
{
    qDebug() << "MainDialogWindow::MainDialogWindow()";
//    setWindowFlags(Qt::Tool);
    setWindowFlags(Qt::WindowTitleHint | Qt::WindowSystemMenuHint);
//    Qt::WindowFlags winFlags = windowFlags();
//    setWindowFlags(winFlags & (!Qt::WindowContextHelpButtonHint));
    setWindowTitle("Neslab settings");
    mainWidget = new MainWidget(ns, comPortName, autoReadTSettings, this);
//    dialogButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    dialogButtonBox = new QDialogButtonBox(QDialogButtonBox::Close);
    statusBar = new QStatusBar;
    statusBar->addWidget(mainWidget->getCommandStatusLabel());
    statusBar->setStyleSheet("QStatusBar::item {border: none;}");

//    connect(buttonBox, &QDialogButtonBox::accepted, this, &NeslabusWidgets::MainDialogWindow::accept);
    connect(dialogButtonBox, &QDialogButtonBox::rejected, this, &NeslabusWidgets::MainDialogWindow::reject);
//    connect(dialogButtonBox, &QDialogButtonBox::rejected, this, &NeslabusWidgets::MainDialogWindow::close);

    mainVLayout = new QVBoxLayout;
    mainVLayout->setContentsMargins(0, 0, 0, 0);
    mainVLayout->addWidget(mainWidget);
    dialogButtonBoxHLayout = new QHBoxLayout;
    dialogButtonBoxHLayout->addWidget(dialogButtonBox);
    dialogButtonBoxHLayout->addSpacing(18);
    mainVLayout->addLayout(dialogButtonBoxHLayout);
    mainVLayout->addWidget(statusBar);

    setLayout(mainVLayout);

    layout()->setSizeConstraint(QLayout::SetFixedSize);
}

void MainDialogWindow::onStartAutoT()
{
    mainWidget->onStartAutoT();
}

void MainDialogWindow::onFinishAutoT()
{
    mainWidget->onFinishAutoT();
}

void MainDialogWindow::onAutoTStarted()
{
    emit autoTStarted();
}

void MainDialogWindow::onAutoTFinished()
{
    emit autoTFinished();
}

void MainDialogWindow::hideEvent(QHideEvent *event)
{
    mainWidget->onHidded();
    QDialog::hideEvent(event);
}

void MainDialogWindow::showEvent(QShowEvent *event)
{
    mainWidget->onShown();
    QDialog::showEvent(event);
}

const int MainWidget::initAutoReadRepetitionTime = 1000;
const bool MainWidget::initAutoSaveTemperatures = false;
const int MainWidget::initAutoSaveRepetitionTime = 10;

MainWidget::MainWidget(QWidget *parent) :
    QWidget(parent)
{
    thread = new QThread(this);

    Neslab *ns = new Neslab;
    ns->moveToThread(thread);
    connect(thread, &QThread::finished, ns, &Neslab::deleteLater);
    thread->start();

    extAutoReadTSettings = false;

    QSettings settings(QCoreApplication::applicationDirPath() + "/" + QCoreApplication::applicationName() + ".ini",QSettings::IniFormat);
    settings.beginGroup("Neslabus");
    comPortName_ = new QString(settings.value("COMPort").toString());
    bool autoReadT = settings.value("autoReadT").toBool();
    bool ok;
    int autoReadRepetitionTime = settings.value("autoReadRepetitionTime").toInt(&ok);
    if (!ok && autoReadRepetitionTime < 1000) {
        autoReadRepetitionTime = initAutoReadRepetitionTime;
    }
    int autoSaveRepetitionTime = settings.value("autoSaveRepetitionTime").toInt(&ok);
    if (!ok && autoSaveRepetitionTime < 1000) {
        autoSaveRepetitionTime = initAutoSaveRepetitionTime;
    }
    QString autoSaveDirectory = settings.value("autoSaveDirectory").toString();
    QString autoSaveFileName = settings.value("autoSaveFileName").toString();
    settings.endGroup();
    autoReadTSettings_ = new AutoReadT::Settings{autoReadT, autoReadRepetitionTime, initAutoSaveTemperatures,
            autoSaveRepetitionTime, autoSaveDirectory, autoSaveFileName};

    initMainWidget(ns);
}

MainWidget::MainWidget(Neslab *ns, QString *comPortName, AutoReadT::Settings *autoReadTSettings, QWidget *parent) :
    QWidget(parent)
{
    thread = nullptr;

    extAutoReadTSettings = true;
    comPortName_ = comPortName;
    autoReadTSettings_ = autoReadTSettings;

    initMainWidget(ns);
}

void MainWidget::initMainWidget(Neslab *ns)
{
    qDebug() << "NeslabusWidgets::initMainWidget()";
    winIsVisible = false;
    extAutoReadT = false;
    neslab = ns;
    currT_ = 0.0;
    autoReadTimer = new QTimer(this);
    setpoint_ = 0.0;
    setSetpoint_ = 0.0;
    lowTLim_ = -5.0;
    highTLim_ = 99.0;
    heatP_ = 0.6;
    heatI_ = 0.6;
    heatD_ = 0.0;
    coolP_ = 0.6;
    coolI_ = 0.6;
    coolD_ = 0.0;
    bathStatus_ = new NeslabTraits::BathStatus;
    powerOnOffParm_ = new NeslabTraits::PowerOnOffParm;

    connected_ = neslab->isConnected();
    connecting_ = false;
    connectCommandsUpdate_ = new bool[static_cast<int>(NeslabTraits::Command::None)];
    for (int ii = 0; ii < static_cast<int>(NeslabTraits::Command::None); ++ii)
        connectCommandsUpdate_[ii] = false;
    sensorEnabled_ = false;
//    faultsEnabled_ = false;
    muted_ = false;
//    autoRestart_ = false;
    enhancedPrecision_ = false;
    fullRangeCool_ = false;
//    serialCommEnabled_ = false;

    refreshingPortsComboBoxContent = false;

    temperaturesFile = nullptr;
    temperaturesTextStream = nullptr;
    startMeasTime = new QDateTime;
    autosaveTcounter = 0;
    autoReadingT = false;
    rewriteChecked = false;

    advancedSettingsCheck = new QCheckBox(tr("Advanced settings"));
    advancedSettingsCheck->setChecked(false);

    versionLabel = new QLabel(tr("Version: "));

    connectButton = new QPushButton(tr("Connect"));
    connectIndicator = new QPushButton;
    QSize indicatorSize(10, 10);
    connectIndicator->setFixedSize(indicatorSize);
    connectIndicator->setEnabled(false);
    if (connected_) {
        connectIndicator->setStyleSheet("background-color: green");
    } else {
        connectIndicator->setStyleSheet("background-color: red");
    }
    turnedOn_ = false;
    turnOnButton = new QPushButton(tr("On/Off"));
    turnOnIndicator = new QPushButton;
    turnOnIndicator->setFixedSize(indicatorSize);
    turnOnIndicator->setEnabled(false);
    turnOnIndicator->setStyleSheet("background-color: red");

    setpointGroupBox = new QGroupBox(tr("Setpoint:"));
    setSetpointSpinBox = new QDoubleSpinBox;
    setSetpointSpinBox->setMinimum(-10.0);
    setSetpointSpinBox->setMaximum(100.0);
    setSetpointSpinBox->setSingleStep(1.0);
    setSetpointSpinBox->setDecimals(2);
    setSetpointSpinBox->setSuffix(tr("°C"));
    setSetpointButton = new QPushButton(tr("Set"));
    setpointLabel = new QLabel(tr("Setpoint: %L1 °C").arg(setpoint_, 0, 'f', 2));

    temperatureGroupBox = new QGroupBox(tr("Temperature:"));
    autoTemperatureCheck = new QCheckBox(tr("Auto read"));
    autoTemperatureCheck->setChecked(autoReadTSettings_->autoReadT);
    autoTSettingsButton = new QPushButton(tr("Settings"));
    readTemperatureButton = new QPushButton(tr("Read"));
    temperatureLabel = new QLabel(tr("%L1 °C").arg(currT_, 0, 'f', 2));

    lowTemperatureLimitGroupBox = new QGroupBox(tr("Low Temperature Limit:"));
    setLowTemperatureLimitSpinBox = new QDoubleSpinBox;
    setLowTemperatureLimitSpinBox->setMinimum(-10.0);
    setLowTemperatureLimitSpinBox->setMaximum(100.0);
    setLowTemperatureLimitSpinBox->setSingleStep(1.0);
    setLowTemperatureLimitSpinBox->setDecimals(2);
    setLowTemperatureLimitSpinBox->setSuffix(tr("°C"));
    setLowTemperatureLimitButton = new QPushButton(tr("Set"));
    lowTemperatureLimitLabel = new QLabel(tr("Low Temperature Limit: %L1 °C").arg(0.0, 0, 'f', 2));

    highTemperatureLimitGroupBox = new QGroupBox(tr("High Temperature Limit:"));
    setHighTemperatureLimitSpinBox = new QDoubleSpinBox;
    setHighTemperatureLimitSpinBox->setMinimum(-10.0);
    setHighTemperatureLimitSpinBox->setMaximum(100.0);
    setHighTemperatureLimitSpinBox->setSingleStep(1.0);
    setHighTemperatureLimitSpinBox->setDecimals(2);
    setHighTemperatureLimitSpinBox->setSuffix(tr("°C"));
    setHighTemperatureLimitButton = new QPushButton(tr("Set"));
    highTemperatureLimitLabel = new QLabel(tr("High Temperature Limit: %L1 °C").arg(0.0, 0, 'f', 2));

    heatProportionalBandGroupBox = new QGroupBox(tr("Heat Proportional Band (0-99.9):"));
    setHeatProportionalBandSpinBox = new QDoubleSpinBox;
    setHeatProportionalBandSpinBox->setMinimum(0.1);
    setHeatProportionalBandSpinBox->setMaximum(99.9);
    setHeatProportionalBandSpinBox->setSingleStep(1.0);
    setHeatProportionalBandSpinBox->setDecimals(1);
    setHeatProportionalBandButton = new QPushButton(tr("Set"));
    heatProportionalBandLabel = new QLabel(tr("Heat Proportional Band: %L1").arg(0.0, 0, 'f', 1));;

    heatIntegralGroupBox = new QGroupBox(tr("Heat Integral (0-9.99):"));
    setHeatIntegralSpinBox = new QDoubleSpinBox;
    setHeatIntegralSpinBox->setMinimum(0.0);
    setHeatIntegralSpinBox->setMaximum(9.99);
    setHeatIntegralSpinBox->setSingleStep(0.1);
    setHeatIntegralSpinBox->setDecimals(2);
    setHeatIntegralButton = new QPushButton(tr("Set"));
    heatIntegralLabel = new QLabel(tr("Heat Integral: %L1").arg(0.0, 0, 'f', 2));;

    heatDerivativeGroupBox = new QGroupBox(tr("Heat Derivative (0-5.0):"));
    setHeatDerivativeSpinBox = new QDoubleSpinBox;
    setHeatDerivativeSpinBox->setMinimum(0.0);
    setHeatDerivativeSpinBox->setMaximum(5.00);
    setHeatDerivativeSpinBox->setSingleStep(0.1);
    setHeatDerivativeSpinBox->setDecimals(2);
    setHeatDerivativeButton = new QPushButton(tr("Set"));
    heatDerivativeLabel = new QLabel(tr("Heat Derivative: %L1").arg(0.0, 0, 'f', 2));

    coolProportionalBandGroupBox = new QGroupBox(tr("Cool Proportional Band (0-99.9):"));
    setCoolProportionalBandSpinBox = new QDoubleSpinBox;
    setCoolProportionalBandSpinBox->setMinimum(0.1);
    setCoolProportionalBandSpinBox->setMaximum(99.9);
    setCoolProportionalBandSpinBox->setSingleStep(1.0);
    setCoolProportionalBandSpinBox->setDecimals(1);
    setCoolProportionalBandButton = new QPushButton(tr("Set"));
    coolProportionalBandLabel = new QLabel(tr("Cool Proportional Band: %L1").arg(0.0, 0, 'f', 1));;

    coolIntegralGroupBox = new QGroupBox(tr("Cool Integral (0-9.99):"));
    setCoolIntegralSpinBox = new QDoubleSpinBox;
    setCoolIntegralSpinBox->setMinimum(0.0);
    setCoolIntegralSpinBox->setMaximum(9.99);
    setCoolIntegralSpinBox->setSingleStep(0.1);
    setCoolIntegralSpinBox->setDecimals(2);
    setCoolIntegralButton = new QPushButton(tr("Set"));
    coolIntegralLabel = new QLabel(tr("Cool Integral: %L1").arg(0.0, 0, 'f', 2));;

    coolDerivativeGroupBox = new QGroupBox(tr("Cool Derivative (0-5.0):"));
    setCoolDerivativeSpinBox = new QDoubleSpinBox;
    setCoolDerivativeSpinBox->setMinimum(0.0);
    setCoolDerivativeSpinBox->setMaximum(5.00);
    setCoolDerivativeSpinBox->setSingleStep(0.1);
    setCoolDerivativeSpinBox->setDecimals(2);
    setCoolDerivativeButton = new QPushButton(tr("Set"));
    coolDerivativeLabel = new QLabel(tr("Cool Derivative: %L1").arg(0.0, 0, 'f', 2));;

    readStatusButton = new QPushButton(tr("Read Status"));
    refreshPortsButton = new QPushButton(tr("Refresh Ports"));
    portsLabel = new QLabel(tr("Select port:"));
    portsComboBox = new QComboBox();
    portsLabel->setBuddy(portsComboBox);  // buddy accepts focus instead of label (for editing)
    int index = 0;
    refreshingPortsComboBoxContent = true;
    foreach (const NeslabTraits::ComPortParams &comPortParams, Neslab::availablePorts()) {
        portsComboBox->insertItem(index++, comPortParams.portName);
    }
    if (connected_) {
        *comPortName_ = neslab->comPortName();
    }
    if ((index = portsComboBox->findText(*comPortName_)) != -1)
        portsComboBox->setCurrentIndex(index);
    refreshingPortsComboBoxContent = false;
    if (!connected_ || index == -1) {
        neslab->setComPortName(portsComboBox->currentText());
    }
    *comPortName_ = portsComboBox->currentText();

    bathSettingsGroupBox = new QGroupBox(tr("Bath Settings:"));
    enableSensorCheck = new QCheckBox(tr("Enable Sensor"));
    enableFaultsCheck = new QCheckBox(tr("Enable Faults"));
    muteCheck = new QCheckBox(tr("Mute"));
    autoRestartCheck = new QCheckBox(tr("Autorestart"));
    enhancedPrecisionCheck = new QCheckBox(tr("Enhanced precision"));
    fullRangeCoolCheck = new QCheckBox(tr("Full Range Cool"));
    serialCommEnableCheck = new QCheckBox(tr("Enable Serial Comm"));
    serialCommEnableCheck->setChecked(true);

    defaultValuesButton = new QPushButton(tr("Default Values"));
    refreshButton = new QPushButton(tr("Refresh"));

    commandStatusLabel = new QLabel(tr("Ready"));
    commandStatusLabel->setStyleSheet("color: green");

    connect(neslab, &Neslab::sendingCommand, this, &MainWidget::onSendingCommand);
    connect(neslab, &Neslab::gotResponse, this, &MainWidget::onGotResponse);
    connect(advancedSettingsCheck, &QCheckBox::stateChanged, this, &MainWidget::onAdvancedSettingsCheckStateChanged);
    connect(connectButton, &QPushButton::clicked, this, &MainWidget::onConnectButtonClicked);
    connect(neslab, &Neslab::connected, this, &MainWidget::onConnected);
    connect(neslab, &Neslab::disconnected, this, &MainWidget::onDisconnected);
    connect(neslab, &Neslab::portOpeningFailed, this, &MainWidget::onPortOpeningFailed);
    connect(neslab, &Neslab::connectionFailed, this, &MainWidget::onConnectionFailed);
    connect(neslab, &Neslab::readAcknowledgeFinished, this, &MainWidget::onReadAcknowledgeFinished);
    connect(turnOnButton, &QPushButton::clicked, this, &MainWidget::onTurnOnButtonClicked);
    connect(neslab, &Neslab::setOnOffFinished, this, &MainWidget::onBathSettingsUpdated);
    connect(neslab, &Neslab::turnedOn, this, &MainWidget::onTurnedOnOff);
    connect(neslab, &Neslab::turnedOff, this, &MainWidget::onTurnedOnOff);
    connect(setSetpointSpinBox, &QDoubleSpinBox::editingFinished, this, &MainWidget::onSetSetpointSpinBoxEditingFinished);
    connect(setSetpointButton, &QPushButton::clicked, this, &MainWidget::onSetSetpointButtonClicked);
    connect(neslab, &Neslab::readSetpointFinished, this, &MainWidget::onReadSetpointFinished);
    connect(autoTemperatureCheck, &QCheckBox::stateChanged, this, &MainWidget::onAutoTemperatureCheckStateChanged);
    connect(autoTSettingsButton, &QPushButton::clicked, this, &MainWidget::onAutoTSettingsButtonClicked);
    connect(autoReadTimer, &QTimer::timeout, this, &MainWidget::onAutoReadTimerTimeout);
    connect(readTemperatureButton, &QPushButton::clicked, this, &MainWidget::onReadTemperatureButtonClicked);
    connect(neslab, &Neslab::readExternalSensorFinished, this, &MainWidget::onReadExternalSensorFinished);
    connect(neslab, &Neslab::readInternalTemperatureFinished, this, &MainWidget::onReadInternalTemperatureFinished);
    connect(setLowTemperatureLimitSpinBox, &QDoubleSpinBox::editingFinished, this, &MainWidget::onSetLowTemperatureLimitSpinBoxEditingFinished);
    connect(setLowTemperatureLimitButton, &QPushButton::clicked, this, &MainWidget::onSetLowTemperatureLimitButtonClicked);
    connect(neslab, &Neslab::readLowTemperatureLimitFinished, this, &MainWidget::onReadLowTemperatureLimitFinished);
    connect(setHighTemperatureLimitSpinBox, &QDoubleSpinBox::editingFinished, this, &MainWidget::onSetHighTemperatureLimitSpinBoxEditingFinished);
    connect(setHighTemperatureLimitButton, &QPushButton::clicked, this, &MainWidget::onSetHighTemperatureLimitButtonClicked);
    connect(neslab, &Neslab::readHighTemperatureLimitFinished, this, &MainWidget::onReadHighTemperatureLimitFinished);
    connect(setHeatProportionalBandSpinBox, &QDoubleSpinBox::editingFinished, this, &MainWidget::onSetHeatProportionalBandSpinBoxEditingFinished);
    connect(setHeatProportionalBandButton, &QPushButton::clicked, this, &MainWidget::onSetHeatProportionalBandButtonClicked);
    connect(neslab, &Neslab::readHeatProportionalBandFinished, this, &MainWidget::onReadHeatProportionalBandFinished);
    connect(setHeatIntegralSpinBox, &QDoubleSpinBox::editingFinished, this, &MainWidget::onSetHeatIntegralSpinBoxEditingFinished);
    connect(setHeatIntegralButton, &QPushButton::clicked, this, &MainWidget::onSetHeatIntegralButtonClicked);
    connect(neslab, &Neslab::readHeatIntegralFinished, this, &MainWidget::onReadHeatIntegralFinished);
    connect(setHeatDerivativeSpinBox, &QDoubleSpinBox::editingFinished, this, &MainWidget::onSetHeatDerivativeSpinBoxEditingFinished);
    connect(setHeatDerivativeButton, &QPushButton::clicked, this, &MainWidget::onSetHeatDerivativeButtonClicked);
    connect(neslab, &Neslab::readHeatDerivativeFinished, this, &MainWidget::onReadHeatDerivativeFinished);
    connect(setCoolProportionalBandSpinBox, &QDoubleSpinBox::editingFinished, this, &MainWidget::onSetCoolProportionalBandSpinBoxEditingFinished);
    connect(setCoolProportionalBandButton, &QPushButton::clicked, this, &MainWidget::onSetCoolProportionalBandButtonClicked);
    connect(neslab, &Neslab::readCoolProportionalBandFinished, this, &MainWidget::onReadCoolProportionalBandFinished);
    connect(setCoolIntegralSpinBox, &QDoubleSpinBox::editingFinished, this, &MainWidget::onSetCoolIntegralSpinBoxEditingFinished);
    connect(setCoolIntegralButton, &QPushButton::clicked, this, &MainWidget::onSetCoolIntegralButtonClicked);
    connect(neslab, &Neslab::readCoolIntegralFinished, this, &MainWidget::onReadCoolIntegralFinished);
    connect(setCoolDerivativeSpinBox, &QDoubleSpinBox::editingFinished, this, &MainWidget::onSetCoolDerivativeSpinBoxEditingFinished);
    connect(setCoolDerivativeButton, &QPushButton::clicked, this, &MainWidget::onSetCoolDerivativeButtonClicked);
    connect(neslab, &Neslab::readCoolDerivativeFinished, this, &MainWidget::onReadCoolDerivativeFinished);
    connect(readStatusButton, &QPushButton::clicked, neslab, &Neslab::readStatusCommand);
    connect(neslab, &Neslab::readStatusFinished, this, &MainWidget::onReadStatusFinished);
    connect(refreshPortsButton, &QPushButton::clicked, this, &MainWidget::onRefreshPortsButtonClicked);
    connect(portsComboBox, static_cast<void (QComboBox::*)(const QString &)>(&QComboBox::currentIndexChanged), this, &MainWidget::onPortsComboBoxCurrentIndexChanged);
    connect(enableSensorCheck, &QCheckBox::stateChanged, this, &MainWidget::onBathSettingsStateChanged);
    connect(enableFaultsCheck, &QCheckBox::stateChanged, this, &MainWidget::onBathSettingsStateChanged);
    connect(muteCheck, &QCheckBox::stateChanged, this, &MainWidget::onBathSettingsStateChanged);
    connect(autoRestartCheck, &QCheckBox::stateChanged, this, &MainWidget::onBathSettingsStateChanged);
    connect(enhancedPrecisionCheck, &QCheckBox::stateChanged, this, &MainWidget::onBathSettingsStateChanged);
    connect(fullRangeCoolCheck, &QCheckBox::stateChanged, this, &MainWidget::onBathSettingsStateChanged);
    connect(serialCommEnableCheck, &QCheckBox::stateChanged, this, &MainWidget::onBathSettingsStateChanged);
    connect(defaultValuesButton, &QPushButton::clicked, this, &MainWidget::onDefaultValuesButtonClicked);
    connect(refreshButton, &QPushButton::clicked, this, &MainWidget::onRefreshButtonClicked);
    connect(neslab, &Neslab::statusUpdated, this, &MainWidget::onStatusUpdated);

    horizontalLayout = new QHBoxLayout;
    buttonsWidget = new QWidget(this);
//    buttonsWidget->setFixedWidth(180);
    buttonsLayout = new QVBoxLayout(buttonsWidget);
    buttonsLayout->addWidget(advancedSettingsCheck);

    buttonsLayout->addWidget(versionLabel);

    connectLayout = new QHBoxLayout;
    connectLayout->addWidget(connectIndicator, 0, Qt::AlignVCenter);
    connectLayout->addWidget(connectButton, 0, Qt::AlignLeft);
    buttonsLayout->addLayout(connectLayout);

    powerLayout = new QHBoxLayout;
    powerLayout->addWidget(turnOnIndicator, 0, Qt::AlignVCenter);
    powerLayout->addWidget(turnOnButton, 0, Qt::AlignLeft);
    buttonsLayout->addLayout(powerLayout);

    setpointLayout = new QVBoxLayout;
    setSetpointLayout = new QHBoxLayout;
    setSetpointLayout->addWidget(setSetpointSpinBox);
    setSetpointLayout->addStretch();
    setSetpointLayout->addWidget(setSetpointButton);
    setpointLayout->addLayout(setSetpointLayout);
    setpointLayout->addWidget(setpointLabel);
    setpointGroupBox->setLayout(setpointLayout);
    buttonsLayout->addWidget(setpointGroupBox);

    temperatureLayout = new QVBoxLayout;

    readTemperatureLayout = new QHBoxLayout;
    readTemperatureLayout->addWidget(temperatureLabel);
    readTemperatureLayout->addStretch();
    readTemperatureLayout->addWidget(readTemperatureButton);
    temperatureLayout->addLayout(readTemperatureLayout);
    temperatureLayout->addWidget(autoTemperatureCheck, 0, Qt::AlignLeft);
    temperatureLayout->addWidget(autoTSettingsButton, 0, Qt::AlignLeft);
    temperatureGroupBox->setLayout(temperatureLayout);
    buttonsLayout->addWidget(temperatureGroupBox);

    buttonsLayout->addStretch();
    horizontalLayout->addWidget(buttonsWidget);

    advancedSettingsWidget = new QWidget(this);
    advancedSettingsWidget->hide();
//    advancedSettingsWidget->setFixedWidth(540);

    advancedSettingsHLayout = new QHBoxLayout(advancedSettingsWidget);
    advancedSettingsGridLayout = new QGridLayout;

    lowTemperatureLimitGridLayout = new QGridLayout(lowTemperatureLimitGroupBox);
    lowTemperatureLimitGridLayout->addWidget(setLowTemperatureLimitSpinBox, 0, 0);
    lowTemperatureLimitGridLayout->addWidget(setLowTemperatureLimitButton, 0, 1);
    lowTemperatureLimitGridLayout->addWidget(lowTemperatureLimitLabel, 1, 0, 1, 2, Qt::AlignLeft);
    advancedSettingsGridLayout->addWidget(lowTemperatureLimitGroupBox, 0, 0);

    heatProportionalBandGridLayout = new QGridLayout(heatProportionalBandGroupBox);
    heatProportionalBandGridLayout->addWidget(setHeatProportionalBandSpinBox, 0, 0);
    heatProportionalBandGridLayout->addWidget(setHeatProportionalBandButton, 0, 1);
    heatProportionalBandGridLayout->addWidget(heatProportionalBandLabel, 1, 0, 1, 2, Qt::AlignLeft);
    advancedSettingsGridLayout->addWidget(heatProportionalBandGroupBox, 1, 0);

    heatIntegralGridLayout = new QGridLayout(heatIntegralGroupBox);
    heatIntegralGridLayout->addWidget(setHeatIntegralSpinBox, 0, 0);
    heatIntegralGridLayout->addWidget(setHeatIntegralButton, 0, 1);
    heatIntegralGridLayout->addWidget(heatIntegralLabel, 1, 0, 1, 2, Qt::AlignLeft);
    advancedSettingsGridLayout->addWidget(heatIntegralGroupBox, 2, 0);

    heatDerivativeGridLayout = new QGridLayout(heatDerivativeGroupBox);
    heatDerivativeGridLayout->addWidget(setHeatDerivativeSpinBox, 0, 0);
    heatDerivativeGridLayout->addWidget(setHeatDerivativeButton, 0, 1);
    heatDerivativeGridLayout->addWidget(heatDerivativeLabel, 1, 0, 1, 2, Qt::AlignLeft);
    advancedSettingsGridLayout->addWidget(heatDerivativeGroupBox, 3, 0);


    highTemperatureLimitGridLayout = new QGridLayout(highTemperatureLimitGroupBox);
    highTemperatureLimitGridLayout->addWidget(setHighTemperatureLimitSpinBox, 0, 0);
    highTemperatureLimitGridLayout->addWidget(setHighTemperatureLimitButton, 0, 1);
    highTemperatureLimitGridLayout->addWidget(highTemperatureLimitLabel, 1, 0, 1, 2, Qt::AlignLeft);
    advancedSettingsGridLayout->addWidget(highTemperatureLimitGroupBox, 0, 1);

    coolProportionalBandGridLayout = new QGridLayout(coolProportionalBandGroupBox);
    coolProportionalBandGridLayout->addWidget(setCoolProportionalBandSpinBox, 0, 0);
    coolProportionalBandGridLayout->addWidget(setCoolProportionalBandButton, 0, 1);
    coolProportionalBandGridLayout->addWidget(coolProportionalBandLabel, 1, 0, 1, 2, Qt::AlignLeft);
    advancedSettingsGridLayout->addWidget(coolProportionalBandGroupBox, 1, 1);

    coolIntegralGridLayout = new QGridLayout(coolIntegralGroupBox);
    coolIntegralGridLayout->addWidget(setCoolIntegralSpinBox, 0, 0);
    coolIntegralGridLayout->addWidget(setCoolIntegralButton, 0, 1);
    coolIntegralGridLayout->addWidget(coolIntegralLabel, 1, 0, 1, 2, Qt::AlignLeft);
    advancedSettingsGridLayout->addWidget(coolIntegralGroupBox, 2, 1);

    coolDerivativeGridLayout = new QGridLayout(coolDerivativeGroupBox);
    coolDerivativeGridLayout->addWidget(setCoolDerivativeSpinBox, 0, 0);
    coolDerivativeGridLayout->addWidget(setCoolDerivativeButton, 0, 1);
    coolDerivativeGridLayout->addWidget(coolDerivativeLabel, 1, 0, 1, 2, Qt::AlignLeft);
    advancedSettingsGridLayout->addWidget(coolDerivativeGroupBox, 3, 1);

//    advancedSettingsGridLayout->setRowStretch(4, 1);
    advancedSettingsHLayout->addLayout(advancedSettingsGridLayout);


    advancedSettingsVLayout3 = new QVBoxLayout;

    readStatusRefreshPortsHLayout = new QHBoxLayout;
    readStatusRefreshPortsHLayout->addWidget(readStatusButton);
    readStatusRefreshPortsHLayout->addWidget(refreshPortsButton);
    advancedSettingsVLayout3->addLayout(readStatusRefreshPortsHLayout);

    portsHLayout = new QHBoxLayout;
    portsHLayout->addStretch();
    portsHLayout->addWidget(portsLabel, 0, Qt::AlignRight);
    portsHLayout->addWidget(portsComboBox, 0, Qt::AlignRight);
    advancedSettingsVLayout3->addLayout(portsHLayout);

    bathSettingsLayout = new QVBoxLayout(bathSettingsGroupBox);
    bathSettingsLayout->addWidget(enableSensorCheck);
    bathSettingsLayout->addWidget(enableFaultsCheck);
    bathSettingsLayout->addWidget(muteCheck);
    bathSettingsLayout->addWidget(autoRestartCheck);
    bathSettingsLayout->addWidget(enhancedPrecisionCheck);
    bathSettingsLayout->addWidget(fullRangeCoolCheck);
    bathSettingsLayout->addWidget(serialCommEnableCheck);
    advancedSettingsVLayout3->addWidget(bathSettingsGroupBox);

    advancedSettingsVLayout3->addStretch();

    defaultValuesRefreshHLayout = new QHBoxLayout;
    defaultValuesRefreshHLayout->addWidget(defaultValuesButton);
    defaultValuesRefreshHLayout->addWidget(refreshButton);
    advancedSettingsVLayout3->addLayout(defaultValuesRefreshHLayout);

    advancedSettingsHLayout->addLayout(advancedSettingsVLayout3);

    horizontalLayout->addWidget(advancedSettingsWidget);
    setLayout(horizontalLayout);

    if (connected_) {
        onRefreshButtonClicked();
    }
//    if (autoReadTSettings_->autoReadT) {
//        qDebug() << "NeslabusWidgets::MainWidget::initMainWidget(): autoReadT";
//        onAutoTemperatureCheckStateChanged();
//    }
}


QLabel *MainWidget::getCommandStatusLabel()
{
    return commandStatusLabel;
}

void MainWidget::onHidded()
{
    winIsVisible = true;
    if (!extAutoReadT && !autoReadTSettings_->saveTemperatures && autoReadTimer->isActive()) {
        emit autoTFinished();
        autoReadTimer->stop();
    }
}

void MainWidget::onShown()
{
    winIsVisible = false;
    if (!extAutoReadT && autoReadTSettings_->autoReadT && !autoReadTimer->isActive()) {
        emit autoTStarted();
        autoReadTimer->start(autoReadTSettings_->repetitionTime);
    }
}

void MainWidget::onStartAutoT()
{
    extAutoReadT = true;
    if (autoReadTSettings_->autoReadT && !autoReadTimer->isActive()) {
        emit autoTStarted();
        autoReadTimer->start(autoReadTSettings_->repetitionTime);
    }
}

void MainWidget::onFinishAutoT()
{
    extAutoReadT = false;
    if (!autoReadTSettings_->saveTemperatures && autoReadTimer->isActive()) {
        emit autoTFinished();
        autoReadTimer->stop();
    }
}

void MainWidget::onAdvancedSettingsCheckStateChanged()
{
    if (advancedSettingsCheck->isChecked()) {
        advancedSettingsWidget->setVisible(true);
        if (connected_) {
            onRefreshButtonClicked();
        }

    } else {
        advancedSettingsWidget->setVisible(false);
    }
}

void MainWidget::onNoSerialPortAvailable()
{
    portsComboBox->clear();
    emit autoTFinished();
    autoReadTimer->stop();
    QMessageBox::critical(this, tr("There is no serial port available!"), "There is no serial port available!", QMessageBox::Ok);
}

void MainWidget::onSendingCommand()
{
    commandStatusLabel->setText(tr("Sending command..."));
    commandStatusLabel->setStyleSheet("color: red");
}

void MainWidget::onGotResponse()
{
    commandStatusLabel->setText(tr("Ready"));
    commandStatusLabel->setStyleSheet("color: green");
}

void MainWidget::onConnectButtonClicked()
{
    connected_ = false;
    if (portsComboBox->count()) {
        connecting_ = true;
        neslab->connectNeslab();
    } else if (!neslab->availablePorts().isEmpty()) {
        int index = 0;
        foreach (const NeslabTraits::ComPortParams &comPortParams, Neslab::availablePorts()) {
            portsComboBox->insertItem(index++, comPortParams.portName);
        }
        if ((index = portsComboBox->findText(*comPortName_)) != -1)
            portsComboBox->setCurrentIndex(index);
        neslab->setComPortName(portsComboBox->currentText());
        *comPortName_ = portsComboBox->currentText();
        connecting_ = true;
        neslab->connectNeslab();
    } else {
        onNoSerialPortAvailable();
    }
}

void MainWidget::onTurnOnButtonClicked()
{
    if (!connected_) {
        onNotConnected();
        return;
    }
    if (turnedOn_) {
        neslab->turnOff();
    } else {
        neslab->turnOn();
    }
}

void MainWidget::onSetSetpointButtonClicked()
{
    if (!connected_) {
        onNotConnected();
        return;
    }
    setSetpoint_ = setSetpointSpinBox->value();
    neslab->setSetpointCommand(setSetpoint_);
    neslab->readSetpointCommand();
}

void MainWidget::onSetSetpointSpinBoxEditingFinished()
{
    if (setSetpointSpinBox->hasFocus() && !connecting_)
        onSetSetpointButtonClicked();
}

void MainWidget::onAutoTemperatureCheckStateChanged()
{
    if (autoTemperatureCheck->isChecked()) {
        autoReadTSettings_->autoReadT = true;
        readTemperatureButton->setEnabled(false);
        if (connected_) {
            if (autoReadTSettings_->saveTemperatures) {
                if (rewriteChecked) {
                    rewriteChecked = false;
                    startTemperatureMeasurement(autoReadTSettings_->directory, autoReadTSettings_->fileName);
                } else {
                    QFileInfo fileInfo(autoReadTSettings_->directory % QDir::separator() % autoReadTSettings_->fileName);
                    if (fileInfo.exists()) {
                        int ret = QMessageBox::question(this, tr("Overwrite file?"), tr("The file %1 already exists. Do you want to overwrite it?")
                                                        .arg(fileInfo.absoluteFilePath()), QMessageBox::Yes | QMessageBox::No);
                        if (ret == QMessageBox::Yes) {
                            startTemperatureMeasurement(autoReadTSettings_->directory, autoReadTSettings_->fileName);
                        }
                    }
                }
            }
            emit autoTStarted();
            onAutoReadTimerTimeout();
            autoReadTimer->start(autoReadTSettings_->repetitionTime);
        }
    } else {
        autoReadTSettings_->autoReadT = false;
        readTemperatureButton->setEnabled(true);
        emit autoTFinished();
        autoReadTimer->stop();
        if (autoReadTSettings_->saveTemperatures && temperaturesFile) {
            temperaturesFile->close();
            rewriteChecked = false;
        }
    }
}

void MainWidget::onAutoReadTimerTimeout()
{
    autoReadingT = true;
    neslab->readTemperatureCommand();

}

void MainWidget::onAutoTSettingsButtonClicked()
{
    AutoReadT::Dialog autoReadTDialog(*autoReadTSettings_);
    rewriteChecked = true;
    if (autoReadTDialog.exec()) {
        *autoReadTSettings_ = autoReadTDialog.getValues();
        if (autoTemperatureCheck->isChecked() && autoReadTSettings_->saveTemperatures) {
            startTemperatureMeasurement(autoReadTSettings_->directory, autoReadTSettings_->fileName);
        }
    }
}

void MainWidget::onReadTemperatureButtonClicked()
{
    if (!connected_) {
        onNotConnected();
        return;
    }
    neslab->readTemperatureCommand();
}

void MainWidget::onSetLowTemperatureLimitButtonClicked()
{
    if (!connected_) {
        onNotConnected();
        return;
    }
    neslab->setLowTemperatureLimitCommand(setLowTemperatureLimitSpinBox->value());
    neslab->readLowTemperatureLimitCommand();
}

void MainWidget::onSetLowTemperatureLimitSpinBoxEditingFinished()
{
    if (setLowTemperatureLimitSpinBox->hasFocus() && !connecting_)
        onSetLowTemperatureLimitButtonClicked();
}

void MainWidget::onSetHighTemperatureLimitButtonClicked()
{
    if (!connected_) {
        onNotConnected();
        return;
    }
    neslab->setHighTemperatureLimitCommand(setHighTemperatureLimitSpinBox->value());
    neslab->readHighTemperatureLimitCommand();
}

void MainWidget::onSetHighTemperatureLimitSpinBoxEditingFinished()
{
    if (setHighTemperatureLimitSpinBox->hasFocus() && !connecting_)
        onSetHighTemperatureLimitButtonClicked();
}

void MainWidget::onSetHeatProportionalBandButtonClicked()
{
    if (!connected_) {
        onNotConnected();
        return;
    }
    neslab->setHeatProportionalBandCommand(setHeatProportionalBandSpinBox->value());
    neslab->readHeatProportionalBandCommand();
}

void MainWidget::onSetHeatProportionalBandSpinBoxEditingFinished()
{
    if (setHeatProportionalBandSpinBox->hasFocus() && !connecting_)
        onSetHeatProportionalBandButtonClicked();
}

void MainWidget::onSetHeatIntegralButtonClicked()
{
    if (!connected_) {
        onNotConnected();
        return;
    }
    neslab->setHeatIntegralCommand(setHeatIntegralSpinBox->value());
    neslab->readHeatIntegralCommand();
}

void MainWidget::onSetHeatIntegralSpinBoxEditingFinished()
{
    if (setHeatIntegralSpinBox->hasFocus() && !connecting_)
        onSetHeatIntegralButtonClicked();
}

void MainWidget::onSetHeatDerivativeButtonClicked()
{
    if (!connected_) {
        onNotConnected();
        return;
    }
    neslab->setHeatDerivativeCommand(setHeatDerivativeSpinBox->value());
    neslab->readHeatDerivativeCommand();
}

void MainWidget::onSetHeatDerivativeSpinBoxEditingFinished()
{
    if (setHeatDerivativeSpinBox->hasFocus() && !connecting_)
        onSetHeatDerivativeButtonClicked();
}

void MainWidget::onSetCoolProportionalBandButtonClicked()
{
    if (!connected_) {
        onNotConnected();
        return;
    }
    neslab->setCoolProportionalBandCommand(setCoolProportionalBandSpinBox->value());
    neslab->readCoolProportionalBandCommand();
}

void MainWidget::onSetCoolProportionalBandSpinBoxEditingFinished()
{
    if (setCoolProportionalBandSpinBox->hasFocus() && !connecting_)
        onSetCoolProportionalBandButtonClicked();
}

void MainWidget::onSetCoolIntegralButtonClicked()
{
    if (!connected_) {
        onNotConnected();
        return;
    }
    neslab->setCoolIntegralCommand(setCoolIntegralSpinBox->value());
    neslab->readCoolIntegralCommand();
}

void MainWidget::onSetCoolIntegralSpinBoxEditingFinished()
{
    if (setCoolIntegralSpinBox->hasFocus() && !connecting_)
        onSetCoolIntegralButtonClicked();
}

void MainWidget::onSetCoolDerivativeButtonClicked()
{
    if (!connected_) {
        onNotConnected();
        return;
    }
    neslab->setCoolDerivativeCommand(setCoolDerivativeSpinBox->value());
    neslab->readCoolDerivativeCommand();
}

void MainWidget::onSetCoolDerivativeSpinBoxEditingFinished()
{
    if (setCoolDerivativeSpinBox->hasFocus() && !connecting_)
        onSetCoolDerivativeButtonClicked();
}

void MainWidget::onReadStatusButtonClicked()
{
    if (!connected_) {
        onNotConnected();
        return;
    }
    neslab->readStatusCommand();
}

void MainWidget::onBathSettingsStateChanged()
{
    if (!connected_) {
        onNotConnected();
        return;
    }
    NeslabTraits::PowerOnOffParm powerOnOffParm;
    QObject *obj = sender();
    if (obj == enableSensorCheck && enableSensorCheck->isChecked() != (*powerOnOffParm_)[NeslabTraits::PowerOnOff::d2_sensorEnabled]) {
        if (enableSensorCheck->isChecked())
            powerOnOffParm[NeslabTraits::PowerOnOff::d2_sensorEnabled] = 1;
        else
            powerOnOffParm[NeslabTraits::PowerOnOff::d2_sensorEnabled] = 0;
    } else if (obj == enableFaultsCheck && enableFaultsCheck->isChecked() != (*powerOnOffParm_)[NeslabTraits::PowerOnOff::d3_faultsEnabled]) {
        if (enableFaultsCheck->isChecked())
            powerOnOffParm[NeslabTraits::PowerOnOff::d3_faultsEnabled] = 1;
        else
            powerOnOffParm[NeslabTraits::PowerOnOff::d3_faultsEnabled] = 0;
    } else if (obj == muteCheck && muteCheck->isChecked() != (*powerOnOffParm_)[NeslabTraits::PowerOnOff::d4_mute]) {
        if (muteCheck->isChecked())
            powerOnOffParm[NeslabTraits::PowerOnOff::d4_mute] = 1;
        else
            powerOnOffParm[NeslabTraits::PowerOnOff::d4_mute] = 0;
    } else if (obj == autoRestartCheck && autoRestartCheck->isChecked() != (*powerOnOffParm_)[NeslabTraits::PowerOnOff::d5_autoRestart]) {
        if (autoRestartCheck->isChecked())
            powerOnOffParm[NeslabTraits::PowerOnOff::d5_autoRestart] = 1;
        else
            powerOnOffParm[NeslabTraits::PowerOnOff::d5_autoRestart] = 0;
    } else if (obj == enhancedPrecisionCheck && enhancedPrecisionCheck->isChecked() != (*powerOnOffParm_)[NeslabTraits::PowerOnOff::d6_01precDegCEnable]) {
        if (enhancedPrecisionCheck->isChecked())
            powerOnOffParm[NeslabTraits::PowerOnOff::d6_01precDegCEnable] = 1;
        else
            powerOnOffParm[NeslabTraits::PowerOnOff::d6_01precDegCEnable] = 0;
    } else if (obj == fullRangeCoolCheck && fullRangeCoolCheck->isChecked() != (*powerOnOffParm_)[NeslabTraits::PowerOnOff::d7_fullRangeCoolEnable]) {
        if (fullRangeCoolCheck->isChecked())
            powerOnOffParm[NeslabTraits::PowerOnOff::d7_fullRangeCoolEnable] = 1;
        else
            powerOnOffParm[NeslabTraits::PowerOnOff::d7_fullRangeCoolEnable] = 0;
    } else if (obj == serialCommEnableCheck && serialCommEnableCheck->isChecked() != (*powerOnOffParm_)[NeslabTraits::PowerOnOff::d8_serialCommEnable]) {
        if (serialCommEnableCheck->isChecked())
            powerOnOffParm[NeslabTraits::PowerOnOff::d8_serialCommEnable] = 1;
        else
            powerOnOffParm[NeslabTraits::PowerOnOff::d8_serialCommEnable] = 0;
    }
    neslab->setOnOffCommand(powerOnOffParm);
}

void MainWidget::onRefreshPortsButtonClicked()
{
    if (!neslab->availablePorts().isEmpty()) {
        QString msg;
        QString currPort = portsComboBox->currentText();
        QStringList comPortNames;
        foreach (const NeslabTraits::ComPortParams &comPortParams, Neslab::availablePorts()) {
            msg.append("Port name: " % comPortParams.portName % "\n" %
                  "Description: " % comPortParams.description % "\n" %
                  "Description: " % comPortParams.manufacturer % "\n");
            comPortNames.append(comPortParams.portName);
        }
        QMessageBox::information(this, tr("Serial ports information:"), msg, QMessageBox::Ok);
        bool ok = true;
        bool counteq = false;
        if ((counteq = (portsComboBox->count() == comPortNames.count()))) {
            for (int ii = 0; ii < comPortNames.count(); ++ii) {
                if (portsComboBox->findText(comPortNames.at(ii)) < 0) {
                    ok = false;
                }
            }
        }
        if (!counteq || !ok) {
            refreshingPortsComboBoxContent = true;
            portsComboBox->clear();
            portsComboBox->insertItems(1, comPortNames);
            int index;
            if ((index = portsComboBox->findText(currPort)) >= 0)
                portsComboBox->setCurrentIndex(index);
            else if ((index = portsComboBox->findText(*comPortName_)) >= 0)
                portsComboBox->setCurrentIndex(index);
            refreshingPortsComboBoxContent = false;
            if (!connected_ && index == -1) {
                neslab->setComPortName(portsComboBox->currentText());
            }
            *comPortName_ = portsComboBox->currentText();
        }
    } else {
        onDisconnected();
        onNoSerialPortAvailable();
    }
}

void MainWidget::onPortsComboBoxCurrentIndexChanged(const QString & portName)
{
    if (!refreshingPortsComboBoxContent && !portName.isEmpty()) {
        neslab->setComPortName(portName);
        *comPortName_ = portsComboBox->currentText();
    }
}

void MainWidget::onRefreshButtonClicked()
{
    if (!connected_) {
        onNotConnected();
        return;
    }
    NeslabTraits::PowerOnOffParm powerOnOffParm;
    neslab->setOnOffCommand(powerOnOffParm);
    neslab->readAcknowledgeCommand();
    neslab->readLowTemperatureLimitCommand();
    neslab->readHighTemperatureLimitCommand();
    neslab->readHeatProportionalBandCommand();
    neslab->readHeatIntegralCommand();
    neslab->readHeatDerivativeCommand();
    neslab->readCoolProportionalBandCommand();
    neslab->readCoolIntegralCommand();
    neslab->readCoolDerivativeCommand();
    neslab->readSetpointCommand();
    neslab->readTemperatureCommand();
}

void MainWidget::onDefaultValuesButtonClicked()
{
    if (!connected_) {
        onNotConnected();
        return;
    }
    int ret = QMessageBox::question(this, tr("Restore default PID settings?"),
                                    tr("Are you sure to restore default PID settings? The current values will be lost!"),
                                    QMessageBox::Yes | QMessageBox::No ,QMessageBox::No);
    if (ret == QMessageBox::Yes)
    {
        connectCommandsUpdate_[static_cast<int>(NeslabTraits::Command::ReadHeatProportionalBand)] = true;
        connectCommandsUpdate_[static_cast<int>(NeslabTraits::Command::ReadHeatIntegral)] = true;
        connectCommandsUpdate_[static_cast<int>(NeslabTraits::Command::ReadHeatDerivative)] = true;
        connectCommandsUpdate_[static_cast<int>(NeslabTraits::Command::ReadCoolProportionalBand)] = true;
        connectCommandsUpdate_[static_cast<int>(NeslabTraits::Command::ReadCoolIntegral)] = true;
        connectCommandsUpdate_[static_cast<int>(NeslabTraits::Command::ReadCoolDerivative)] = true;
        connecting_ = true;
        neslab->restoreDefaultSettingsCommand();
        neslab->readHeatProportionalBandCommand();
        neslab->readHeatIntegralCommand();
        neslab->readHeatDerivativeCommand();
        neslab->readCoolProportionalBandCommand();
        neslab->readCoolIntegralCommand();
        neslab->readCoolDerivativeCommand();
    }
}

void MainWidget::onConnected()
{
    connected_ = true;
    connectIndicator->setStyleSheet("background-color: green");
    neslab->readTemperatureCommand();

    connectCommandsUpdate_[static_cast<int>(NeslabTraits::Command::ReadLowTemperatureLimit)] = true;
    connectCommandsUpdate_[static_cast<int>(NeslabTraits::Command::ReadHighTemperatureLimit)] = true;
    connectCommandsUpdate_[static_cast<int>(NeslabTraits::Command::ReadHeatProportionalBand)] = true;
    connectCommandsUpdate_[static_cast<int>(NeslabTraits::Command::ReadHeatIntegral)] = true;
    connectCommandsUpdate_[static_cast<int>(NeslabTraits::Command::ReadHeatDerivative)] = true;
    connectCommandsUpdate_[static_cast<int>(NeslabTraits::Command::ReadCoolProportionalBand)] = true;
    connectCommandsUpdate_[static_cast<int>(NeslabTraits::Command::ReadCoolIntegral)] = true;
    connectCommandsUpdate_[static_cast<int>(NeslabTraits::Command::ReadCoolDerivative)] = true;
    connectCommandsUpdate_[static_cast<int>(NeslabTraits::Command::ReadSetpoint)] = true;

    neslab->readAcknowledgeCommand();
    neslab->readLowTemperatureLimitCommand();
    neslab->readHighTemperatureLimitCommand();
    neslab->readHeatProportionalBandCommand();
    neslab->readHeatIntegralCommand();
    neslab->readHeatDerivativeCommand();
    neslab->readCoolProportionalBandCommand();
    neslab->readCoolIntegralCommand();
    neslab->readCoolDerivativeCommand();
    neslab->readSetpointCommand();
    if (autoTemperatureCheck->isChecked()) {
        emit autoTStarted();
        autoReadTimer->start(autoReadTSettings_->repetitionTime);
    }
}

void MainWidget::onDisconnected()
{
    connected_ = false;
    connectIndicator->setStyleSheet("background-color: red");
    emit autoTFinished();
    autoReadTimer->stop();
}

void MainWidget::onNotConnected()
{
    emit autoTFinished();
    autoReadTimer->stop();
    QMessageBox::warning(this, tr("Not connected!"), tr("Bath has not been connected yet. Please connect it."), QMessageBox::Ok);
}

void MainWidget::onPortOpeningFailed()
{
    QMessageBox::critical(this, tr("Serial port openning failed!"), tr("Serial port openning failed! Please, control if "
                                                                    "there is not other application using the port."), QMessageBox::Ok);
}

void MainWidget::onConnectionFailed()
{
    connected_ = false;
    connectIndicator->setStyleSheet("background-color: red");
    commandStatusLabel->setText("Ready");
    commandStatusLabel->setStyleSheet("color: green");
    emit autoTFinished();
    autoReadTimer->stop();
    QMessageBox::critical(this, tr("Connection failed!"), tr("Connection failed!"), QMessageBox::Ok);
}

void MainWidget::onReadAcknowledgeFinished(unsigned char v1, unsigned char v2)
{
    versionLabel->setText(tr("Version: %1.%2").arg(static_cast<int>(v1)).arg(static_cast<int>(v2)));
}

void MainWidget::onTurnedOnOff()
{
    onBathSettingsUpdated();
}

void MainWidget::onReadSetpointFinished(double T, bool enhancedPrec)
{
    if (connecting_) {
        connectCommandsUpdate_[static_cast<int>(NeslabTraits::Command::ReadSetpoint)] = false;
        setSetpointSpinBox->setValue(T);
        bool connectCommandsFinished = true;
        for (int ii = 0; ii < static_cast<int>(NeslabTraits::Command::None); ++ii) {
            if (connectCommandsUpdate_[ii]) {
                connectCommandsFinished = false;
                break;
            }
        }
        if (connectCommandsFinished) {
            connecting_ = false;
        }
    }
    setpoint_ = T;
    if (enhancedPrecision_ != enhancedPrec) {
        enhancedPrecision_ = enhancedPrec;
        enhancedPrecisionCheck->setChecked(enhancedPrecision_);
    }
    setpointLabel->setText(tr("Setpoint: %L1 °C").arg(setpoint_, 0, 'f', 2));
}

void MainWidget::onReadExternalSensorFinished(double T, bool enhancedPrec)
{
    currT_ = T;
    if (enhancedPrecision_ != enhancedPrec) {
        enhancedPrecision_ = enhancedPrec;
        enhancedPrecisionCheck->setChecked(enhancedPrecision_);
    }
    temperatureLabel->setText(tr("%L1 °C").arg(currT_, 0, 'f', 2));
    onSaveTemperature(T, enhancedPrec);
}

void MainWidget::onReadInternalTemperatureFinished(double T, bool enhancedPrec)
{
    currT_ = T;
    if (enhancedPrecision_ != enhancedPrec) {
        enhancedPrecision_ = enhancedPrec;
        enhancedPrecisionCheck->setChecked(enhancedPrecision_);
    }
    temperatureLabel->setText(tr("%L1 °C").arg(currT_, 0, 'f', 2));
    onSaveTemperature(T, enhancedPrec);
}

void MainWidget::onSaveTemperature(double T, bool enhancedPrec)
{
    if (autoTemperatureCheck->isChecked() && autoReadingT && autoReadTSettings_->saveTemperatures &&
            ++autosaveTcounter >= autoReadTSettings_->saveRepetitionTime) {
        if (!temperaturesFile->open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append))
            return;
        double elapsedTime = static_cast<double>((QDateTime::currentDateTime().toMSecsSinceEpoch() -
                                                  startMeasTime->toMSecsSinceEpoch())) / 1000;
        if (enhancedPrec) {
            *temperaturesTextStream << tr("%1\t%2\n").arg(elapsedTime, 10, 'f', 1).arg(T, 6, 'f', 2);
        } else {
            *temperaturesTextStream << tr("%1\t%2\n").arg(elapsedTime, 10, 'f', 1).arg(T, 5, 'f', 1);
        }
        temperaturesTextStream->flush();
        temperaturesFile->close();
        autoReadingT = false;
        autosaveTcounter = 0;
    }
}

void MainWidget::onReadLowTemperatureLimitFinished(double T, bool enhancedPrec)
{
    if (connecting_) {
        connectCommandsUpdate_[static_cast<int>(NeslabTraits::Command::ReadLowTemperatureLimit)] = false;
        setLowTemperatureLimitSpinBox->setValue(T);
        bool connectCommandsFinished = true;
        for (int ii = 0; ii < static_cast<int>(NeslabTraits::Command::None); ++ii) {
            if (connectCommandsUpdate_[ii]) {
                connectCommandsFinished = false;
                break;
            }
        }
        if (connectCommandsFinished) {
            connecting_ = false;
        }
    }
    lowTLim_ = T;
    if (enhancedPrecision_ != enhancedPrec) {
        enhancedPrecision_ = enhancedPrec;
        enhancedPrecisionCheck->setChecked(enhancedPrecision_);
    }
    lowTemperatureLimitLabel->setText(tr("Low Temperature Limit: %L1 °C").arg(lowTLim_, 0, 'f', 2));
}

void MainWidget::onReadHighTemperatureLimitFinished(double T, bool enhancedPrec)
{
    if (connecting_) {
        connectCommandsUpdate_[static_cast<int>(NeslabTraits::Command::ReadHighTemperatureLimit)] = false;
        setHighTemperatureLimitSpinBox->setValue(T);
        bool connectCommandsFinished = true;
        for (int ii = 0; ii < static_cast<int>(NeslabTraits::Command::None); ++ii) {
            if (connectCommandsUpdate_[ii]) {
                connectCommandsFinished = false;
                break;
            }
        }
        if (connectCommandsFinished) {
            connecting_ = false;
        }
    }
    highTLim_ = T;
    if (enhancedPrecision_ != enhancedPrec) {
        enhancedPrecision_ = enhancedPrec;
        enhancedPrecisionCheck->setChecked(enhancedPrecision_);
    }
    highTemperatureLimitLabel->setText(tr("High Temperature Limit: %L1 °C").arg(highTLim_, 0, 'f', 2));
}

void MainWidget::onReadHeatProportionalBandFinished(double P)
{
    if (connecting_) {
        connectCommandsUpdate_[static_cast<int>(NeslabTraits::Command::ReadHeatProportionalBand)] = false;
        setHeatProportionalBandSpinBox->setValue(P);
        bool connectCommandsFinished = true;
        for (int ii = 0; ii < static_cast<int>(NeslabTraits::Command::None); ++ii) {
            if (connectCommandsUpdate_[ii]) {
                connectCommandsFinished = false;
                break;
            }
        }
        if (connectCommandsFinished) {
            connecting_ = false;
        }
    }
    heatP_ = P;
    heatProportionalBandLabel->setText(tr("Heat Proportional Band: %L1").arg(heatP_, 0, 'f', 1));
}

void MainWidget::onReadHeatIntegralFinished(double I)
{
    if (connecting_) {
        connectCommandsUpdate_[static_cast<int>(NeslabTraits::Command::ReadHeatIntegral)] = false;
        setHeatIntegralSpinBox->setValue(I);
        bool connectCommandsFinished = true;
        for (int ii = 0; ii < static_cast<int>(NeslabTraits::Command::None); ++ii) {
            if (connectCommandsUpdate_[ii]) {
                connectCommandsFinished = false;
                break;
            }
        }
        if (connectCommandsFinished) {
            connecting_ = false;
        }
    }
    heatI_ = I;
    heatIntegralLabel->setText(tr("Heat Integral: %L1").arg(heatI_, 0, 'f', 2));
}

void MainWidget::onReadHeatDerivativeFinished(double D)
{
    if (connecting_) {
        connectCommandsUpdate_[static_cast<int>(NeslabTraits::Command::ReadHeatDerivative)] = false;
        setHeatDerivativeSpinBox->setValue(D);
        bool connectCommandsFinished = true;
        for (int ii = 0; ii < static_cast<int>(NeslabTraits::Command::None); ++ii) {
            if (connectCommandsUpdate_[ii]) {
                connectCommandsFinished = false;
                break;
            }
        }
        if (connectCommandsFinished) {
            connecting_ = false;
        }
    }
    heatD_ = D;
    heatDerivativeLabel->setText(tr("Heat Derivative: %L1").arg(heatD_, 0, 'f', 2));
}

void MainWidget::onReadCoolProportionalBandFinished(double P)
{
    if (connecting_) {
        connectCommandsUpdate_[static_cast<int>(NeslabTraits::Command::ReadCoolProportionalBand)] = false;
        setCoolProportionalBandSpinBox->setValue(P);
        bool connectCommandsFinished = true;
        for (int ii = 0; ii < static_cast<int>(NeslabTraits::Command::None); ++ii) {
            if (connectCommandsUpdate_[ii]) {
                connectCommandsFinished = false;
                break;
            }
        }
        if (connectCommandsFinished) {
            connecting_ = false;
        }
    }
    coolP_ = P;
    coolProportionalBandLabel->setText(tr("Cool Proportional Band: %L1").arg(coolP_, 0, 'f', 1));
}

void MainWidget::onReadCoolIntegralFinished(double I)
{
    if (connecting_) {
        connectCommandsUpdate_[static_cast<int>(NeslabTraits::Command::ReadCoolIntegral)] = false;
        setCoolIntegralSpinBox->setValue(I);
        bool connectCommandsFinished = true;
        for (int ii = 0; ii < static_cast<int>(NeslabTraits::Command::None); ++ii) {
            if (connectCommandsUpdate_[ii]) {
                connectCommandsFinished = false;
                break;
            }
        }
        if (connectCommandsFinished) {
            connecting_ = false;
        }
    }
    coolI_ = I;
    coolIntegralLabel->setText(tr("Cool Integral: %L1").arg(coolI_, 0, 'f', 2));
}

void MainWidget::onReadCoolDerivativeFinished(double D)
{
    if (connecting_) {
        connectCommandsUpdate_[static_cast<int>(NeslabTraits::Command::ReadCoolDerivative)] = false;
        setCoolDerivativeSpinBox->setValue(D);
        bool connectCommandsFinished = true;
        for (int ii = 0; ii < static_cast<int>(NeslabTraits::Command::None); ++ii) {
            if (connectCommandsUpdate_[ii]) {
                connectCommandsFinished = false;
                break;
            }
        }
        if (connectCommandsFinished) {
            connecting_ = false;
        }
    }
    coolD_ = D;
    coolDerivativeLabel->setText(tr("Cool Derivative: %L1").arg(coolD_, 0, 'f', 2));
}

void MainWidget::onReadStatusFinished(QString msg)
{
    QMessageBox::information(this, tr("Bath status"), msg, QMessageBox::Ok);
}

void MainWidget::onBathSettingsUpdated()
{
    *powerOnOffParm_ = neslab->getOnOffParms();
    if (turnedOn_ != (*powerOnOffParm_)[NeslabTraits::PowerOnOff::d1_unitOnOff]) {
        turnedOn_ = (*powerOnOffParm_)[NeslabTraits::PowerOnOff::d1_unitOnOff];
        if (turnedOn_) {
            turnOnIndicator->setStyleSheet("background-color: green");
        } else {
            turnOnIndicator->setStyleSheet("background-color: red");
        }
    }
    if (enableSensorCheck->isChecked() != (*powerOnOffParm_)[NeslabTraits::PowerOnOff::d2_sensorEnabled])
        enableSensorCheck->setChecked((*powerOnOffParm_)[NeslabTraits::PowerOnOff::d2_sensorEnabled]);
    if (enableFaultsCheck->isChecked() != (*powerOnOffParm_)[NeslabTraits::PowerOnOff::d3_faultsEnabled])
        enableFaultsCheck->setChecked((*powerOnOffParm_)[NeslabTraits::PowerOnOff::d3_faultsEnabled]);
    if (muteCheck->isChecked() != (*powerOnOffParm_)[NeslabTraits::PowerOnOff::d4_mute])
        muteCheck->setChecked((*powerOnOffParm_)[NeslabTraits::PowerOnOff::d4_mute]);
    if (autoRestartCheck->isChecked() != (*powerOnOffParm_)[NeslabTraits::PowerOnOff::d5_autoRestart])
        autoRestartCheck->setChecked((*powerOnOffParm_)[NeslabTraits::PowerOnOff::d5_autoRestart]);
    if (enhancedPrecisionCheck->isChecked() != (*powerOnOffParm_)[NeslabTraits::PowerOnOff::d6_01precDegCEnable])
        enhancedPrecisionCheck->setChecked((*powerOnOffParm_)[NeslabTraits::PowerOnOff::d6_01precDegCEnable]);
    if (fullRangeCoolCheck->isChecked() != (*powerOnOffParm_)[NeslabTraits::PowerOnOff::d7_fullRangeCoolEnable])
        fullRangeCoolCheck->setChecked((*powerOnOffParm_)[NeslabTraits::PowerOnOff::d7_fullRangeCoolEnable]);
    if (serialCommEnableCheck->isChecked() != (*powerOnOffParm_)[NeslabTraits::PowerOnOff::d8_serialCommEnable])
        serialCommEnableCheck->setChecked((*powerOnOffParm_)[NeslabTraits::PowerOnOff::d8_serialCommEnable]);
}

void MainWidget::onStatusUpdated()
{
    NeslabTraits::BathStatus bathStatus;
    if (neslab->getBathStatus(&bathStatus)) {
        if (static_cast<bool>(bathStatus.d5 & NeslabTraits::BathStatusTraits::d5::RTD2Controlling) !=
                static_cast<bool>(bathStatus_->d5 & NeslabTraits::BathStatusTraits::d5::RTD2Controlling)) {
            if (static_cast<bool>(bathStatus.d5 & NeslabTraits::BathStatusTraits::d5::RTD2Controlling)) {
                sensorEnabled_ = true;
                enableSensorCheck->setChecked(true);
            } else {
                sensorEnabled_ = false;
                enableSensorCheck->setChecked(false);
            }
        }
        if (static_cast<bool>(bathStatus.d4 & NeslabTraits::BathStatusTraits::d4::AlarmMuted) !=
                static_cast<bool>(bathStatus_->d4 & NeslabTraits::BathStatusTraits::d4::AlarmMuted)) {
            if (static_cast<bool>(bathStatus.d4 & NeslabTraits::BathStatusTraits::d4::AlarmMuted)) {
                muted_ = true;
                muteCheck->setChecked(true);
            } else {
                muted_ = false;
                muteCheck->setChecked(false);
            }
        }
        if (static_cast<bool>(bathStatus.d2 & NeslabTraits::BathStatusTraits::d2::RefrigHighTemp) !=
                static_cast<bool>(bathStatus_->d2 & NeslabTraits::BathStatusTraits::d2::RefrigHighTemp)) {
            if (static_cast<bool>(bathStatus.d2 & NeslabTraits::BathStatusTraits::d2::RefrigHighTemp)) {
                fullRangeCool_ = true;
                fullRangeCoolCheck->setChecked(true);
            } else {
                fullRangeCool_ = false;
                fullRangeCoolCheck->setChecked(false);
            }
        }
        *bathStatus_ = bathStatus;
    }
}

void MainWidget::switchPowerConfirmed()
{
    if (neslab->isTurnedOn(&turnedOn_)) {
        if (turnedOn_) {
            turnOnIndicator->setStyleSheet("background-color: green");
        } else {
            turnOnIndicator->setStyleSheet("background-color: red");
        }
    } else {
        neslab->updateStatusCommand();
    }
}

void MainWidget::startTemperatureMeasurement(const QString & directory, const QString & filename)
{
    if (temperaturesFile) {
        if (temperaturesTextStream) {
            temperaturesTextStream->flush();
        }
        temperaturesFile->close();
        temperaturesFile->setFileName(QDir(directory).path() % "/" % filename);
    } else {
        temperaturesFile = new QFile(QDir(directory).path() % "/" % filename);
    }
    if (!temperaturesFile->open(QIODevice::WriteOnly | QIODevice::Text))
        return;
    if (temperaturesTextStream) {
        temperaturesTextStream->setDevice(temperaturesFile);
    } else {
        temperaturesTextStream = new QTextStream(temperaturesFile);
    }
    *startMeasTime = QDateTime::currentDateTime();
    *temperaturesTextStream << tr("# Temperatures from Neslab.\n# Measurement "
                                  "start: %1\n"
                                  "#  time(s)\ttemperature(°C)\n")
                               .arg(startMeasTime->toString(tr("dd.MM.yyyy hh:mm:ss")));
    temperaturesTextStream->flush();
    temperaturesTextStream->setRealNumberPrecision(3);
    temperaturesFile->close();
    autosaveTcounter = autoReadTSettings_->saveRepetitionTime;
}

MainWidget::~MainWidget()
{
    qDebug() << "NeslabusWidgets::MainWidget::~MainWidget()";
    delete[] connectCommandsUpdate_;
    delete bathStatus_;
    delete powerOnOffParm_;

    if (!extAutoReadTSettings) {
        QSettings settings(QCoreApplication::applicationDirPath() + "/" + QCoreApplication::applicationName() + ".ini",QSettings::IniFormat);
        settings.beginGroup("Neslabus");
        settings.setValue("COMPort", *comPortName_);
        settings.setValue("autoReadT", autoReadTSettings_->autoReadT);
        settings.setValue("autoReadRepetitionTime", autoReadTSettings_->repetitionTime);
        settings.setValue("autoSaveRepetitionTime", autoReadTSettings_->saveRepetitionTime);
        settings.setValue("autoSaveDirectory", autoReadTSettings_->directory);
        settings.setValue("autoSaveFileName", autoReadTSettings_->fileName);
        settings.endGroup();
        delete comPortName_;
        delete autoReadTSettings_;
    }
    delete temperaturesFile;
    delete temperaturesTextStream;
    delete startMeasTime;
    if (thread) {
        thread->quit();
        thread->wait();
    }
    qDebug() << "NeslabusWidgets::MainWidget::~MainWidget(): finished";
}

AutoReadT::Dialog::Dialog(QWidget *parent) :
    Dialog({false, 1000, false, 1, "", ""}, parent)
{
}

AutoReadT::Dialog::Dialog(const AutoReadT::Settings &rv, QWidget *parent) :
    QDialog(parent)
{
    returnValues = rv;
    if (returnValues.directory.isEmpty()) {
        returnValues.directory = QDir::toNativeSeparators(QDir::currentPath());
    }
    QDir directory(returnValues.directory);
    int l = directory.absolutePath().length();
    while (!directory.exists()) {
        int ii;
        if ((ii = directory.absolutePath().lastIndexOf(QChar('/'))) >= 0) {
            directory.setPath(directory.absolutePath().mid(0, ii));
            if (l <= directory.absolutePath().length()) {
                directory.setPath(QDir::currentPath());
                break;
            }
        } else {
            directory.setPath(QDir::currentPath());
            break;
        }
    }
    returnValues.directory = QDir::toNativeSeparators(directory.absolutePath());


    if (returnValues.fileName.isEmpty()) {
        returnValues.fileName = "tempT.dat";
    }
    QFileInfo fileInfo(QDir::toNativeSeparators(directory.absolutePath() % QDir::separator() % returnValues.fileName));
    returnValues.fileName = fileInfo.fileName();

    saveTemperaturesChanged = true; // indicates if the state of
    // saveTemperaturesGroupBox changed to ask, if you want to replace the
    // file. It was intended to the case, when you change the saving properties
    // but do not change the file name and so do not want to rewrite the values
    // in it. If you want to switch this behavior off, set this variable to
    // true.

    repetitionTimeLabel = new QLabel(tr("Auto measure temperature each:"));
    repetitionTimeSpinBox = new QDoubleSpinBox;
    repetitionTimeSpinBox->setMinimum(1.0);
    repetitionTimeSpinBox->setDecimals(1);
    repetitionTimeSpinBox->setSingleStep(1.0);
    repetitionTimeSpinBox->setValue((rv.repetitionTime + 500) / 1000);
    repetitionTimeSpinBox->setSuffix(tr(" s"));
    repetitionTimeLabel->setBuddy(repetitionTimeSpinBox);

    saveTemperaturesGroupBox = new QGroupBox(tr("Autosave temperatures"));
    saveTemperaturesGroupBox->setCheckable(true);
    saveTemperaturesGroupBox->setChecked(rv.saveTemperatures);
    saveRepetitionTimeLabel1 = new QLabel(tr("Save each"));
    saveRepetitionTimeLabel2 = new QLabel(tr("temperature"));
    saveRepetitionTimeSpinBox = new QSpinBox;
    saveRepetitionTimeSpinBox->setMinimum(1);
    saveRepetitionTimeSpinBox->setValue(rv.saveRepetitionTime);
    saveRepetitionTimeLabel1->setBuddy(saveRepetitionTimeSpinBox);
    filenameLabel = new QLabel(tr("Enter the filename:"));
    filenameEdit = new QLineEdit;
    filenameEdit->setText(rv.fileName);
    filenameEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    filenameLabel->setBuddy(filenameEdit);
    filenameButton = new QPushButton(tr("..."));
    QSize textSize = filenameButton->fontMetrics().size(Qt::TextShowMnemonic, filenameButton->text());
    QStyleOptionButton opt;
    opt.initFrom(filenameButton);
    opt.rect.setSize(textSize);
    filenameButton->setFixedWidth(filenameButton->style()->sizeFromContents(QStyle::CT_PushButton,
                                                                            &opt,
                                                                            textSize,
                                                                            filenameButton)
                                  .width());

    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

    connect(buttonBox, &QDialogButtonBox::accepted, this, &AutoReadT::Dialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &AutoReadT::Dialog::reject);
    connect(filenameEdit, &QLineEdit::editingFinished, this, &AutoReadT::Dialog::onFilenameEditEditingFinished);
    connect(filenameButton, &QPushButton::clicked, this, &AutoReadT::Dialog::onFilenameButtonClicked);
    connect(saveTemperaturesGroupBox, &QGroupBox::clicked, this, &AutoReadT::Dialog::onSaveTemperaturesGroupBoxClicked);

    mainVLayout = new QVBoxLayout;
    mainVLayout->setSizeConstraint(QLayout::SetFixedSize);

    repetitionTimeHLayout = new QHBoxLayout;
    repetitionTimeHLayout->addStretch();
    repetitionTimeHLayout->addWidget(repetitionTimeLabel);
    repetitionTimeHLayout->addWidget(repetitionTimeSpinBox);
    mainVLayout->addLayout(repetitionTimeHLayout);

    saveTemperaturesVLayout = new QVBoxLayout;

    saveRepetitionTimeHLayout = new QHBoxLayout;
    saveRepetitionTimeHLayout->addStretch();
    saveRepetitionTimeHLayout->addWidget(saveRepetitionTimeLabel1);
    saveRepetitionTimeHLayout->addWidget(saveRepetitionTimeSpinBox);
    saveRepetitionTimeHLayout->addWidget(saveRepetitionTimeLabel2);
    saveTemperaturesVLayout->addLayout(saveRepetitionTimeHLayout);

    filenameVLayout = new QVBoxLayout;
    filenameVLayout->addWidget(filenameLabel, 0, Qt::AlignLeft);
    filenameHLayout = new QHBoxLayout;
    filenameHLayout->addWidget(filenameEdit);
    filenameHLayout->addWidget(filenameButton);
    filenameVLayout->addLayout(filenameHLayout);
    saveTemperaturesVLayout->addLayout(filenameVLayout);

    saveTemperaturesGroupBox->setLayout(saveTemperaturesVLayout);

    mainVLayout->addWidget(saveTemperaturesGroupBox);

    mainVLayout->addWidget(buttonBox);

    setLayout(mainVLayout);
}

void NeslabusWidgets::AutoReadT::Dialog::keyPressEvent(QKeyEvent *e)
{
    if(e->key() == Qt::Key_Enter || e->key() == Qt::Key_Return)
        return;
    QDialog::keyPressEvent(e);
}

const AutoReadT::Settings &AutoReadT::Dialog::getValues()
{
    return returnValues;
}

void AutoReadT::Dialog::accept()
{
    returnValues.repetitionTime = repetitionTimeSpinBox->value() * 1000 + 0.5;
    if ((returnValues.saveTemperatures = saveTemperaturesGroupBox->isChecked())) {
        if (!filenameEdit->text().isEmpty()) {
            QFileInfo newFileInfo(filenameEdit->text());
            if (newFileInfo.isRelative()) {
                newFileInfo.setFile(returnValues.directory % QDir::separator() % filenameEdit->text());
            }
            QFileInfo fileInfo(returnValues.directory % QDir::separator() % returnValues.fileName);
            if (fileInfo.absoluteFilePath() != newFileInfo.absoluteFilePath() || saveTemperaturesChanged) {
                if (newFileInfo.exists()) {
                    int ret = QMessageBox::question(this, tr("Overwrite file?"), tr("The file %1 already exists. Do you want to overwrite it?")
                                                    .arg(newFileInfo.absoluteFilePath()), QMessageBox::Yes | QMessageBox::No);
                    if (ret == QMessageBox::Yes) {
                        if (newFileInfo.isWritable()) {
                            returnValues.directory = QDir::toNativeSeparators(newFileInfo.absolutePath());
                            returnValues.fileName = newFileInfo.fileName();
                        } else {
                            QMessageBox::critical(this, tr("Can't write!"), tr("Can't write to the file %1!").arg(newFileInfo.absoluteFilePath()));
                            return;
                        }
                    } else {
                        return;
                    }
                } else if (!newFileInfo.absoluteDir().exists()) {
                    int ret = QMessageBox::question(this, tr("Create folder?"), tr("The folder %1 does not exist. Do you want to create it?")
                                              .arg(newFileInfo.absoluteFilePath()), QMessageBox::Yes | QMessageBox::No);
                    if (ret == QMessageBox::Yes) {
                        QDir dir;
                        if (dir.mkpath(newFileInfo.absolutePath())) {
                            returnValues.directory = QDir::toNativeSeparators(newFileInfo.absolutePath());
                            returnValues.fileName = newFileInfo.fileName();
                        } else {
                            QMessageBox::warning(this, tr("Invalid path!"), tr("Enter a valid path to the file."));
                            return;
                        }
                    } else {
                        return;
                    }
                }
            } else {
                if (newFileInfo.exists() && !newFileInfo.isWritable()) {
                    QMessageBox::critical(this, tr("Can't write!"), tr("Can't write to the file %1!").arg(newFileInfo.absoluteFilePath()));
                    return;
                }
            }
            returnValues.saveRepetitionTime = saveRepetitionTimeSpinBox->value();
        } else {
            QMessageBox::warning(this, tr("Invalid file name!"), tr("Enter a valid file name."));
            return;
        }
    }
    QDialog::accept();
}

void AutoReadT::Dialog::onFilenameEditEditingFinished()
{
    if (filenameEdit->hasFocus()) {
        if (!filenameEdit->text().isEmpty()) {
            QFileInfo newFileInfo(filenameEdit->text());
            bool relative;
            if (newFileInfo.isRelative()) {
                newFileInfo.setFile(returnValues.directory % QDir::separator() % filenameEdit->text());
                relative = true;
            }
            if (newFileInfo.exists()) {
                int ret = QMessageBox::question(this, tr("Overwrite file?"), tr("The file %1 already exists. Do you want to overwrite it?")
                                          .arg(newFileInfo.absoluteFilePath()), QMessageBox::Yes | QMessageBox::No);
                if (ret == QMessageBox::Yes) {
                    if (relative) {
                        returnValues.fileName = QDir::toNativeSeparators(QFileInfo(filenameEdit->text()).filePath());
                    } else {
                        returnValues.directory = QDir::toNativeSeparators(newFileInfo.absolutePath());
                        returnValues.fileName = newFileInfo.fileName();
                    }
                }
            } else if (!newFileInfo.absoluteDir().exists()) {
                int ret = QMessageBox::question(this, tr("Create folder?"), tr("The folder %1 does not exist. Do you want to create it?")
                                          .arg(newFileInfo.absoluteFilePath()), QMessageBox::Yes | QMessageBox::No);
                if (ret == QMessageBox::Yes) {
                    QDir dir;
                    if (dir.mkpath(newFileInfo.absolutePath())) {
                        if (relative) {
                            returnValues.fileName = QDir::toNativeSeparators(QFileInfo(filenameEdit->text()).filePath());
                        } else {
                            returnValues.directory = QDir::toNativeSeparators(newFileInfo.absolutePath());
                            returnValues.fileName = newFileInfo.fileName();
                        }
                    } else {
                        QMessageBox::warning(this, tr("Invalid path!"), tr("Enter a valid path to the file."));
                    }
                }
            } else {
                if (relative) {
                    returnValues.fileName = QDir::toNativeSeparators(QFileInfo(filenameEdit->text()).filePath());
                } else {
                    returnValues.directory = QDir::toNativeSeparators(newFileInfo.absolutePath());
                    returnValues.fileName = newFileInfo.fileName();
                }
            }
            saveTemperaturesChanged = false;
        } else {
            QMessageBox::warning(this, tr("Invalid file name!"), tr("Enter a valid file name."));
        }
    }
}

void AutoReadT::Dialog::onFilenameButtonClicked()
{
    QFileInfo fileInfo(returnValues.directory % QDir::separator() % returnValues.fileName);
    QString filename = QFileDialog::getSaveFileName(this, tr("Open File"), fileInfo.filePath(), tr("Dat files (*.dat);;All files (*.*)"));

    if (!filename.isEmpty()) {
        QFileInfo newFileInfo(filename);
        if (newFileInfo.exists()) {
            returnValues.directory = QDir::toNativeSeparators(newFileInfo.absolutePath());
            returnValues.fileName = newFileInfo.fileName();
        }
        returnValues.directory = QDir::toNativeSeparators(newFileInfo.absolutePath());
        returnValues.fileName = newFileInfo.fileName();
        if ((filenameEdit->text() != newFileInfo.completeBaseName()) ||
                (filenameEdit->text() != newFileInfo.fileName())) {
            filenameEdit->setText(newFileInfo.fileName());
        }
        saveTemperaturesChanged = false;
    }
}

void AutoReadT::Dialog::onSaveTemperaturesGroupBoxClicked()
{
    if (saveTemperaturesGroupBox->isChecked()) {
        saveTemperaturesChanged = true;
    }
}

AutoReadT::Dialog::~Dialog()
{
}
