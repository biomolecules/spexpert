#include "mainwindow.h"

#include <biomolecules/sprelay/core/k8090.h>
#include <biomolecules/sprelay/core/k8090_defines.h>

#include "appcore.h"
#include "appstate.h"
#include "neslabusmainwidget.h"
#include "centralwidget.h"
#include "experimentsetup.h"
#include "relay_options_widgets.h"
#include "relay.h"
#include "stagesetup.h"
#include "stagecontrol.h"
#include <QCoreApplication>
#include <QSettings>
#include <QMenuBar>
#include <QStatusBar>
#include <QMessageBox>
#include <QAction>
#include <QDir>
#include <QCloseEvent>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent), relayControlPanel_{nullptr}, relaySettingsDialog_{nullptr}
{
    QSettings settings(QCoreApplication::applicationDirPath() + "/" + QCoreApplication::applicationName() + ".ini",QSettings::IniFormat);
    settings.beginGroup("MainApp");
    defaultLanguage = settings.value("language").toString();
    settings.endGroup();

    centralWidget = new CentralWidget(this);

//    statusBar()->setSizeGripEnabled(false);
//    statusBar()->setStyleSheet("QStatusBar::item {border: none;}");
    statusBar()->setStyleSheet("QStatusBar {border-top: 1px solid gray;}");

    neslabDialogWindow = nullptr;

    appCore = new AppCore(centralWidget, statusBar(), this, this);
    setCentralWidget(centralWidget);

    createActions();
    createMenus();
    readSettings();

    connect(centralWidget, &CentralWidget::startExperiment,
            appCore, &AppCore::startExperiment);
    connect(centralWidget, &CentralWidget::stopExperiment,
            appCore, &AppCore::stopExperiment);
    connect(centralWidget, &CentralWidget::goToMeas,
            appCore, &AppCore::onGoToMeas);
    connect(centralWidget, &CentralWidget::goToCal,
            appCore, &AppCore::onGoToCal);
    connect(appCore, &AppCore::experimentStarted,
            centralWidget, &CentralWidget::onExperimentStarted);
    connect(appCore, &AppCore::experimentStarted,
            this, &MainWindow::onExperimentStarted);
    connect(appCore, &AppCore::experimentFinished,
            centralWidget, &CentralWidget::onExperimentFinished);
    connect(appCore, &AppCore::experimentFinished,
            this, &MainWindow::onExperimentFinished);
    connect(appCore, &AppCore::startReadingTemperature,
            this, &MainWindow::onStartReadingTemperature);
    connect(appCore, &AppCore::finishReadingTemperature,
            this, &MainWindow::onFinishReadingTemperature);
    connect(appCore->appState()->k8090(), &biomolecules::sprelay::core::k8090::K8090::connected,
            this, &MainWindow::onK8090Connected);

    setWindowTitle(tr("SpExpert"));
}

void MainWindow::onStartReadingTemperature()
{
    if (!neslabDialogWindow) {
        neslabDialogWindow =
                new NeslabusWidgets::MainDialogWindow(appCore->appState()->neslab(),
                                                      &neslabComPortName,
                                                      appCore->appState()->autoReadTSettings());
    }
    neslabDialogWindow->onStartAutoT();
}

