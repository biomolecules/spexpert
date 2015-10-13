#include "experimentsetup.h"

#include "appstate.h"
#include "appcore.h"
#include "stagecontrol.h"
#include "exptasks.h"
#include "neslab.h"

#include <QStringBuilder>
#include <QTabWidget>
#include <QDialogButtonBox>
#include <QVBoxLayout>

#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QDoubleSpinBox>
#include <QGroupBox>
#include <QCheckBox>
#include <QStyle>
#include <QStyleOptionButton>
#include <QFileInfo>
#include <QDir>
#include <QMessageBox>
#include <QFileDialog>
#include <QKeyEvent>

#include <QHeaderView>

#include <QDebug>

ExpSettingsTab::ExpSettingsTab(AppStateTraits::InitWinSpecParams &params, AppState *appState, bool extRange, int expNumber, QWidget *parent) :
    QWidget(parent), params_(params), appState_(appState), extRange_(extRange), expNumber_(expNumber)
{
    QWidget *expSettingsWidget = new QWidget;

    QLabel *filenameLabel = new QLabel(tr("Enter the filename base:"));
    filenameEdit = new QLineEdit;
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
    if (extRange_ && params_.extRan.autoFileNames) {
        filenameEdit->setEnabled(false);
        filenameEdit->setText(QString(params_.extRan.fileNameBase % "_%1_").arg(expNumber_));
        filenameButton->setEnabled(false);
    } else {
        filenameEdit->setText(params_.expe.at(expNumber_).fileNameBase);
    }

    expGroupBox = new QGroupBox(tr("Experiment"));

    QLabel *expoLabel = new QLabel(tr("Exposure:"));
    expoSpinBox = new QDoubleSpinBox;
    expoSpinBox->setMinimum(0.01);
    expoSpinBox->setDecimals(2);
    expoSpinBox->setSingleStep(1.0);
    expoSpinBox->setValue(params_.expe.at(expNumber).expo);
    expoSpinBox->setSuffix(tr(" s"));
    expoLabel->setBuddy(expoSpinBox);

    QLabel *accLabel = new QLabel(tr("Accumulations:"));
    accSpinBox = new QSpinBox;
    accSpinBox->setMinimum(1);
    accSpinBox->setValue(params_.expe.at(expNumber).acc);
    accLabel->setBuddy(accSpinBox);

    QLabel *frmLabel = new QLabel(tr("Frames:"));
    frmSpinBox = new QSpinBox;
    frmSpinBox->setMinimum(1);
    frmSpinBox->setValue(params_.expe.at(expNumber).frm);
    frmLabel->setBuddy(frmSpinBox);

    QLabel *grPosLabel;
    if (extRange_) {
        grPosLabel = new QLabel(tr("Grating pos.:"));
        grPosSpinBox = new QSpinBox;
        grPosSpinBox->setMinimum(0);
        grPosSpinBox->setMaximum(1e7);
        grPosSpinBox->setValue(params_.expe.at(expNumber).grPos);
        grPosLabel->setBuddy(grPosSpinBox);
    }

    autoCalGroupBox = new QGroupBox(tr("Automatic calibration"));
    autoCalGroupBox->setCheckable(true);
    autoCalGroupBox->setChecked(params_.cal.at(expNumber).autoCal);
    if (!params.extRan.extendedRange) {
        onAutoCalGroupBoxClicked(params_.cal.at(expNumber).autoCal);
    }

//    QLabel *eachMeasLabel = new QLabel(tr("Calibration each:"));
//    eachMeasSpinBox = new QSpinBox;
//    eachMeasSpinBox->setMinimum(1);
//    eachMeasSpinBox->setValue(params_.cal.at(expNumber).eachMeas);
//    eachMeasLabel->setBuddy(eachMeasSpinBox);

    QLabel *calExpoLabel = new QLabel(tr("Exposure:"));
    calExpoSpinBox = new QDoubleSpinBox;
    calExpoSpinBox->setMinimum(0.01);
    calExpoSpinBox->setDecimals(2);
    calExpoSpinBox->setSingleStep(1.0);
    calExpoSpinBox->setValue(params_.cal.at(expNumber).expo);
    calExpoSpinBox->setSuffix(tr(" s"));
    calExpoLabel->setBuddy(calExpoSpinBox);

    QLabel *calAccLabel = new QLabel(tr("Accumulations:"));
    calAccSpinBox = new QSpinBox;
    calAccSpinBox->setMinimum(1);
    calAccSpinBox->setValue(params_.cal.at(expNumber).acc);
    calAccLabel->setBuddy(calAccSpinBox);

    QLabel *calFrmLabel = new QLabel(tr("Frames:"));
    calFrmSpinBox = new QSpinBox;
    calFrmSpinBox->setMinimum(1);
    calFrmSpinBox->setValue(params_.cal.at(expNumber).frm);
    calFrmLabel->setBuddy(calFrmSpinBox);

    connect(filenameButton, &QPushButton::clicked,
            this, &ExpSettingsTab::onFilenameButtonClicked);
    connect(filenameEdit, &QLineEdit::editingFinished,
            this, &ExpSettingsTab::onFilenameEditEditingFinished);


    QHBoxLayout *mainHLayout = new QHBoxLayout;
    setLayout(mainHLayout);
    QVBoxLayout *mainVLayout = new QVBoxLayout;
    mainHLayout->addLayout(mainVLayout);
    mainVLayout->setContentsMargins(0, 0, 0, 0);

    mainVLayout->addWidget(expSettingsWidget);

    QHBoxLayout *widgetHLayout = new QHBoxLayout;
    expSettingsWidget->setLayout(widgetHLayout);
    QVBoxLayout *widgetVLayout = new QVBoxLayout;
    widgetHLayout->addLayout(widgetVLayout);

    QVBoxLayout *vLayout = new QVBoxLayout;
    widgetVLayout->addLayout(vLayout);
    vLayout->addWidget(filenameLabel, 0, Qt::AlignLeft);
    QHBoxLayout *hLayout = new QHBoxLayout;
    vLayout->addLayout(hLayout);
    hLayout->addWidget(filenameEdit);
    hLayout->addWidget(filenameButton);

    widgetVLayout->addWidget(expGroupBox);

    QGridLayout *gLayout = new QGridLayout;
    expGroupBox->setLayout(gLayout);
    gLayout->setColumnStretch(0, 1);

    gLayout->addWidget(expoLabel, 0, 0, Qt::AlignRight);
    gLayout->addWidget(expoSpinBox, 0, 1);

    gLayout->addWidget(accLabel, 1, 0, Qt::AlignRight);
    gLayout->addWidget(accSpinBox, 1, 1);

    gLayout->addWidget(frmLabel, 2, 0, Qt::AlignRight);
    gLayout->addWidget(frmSpinBox, 2, 1);

    widgetVLayout = new QVBoxLayout;
    widgetHLayout->addLayout(widgetVLayout);
    widgetVLayout->addStretch();

    if (extRange) {
        hLayout = new QHBoxLayout;
        widgetVLayout->addLayout(hLayout);
        hLayout->addWidget(grPosLabel, 1, Qt::AlignRight);
        hLayout->addWidget(grPosSpinBox);
    }
    widgetVLayout->addWidget(autoCalGroupBox);

    gLayout = new QGridLayout;
    autoCalGroupBox->setLayout(gLayout);
    gLayout->setColumnStretch(0, 1);

    int rowCount = 0;
//    gLayout->addWidget(eachMeasLabel, rowCount, 0, Qt::AlignRight);
//    gLayout->addWidget(eachMeasSpinBox, rowCount++, 1);

    gLayout->addWidget(calExpoLabel, rowCount, 0, Qt::AlignRight);
    gLayout->addWidget(calExpoSpinBox, rowCount++, 1);

    gLayout->addWidget(calAccLabel, rowCount, 0, Qt::AlignRight);
    gLayout->addWidget(calAccSpinBox, rowCount++, 1);

    gLayout->addWidget(calFrmLabel, rowCount, 0, Qt::AlignRight);
    gLayout->addWidget(calFrmSpinBox, rowCount++, 1);

    mainVLayout->addStretch();

    mainHLayout->addStretch();
}

bool ExpSettingsTab::saveParams()
{
    if (!(extRange_ && params_.extRan.autoFileNames)) {
        if (!filenameEdit->text().isEmpty()) {
            QFileInfo newFileInfo(filenameEdit->text());
            if (newFileInfo.isRelative()) {
                newFileInfo.setFile(params_.expe.at(expNumber_).directory % QDir::separator() % filenameEdit->text());
            }
            QFileInfo fileInfo(params_.expe.at(expNumber_).directory % QDir::separator() % params_.expe.at(expNumber_).fileNameBase);
            if (fileInfo.absoluteFilePath() != newFileInfo.absoluteFilePath()) {
                if (!newFileInfo.absoluteDir().exists()) {
                    int ret = QMessageBox::question(this, tr("Create folder?"), tr("The folder %1 does not exist. Do you want to create it?")
                                                    .arg(newFileInfo.absolutePath()), QMessageBox::Yes | QMessageBox::No);
                    if (ret == QMessageBox::Yes) {
                        QDir dir;
                        if (dir.mkpath(newFileInfo.absolutePath())) {
                            params_.expe[expNumber_].directory = QDir::toNativeSeparators(newFileInfo.absolutePath());
                            params_.expe[expNumber_].fileNameBase = newFileInfo.completeBaseName();
                        } else {
                            QMessageBox::warning(this, tr("Invalid path!"), tr("Enter a valid path to the file."));
                            return false;
                        }
                    } else {
                        return false;
                    }
                } else {
                    params_.expe[expNumber_].directory = QDir::toNativeSeparators(newFileInfo.absolutePath());
                    params_.expe[expNumber_].fileNameBase = newFileInfo.completeBaseName();
                }
            }
        } else {
            QMessageBox::warning(this, tr("Invalid file name!"), tr("Enter a valid file name."));
            return false;
        }
    }
    if (extRange_)
        params_.expe[expNumber_].grPos = grPosSpinBox->value();
    params_.expe[expNumber_].expo = expoSpinBox->value();
    params_.expe[expNumber_].acc = accSpinBox->value();
    params_.expe[expNumber_].frm = frmSpinBox->value();

    params_.cal[expNumber_].autoCal = autoCalGroupBox->isChecked();
    if (params_.cal.at(expNumber_).autoCal) {
//        params_.cal[expNumber_].eachMeas = eachMeasSpinBox->value();
        params_.cal[expNumber_].expo = calExpoSpinBox->value();
        params_.cal[expNumber_].acc = calAccSpinBox->value();
        params_.cal[expNumber_].frm = calFrmSpinBox->value();
    }

    return true;
}

void ExpSettingsTab::onFilenameButtonClicked()
{
    QFileInfo fileInfo(params_.expe.at(expNumber_).directory %
                       QDir::separator() %
                       params_.expe.at(expNumber_).fileNameBase);
    QString filename = QFileDialog::getSaveFileName(
                this, tr("Open File"), fileInfo.filePath(),
                tr("WinSpec files (*.spe);;All files (*.*)"), 0,
                QFileDialog::DontConfirmOverwrite);
    if (!filename.isEmpty()) {
        QFileInfo newFileInfo(filename);
        params_.expe[expNumber_].directory =
                QDir::toNativeSeparators(newFileInfo.absolutePath());
        params_.expe[expNumber_].fileNameBase = newFileInfo.completeBaseName();
        if (filenameEdit->text() != newFileInfo.completeBaseName()) {
            filenameEdit->setText(newFileInfo.completeBaseName());
        }
    }
}

void ExpSettingsTab::onFilenameEditEditingFinished()
{
    if (filenameEdit->hasFocus()) {
        if (!filenameEdit->text().isEmpty()) {
            QFileInfo newFileInfo(filenameEdit->text());
            bool relative;
            if (newFileInfo.isRelative()) {
                newFileInfo.setFile(params_.expe.at(expNumber_).directory % QDir::separator() % filenameEdit->text());
                relative = true;
            }
            if (!newFileInfo.absoluteDir().exists()) {
                int ret = QMessageBox::question(this, tr("Create folder?"), tr("The folder %1 does not exist. Do you want to create it?")
                                          .arg(newFileInfo.absolutePath()), QMessageBox::Yes | QMessageBox::No);
                if (ret == QMessageBox::Yes) {
                    QDir dir;
                    if (dir.mkpath(newFileInfo.absolutePath())) {
                        if (relative) {
                            params_.expe[expNumber_].fileNameBase = QFileInfo(filenameEdit->text()).completeBaseName();
                        } else {
                            params_.expe[expNumber_].directory = QDir::toNativeSeparators(newFileInfo.absolutePath());
                            params_.expe[expNumber_].fileNameBase = newFileInfo.completeBaseName();
                        }
                    } else {
                        QMessageBox::warning(this, tr("Invalid path!"), tr("Enter a valid path to the file."));
                    }
                }
            } else {
                if (relative) {
                    params_.expe[expNumber_].fileNameBase = QFileInfo(filenameEdit->text()).completeBaseName();
                } else {
                    params_.expe[expNumber_].directory = QDir::toNativeSeparators(newFileInfo.absolutePath());
                    params_.expe[expNumber_].fileNameBase = newFileInfo.completeBaseName();
                }
            }
        } else {
            QMessageBox::warning(this, tr("Invalid file name!"), tr("Enter a valid file name."));
        }
    }
}

void ExpSettingsTab::onAutoCalGroupBoxClicked(bool checked)
{
    if (checked) {
        if (!appState_->stageControl()->initialized()){
            int ret = QMessageBox::question(this, tr("Initialize Stage?"), tr("Stage is not initialized yet. Do you want to initialize it?"),
                                            QMessageBox::Yes | QMessageBox::No);
            if (ret == QMessageBox::Yes) {
                if (appState_->stageControl()->motorRunning()) {
                    QMessageBox::critical(this, tr("Stage is already running!"), tr("The stage is already running, stop it, please!"), QMessageBox::Ok);
                } else {
                    qDebug() << "ExpSettingsTab::onAutoCalGroupBoxClicked(): emitting initStage";
                    appState_->appCore()->onInitStage();
                }
            }
        }
    }
}

void ExpSettingsTab::onInitStageFinished()
{
    if (!appState_->stageControl()->initialized()) {
        QMessageBox::critical(this, tr("Stage initialization failed!"), tr("Initialization of stage failed!"), QMessageBox::Ok);
        autoCalGroupBox->setChecked(false);
    }
}

ExpSettingsTab::~ExpSettingsTab()
{
}