void MainWindow::onFinishReadingTemperature()
{
    if (neslabDialogWindow) {
        neslabDialogWindow->onFinishAutoT();
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    delete neslabDialogWindow;

    QSettings settings(QCoreApplication::applicationDirPath() + "/" + QCoreApplication::applicationName() + ".ini",QSettings::IniFormat);
    settings.beginGroup("MainApp");
    settings.setValue("language", defaultLanguage);
    settings.endGroup();
    event->accept();

    settings.beginGroup("Experiment");
    QList<AppStateTraits::ExpWinSpecParams> &expe =
            appCore->appState()->initWinSpecParams()->expe;

    settings.beginWriteArray("Expe", expe.size());
    for (int ii = 0; ii < expe.size(); ++ii) {
        settings.setArrayIndex(ii);
        settings.setValue("fileNameBase", expe[ii].fileNameBase);
        settings.setValue("directory", expe[ii].directory);
        settings.setValue("grPos", expe[ii].grPos);
        settings.setValue("expo", expe[ii].expo);
        settings.setValue("acc", expe[ii].acc);
        settings.setValue("frm", expe[ii].frm);
    }
    settings.endArray();

    QList<AppStateTraits::CalWinSpecParams> &cal =
            appCore->appState()->initWinSpecParams()->cal;

    settings.beginWriteArray("Cal", cal.size());
    for (int ii = 0; ii < cal.size(); ++ii) {
        settings.setArrayIndex(ii);
        settings.setValue("eachMeas", cal[ii].eachMeas);
        settings.setValue("autoCal", cal[ii].autoCal);
        settings.setValue("expo", cal[ii].expo);
        settings.setValue("acc", cal[ii].acc);
        settings.setValue("frm", cal[ii].frm);
        settings.setValue("enableLampSwitch", cal[ii].enableLampSwitch);
    }
    settings.endArray();

    settings.beginGroup("BatchExp");
    AppStateTraits::BatchExpParams &batch =
            appCore->appState()->initWinSpecParams()->batch;
    settings.setValue("batchExp", batch.batchExp);
    settings.setValue("numSpectra", batch.numSpectra);
    settings.setValue("firstNum", batch.firstNum);
    settings.setValue("step", batch.step);
    settings.setValue("numDigits", batch.numDigits);
    settings.setValue("delayMeas", batch.delayMeas);
    settings.setValue("delay",batch.delay.toMSec());
    settings.endGroup();

    settings.beginGroup("TExp");
    AppStateTraits::TExpParams &tExp =
            appCore->appState()->initWinSpecParams()->tExp;
    settings.setValue("tExp",tExp.tExp);
    settings.setValue("startT", tExp.startT);
    settings.setValue("stepT", tExp.stepT);
    settings.setValue("endT", tExp.endT);
    settings.setValue("afterMeas", tExp.afterMeas);
    settings.setValue("afterMeasT", tExp.afterMeasT);
    settings.setValue("sameAsT", tExp.sameAsT);
    settings.setValue("firstNum", tExp.firstNum);
    settings.setValue("step", tExp.step);
    settings.setValue("numDigits", tExp.numDigits);
    settings.setValue("initDelayMeas", tExp.initDelayMeas);
    settings.setValue("initDelay",tExp.initDelay.toMSec());
    settings.setValue("delayMeas", tExp.delayMeas);
    settings.setValue("delay",tExp.delay.toMSec());
    settings.setValue("loop", tExp.loop);
    settings.setValue("loopDelay",tExp.loopDelay.toMSec());
    settings.endGroup();

    settings.beginGroup("ExtRan");
    AppStateTraits::ExtendedRangeParams &extRan =
            appCore->appState()->initWinSpecParams()->extRan;
    settings.setValue("extendedRange", extRan.extendedRange);
    settings.setValue("autoFileNames", extRan.autoFileNames);
    settings.setValue("fileNameBase", extRan.fileNameBase);
    settings.setValue("directory", extRan.directory);
    settings.endGroup();

    settings.endGroup();

    NeslabusWidgets::AutoReadT::Settings* autoReadTSettings(appCore->appState()->autoReadTSettings());

    settings.beginGroup("Neslabus");
    settings.setValue("COMPort", neslabComPortName);
    settings.setValue("autoReadT", autoReadTSettings->autoReadT);
    settings.setValue("autoReadRepetitionTime", autoReadTSettings->repetitionTime);
    settings.setValue("autoSaveRepetitionTime", autoReadTSettings->saveRepetitionTime);
    settings.setValue("autoSaveDirectory", autoReadTSettings->directory);
    settings.setValue("autoSaveFileName", autoReadTSettings->fileName);
    settings.endGroup();

    StageControlTraits::Params *stageParams = appCore->appState()->stageParams();
    settings.beginGroup("Stage");
    settings.setValue("currPosRefType", static_cast<int>(stageParams->currPosRefType));
    settings.setValue("initType", static_cast<int>(stageParams->initType));
    settings.setValue("range", stageParams->range);
    settings.setValue("measPos", stageParams->measPos);
    settings.setValue("measPosRefType", static_cast<int>(stageParams->measRefType));
    settings.setValue("measInitType", static_cast<int>(stageParams->measInitType));
    settings.setValue("calPos", stageParams->calPos);
    settings.setValue("calPosRefType", static_cast<int>(stageParams->calRefType));
    settings.setValue("calInitType", static_cast<int>(stageParams->calInitType));
    settings.endGroup();

    settings.beginGroup("relay");
    biomolecules::spexpert::relay::Settings* relaySettings = appCore->appState()->relaySettings();
    settings.setValue("COMPort", k8090ComPortName);
    unsigned int relay_id_repr;
    for (relay_id_repr = 0; relay_id_repr < 8; ++relay_id_repr) {
        if ((relaySettings->calibration_lamp_switch_id
            & static_cast<biomolecules::sprelay::core::k8090::RelayID>(1u << relay_id_repr))
            != biomolecules::sprelay::core::k8090::RelayID::None) {
            break;
        }
    }
    if (relay_id_repr > 7) {
        relay_id_repr = 0;
    }
    settings.setValue("calibrationLampSwitchId", relay_id_repr);
    if (relaySettings->calibration_lamp_switch_on) {
        settings.setValue("calibrationLampSwitchAction",
            static_cast<unsigned int>(biomolecules::sprelay::core::k8090::CommandID::RelayOn));
    } else {
        settings.setValue("calibrationLampSwitchAction",
            static_cast<unsigned int>(biomolecules::sprelay::core::k8090::CommandID::RelayOff));
    }
    settings.setValue("calibrationLampSwitchDelayMSec", relaySettings->calibration_lamp_switch_delay_msec);
    settings.endGroup();
}

void MainWindow::onLanguageChanged(QAction *action)
{
    if (defaultLanguage != action->data().toString()) {
        defaultLanguage = action->data().toString();
        QMessageBox::warning(this, tr("Restart required!"), tr("The language change will take effect after a restart of application."), QMessageBox::Ok);
    }
}

void MainWindow::onExperimentActionTrigered()
{
    ExperimentSetup experimentSetup(appCore->appState(), this);
    connect(&experimentSetup, &ExperimentSetup::initStage,
            appCore, &AppCore::onInitStage);
    connect(appCore, &AppCore::initStageFinished,
            &experimentSetup, &ExperimentSetup::onInitStageFinished);
    if (experimentSetup.exec()) {

    }
}

void MainWindow::onStageSetupActionTrigered()
{
    StageSetup stageSetup(appCore->appState(), this);
    if (stageSetup.exec()) {

    }
}

void MainWindow::onNeslabSetupActionTrigered()
{
    if (!neslabDialogWindow) {
        neslabDialogWindow =
                new NeslabusWidgets::MainDialogWindow(appCore->appState()->neslab(),
                                                      &neslabComPortName,
                                                      appCore->appState()->autoReadTSettings());
    }
    neslabDialogWindow->show();
    neslabDialogWindow->raise();
    neslabDialogWindow->activateWindow();
}

void MainWindow::onRelayControlPanelActionTrigered()
{
    if (!relayControlPanel_) {
        relayControlPanel_ =
            new biomolecules::spexpert::gui::RelayControlPanel(
                appCore->appState()->k8090(),
                k8090ComPortName,
                this);
    }
    relayControlPanel_->show();
    relayControlPanel_->raise();
    relayControlPanel_->activateWindow();
}

void MainWindow::onRelaySettingsDialogActionTrigered()
{
    if (!relaySettingsDialog_) {
        relaySettingsDialog_ =
            new biomolecules::spexpert::gui::RelaySettingsDialog(
                appCore->appState(),
                this);
    }
    relaySettingsDialog_->show();
    relaySettingsDialog_->raise();
    relaySettingsDialog_->activateWindow();
}

void MainWindow::onExperimentStarted()
{
    experimentAction->setEnabled(false);
    stageSetupAction->setEnabled(false);
}

void MainWindow::onExperimentFinished()
{
    experimentAction->setEnabled(true);
    stageSetupAction->setEnabled(true);
}

void MainWindow::onK8090Connected()
{
    k8090ComPortName = appCore->appState()->k8090()->comPortName();
}

void MainWindow::createActions()
{
    exitAction = new QAction(tr("E&xit"), this);
    exitAction->setShortcut(tr("Ctrl+Q"));
    exitAction->setStatusTip(tr("Exit the application"));
    connect(exitAction, &QAction::triggered, this, &MainWindow::close);


    experimentAction = new QAction(tr("&Experiment setup"), this);
    experimentAction->setStatusTip(tr("Open experiment setup."));
    connect(experimentAction, &QAction::triggered, this, &MainWindow::onExperimentActionTrigered);

    stageSetupAction = new QAction(tr("St&age settings"), this);
    stageSetupAction->setStatusTip(tr("Open stage settings."));
    connect(stageSetupAction, &QAction::triggered, this, &MainWindow::onStageSetupActionTrigered);

    neslabSetupAction = new QAction(tr("Nes&lab settings"), this);
    neslabSetupAction->setStatusTip(tr("Open Neslab bath and temperature acquisition settings."));
    connect(neslabSetupAction, &QAction::triggered, this, &MainWindow::onNeslabSetupActionTrigered);

    relayControlPanelAction_ = new QAction{tr("&Control Panel"), this};
    relayControlPanelAction_->setStatusTip(tr("Relay card control panel."));
    connect(relayControlPanelAction_, &QAction::triggered, this, &MainWindow::onRelayControlPanelActionTrigered);

    relaySettingsDialogAction_ = new QAction{tr("&Settings"), this};
    relaySettingsDialogAction_->setStatusTip(tr("Relay settings."));
    connect(relaySettingsDialogAction_, &QAction::triggered, this, &MainWindow::onRelaySettingsDialogActionTrigered);
}

void MainWindow::createMenus()
{
    fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addSeparator();
    fileMenu->addAction(exitAction);

    setupMenu = menuBar()->addMenu(tr("&Setup"));
    setupMenu->addAction(experimentAction);
    setupMenu->addAction(stageSetupAction);
    setupMenu->addAction(neslabSetupAction);
    QMenu* relayMenu = setupMenu->addMenu(tr("&Relay"));
    relayMenu->addAction(relayControlPanelAction_);
    relayMenu->addAction(relaySettingsDialogAction_);

    createLanguageMenu();
}

void MainWindow::createLanguageMenu()
{
    languageMenu = menuBar()->addMenu(tr("&Language"));

    langActionGroup = new QActionGroup(languageMenu);
    langActionGroup->setExclusive(true);

    connect(langActionGroup, &QActionGroup::triggered, this, &MainWindow::onLanguageChanged);
//    connect(langActionGroup, SIGNAL(triggered(QAction*)), this, SLOT(onLanguageChanged(QAction*)));

    // format language
    QString defaultLocale = QLocale().name(); // e.g. "de_DE"
    defaultLocale.truncate(defaultLocale.lastIndexOf('_')); // e.g. "de"

    langPath = QCoreApplication::applicationDirPath() + "/languages";
    QDir dir(langPath);
    QStringList fileNames = dir.entryList(QStringList("spexpert_*.qm"));

    for (int i = 0; i < fileNames.size(); ++i) {
        // get locale extracted by filename
        QString locale;
        locale = fileNames[i]; // "spexpert_de.qm"
        locale.truncate(locale.lastIndexOf('.')); // "spexpert_de"
        locale.remove(0, locale.indexOf('_') + 1); // "de"
        QString lang = QLocale::languageToString(QLocale(locale).language());
//        QIcon ico(QString("%1/%2.png").arg(m_langPath).arg(locale));

//        QAction *action = new QAction(ico, lang, this);
        QAction *action = new QAction(lang, this);
        action->setCheckable(true);
        action->setData(locale);

        languageMenu->addAction(action);
        langActionGroup->addAction(action);

        // set default translators and language checked
        if (defaultLocale == locale)
        {
            action->setChecked(true);
        }
    }
}

void MainWindow::readSettings()
{
    QSettings settings(QCoreApplication::applicationDirPath() + "/" + QCoreApplication::applicationName() + ".ini",QSettings::IniFormat);

    settings.beginGroup("Experiment");
    bool ok;
    int expeSize = settings.beginReadArray("Expe");
    settings.endArray();
    int calSize = settings.beginReadArray("Cal");
    settings.endArray();
    if (calSize > expeSize) {
        calSize = expeSize;
    } else {
        expeSize = calSize;
    }
    settings.beginReadArray("Expe");
    QList<AppStateTraits::ExpWinSpecParams> &expe =
            appCore->appState()->initWinSpecParams()->expe;
    if (expeSize) {
        expe.clear();
        AppStateTraits::ExpWinSpecParams params;
        for (int ii = 0; ii < expeSize; ++ii) {
            settings.setArrayIndex(ii);
            params.fileNameBase = settings.value("fileNameBase", "temp").toString();
            params.directory = settings.value("directory", "").toString();
            params.grPos = settings.value("grPos", 0).toInt(&ok);
            if (!ok || params.grPos < 1)
                params.grPos = 0;
            params.expo = settings.value("expo", 1.0).toDouble(&ok);
            if (!ok || params.expo < 0.01)
                params.expo = 1.0;
            params.acc = settings.value("acc", 1).toInt(&ok);
            if (!ok || params.acc < 1)
                params.acc = 1;
            params.frm = settings.value("frm", 1).toInt(&ok);
            if (!ok || params.frm < 1)
                params.frm = 1;
            expe.insert(ii, params);
        }
    } else {
        expe.clear();
        AppStateTraits::ExpWinSpecParams params;
        params.grPos = 0;
        params.fileNameBase = "temp";
        params.directory = "";
        params.expo = 1.0;
        params.acc = 1;
        params.frm = 1;
        expe.append(params);
    }
    settings.endArray();

    settings.beginReadArray("Cal");
    QList<AppStateTraits::CalWinSpecParams> &cal =
            appCore->appState()->initWinSpecParams()->cal;
    if (calSize) {
        cal.clear();
        AppStateTraits::CalWinSpecParams params;
        for (int ii = 0; ii < calSize; ++ii) {
            settings.setArrayIndex(ii);
            params.eachMeas = settings.value("eachMeas", 1).toInt(&ok);
            if (!ok || params.eachMeas < 1)
                params.eachMeas = 1;
            params.autoCal = settings.value("autoCal", true).toBool();
            params.expo = settings.value("expo", 1.0).toDouble(&ok);
            if (!ok || params.expo < 0.01)
                params.expo = 1.0;
            params.acc = settings.value("acc", 1).toInt(&ok);
            if (!ok || params.acc < 1)
                params.acc = 1;
            params.frm = settings.value("frm", 1).toInt(&ok);
            if (!ok || params.frm < 1)
                params.frm = 1;
            params.enableLampSwitch = settings.value("enableLampSwitch", true).toBool();
            cal.insert(ii, params);
        }
    } else {
        cal.clear();
        AppStateTraits::CalWinSpecParams params;
        params.eachMeas = 1;
        params.autoCal = true;
        params.expo = 1.0;
        params.acc = 1;
        params.frm = 1;
        params.enableLampSwitch = true;
        cal.append(params);
    }
    settings.endArray();

    settings.beginGroup("BatchExp");
    AppStateTraits::BatchExpParams &batch =
            appCore->appState()->initWinSpecParams()->batch;
    batch.batchExp = settings.value("batchExp", false).toBool();
    batch.numSpectra = settings.value("numSpectra", 1).toInt(&ok);
    if (!ok || batch.numSpectra < 1)
        batch.numSpectra = 1;
    batch.firstNum = settings.value("firstNum", 1).toInt(&ok);
    if (!ok || batch.firstNum < 1)
        batch.firstNum = 1;
    batch.step = settings.value("step", 1).toInt(&ok);
    if (!ok)
        batch.step = 1;
    batch.numDigits = settings.value("numDigits", 3).toInt(&ok);
    if (!ok || batch.numDigits < 1)
        batch.numDigits = 1;
    batch.delayMeas = settings.value("delayMeas", true).toBool();
    int iDelay = settings.value("delay", 1000).toInt(&ok);
    if (!ok || iDelay < 0)
        batch.delay.fromMSec(1000);
    else
        batch.delay.fromMSec(iDelay);

    settings.endGroup();

    settings.beginGroup("TExp");
    AppStateTraits::TExpParams &tExp =
            appCore->appState()->initWinSpecParams()->tExp;
    tExp.tExp = settings.value("tExp", false).toBool();
    tExp.startT = settings.value("startT", 20.0).toDouble(&ok);
    if (!ok || tExp.startT < -5.0)
        tExp.startT = 20.0;
    tExp.stepT = settings.value("stepT", 5.0).toDouble(&ok);
    if (!ok || tExp.stepT < 0.01)
        tExp.stepT = 5.0;
    tExp.endT = settings.value("endT", 20.0).toDouble(&ok);
    if (!ok || tExp.endT < -5.0)
        tExp.endT = 20.0;
    tExp.afterMeas = settings.value("afterMeas", true).toBool();
    tExp.afterMeasT = settings.value("afterMeasT", 20.0).toDouble(&ok);
    if (!ok || tExp.afterMeasT < -5.0)
        tExp.afterMeasT = 20.0;
    tExp.sameAsT = settings.value("sameAsT", false).toBool();
    tExp.firstNum = settings.value("firstNum", 1).toInt(&ok);
    if (!ok || tExp.firstNum < 1)
        tExp.firstNum = 1;
    tExp.step = settings.value("step", 1).toInt(&ok);
    if (!ok)
        tExp.step = 1;
    tExp.numDigits = settings.value("numDigits", 3).toDouble(&ok);
    if (!ok || tExp.startT < 1)
        tExp.startT = 3;
    tExp.initDelayMeas = settings.value("initDelayMeas", false).toBool();
    iDelay = settings.value("initDelay", 1000).toInt(&ok);
    if (!ok || iDelay < 0)
        tExp.initDelay.fromMSec(1000);
    else
        tExp.initDelay.fromMSec(iDelay);
    tExp.delayMeas = settings.value("delayMeas", true).toBool();
    iDelay = settings.value("delay", 1000).toInt(&ok);
    if (!ok || iDelay < 0)
        tExp.delay.fromMSec(1000);
    else
        tExp.delay.fromMSec(iDelay);
    tExp.loop = settings.value("loop", false).toBool();
    iDelay = settings.value("loopDelay", 1000).toInt(&ok);
    if (!ok || iDelay < 0)
        tExp.loopDelay.fromMSec(1000);
    else
        tExp.loopDelay.fromMSec(iDelay);
    settings.endGroup();

    settings.beginGroup("ExtRan");
    AppStateTraits::ExtendedRangeParams &extRan =
            appCore->appState()->initWinSpecParams()->extRan;
    extRan.extendedRange = settings.value("extendedRange", false).toBool();
    extRan.autoFileNames = settings.value("autoFileNames", true).toBool();
    extRan.fileNameBase = settings.value("fileNameBase", "temp").toString();
    extRan.directory = settings.value("directory", "").toString();
    settings.endGroup();

    settings.endGroup();


    settings.beginGroup("Neslabus");
    neslabComPortName = settings.value("COMPort").toString();
    bool autoReadT = settings.value("autoReadT").toBool();
    int autoReadRepetitionTime =
            settings.value(
                "autoReadRepetitionTime",
                NeslabusWidgets::MainWidget::initAutoReadRepetitionTime)
            .toInt(&ok);
    if (!ok || autoReadRepetitionTime < 1000) {
       autoReadRepetitionTime =
               NeslabusWidgets::MainWidget::initAutoReadRepetitionTime;
    }
    int autoSaveRepetitionTime =
            settings.value(
                "autoSaveRepetitionTime",
                NeslabusWidgets::MainWidget::initAutoSaveRepetitionTime)
            .toInt(&ok);
    if (!ok || autoSaveRepetitionTime < 1000) {
        autoSaveRepetitionTime =
                NeslabusWidgets::MainWidget::initAutoSaveRepetitionTime;
    }
    QString autoSaveDirectory = settings.value("autoSaveDirectory",
                                               "").toString();
    QString autoSaveFileName = settings.value("autoSaveFileName",
                                              "temp").toString();
    settings.endGroup();
    *appCore->appState()->autoReadTSettings() = {autoReadT,
            autoReadRepetitionTime,
            NeslabusWidgets::MainWidget::initAutoSaveTemperatures,
            autoSaveRepetitionTime, autoSaveDirectory, autoSaveFileName};


    settings.beginGroup("Stage");
    int currPosRefType = settings.value("currPosRefType", 0).toInt(&ok);
    if (!ok || currPosRefType < 0 || currPosRefType > 2) {
        currPosRefType = 0;
    }
    appCore->appState()->stageParams()->currPosRefType =
            static_cast<StageControlTraits::ReferenceType>(currPosRefType);

    int initType = settings.value(
                "initType",
                static_cast<int>(StageControlTraits::InitType::LowerLimit))
            .toInt(&ok);
    if (!ok || initType < 1 || initType > ((1 << 1) + 1)) {
        initType = static_cast<int>(StageControlTraits::InitType::LowerLimit);
    }
    appCore->appState()->stageParams()->initType =
            static_cast<StageControlTraits::InitType>(initType);

    int range = settings.value("range", StageControl::defaultStageRange)
            .toInt(&ok);
    if (!ok || range < 1)
        range = StageControl::defaultStageRange;
    appCore->appState()->stageParams()->range = range;

    int measPos = settings.value("measPos", 1000).toInt(&ok);
    if (!ok || measPos < 0)
        measPos = 1000;
    appCore->appState()->stageParams()->measPos = measPos;

    int measRefType = settings.value(
                "measPosRefType",
                static_cast<int>(StageControlTraits::ReferenceType::LowerLimit))
            .toInt(&ok);
    if (!ok || measRefType > 2) {
        measRefType =
                static_cast<int>(StageControlTraits::ReferenceType::LowerLimit);
    }
    appCore->appState()->stageParams()->measRefType =
            static_cast<StageControlTraits::ReferenceType>(measRefType);

    int measInitType = settings.value(
                "measInitType",
                static_cast<int>(StageControlTraits::InitType::None))
            .toInt(&ok);
    if (!ok || measInitType < 0 || measInitType > (1 << 1)) {
        measInitType = static_cast<int>(StageControlTraits::InitType::None);
    }
    appCore->appState()->stageParams()->measInitType =
            static_cast<StageControlTraits::InitType>(measInitType);

    int calPos = settings.value("calPos", 1000).toInt(&ok);
    if (!ok || calPos < 0)
        calPos = 1000;
    appCore->appState()->stageParams()->calPos = calPos;

    int calRefType = settings.value(
                "calPosRefType",
                static_cast<int>(StageControlTraits::ReferenceType::UpperLimit))
            .toInt(&ok);
    if (!ok || calRefType > 2) {
        calRefType =
                static_cast<int>(StageControlTraits::ReferenceType::UpperLimit);
    }
    appCore->appState()->stageParams()->calRefType =
            static_cast<StageControlTraits::ReferenceType>(calRefType);

    int calInitType = settings.value(
                "calInitType",
                static_cast<int>(StageControlTraits::InitType::None))
            .toInt(&ok);
    if (!ok || calInitType < 0 || calInitType > (1 << 1)) {
        calInitType = static_cast<int>(StageControlTraits::InitType::None);
    }
    appCore->appState()->stageParams()->calInitType =
            static_cast<StageControlTraits::InitType>(calInitType);

    appCore->appState()->setStageParamsToStage();
    settings.endGroup();

    settings.beginGroup("relay");
    k8090ComPortName = settings.value("COMPort").toString();
    appCore->appState()->k8090()->setComPortName(k8090ComPortName);

    unsigned int calibrationLampSwitchId = settings.value("calibrationLampSwitchId", 0).toUInt(&ok);
    if (!ok || calibrationLampSwitchId >= 8) {
        calibrationLampSwitchId = 0;
    }
    appCore->appState()->relaySettings()->calibration_lamp_switch_id
        = biomolecules::sprelay::core::k8090::from_number(calibrationLampSwitchId);

    auto calibration_relay_action = static_cast<biomolecules::sprelay::core::k8090::CommandID>(
        settings.value("calibrationLampSwitchAction", 0).toUInt(&ok));
    if (calibration_relay_action == biomolecules::sprelay::core::k8090::CommandID::RelayOff)
        appCore->appState()->relaySettings()->calibration_lamp_switch_on = false;
    else {
        appCore->appState()->relaySettings()->calibration_lamp_switch_on = true;
    }
    unsigned int calSwitchDelayMSec = settings.value("calibrationLampSwitchDelayMSec", 1000).toUInt(&ok);
    if (!ok) {
        calSwitchDelayMSec = 1000;
    }
    appCore->appState()->relaySettings()->calibration_lamp_switch_delay_msec = calSwitchDelayMSec;
    settings.endGroup();
}

MainWindow::~MainWindow()
{
}