BatchExpTab::BatchExpTab(AppStateTraits::InitWinSpecParams &params, QWidget *parent) :
    QWidget(parent), params_(params)
{
    batchExpGroupBox = new QGroupBox(tr("Batch experiment"));
    batchExpGroupBox->setCheckable(true);
    batchExpGroupBox->setChecked(params_.batch.batchExp);

    QLabel *numSpectraLabel = new QLabel(tr("Number of spectra:"));
    numSpectraSpinBox = new QSpinBox;
    numSpectraSpinBox->setMinimum(1);
    numSpectraSpinBox->setMaximum(1e7-1);
    numSpectraSpinBox->setValue(params_.batch.numSpectra);
    numSpectraLabel->setBuddy(numSpectraSpinBox);

    QLabel *firstNumLabel = new QLabel(tr("First number:"));
    firstNumSpinBox = new QSpinBox;
    firstNumSpinBox->setMinimum(-1e7+1);
    firstNumSpinBox->setMaximum(1e7-1);
    firstNumSpinBox->setValue(params_.batch.firstNum);
    firstNumLabel->setBuddy(firstNumSpinBox);

    QLabel *stepLabel = new QLabel(tr("Step:"));
    stepSpinBox = new QSpinBox;
    stepSpinBox->setMinimum(-1e4+1);
    stepSpinBox->setMaximum(1e4-1);
    stepSpinBox->setValue(params_.batch.step);
    stepLabel->setBuddy(stepSpinBox);

    QLabel *numDigitsLabel = new QLabel(tr("Number of digits:"));
    numDigitsSpinBox = new QSpinBox;
    numDigitsSpinBox->setMinimum(1);
    numDigitsSpinBox->setMaximum(20);
    numDigitsSpinBox->setValue(params_.batch.numDigits);
    numDigitsLabel->setBuddy(numDigitsSpinBox);

    delayGroupBox = new QGroupBox(tr("Delay between measuremets"));
    delayGroupBox->setCheckable(true);
    delayGroupBox->setChecked(params_.batch.delayMeas);

    int secs = params_.batch.delay.toSec();
    int mins = secs / 60;
    secs = secs % 60;
    int hours = mins / 60;
    mins = mins % 60;

    hoursSpinBox = new QSpinBox;
    hoursSpinBox->setMinimum(0);
    hoursSpinBox->setMaximum(1000);
    hoursSpinBox->setValue(hours);
    hoursSpinBox->setSuffix(tr(" h"));

    minsSpinBox = new QSpinBox;
    minsSpinBox->setMinimum(0);
    minsSpinBox->setMaximum(60000);
    minsSpinBox->setValue(mins);
    minsSpinBox->setSuffix(tr(" min"));

    secsSpinBox = new QSpinBox;
    secsSpinBox->setMinimum(0);
    secsSpinBox->setMaximum(360000);
    secsSpinBox->setValue(secs);
    secsSpinBox->setSuffix(tr(" s"));

    connect(batchExpGroupBox, &QGroupBox::clicked,
            this, &BatchExpTab::onBatchExpGroupBoxClicked);

    QHBoxLayout *mainHLayout = new QHBoxLayout;
    setLayout(mainHLayout);

    QVBoxLayout *mainVLayout = new QVBoxLayout;
    mainHLayout->addLayout(mainVLayout);
    mainVLayout->addWidget(batchExpGroupBox);

    QHBoxLayout *boxHLayout = new QHBoxLayout;
    batchExpGroupBox->setLayout(boxHLayout);

    QGridLayout *gLayout = new QGridLayout;
    boxHLayout->addLayout(gLayout);
    gLayout->setColumnStretch(0, 1);

    gLayout->addWidget(numSpectraLabel, 0, 0, Qt::AlignRight);
    gLayout->addWidget(numSpectraSpinBox, 0, 1);

    gLayout->addWidget(firstNumLabel, 1, 0, Qt::AlignRight);
    gLayout->addWidget(firstNumSpinBox, 1, 1);

    gLayout->addWidget(stepLabel, 2, 0, Qt::AlignRight);
    gLayout->addWidget(stepSpinBox, 2, 1);

    gLayout->addWidget(numDigitsLabel, 3, 0, Qt::AlignRight);
    gLayout->addWidget(numDigitsSpinBox, 3, 1);

    QVBoxLayout *boxVLayout = new QVBoxLayout;
    boxHLayout->addLayout(boxVLayout);
    boxVLayout->addWidget(delayGroupBox);
    QHBoxLayout *hLayout = new QHBoxLayout;
    delayGroupBox->setLayout(hLayout);

    hLayout->addWidget(hoursSpinBox);
    hLayout->addWidget(minsSpinBox);
    hLayout->addWidget(secsSpinBox);

    boxVLayout->addStretch();

    mainVLayout->addStretch(1);

    mainHLayout->addStretch();
}

bool BatchExpTab::saveParams()
{
    params_.batch.batchExp = batchExpGroupBox->isChecked();
    if (params_.batch.batchExp) {
        params_.batch.numSpectra = numSpectraSpinBox->value();
        params_.batch.firstNum = firstNumSpinBox->value();
        params_.batch.step = stepSpinBox->value();
        if (params_.batch.step == 0) {
            params_.batch.step = 1;
            QMessageBox::warning(this, tr("Step shouldn't be 0!"), tr("The filename counter step shouldn't be 0. It will bee reset to %1").arg(1),
                                 QMessageBox::Ok);
        }
        params_.batch.numDigits = numDigitsSpinBox->value();

        int numDigits1 = ExperimentSetup::countNumDigits(params_.batch.firstNum);
        int numDigits2 = ExperimentSetup::countNumDigits((params_.batch.numSpectra - 1) * params_.batch.step + params_.batch.firstNum);;

        int numDigits;
        if (numDigits1 > numDigits2)
            numDigits = numDigits1;
        else
            numDigits = numDigits2;


        if (numDigits >  params_.batch.numDigits)
        {
            params_.batch.numDigits = numDigits;
            QMessageBox::warning(this, tr("Small number of digits in batch experiment.") ,tr("The number of digits for file name in batch experiment is too small. It will bee increased to %1").arg(numDigits),
                                 QMessageBox::Ok);
        }

        params_.batch.delayMeas = delayGroupBox->isChecked();
        if (params_.batch.delayMeas) {
            params_.batch.delay = TimeSpan(hoursSpinBox->value(), minsSpinBox->value(), secsSpinBox->value());
        }
    }
    return true;
}

void BatchExpTab::onBatchExpGroupBoxClicked(bool checked)
{
    params_.batch.batchExp = checked;
    emit batchExpStateChanged(checked);
}

TSettingsTab::TSettingsTab(AppStateTraits::InitWinSpecParams &params, Neslab *neslab,
                           QWidget *parent) :
    QWidget(parent), neslab_(neslab), params_(params)
{
    gotLowTLim = false;
    gotHighTLim = false;
    tMeasGroupBox = new QGroupBox(tr("Temperature measurement"));
    tMeasGroupBox->setCheckable(true);
    tMeasGroupBox->setChecked(params_.tExp.tExp);
    onTMeasGroupBoxClicked(params_.tExp.tExp);

    startTGroupBox = new QGroupBox(tr("Start temperature"));

    startTSpinBox = new QDoubleSpinBox;
    startTSpinBox->setMinimum(-5.00);
    startTSpinBox->setMaximum(99.00);
    startTSpinBox->setDecimals(2);
    startTSpinBox->setSingleStep(1.0);
    startTSpinBox->setValue(params_.tExp.startT);
    startTSpinBox->setSuffix(tr(" °C"));

    setStartTPushButton = new QPushButton(tr("Set"));

    QLabel *stepTLabel = new QLabel(tr("Temperature step:"));
    stepTSpinBox = new QDoubleSpinBox;
    stepTSpinBox->setMinimum(0.01);
    stepTSpinBox->setDecimals(2);
    stepTSpinBox->setSingleStep(1.0);
    stepTSpinBox->setValue(params_.tExp.stepT);
    stepTSpinBox->setSuffix(tr(" °C"));
    stepTLabel->setBuddy(stepTSpinBox);

    QLabel *endTLabel = new QLabel(tr("End temperature:"));
    endTSpinBox = new QDoubleSpinBox;
    endTSpinBox->setMinimum(-5.00);
    endTSpinBox->setMaximum(99.00);
    endTSpinBox->setDecimals(2);
    endTSpinBox->setSingleStep(1.0);
    endTSpinBox->setValue(params_.tExp.endT);
    endTSpinBox->setSuffix(tr(" °C"));
    endTLabel->setBuddy(stepTSpinBox);

    initDelayGroupBox = new QGroupBox(tr("Initial delay"));
    initDelayGroupBox->setCheckable(true);
    initDelayGroupBox->setChecked(params_.tExp.initDelayMeas);

    int secs = params_.tExp.initDelay.toSec();
    int mins = secs / 60;
    secs = secs % 60;
    int hours = mins / 60;
    mins = mins % 60;

    initHoursSpinBox = new QSpinBox;
    initHoursSpinBox->setMinimum(0);
    initHoursSpinBox->setMaximum(1000);
    initHoursSpinBox->setValue(hours);
    initHoursSpinBox->setSuffix(tr(" h"));

    initMinsSpinBox = new QSpinBox;
    initMinsSpinBox->setMinimum(0);
    initMinsSpinBox->setMaximum(60000);
    initMinsSpinBox->setValue(mins);
    initMinsSpinBox->setSuffix(tr(" min"));

    initSecsSpinBox = new QSpinBox;
    initSecsSpinBox->setMinimum(0);
    initSecsSpinBox->setMaximum(360000);
    initSecsSpinBox->setValue(secs);
    initSecsSpinBox->setSuffix(tr(" s"));

    delayGroupBox = new QGroupBox(tr("Delay between measuremets"));
    delayGroupBox->setCheckable(true);
    delayGroupBox->setChecked(params_.tExp.delayMeas);

    secs = params_.tExp.delay.toSec();
    mins = secs / 60;
    secs = secs % 60;
    hours = mins / 60;
    mins = mins % 60;

    hoursSpinBox = new QSpinBox;
    hoursSpinBox->setMinimum(0);
    hoursSpinBox->setMaximum(1000);
    hoursSpinBox->setValue(hours);
    hoursSpinBox->setSuffix(tr(" h"));

    minsSpinBox = new QSpinBox;
    minsSpinBox->setMinimum(0);
    minsSpinBox->setMaximum(60000);
    minsSpinBox->setValue(mins);
    minsSpinBox->setSuffix(tr(" min"));

    secsSpinBox = new QSpinBox;
    secsSpinBox->setMinimum(0);
    secsSpinBox->setMaximum(360000);
    secsSpinBox->setValue(secs);
    secsSpinBox->setSuffix(tr(" s"));

    loopGroupBox = new QGroupBox(tr("Loop"));
    loopGroupBox->setCheckable(true);
    loopGroupBox->setChecked(params_.tExp.loop);

    secs = params_.tExp.loopDelay.toSec();
    mins = secs / 60;
    secs = secs % 60;
    hours = mins / 60;
    mins = mins % 60;

    loopHoursSpinBox = new QSpinBox;
    loopHoursSpinBox->setMinimum(0);
    loopHoursSpinBox->setMaximum(1000);
    loopHoursSpinBox->setValue(hours);
    loopHoursSpinBox->setSuffix(tr(" h"));

    loopMinsSpinBox = new QSpinBox;
    loopMinsSpinBox->setMinimum(0);
    loopMinsSpinBox->setMaximum(60000);
    loopMinsSpinBox->setValue(mins);
    loopMinsSpinBox->setSuffix(tr(" min"));

    loopSecsSpinBox = new QSpinBox;
    loopSecsSpinBox->setMinimum(0);
    loopSecsSpinBox->setMaximum(360000);
    loopSecsSpinBox->setValue(secs);
    loopSecsSpinBox->setSuffix(tr(" s"));

    afterMeasGroupBox = new QGroupBox(tr("After measurement"));
    afterMeasGroupBox->setCheckable(true);
    afterMeasGroupBox->setChecked(params_.tExp.afterMeas);
    QLabel *afterMeasLabel = new QLabel(tr("Set temperature:"));
    afterMeasSpinBox = new QDoubleSpinBox;
    afterMeasSpinBox->setMinimum(-5.00);
    afterMeasSpinBox->setMaximum(99.00);
    afterMeasSpinBox->setDecimals(2);
    afterMeasSpinBox->setSingleStep(1.0);
    afterMeasSpinBox->setValue(params_.tExp.afterMeasT);
    afterMeasSpinBox->setSuffix(tr(" °C"));
    afterMeasLabel->setBuddy(afterMeasSpinBox);

    filenamesGroupBox = new QGroupBox(tr("Filenames"));

    sameAsTCheckBox = new QCheckBox(tr("temperature"));
    sameAsTCheckBox->setChecked(params_.tExp.sameAsT);

    QLabel *firstNumLabel = new QLabel(tr("First number:"));
    firstNumSpinBox = new QSpinBox;
    firstNumSpinBox->setMinimum(-1e7+1);
    firstNumSpinBox->setMaximum(1e7-1);
    firstNumSpinBox->setValue(params_.tExp.firstNum);
    firstNumLabel->setBuddy(firstNumSpinBox);

    QLabel *stepLabel = new QLabel(tr("Step:"));
    stepSpinBox = new QSpinBox;
    stepSpinBox->setMinimum(-1e4+1);
    stepSpinBox->setMaximum(1e4-1);
    stepSpinBox->setValue(params_.tExp.step);
    stepLabel->setBuddy(stepSpinBox);

    QLabel *numDigitsLabel = new QLabel(tr("Number of digits:"));
    numDigitsSpinBox = new QSpinBox;
    numDigitsSpinBox->setMinimum(1);
    numDigitsSpinBox->setMaximum(20);
    numDigitsSpinBox->setValue(params_.tExp.numDigits);
    numDigitsLabel->setBuddy(numDigitsSpinBox);

    if (params_.tExp.sameAsT) {
        firstNumSpinBox->setEnabled(false);
        stepSpinBox->setEnabled(false);
        numDigitsSpinBox->setEnabled(false);
    }

    connect(tMeasGroupBox, &QGroupBox::clicked,
            this, &TSettingsTab::onTMeasGroupBoxClicked);
    connect(setStartTPushButton, &QPushButton::released,
            this, &TSettingsTab::onSetStartTButtonReleased);
    connect(sameAsTCheckBox, &QCheckBox::stateChanged,
            this, &TSettingsTab::onSameAsTCheckBoxStateChanged);

    QVBoxLayout *mainVLayout = new QVBoxLayout;
    setLayout(mainVLayout);

    mainVLayout->addWidget(tMeasGroupBox);

    QGridLayout *boxGLayout = new QGridLayout;
    tMeasGroupBox->setLayout(boxGLayout);

    QVBoxLayout *vLayout = new QVBoxLayout;
    boxGLayout->addLayout(vLayout, 0, 0);

    vLayout->addWidget(startTGroupBox);
    QHBoxLayout *hLayout = new QHBoxLayout;
    startTGroupBox->setLayout(hLayout);

    hLayout->addStretch();
    hLayout->addWidget(setStartTPushButton);
    hLayout->addWidget(startTSpinBox);

    QGridLayout *gLayout = new QGridLayout;
    vLayout->addLayout(gLayout);
    gLayout->setColumnStretch(0, 1);

    gLayout->addWidget(stepTLabel, 1, 0, Qt::AlignRight);
    gLayout->addWidget(stepTSpinBox, 1, 1);

    gLayout->addWidget(endTLabel, 2, 0, Qt::AlignRight);
    gLayout->addWidget(endTSpinBox, 2, 1);

    vLayout->addStretch();

    vLayout = new QVBoxLayout;
    boxGLayout->addLayout(vLayout, 0, 1);

    vLayout->addWidget(filenamesGroupBox);
    gLayout = new QGridLayout;
    filenamesGroupBox->setLayout(gLayout);
    gLayout->setColumnStretch(0, 1);

    gLayout->addWidget(sameAsTCheckBox, 0, 0, 1, 2, Qt::AlignRight);

    gLayout->addWidget(firstNumLabel, 1, 0, Qt::AlignRight);
    gLayout->addWidget(firstNumSpinBox, 1, 1);

    gLayout->addWidget(stepLabel, 2, 0, Qt::AlignRight);
    gLayout->addWidget(stepSpinBox, 2, 1);

    gLayout->addWidget(numDigitsLabel, 3, 0, Qt::AlignRight);
    gLayout->addWidget(numDigitsSpinBox, 3, 1);

    vLayout->addStretch();

    boxGLayout->addWidget(afterMeasGroupBox, 1, 0);
    hLayout = new QHBoxLayout;
    afterMeasGroupBox->setLayout(hLayout);
    hLayout->addWidget(afterMeasLabel, 1, Qt::AlignRight);
    hLayout->addWidget(afterMeasSpinBox);

    boxGLayout->addWidget(delayGroupBox, 2, 0);
    hLayout = new QHBoxLayout;
    delayGroupBox->setLayout(hLayout);

    hLayout->addWidget(hoursSpinBox);
    hLayout->addWidget(minsSpinBox);
    hLayout->addWidget(secsSpinBox);

    boxGLayout->addWidget(initDelayGroupBox, 1, 1);
    hLayout = new QHBoxLayout;
    initDelayGroupBox->setLayout(hLayout);

    hLayout->addWidget(initHoursSpinBox);
    hLayout->addWidget(initMinsSpinBox);
    hLayout->addWidget(initSecsSpinBox);

    boxGLayout->addWidget(loopGroupBox, 2, 1);
    hLayout = new QHBoxLayout;
    loopGroupBox->setLayout(hLayout);

    hLayout->addWidget(loopHoursSpinBox);
    hLayout->addWidget(loopMinsSpinBox);
    hLayout->addWidget(loopSecsSpinBox);
}

bool TSettingsTab::saveParams()
{
    if (params_.tExp.tExp) {
        params_.tExp.startT = startTSpinBox->value();
        params_.tExp.stepT = stepTSpinBox->value();
        params_.tExp.endT = endTSpinBox->value();
        params_.tExp.initDelayMeas = initDelayGroupBox->isChecked();
        if (params_.tExp.initDelayMeas) {
            params_.tExp.initDelay = TimeSpan(initHoursSpinBox->value(), initMinsSpinBox->value(), initSecsSpinBox->value());
        }
        params_.tExp.delayMeas = delayGroupBox->isChecked();
        if (params_.tExp.delayMeas) {
            params_.tExp.delay = TimeSpan(hoursSpinBox->value(), minsSpinBox->value(), secsSpinBox->value());
        }
        params_.tExp.loop = loopGroupBox->isChecked();
        if (params_.tExp.loop) {
            params_.tExp.loopDelay = TimeSpan(loopHoursSpinBox->value(), loopMinsSpinBox->value(), loopSecsSpinBox->value());
        }
        params_.tExp.afterMeas = afterMeasGroupBox->isChecked();
        if (params_.tExp.afterMeas) {
            params_.tExp.afterMeasT = afterMeasSpinBox->value();
        }
        params_.tExp.sameAsT = sameAsTCheckBox->isChecked();
        if (!params_.tExp.sameAsT) {
            params_.tExp.firstNum = firstNumSpinBox->value();
            params_.tExp.step = stepSpinBox->value();
            if (params_.tExp.step == 0)
            {
                params_.tExp.step = 1;
                QMessageBox::warning(this, tr("Step shouldn't be 0!"), tr("The filename counter step shouldn't be 0. It will bee reset to %1").arg(1),
                                     QMessageBox::Ok);
            }
            params_.tExp.numDigits = numDigitsSpinBox->value();
            int numDigits1 = ExperimentSetup::countNumDigits(params_.tExp.firstNum);
            int sign = ((params_.tExp.endT == params_.tExp.startT) ?
                            0 :
                            (params_.tExp.endT > params_.tExp.startT) ?
                                1 : -1);
            int numTMeas = sign * int((params_.tExp.endT - params_.tExp.startT) /
                    params_.tExp.stepT) + 1;

            int numDigits2 = ExperimentSetup::countNumDigits((numTMeas - 1) * params_.tExp.step + params_.tExp.firstNum);;

            int numDigits;
            if (numDigits1 > numDigits2)
                numDigits = numDigits1;
            else
                numDigits = numDigits2;
            if (numDigits >  params_.tExp.numDigits)
            {
                params_.tExp.numDigits = numDigits;
                QMessageBox::warning(this, tr("Small number of digits in temperature experiment."), tr("The number of digits for file name in temperature experiment is too small. It will bee increased to %1").arg(numDigits),
                                     QMessageBox::Ok);
            }
        }
    }
    return true;
}

void TSettingsTab::onNeslabConnected()
{
    connect(neslab_, &Neslab::readLowTemperatureLimitFinished,
            this, &TSettingsTab::onReadLowTemperatureLimitFinished);
    connect(neslab_, &Neslab::readHighTemperatureLimitFinished,
            this, &TSettingsTab::onReadHighTemperatureLimitFinished);
    neslab_->readLowTemperatureLimitCommand();
    neslab_->readHighTemperatureLimitCommand();
}

void TSettingsTab::onReadLowTemperatureLimitFinished(double T, bool /*enhancedPrec*/)
{
    startTSpinBox->setMinimum(T);
    endTSpinBox->setMinimum(T);
    afterMeasSpinBox->setMinimum(T);
    if (gotHighTLim) {
        enableTMeas();
    } else {
        gotLowTLim = true;
    }
}

void TSettingsTab::onReadHighTemperatureLimitFinished(double T, bool /*enhancedPrec*/)
{
    startTSpinBox->setMaximum(T);
    endTSpinBox->setMaximum(T);
    afterMeasSpinBox->setMaximum(T);
    if (gotLowTLim) {
        enableTMeas();
    } else {
        gotHighTLim = true;
    }
}

void TSettingsTab::onTMeasGroupBoxClicked(bool checked)
{
    if (checked) {
        if (neslab_->isConnected()) {
            if (gotLowTLim && gotHighTLim) {
                enableTMeas();
            } else {
                tMeasGroupBox->setEnabled(false);
                if (!gotLowTLim) {
                    connect(neslab_, &Neslab::readLowTemperatureLimitFinished,
                            this, &TSettingsTab::onReadLowTemperatureLimitFinished);
                    neslab_->readLowTemperatureLimitCommand();
                }
                if (!gotHighTLim) {
                    connect(neslab_, &Neslab::readHighTemperatureLimitFinished,
                            this, &TSettingsTab::onReadHighTemperatureLimitFinished);
                    neslab_->readHighTemperatureLimitCommand();
                }
            }
        } else {
            int ret = QMessageBox::question(this, tr("Connect Neslab?"), tr("Neslab thermostated bath is not connected yet. Do you want to connect it?"),
                                            QMessageBox::Yes | QMessageBox::No);
            if (ret == QMessageBox::Yes) {
                tMeasGroupBox->setEnabled(false);
                connect(neslab_, &Neslab::connected,
                        this, &TSettingsTab::onNeslabConnected);
                neslab_->connectNeslab();
            }
        }
    } else {
        params_.tExp.tExp = false;
        emit tMeasStateChanged(false);
    }
}

void TSettingsTab::onSameAsTCheckBoxStateChanged(int state)
{
    if (state == Qt::Checked) {
        firstNumSpinBox->setEnabled(false);
        stepSpinBox->setEnabled(false);
        numDigitsSpinBox->setEnabled(false);
    } else {
        firstNumSpinBox->setEnabled(true);
        stepSpinBox->setEnabled(true);
        numDigitsSpinBox->setEnabled(true);
    }
}

void TSettingsTab::onSetStartTButtonReleased()
{
    qDebug() << "TSettingsTab::onSetStartTButtonReleased()";
    neslab_->setSetpointCommand(startTSpinBox->value());
    neslab_->readSetpointCommand();
}

void TSettingsTab::enableTMeas()
{
    tMeasGroupBox->setEnabled(true);
    params_.tExp.tExp = true;
    emit tMeasStateChanged(true);
}

ExtRanTabTraits::Model::Model(
        AppStateTraits::InitWinSpecParams &initWinSpecParams, QObject *parent) :
    QAbstractTableModel(parent), initWinSpecParams_(initWinSpecParams)
{
    ExtRanTabTraits::Params params;
    for (int ii = 0; ii < initWinSpecParams_.expe.size(); ++ii) {
        params.fileName = initWinSpecParams_.expe.at(ii).fileNameBase;
        params.grPos = initWinSpecParams_.expe.at(ii).grPos;
        params.expo = initWinSpecParams_.expe.at(ii).expo;
        params.acc = initWinSpecParams_.expe.at(ii).acc;
        params.frm = initWinSpecParams_.expe.at(ii).frm;
        params_.append(params);
    }
}

Qt::ItemFlags ExtRanTabTraits::Model::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::ItemIsEnabled;

//    return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
    return QAbstractItemModel::flags(index);
}

int ExtRanTabTraits::Model::rowCount(const QModelIndex &/*parent*/) const
{
    return params_.size();
}

int ExtRanTabTraits::Model::columnCount(const QModelIndex &/*parent*/) const
{
    return 5;
}

QVariant ExtRanTabTraits::Model::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (index.row() >= params_.size())
        return QVariant();

    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        switch (index.column()) {
        case 0 :
            return params_.at(index.row()).fileName;
            break;
        case 1 :
            return params_.at(index.row()).grPos;
            break;
        case 2 :
            return params_.at(index.row()).expo;
            break;
        case 3 :
            return params_.at(index.row()).acc;
            break;
        case 4 :
            return params_.at(index.row()).frm;
            break;
        default :
            return QVariant();
        }
    } else
        return QVariant();
}

bool ExtRanTabTraits::Model::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (index.isValid() && role == Qt::EditRole) {
        bool ok;
        switch (index.column()) {
        case 0:
            params_[index.row()].fileName = value.toString();
            break;
        case 1:
        {
            int val = value.toInt(&ok);
            if (!ok)
                return false;
            else
                params_[index.row()].grPos = val;
            break;
        }
        case 2:
        {
            double val = value.toDouble(&ok);
            if (!ok)
                return false;
            else
                params_[index.row()].expo = val;
            break;
        }
        case 3:
        {
            int val = value.toInt(&ok);
            if (!ok)
                return false;
            else
                params_[index.row()].acc = val;
            break;
        }
        case 4:
        {
            int val = value.toInt(&ok);
            if (!ok)
                return false;
            else
                params_[index.row()].frm = val;
            break;
        }

        }
        emit dataChanged(index, index);
        return true;
    }
    return false;
}

QVariant ExtRanTabTraits::Model::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    if (orientation == Qt::Horizontal) {
        switch (section) {
        case 0 :
            return "File name";
            break;
        case 1 :
            return "Gr. Pos.";
            break;
        case 2 :
            return "Exp";
            break;
        case 3 :
            return "Acc";
            break;
        case 4 :
            return "Frm";
            break;
        default :
            return QVariant();
        }
    }
    else
        return QString("%1").arg(section);
}

bool ExtRanTabTraits::Model::insertRows(int row, int count, const QModelIndex &parent)
{
    beginInsertRows(parent, row, row + count - 1);
    ExtRanTabTraits::Params params = {"temp", 1, 1.0, 1, 1};
    for (int ii = row; ii < row + count; ++ii) {
        params_.insert(ii, params);
    }

    endInsertRows();
    return true;

}

bool ExtRanTabTraits::Model::removeRows(int row, int count, const QModelIndex &parent)
{
    beginRemoveRows(parent, row, row + count - 1);
    for (int ii = row + count - 1; ii >= row; --ii) {
        params_.removeAt(ii);
    }
    endRemoveRows();
    return true;

}

bool ExtRanTabTraits::Model::moveRows(const QModelIndex &sourceParent, int sourceRow, int count, const QModelIndex &destinationParent, int destinationChild)
{
    beginMoveRows(sourceParent, sourceRow, sourceRow + count - 1, destinationParent, destinationChild);
    if (sourceRow > destinationChild) {
        for (int ii = 0; ii < count; ++ii) {
            params_.move(sourceRow, destinationChild + ii);
        }
    } else {
        for (int ii = 0; ii < count; ++ii) {
            params_.move(sourceRow, destinationChild - count);
        }
    }
    endMoveRows();
    return true;
}

ExtRanTabTraits::Dialog::Dialog(AppStateTraits::InitWinSpecParams &params, AppState *appState, int expNumber, QWidget *parent) :
    QDialog(parent), saveParams_(params)
{
    params_ = new AppStateTraits::InitWinSpecParams(params);
    expSettings = new ExpSettingsTab(*params_, appState, true, expNumber);

    dialogButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok
                                           | QDialogButtonBox::Cancel);

    connect(dialogButtonBox, &QDialogButtonBox::accepted,
            this, &QDialog::accept);
    connect(dialogButtonBox, &QDialogButtonBox::rejected,
            this, &QDialog::reject);


    QVBoxLayout *mainVLayout = new QVBoxLayout;
    setLayout(mainVLayout);
    mainVLayout->addWidget(expSettings);
    mainVLayout->addWidget(dialogButtonBox);
}

void ExtRanTabTraits::Dialog::accept()
{
    if (!expSettings->saveParams())
        return;
    saveParams_ = *params_;
    QDialog::accept();
}

ExtRanTabTraits::Dialog::~Dialog()
{
    delete params_;
}

void ExtRanTabTraits::Dialog::keyPressEvent(QKeyEvent *e)
{
    if(e->key() == Qt::Key_Enter || e->key() == Qt::Key_Return)
        return;
    QDialog::keyPressEvent(e);
}

ExtRanTabTraits::ViewEventFilter::ViewEventFilter(QObject *parent) :
    QObject(parent)
{
}

bool ExtRanTabTraits::ViewEventFilter::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        switch (keyEvent->key()) {
        case Qt::Key_Return :
        case Qt::Key_Enter :
        case Qt::Key_Delete :
        case Qt::Key_Insert :
        case Qt::Key_PageUp :
        case Qt::Key_PageDown :
            emit pressedKey(keyEvent->key());
            return true;
        default:
            break;
        }
    }
    // standard event processing
    return QObject::eventFilter(obj, event);
}

ExtRanTab::ExtRanTab(AppStateTraits::InitWinSpecParams &params, AppState *appState, QWidget *parent) :
    QWidget(parent), params_(params), appState_(appState)
{
    extRanGroupBox = new QGroupBox(tr("Extended Range"));
    extRanGroupBox->setCheckable(true);
    extRanGroupBox->setChecked(params_.extRan.extendedRange);
    if (params_.extRan.extendedRange) {
        int useStage = false;
        for (int ii = 0; ii < params.expe.size(); ++ii) {
            if (params.cal.at(ii).autoCal) {
                useStage = true;
                break;
            }
        }
        if (useStage) {
            if (!appState_->stageControl()->initialized()){
                int ret = QMessageBox::question(this, tr("Initialize Stage?"), tr("Stage is not initialized yet. Do you want to initialize it?"),
                                                QMessageBox::Yes | QMessageBox::No);
                if (ret == QMessageBox::Yes) {
                    if (appState_->stageControl()->motorRunning()) {
                        QMessageBox::critical(this, tr("Stage is already running!"), tr("The stage is already running, stop it, please!"), QMessageBox::Ok);
                    } else {
                        qDebug() << "ExpSettingsTab::onAutoCalGroupBoxClicked(): emitting initStage";
                        appState_->appCore()->onInitStage();
                    }
                }
            }
        }
    }

    autoFilenamesGroupBox = new QGroupBox(tr("Automatic filenames"));
    autoFilenamesGroupBox->setCheckable(true);
    autoFilenamesGroupBox->setChecked(params_.extRan.autoFileNames);
    QLabel *filenameLabel = new QLabel(tr("Enter the filename base:"));
    filenameEdit = new QLineEdit;
    filenameEdit->setText(params_.extRan.fileNameBase);
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

    model = new ExtRanTabTraits::Model(params_, this);
    if (params_.extRan.autoFileNames) {
        autorenameFilenames();
    }
    view = new QTableView;
    int h = fontMetrics().height() + 2;
    QHeaderView *verticalHeader = view->verticalHeader();
    verticalHeader->sectionResizeMode(QHeaderView::Fixed);
    verticalHeader->setDefaultSectionSize(h);
    view->verticalHeader()->hide();
    view->setShowGrid(false);
    view->setModel(model);
    view->resizeColumnsToContents();
    view->setSelectionBehavior(QAbstractItemView::SelectRows);

    ExtRanTabTraits::ViewEventFilter *viewEventFilter =
            new ExtRanTabTraits::ViewEventFilter(this);
    view->installEventFilter(viewEventFilter);

    addButton = new QPushButton(tr("Add"));
    editButton = new QPushButton(tr("Edit"));
    removeButton = new QPushButton(tr("Remove"));
    moveUpButton = new QPushButton(tr("Move up"));
    moveDownButton = new QPushButton(tr("Move down"));

    connect(filenameEdit, &QLineEdit::editingFinished,
            this, &ExtRanTab::onFilenameEditEditingFinished);
    connect(filenameButton, &QPushButton::released,
            this, &ExtRanTab::onFilenameButtonReleased);
    connect(extRanGroupBox, &QGroupBox::clicked,
            this, &ExtRanTab::onExtRanGroupBoxClicked);
    connect(autoFilenamesGroupBox, &QGroupBox::clicked,
            this, &ExtRanTab::onAutoFilenamesGroupBoxClicked);
    connect(addButton, &QPushButton::clicked,
            this, &ExtRanTab::onAddButtonClicked);
    connect(editButton, &QPushButton::clicked,
            this, &ExtRanTab::onEditButtonClicked);
    connect(removeButton, &QPushButton::clicked,
            this, &ExtRanTab::onRemoveButtonClicked);
    connect(moveUpButton, &QPushButton::clicked,
            this, &ExtRanTab::onMoveUpButtonClicked);
    connect(moveDownButton, &QPushButton::clicked,
            this, &ExtRanTab::onMoveDownButtonClicked);
    connect(viewEventFilter, &ExtRanTabTraits::ViewEventFilter::pressedKey,
            this, &ExtRanTab::onViewPressedKey);

    connect(view, &QTableView::doubleClicked,
            this, &ExtRanTab::onViewDoubleClicked);

    QVBoxLayout *mainVLayout = new QVBoxLayout;
    setLayout(mainVLayout);
    mainVLayout->addWidget(extRanGroupBox);

    QVBoxLayout *boxVLayout = new QVBoxLayout;
    extRanGroupBox->setLayout(boxVLayout);

    boxVLayout->addWidget(autoFilenamesGroupBox);
    QVBoxLayout *vLayout = new QVBoxLayout;
    autoFilenamesGroupBox->setLayout(vLayout);
    vLayout->addWidget(filenameLabel, 0, Qt::AlignLeft);
    QHBoxLayout *hLayout = new QHBoxLayout;
    vLayout->addLayout(hLayout);
    hLayout->addWidget(filenameEdit);
    hLayout->addWidget(filenameButton);
    hLayout->addStretch();

    hLayout = new QHBoxLayout;
    boxVLayout->addLayout(hLayout);
    hLayout->addWidget(view);

    vLayout = new QVBoxLayout;
    hLayout->addLayout(vLayout);
    vLayout->addWidget(addButton);
    vLayout->addWidget(editButton);
    vLayout->addWidget(removeButton);
    vLayout->addWidget(moveUpButton);
    vLayout->addWidget(moveDownButton);
    vLayout->addStretch();
}

bool ExtRanTab::saveParams()
{
    if (!params_.extRan.extendedRange)
        return true;

    if (params_.extRan.autoFileNames) {
        if (!filenameEdit->text().isEmpty()) {
            QFileInfo newFileInfo(filenameEdit->text());
            if (newFileInfo.isRelative()) {
                newFileInfo.setFile(params_.extRan.directory % QDir::separator() % filenameEdit->text());
            }
            QFileInfo fileInfo(params_.extRan.directory % QDir::separator() % params_.extRan.fileNameBase);
            if (fileInfo.absoluteFilePath() != newFileInfo.absoluteFilePath()) {
                if (!newFileInfo.absoluteDir().exists()) {
                    int ret = QMessageBox::question(this, tr("Create folder?"), tr("The folder %1 does not exist. Do you want to create it?")
                                                    .arg(newFileInfo.absolutePath()), QMessageBox::Yes | QMessageBox::No);
                    if (ret == QMessageBox::Yes) {
                        QDir dir;
                        if (dir.mkpath(newFileInfo.absolutePath())) {
                            params_.extRan.directory = QDir::toNativeSeparators(newFileInfo.absolutePath());
                            params_.extRan.fileNameBase = newFileInfo.completeBaseName();
                        } else {
                            QMessageBox::warning(this, tr("Invalid path!"), tr("Enter a valid path to the file."));
                            return false;
                        }
                    } else {
                        return false;
                    }
                } else {
                    params_.extRan.directory = QDir::toNativeSeparators(newFileInfo.absolutePath());
                    params_.extRan.fileNameBase = newFileInfo.completeBaseName();
                }
            }
        } else {
            QMessageBox::warning(this, tr("Invalid file name!"), tr("Enter a valid file name."));
            return false;
        }
    }
    return true;
}

void ExtRanTab::autorenameFilenames()
{
    if (params_.extRan.autoFileNames) {
        if (params_.tExp.tExp || params_.batch.batchExp) {
            for (int ii = 0; ii < model->rowCount(); ++ii) {
                model->setData(model->index(ii, 0),
                               QString(params_.extRan.fileNameBase % "%1_")
                               .arg(ii));
            }
        } else {
            for (int ii = 0; ii < model->rowCount(); ++ii) {
                model->setData(model->index(ii, 0),
                               QString(params_.extRan.fileNameBase % "%1")
                               .arg(ii));
            }
        }
    } else {
        for (int ii = 0; ii < model->rowCount(); ++ii) {
            model->setData(model->index(ii, 0),
                           QString(params_.expe.at(ii).fileNameBase));
        }
    }
}

void ExtRanTab::onExtRanGroupBoxClicked(bool state)
{
    params_.extRan.extendedRange = state;
    emit extRanStateChanged(state);
}

void ExtRanTab::onAutoFilenamesGroupBoxClicked(bool state)
{
    params_.extRan.autoFileNames = state;
    for (int ii = 0; ii < model->rowCount(); ++ii) {
        if (state) {
            model->setData(model->index(ii, 0),
                           QString(params_.extRan.fileNameBase % "%1_")
                           .arg(ii));
        } else {
            model->setData(model->index(ii, 0),
                           params_.expe.at(ii).fileNameBase);
        }
    }
}

void ExtRanTab::onFilenameButtonReleased()
{
    QFileInfo fileInfo(params_.extRan.directory %
                       QDir::separator() %
                       params_.extRan.fileNameBase);
    QString filename = QFileDialog::getSaveFileName(
                this, tr("Open File"), fileInfo.filePath(),
                tr("WinSpec files (*.spe);;All files (*.*)"), 0,
                QFileDialog::DontConfirmOverwrite);
    if (!filename.isEmpty()) {
        QFileInfo newFileInfo(filename);
        params_.extRan.directory =
                QDir::toNativeSeparators(newFileInfo.absolutePath());
        params_.extRan.fileNameBase = newFileInfo.completeBaseName();
        if (filenameEdit->text() != newFileInfo.completeBaseName()) {
            filenameEdit->setText(newFileInfo.completeBaseName());
        }
    }
    for (int ii = 0; ii < model->rowCount(); ++ii) {
        model->setData(model->index(ii, 0),
                       QString(params_.extRan.fileNameBase % "%1_")
                       .arg(ii));
    }
}

void ExtRanTab::onFilenameEditEditingFinished()
{
    if (filenameEdit->hasFocus()) {
        if (!filenameEdit->text().isEmpty()) {
            QFileInfo newFileInfo(filenameEdit->text());
            bool relative;
            if (newFileInfo.isRelative()) {
                newFileInfo.setFile(params_.extRan.directory % QDir::separator() % filenameEdit->text());
                relative = true;
            }
            if (!newFileInfo.absoluteDir().exists()) {
                int ret = QMessageBox::question(this, tr("Create folder?"), tr("The folder %1 does not exist. Do you want to create it?")
                                          .arg(newFileInfo.absolutePath()), QMessageBox::Yes | QMessageBox::No);
                if (ret == QMessageBox::Yes) {
                    QDir dir;
                    if (dir.mkpath(newFileInfo.absolutePath())) {
                        if (relative) {
                            params_.extRan.fileNameBase = QFileInfo(filenameEdit->text()).completeBaseName();
                        } else {
                            params_.extRan.directory = QDir::toNativeSeparators(newFileInfo.absolutePath());
                            params_.extRan.fileNameBase = newFileInfo.completeBaseName();
                        }
                    } else {
                        QMessageBox::warning(this, tr("Invalid path!"), tr("Enter a valid path to the file."));
                    }
                }
            } else {
                if (relative) {
                    params_.extRan.fileNameBase = QFileInfo(filenameEdit->text()).completeBaseName();
                } else {
                    params_.extRan.directory = QDir::toNativeSeparators(newFileInfo.absolutePath());
                    params_.extRan.fileNameBase = newFileInfo.completeBaseName();
                }
            }
        } else {
            QMessageBox::warning(this, tr("Invalid file name!"), tr("Enter a valid file name."));
        }
    }
    for (int ii = 0; ii < model->rowCount(); ++ii) {
        model->setData(model->index(ii, 0),
                       QString(params_.extRan.fileNameBase % "_%1_")
                       .arg(ii));
    }
}

void ExtRanTab::onAddButtonClicked()
{
    QModelIndexList selectedList = view->selectionModel()->selectedRows();
    if (model->rowCount()) {
        if (selectedList.isEmpty()) {
            addItem(model->rowCount());
        } else {
            addItem(selectedList.at(0).row());
        }
    } else {
        addItem(-1);
    }
    if (params_.extRan.autoFileNames) {
        autorenameFilenames();
    }
}

void ExtRanTab::onEditButtonClicked()
{
    QModelIndexList selectedList = view->selectionModel()->selectedRows();
    if (selectedList.isEmpty())
        return;

    editItem(selectedList.at(0).row());
}

void ExtRanTab::onRemoveButtonClicked()
{
    QModelIndexList selectedList = view->selectionModel()->selectedRows();
    if (selectedList.isEmpty())
        return;

    removeItem(selectedList.at(0).row());
}

void ExtRanTab::onMoveUpButtonClicked()
{
    QModelIndexList selectedList = view->selectionModel()->selectedRows();
    if (selectedList.isEmpty())
        return;

    int expNumber = selectedList.at(0).row();
    if (expNumber >= params_.expe.size() || expNumber < 1)
        return;

    params_.expe.move(expNumber, expNumber - 1);
    model->moveRow(QModelIndex(), expNumber, QModelIndex(), expNumber - 1);
    if (params_.extRan.autoFileNames) {
        autorenameFilenames();
    }
}

void ExtRanTab::onMoveDownButtonClicked()
{
    QModelIndexList selectedList = view->selectionModel()->selectedRows();
    if (selectedList.isEmpty())
        return;

    int expNumber = selectedList.at(0).row();
    if (expNumber >= params_.expe.size() - 1 || expNumber < 0)
        return;

    params_.expe.move(expNumber, expNumber + 1);
    // the row have to be moved by 2, because the new rows is inserted before
    // the destination row counted as before moving.
    // See QAbstractItemModel::beginMoveRows()
    model->moveRow(QModelIndex(), expNumber, QModelIndex(), expNumber + 2);
    if (params_.extRan.autoFileNames) {
        autorenameFilenames();
    }
}

void ExtRanTab::onViewDoubleClicked(const QModelIndex &index)
{
    editItem(index.row());
}

void ExtRanTab::onViewPressedKey(int key)
{
    switch (key) {
    case Qt::Key_Enter :
    case Qt::Key_Return :
        onEditButtonClicked();
        break;
    case Qt::Key_Delete :
        onRemoveButtonClicked();
        break;
    case Qt::Key_Insert :
        onAddButtonClicked();
        break;
    case Qt::Key_PageUp :
        onMoveUpButtonClicked();
        break;
    case Qt::Key_PageDown :
        onMoveDownButtonClicked();
        break;
    default:
        break;
    }
}

void ExtRanTab::addItem(int expNumber)
{
    int addExpNumber = expNumber + 1;
    // controlls if there is any selection
    if (addExpNumber > params_.expe.size()) {
        addExpNumber = params_.expe.size();
    }
    QString fileNameBase;
    AppStateTraits::ExpWinSpecParams expWinSpecParams;
    AppStateTraits::CalWinSpecParams calWinSpecParams;
    if (addExpNumber) {
        expWinSpecParams = {params_.expe.at(addExpNumber - 1).grPos,
                            params_.expe.at(addExpNumber - 1).fileNameBase,
                            params_.expe.at(addExpNumber - 1).directory,
                            params_.expe.at(addExpNumber - 1).expo,
                            params_.expe.at(addExpNumber - 1).acc,
                            params_.expe.at(addExpNumber - 1).frm};
        calWinSpecParams = {params_.cal.at(addExpNumber - 1).autoCal,
                            params_.cal.at(addExpNumber - 1).eachMeas,
                            params_.cal.at(addExpNumber - 1).expo,
                            params_.cal.at(addExpNumber - 1).acc,
                            params_.cal.at(addExpNumber - 1).frm};
        if (params_.tExp.tExp || params_.batch.batchExp) {
            fileNameBase = QString("temp%1_").arg(addExpNumber);
        } else {
            fileNameBase = QString("temp%1").arg(addExpNumber);
        }
    } else {
        fileNameBase = "temp";
        expWinSpecParams = {0, fileNameBase, "", 1.0, 1, 1};
        calWinSpecParams = {true, 1, 1.0, 1, 1};
    }
    params_.expe.insert(addExpNumber, expWinSpecParams);
    params_.cal.insert(addExpNumber, calWinSpecParams);
    ExtRanTabTraits::Dialog dialog(params_, appState_, addExpNumber, this);
    if (dialog.exec()) {
        model->insertRow(addExpNumber);
        model->setData(model->index(addExpNumber, 0),
                       params_.expe.at(addExpNumber).fileNameBase);
        model->setData(model->index(addExpNumber, 1),
                       params_.expe.at(addExpNumber).grPos);
        model->setData(model->index(addExpNumber, 2),
                       params_.expe.at(addExpNumber).expo);
        model->setData(model->index(addExpNumber, 3),
                       params_.expe.at(addExpNumber).acc);
        model->setData(model->index(addExpNumber, 4),
                       params_.expe.at(addExpNumber).frm);
    } else {
        params_.expe.removeAt(addExpNumber);
        params_.cal.removeAt(addExpNumber);
    }
}

void ExtRanTab::editItem(int expNumber)
{
    ExtRanTabTraits::Dialog dialog(params_, appState_, expNumber, this);
    if (dialog.exec()) {
        if (!params_.extRan.autoFileNames) {
            model->setData(model->index(expNumber, 0),
                           params_.expe.at(expNumber).fileNameBase);
        }
        model->setData(model->index(expNumber, 1),
                       params_.expe.at(expNumber).grPos);
        model->setData(model->index(expNumber, 2),
                       params_.expe.at(expNumber).expo);
        model->setData(model->index(expNumber, 3),
                       params_.expe.at(expNumber).acc);
        model->setData(model->index(expNumber, 4),
                       params_.expe.at(expNumber).frm);
    }
}

void ExtRanTab::removeItem(int expNumber)
{
    if (model->rowCount() > 1) {
        model->removeRow(expNumber);
        params_.expe.removeAt(expNumber);
        params_.cal.removeAt(expNumber);
        if (params_.extRan.autoFileNames) {
            autorenameFilenames();
        }
    }
}

ExperimentSetup::ExperimentSetup(AppState *appState, QWidget *parent) :
    QDialog(parent), appState_(appState), neslab_(appState->neslab())
{
    params_ = new AppStateTraits::InitWinSpecParams(
                *(appState_->initWinSpecParams()));
    tabWidget = new QTabWidget;
    expSettingsTab = new ExpSettingsTab(*params_, appState_, false, 0);
    batchExpTab = new BatchExpTab(*params_);
    tSettingsTab = new TSettingsTab(*params_, neslab_);
    extRanTab = new ExtRanTab(*params_, appState_);
    tabWidget->addTab(expSettingsTab, tr("Experiment"));
    tabWidget->addTab(batchExpTab, tr("Batch"));
    tabWidget->addTab(tSettingsTab, tr("Temperature"));
    tabWidget->addTab(extRanTab, tr("Extended Range"));

    expSettingsTab->setEnabled(!(params_->extRan.extendedRange));

    dialogButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok
                                           | QDialogButtonBox::Cancel);

    connect(expSettingsTab, &ExpSettingsTab::initStage,
            this, &ExperimentSetup::onExpSettingsTabInitStage);
    connect(extRanTab, &ExtRanTab::extRanStateChanged,
            this, &ExperimentSetup::onExtRanStateChanged);
    connect(batchExpTab, &BatchExpTab::batchExpStateChanged,
            extRanTab, &ExtRanTab::autorenameFilenames);
    connect(tSettingsTab, &TSettingsTab::tMeasStateChanged,
            extRanTab, &ExtRanTab::autorenameFilenames);
    connect(dialogButtonBox, &QDialogButtonBox::accepted,
            this, &ExperimentSetup::accept);
    connect(dialogButtonBox, &QDialogButtonBox::rejected,
            this, &QDialog::reject);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(tabWidget);
    mainLayout->addWidget(dialogButtonBox);
    setLayout(mainLayout);

    setWindowTitle(tr("Experiment Setup"));
}

void ExperimentSetup::keyPressEvent(QKeyEvent *e)
{
    if(e->key() == Qt::Key_Enter || e->key() == Qt::Key_Return)
        return;
    QDialog::keyPressEvent(e);
}

int ExperimentSetup::countNumDigits(int number)
{
    int digits = 0;
    if (number < 0) digits = 1; // remove this line if '-' counts as a digit
    while (number) {
        number /= 10;
        digits++;
    }
    return digits;
}

void ExperimentSetup::accept()
{
    if (!params_->extRan.extendedRange && !expSettingsTab->saveParams())
        return;
    if (!batchExpTab->saveParams())
        return;
    if (!tSettingsTab->saveParams())
        return;

    *(appState_->initWinSpecParams()) = *params_;
    QDialog::accept();
}

void ExperimentSetup::onExpSettingsTabInitStage()
{
    qDebug() << "ExperimentSetup::onExpSettingsTabInitStage()";
    emit initStage();
}

void ExperimentSetup::onInitStageFinished()
{
    expSettingsTab->onInitStageFinished();
}

void ExperimentSetup::onExtRanStateChanged(bool state)
{
    expSettingsTab->setEnabled(!state);
}

ExperimentSetup::~ExperimentSetup()
{
    delete params_;
}
