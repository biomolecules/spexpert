#include "stagesetup.h"

#include "appstate.h"
#include "stagecontrol.h"
#include "exptasks.h"

#include <QKeyEvent>
#include <QPushButton>
#include <QGroupBox>
#include <QLabel>
#include <QComboBox>
#include <QSpinBox>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QMessageBox>

#include <QDebug>

StageSetup::StageSetup(AppState *appState, QWidget *parent) :
    QDialog(parent), appState_(appState), stageControl_(appState->stageControl())
{
    params_ = new StageControlTraits::Params(*appState_->stageParams());

    initStage = nullptr;
    goToLimStage = nullptr;
    goToPosStage = nullptr;

    initButton = new QPushButton(tr("Initialize"));
    initButton->setEnabled(true);

    goUpLimButton = new QPushButton(tr("Go Up Lim"));
    goUpLimButton->setEnabled(false);

    goLowLimButton = new QPushButton(tr("Go Low Lim"));
    goLowLimButton->setEnabled(false);

    stopButton = new QPushButton(tr("Stop"));
    stopButton->setEnabled(true);

    posGroupBox = new QGroupBox(tr("Stage position"));

    QLabel *posRefLabel = new QLabel(tr("Reference:"));
    posRefComboBox = new QComboBox;
    posRefComboBox->insertItem(0, tr("Absolute"), static_cast<int>(StageControlTraits::ReferenceType::Absolute));
    posRefComboBox->insertItem(1, tr("Lower Limit"), static_cast<int>(StageControlTraits::ReferenceType::LowerLimit));
    posRefComboBox->insertItem(2, tr("Upper Limit"), static_cast<int>(StageControlTraits::ReferenceType::UpperLimit));
    int index;
    switch (params_->currPosRefType) {
    case StageControlTraits::ReferenceType::Absolute :
        index = posRefComboBox->findData(static_cast<int>(StageControlTraits::ReferenceType::Absolute));
        if (index != -1)
            posRefComboBox->setCurrentIndex(index);
        break;
    case StageControlTraits::ReferenceType::LowerLimit :
        index = posRefComboBox->findData(static_cast<int>(StageControlTraits::ReferenceType::LowerLimit));
        if (index != -1)
            posRefComboBox->setCurrentIndex(index);
        break;
    case StageControlTraits::ReferenceType::UpperLimit :
        index = posRefComboBox->findData(static_cast<int>(StageControlTraits::ReferenceType::UpperLimit));
        if (index != -1)
            posRefComboBox->setCurrentIndex(index);
        break;
    }

    posRefLabel->setBuddy(posRefComboBox);

    QLabel *upperLimitTextLabel = new QLabel(tr("Upper limit:"));
    upperLimitLabel = new QLabel(tr("0"));

    QLabel *lowerLimitTextLabel = new QLabel(tr("Lower limit:"));
    lowerLimitLabel = new QLabel(tr("0"));

    QLabel *currPosTextLabel = new QLabel(tr("Current pos.:"));
    currPosLabel = new QLabel(tr("0"));

    initGroupBox = new QGroupBox("Initialization");
    initGroupBox->setEnabled(true);

    initComboBox = new QComboBox;
    initComboBox->insertItem(0, tr("Lower Limit"), static_cast<int>(StageControlTraits::InitType::LowerLimit));
    initComboBox->insertItem(1, tr("Upper Limit"), static_cast<int>(StageControlTraits::InitType::UpperLimit));
    initComboBox->insertItem(2, tr("Both"), static_cast<int>(StageControlTraits::InitType::LowerLimit | StageControlTraits::InitType::UpperLimit));

    QLabel *rangeLabel = new QLabel("Full range:");
    rangeSpinBox = new QSpinBox;
    rangeSpinBox->setMinimum(1);
    rangeSpinBox->setMaximum(1e8-1);
    rangeSpinBox->setValue(params_->range);
    rangeLabel->setBuddy(rangeSpinBox);

    qDebug() << "StageSetup::StageSetup(): initType: " << static_cast<int>(params_->initType);
    if (static_cast<bool>(params_->initType & StageControlTraits::InitType::LowerLimit)
            && static_cast<bool>(params_->initType & StageControlTraits::InitType::UpperLimit)) {
        index = initComboBox->findData(static_cast<int>(StageControlTraits::InitType::LowerLimit | StageControlTraits::InitType::UpperLimit));
        if (index != -1)
            initComboBox->setCurrentIndex(index);
        rangeSpinBox->setEnabled(false);
    } else if (static_cast<bool>(params_->initType &
                                 StageControlTraits::InitType::LowerLimit)) {
        index = initComboBox->findData(static_cast<int>(StageControlTraits::InitType::LowerLimit));
        if (index != -1)
            initComboBox->setCurrentIndex(index);
        rangeSpinBox->setEnabled(true);
    } else if (static_cast<bool>(params_->initType &
                                 StageControlTraits::InitType::UpperLimit)) {
        index = initComboBox->findData(static_cast<int>(StageControlTraits::InitType::UpperLimit));
        if (index != -1)
            initComboBox->setCurrentIndex(index);
        rangeSpinBox->setEnabled(true);
    }


    measGroupBox = new QGroupBox(tr("Measurement"));
    measGroupBox->setEnabled(false);

    QLabel *measPosLabel = new QLabel(tr("Position:"));
    measPosSpinBox = new QSpinBox;
    measPosSpinBox->setMinimum(-1e8+1);
    measPosSpinBox->setMaximum(1e8-1);
    measPosSpinBox->setValue(params_->measPos);
    measPosLabel->setBuddy(measPosSpinBox);

    QLabel *measRefLabel = new QLabel(tr("Reference:"));
    measRefComboBox = new QComboBox;
    measRefComboBox->insertItem(0, tr("Absolute"), static_cast<int>(StageControlTraits::ReferenceType::Absolute));
    measRefComboBox->insertItem(1, tr("Lower Limit"), static_cast<int>(StageControlTraits::ReferenceType::LowerLimit));
    measRefComboBox->insertItem(2, tr("Upper Limit"), static_cast<int>(StageControlTraits::ReferenceType::UpperLimit));
    measRefLabel->setBuddy(measRefComboBox);
    switch (params_->measRefType) {
    case StageControlTraits::ReferenceType::Absolute :
        index = measRefComboBox->findData(static_cast<int>(StageControlTraits::ReferenceType::Absolute));
        if (index != -1)
            measRefComboBox->setCurrentIndex(index);
        break;
    case StageControlTraits::ReferenceType::LowerLimit :
        index = measRefComboBox->findData(static_cast<int>(StageControlTraits::ReferenceType::LowerLimit));
        if (index != -1)
            measRefComboBox->setCurrentIndex(index);
        break;
    case StageControlTraits::ReferenceType::UpperLimit :
        index = measRefComboBox->findData(static_cast<int>(StageControlTraits::ReferenceType::UpperLimit));
        if (index != -1)
            measRefComboBox->setCurrentIndex(index);
        break;
    }

//    QLabel *measInitLabel = new QLabel(tr("Initialization:"));
//    measInitComboBox = new QComboBox;
//    measInitComboBox->insertItem(0, tr("None"), static_cast<int>(StageControlTraits::InitType::None));
//    measInitComboBox->insertItem(1, tr("Lower Limit"), static_cast<int>(StageControlTraits::InitType::LowerLimit));
//    measInitComboBox->insertItem(2, tr("Upper Limit"), static_cast<int>(StageControlTraits::InitType::UpperLimit));
//    measInitComboBox->insertItem(3, tr("Both"), static_cast<int>(StageControlTraits::InitType::LowerLimit | StageControlTraits::InitType::UpperLimit));
//    measInitLabel->setBuddy(measInitComboBox);
//    if (static_cast<bool>(params_->measInitType &
//                          (StageControlTraits::InitType::LowerLimit |
//                           StageControlTraits::InitType::UpperLimit))) {
//        index = measInitComboBox->findData(static_cast<int>(StageControlTraits::InitType::LowerLimit | StageControlTraits::InitType::UpperLimit));
//        if (index != -1)
//            measInitComboBox->setCurrentIndex(index);
//    } else if (static_cast<bool>(params_->measInitType &
//                                 StageControlTraits::InitType::LowerLimit)) {
//        index = measInitComboBox->findData(static_cast<int>(StageControlTraits::InitType::LowerLimit));
//        if (index != -1)
//            measInitComboBox->setCurrentIndex(index);
//    } else if (static_cast<bool>(params_->measInitType &
//                                 StageControlTraits::InitType::UpperLimit)) {
//        index = measInitComboBox->findData(static_cast<int>(StageControlTraits::InitType::UpperLimit));
//        if (index != -1)
//            measInitComboBox->setCurrentIndex(index);
//    } else {
//        index = measInitComboBox->findData(static_cast<int>(StageControlTraits::InitType::None));
//        if (index != -1)
//            measInitComboBox->setCurrentIndex(index);
//    }

    measGoButton = new QPushButton(tr("Go"));

    calGroupBox = new QGroupBox(tr("Calibration"));
    calGroupBox->setEnabled(false);

    QLabel *calPosLabel = new QLabel(tr("Position:"));
    calPosSpinBox = new QSpinBox;
    calPosSpinBox->setMinimum(-1e8+1);
    calPosSpinBox->setMaximum(1e8-1);
    calPosSpinBox->setValue(params_->calPos);
    calPosLabel->setBuddy(measPosSpinBox);

    QLabel *calRefLabel = new QLabel(tr("Reference:"));
    calRefComboBox = new QComboBox;
    calRefComboBox->insertItem(0, tr("Absolute"), static_cast<int>(StageControlTraits::ReferenceType::Absolute));
    calRefComboBox->insertItem(1, tr("Lower Limit"), static_cast<int>(StageControlTraits::ReferenceType::LowerLimit));
    calRefComboBox->insertItem(2, tr("Upper Limit"), static_cast<int>(StageControlTraits::ReferenceType::UpperLimit));
    calRefLabel->setBuddy(measRefComboBox);
    switch (params_->calRefType) {
    case StageControlTraits::ReferenceType::Absolute :
        index = calRefComboBox->findData(static_cast<int>(StageControlTraits::ReferenceType::Absolute));
        if (index != -1)
            calRefComboBox->setCurrentIndex(index);
        break;
    case StageControlTraits::ReferenceType::LowerLimit :
        index = calRefComboBox->findData(static_cast<int>(StageControlTraits::ReferenceType::LowerLimit));
        if (index != -1)
            calRefComboBox->setCurrentIndex(index);
        break;
    case StageControlTraits::ReferenceType::UpperLimit :
        index = calRefComboBox->findData(static_cast<int>(StageControlTraits::ReferenceType::UpperLimit));
        if (index != -1)
            calRefComboBox->setCurrentIndex(index);
        break;
    }

//    QLabel *calInitLabel = new QLabel(tr("Initialization:"));
//    calInitComboBox = new QComboBox;
//    calInitComboBox->insertItem(0, tr("None"), static_cast<int>(StageControlTraits::InitType::None));
//    calInitComboBox->insertItem(1, tr("Lower Limit"), static_cast<int>(StageControlTraits::InitType::LowerLimit));
//    calInitComboBox->insertItem(2, tr("Upper Limit"), static_cast<int>(StageControlTraits::InitType::UpperLimit));
//    calInitComboBox->insertItem(3, tr("Both"), static_cast<int>(StageControlTraits::InitType::LowerLimit | StageControlTraits::InitType::UpperLimit));
//    calInitLabel->setBuddy(measInitComboBox);
//    if (static_cast<bool>(params_->calInitType &
//                          (StageControlTraits::InitType::LowerLimit |
//                           StageControlTraits::InitType::UpperLimit))) {
//        index = calInitComboBox->findData(static_cast<int>(StageControlTraits::InitType::LowerLimit | StageControlTraits::InitType::UpperLimit));
//        if (index != -1)
//            calInitComboBox->setCurrentIndex(index);
//    } else if (static_cast<bool>(params_->calInitType &
//                                 StageControlTraits::InitType::LowerLimit)) {
//        index = calInitComboBox->findData(static_cast<int>(StageControlTraits::InitType::LowerLimit));
//        if (index != -1)
//            calInitComboBox->setCurrentIndex(index);
//    } else if (static_cast<bool>(params_->calInitType &
//                                 StageControlTraits::InitType::UpperLimit)) {
//        index = calInitComboBox->findData(static_cast<int>(StageControlTraits::InitType::UpperLimit));
//        if (index != -1)
//            calInitComboBox->setCurrentIndex(index);
//    } else {
//        index = calInitComboBox->findData(static_cast<int>(StageControlTraits::InitType::None));
//        if (index != -1)
//            calInitComboBox->setCurrentIndex(index);
//    }

    calGoButton = new QPushButton(tr("Go"));

    dialogButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok
                                           | QDialogButtonBox::Cancel);

    connect(stageControl_, &StageControl::lowerLimChanged, this, &StageSetup::onLowerLimChanged);
    connect(stageControl_, &StageControl::upperLimChanged, this, &StageSetup::onUpperLimChanged);
    connect(stageControl_, &StageControl::currPosChanged, this, &StageSetup::onCurrPosChanged);

    connect(initComboBox, static_cast<void (QComboBox::*)(int index)>(&QComboBox::currentIndexChanged), this, &StageSetup::onInitComboboxCurrentIndexChanged);
    connect(posRefComboBox, static_cast<void (QComboBox::*)(int index)>(&QComboBox::currentIndexChanged), this, &StageSetup::onPosRefComboboxCurrentIndexChanged);
    connect(initButton, &QPushButton::released,
            this, &StageSetup::onInitButtonReleased);
    connect(goUpLimButton, &QPushButton::released,
            this, &StageSetup::onGoUpLimButtonReleased);
    connect(goLowLimButton, &QPushButton::released,
            this, &StageSetup::onGoLowLimButtonReleased);
    connect(measGoButton, &QPushButton::released,
            this, &StageSetup::onMeasGoButtonReleased);
    connect(calGoButton, &QPushButton::released,
            this, &StageSetup::onCalGoButtonReleased);
    connect(stopButton, &QPushButton::released,
            this, &StageSetup::onStopButtonReleased);
    connect(dialogButtonBox, &QDialogButtonBox::accepted,
            this, &StageSetup::accept);
    connect(dialogButtonBox, &QDialogButtonBox::rejected,
            this, &QDialog::reject);

    QVBoxLayout *mainVLayout = new QVBoxLayout;
    setLayout(mainVLayout);

    QHBoxLayout *mainHLayout = new QHBoxLayout;
    mainVLayout->addLayout(mainHLayout);

    QVBoxLayout *vLayout = new QVBoxLayout;
    mainHLayout->addLayout(vLayout);

    QHBoxLayout *hLayout = new QHBoxLayout;
    vLayout->addLayout(hLayout);
    hLayout->addWidget(initButton);
    hLayout->addWidget(goUpLimButton);
    hLayout->addWidget(goLowLimButton);
    hLayout->addWidget(stopButton);
    hLayout->addStretch();

    hLayout = new QHBoxLayout;
    vLayout->addLayout(hLayout);
    hLayout->addWidget(posGroupBox);

    QHBoxLayout *boxHLayout = new QHBoxLayout;
    posGroupBox->setLayout(boxHLayout);

    QVBoxLayout *boxVLayout = new QVBoxLayout;
    boxHLayout->addLayout(boxVLayout);
    boxVLayout->addWidget(posRefLabel);
    boxVLayout->addWidget(posRefComboBox);
    boxVLayout->addStretch();

    boxVLayout = new QVBoxLayout;
    boxHLayout->addLayout(boxVLayout);
    QGridLayout *boxGLayout = new QGridLayout;
    boxVLayout->addLayout(boxGLayout);
    boxGLayout->addWidget(upperLimitTextLabel, 0, 0, 1, 1, Qt::AlignRight);
    boxGLayout->addWidget(upperLimitLabel, 0, 1, 1, 1, Qt::AlignLeft);
    boxGLayout->addWidget(lowerLimitTextLabel, 1, 0, 1, 1, Qt::AlignRight);
    boxGLayout->addWidget(lowerLimitLabel, 1, 1, 1, 1, Qt::AlignLeft);
    boxGLayout->addWidget(currPosTextLabel, 2, 0, 1, 1, Qt::AlignRight);
    boxGLayout->addWidget(currPosLabel, 2, 1, 1, 1, Qt::AlignLeft);

    boxVLayout->addStretch();

    hLayout->addWidget(initGroupBox);
    boxHLayout = new QHBoxLayout;
    initGroupBox->setLayout(boxHLayout);
    boxVLayout = new QVBoxLayout;
    boxHLayout->addLayout(boxVLayout);
    boxVLayout->addWidget(initComboBox);
    boxVLayout->addWidget(rangeLabel);
    boxVLayout->addWidget(rangeSpinBox);
    boxVLayout->addStretch();
    boxHLayout->addStretch();

    mainHLayout->addWidget(measGroupBox);
    boxVLayout = new QVBoxLayout;
    measGroupBox->setLayout(boxVLayout);

    boxGLayout = new QGridLayout;
    boxVLayout->addLayout(boxGLayout);
    boxGLayout->addWidget(measPosLabel, 0, 0, 1, 1, Qt::AlignRight);
    boxGLayout->addWidget(measPosSpinBox, 0, 1);
    boxGLayout->addWidget(measRefLabel, 1, 0, 1, 1, Qt::AlignRight);
    boxGLayout->addWidget(measRefComboBox, 1, 1);
//    boxGLayout->addWidget(measInitLabel, 2, 0, 1, 1, Qt::AlignRight);
//    boxGLayout->addWidget(measInitComboBox, 2, 1);

    boxVLayout->addStretch();
    boxVLayout->addWidget(measGoButton, 0, Qt::AlignCenter);

    mainHLayout->addWidget(calGroupBox);
    boxVLayout = new QVBoxLayout;
    calGroupBox->setLayout(boxVLayout);

    boxGLayout = new QGridLayout;
    boxVLayout->addLayout(boxGLayout);
    boxGLayout->addWidget(calPosLabel, 0, 0, 1, 1, Qt::AlignRight);
    boxGLayout->addWidget(calPosSpinBox, 0, 1);
    boxGLayout->addWidget(calRefLabel, 1, 0, 1, 1, Qt::AlignRight);
    boxGLayout->addWidget(calRefComboBox, 1, 1);
//    boxGLayout->addWidget(calInitLabel, 2, 0, 1, 1, Qt::AlignRight);
//    boxGLayout->addWidget(calInitComboBox, 2, 1);

    boxVLayout->addStretch();
    boxVLayout->addWidget(calGoButton, 0, Qt::AlignCenter);

    mainVLayout->addWidget(dialogButtonBox);

    setWindowTitle(tr("Stage Setup"));

    if (stageControl_->initialized()) {
        onLowerLimChanged(stageControl_->lowerLim(StageControlTraits::ReferenceType::Absolute));
        onUpperLimChanged(stageControl_->upperLim(StageControlTraits::ReferenceType::Absolute));
        onCurrPosChanged(stageControl_->curPos(StageControlTraits::ReferenceType::Absolute));
        onInitialized();
    }
}

void StageSetup::keyPressEvent(QKeyEvent *e)
{
    if(e->key() == Qt::Key_Enter || e->key() == Qt::Key_Return)
        return;
    QDialog::keyPressEvent(e);
}

void StageSetup::accept()
{
    bool ok;
    params_->currPosRefType = static_cast<StageControlTraits::ReferenceType>(posRefComboBox->currentData().toInt(&ok));
    params_->initType = static_cast<StageControlTraits::InitType>(initComboBox->currentData().toInt(&ok));
    params_->range = rangeSpinBox->value();
    params_->measPos = measPosSpinBox->value();
    params_->measRefType = static_cast<StageControlTraits::ReferenceType>(measRefComboBox->currentData().toInt(&ok));
//    params_->measInitType = static_cast<StageControlTraits::InitType>(measInitComboBox->currentData().toInt(&ok));
    params_->calPos = calPosSpinBox->value();
    params_->calRefType = static_cast<StageControlTraits::ReferenceType>(calRefComboBox->currentData().toInt(&ok));
//    params_->calInitType = static_cast<StageControlTraits::InitType>(calInitComboBox->currentData().toInt(&ok));

    stageControl_->setStageRange(params_->range);
    stageControl_->setMeasPos(params_->measPos, params_->measRefType);
    stageControl_->setCalPos(params_->calPos, params_->calRefType);
    *appState_->stageParams() = *params_;
    QDialog::accept();
}

void StageSetup::reject()
{
    stageControl_->setStageRange(appState_->stageParams()->range);
    stageControl_->setMeasPos(appState_->stageParams()->measPos, appState_->stageParams()->measRefType);
    stageControl_->setMeasPos(appState_->stageParams()->calPos, appState_->stageParams()->calRefType);
    QDialog::reject();
}

void StageSetup::onInitialized()
{
//    onLowerLimChanged(stageControl_->lowerLim(StageControlTraits::ReferenceType::Absolute));
//    onUpperLimChanged(stageControl_->upperLim(StageControlTraits::ReferenceType::Absolute));
//    onCurrPosChanged(stageControl_->curPos(StageControlTraits::ReferenceType::Absolute));
    initButton->setEnabled(true);
    goUpLimButton->setEnabled(true);
    goLowLimButton->setEnabled(true);
    stopButton->setEnabled(true);
    measGroupBox->setEnabled(true);
    calGroupBox->setEnabled(true);
    measGoButton->setEnabled(true);
    calGoButton->setEnabled(true);
}

void StageSetup::onFailed()
{
    goUpLimButton->setEnabled(false);
    goLowLimButton->setEnabled(false);
    stopButton->setEnabled(true);
    measGroupBox->setEnabled(false);
    calGroupBox->setEnabled(false);
}

void StageSetup::onStageStarted()
{
    initButton->setEnabled(false);
    goUpLimButton->setEnabled(false);
    goLowLimButton->setEnabled(false);
    measGoButton->setEnabled(false);
    calGoButton->setEnabled(false);
    stopButton->setEnabled(true);
}

void StageSetup::onStageFinished()
{
    qDebug() << "StageSetup::onStageFinished()";
    initButton->setEnabled(true);
    goUpLimButton->setEnabled(true);
    goLowLimButton->setEnabled(true);
    measGoButton->setEnabled(true);
    calGoButton->setEnabled(true);
    stopButton->setEnabled(true);
    qDebug() << "StageSetup::onStageFinished(): finished";
}

void StageSetup::onInitStageFinished()
{
    delete initStage;
    initStage = nullptr;
    if (stageControl_->initialized()) {
        onInitialized();
    } else {
        onFailed();
    }
}

void StageSetup::onGoToLimStageFinished()
{
    delete goToLimStage;
    goToLimStage = nullptr;
    onStageFinished();
}

void StageSetup::onGoToPosStageFinished()
{
    delete goToPosStage;
    goToPosStage = nullptr;
    onStageFinished();
}

void StageSetup::onCurrPosChanged(int pos)
{
    absoluteCurrPos = pos;
    switch (params_->currPosRefType) {
    case StageControlTraits::ReferenceType::Absolute :
        currPosLabel->setText(tr("%1").arg(absoluteCurrPos));
        break;
    case StageControlTraits::ReferenceType::LowerLimit :
        currPosLabel->setText(tr("%1").arg(absoluteCurrPos - absoluteLowerLim));
        break;
    case StageControlTraits::ReferenceType::UpperLimit :
        currPosLabel->setText(tr("%1").arg(absoluteUpperLim - absoluteCurrPos));
        break;
    }
}

void StageSetup::onLowerLimChanged(int lim)
{
    absoluteLowerLim = lim;
    switch (params_->currPosRefType) {
    case StageControlTraits::ReferenceType::Absolute :
        lowerLimitLabel->setText(tr("%1").arg(absoluteLowerLim));
        break;
    case StageControlTraits::ReferenceType::LowerLimit :
        upperLimitLabel->setText(tr("%1").arg(absoluteUpperLim - absoluteLowerLim));
        currPosLabel->setText(tr("%1").arg(absoluteCurrPos - absoluteLowerLim));
        break;
    case StageControlTraits::ReferenceType::UpperLimit :
        lowerLimitLabel->setText(tr("%1").arg(absoluteUpperLim - absoluteLowerLim));
        break;
    }
}

void StageSetup::onUpperLimChanged(int lim)
{
    absoluteUpperLim = lim;
    switch (params_->currPosRefType) {
    case StageControlTraits::ReferenceType::Absolute :
        upperLimitLabel->setText(tr("%1").arg(absoluteUpperLim));
        break;
    case StageControlTraits::ReferenceType::LowerLimit :
        upperLimitLabel->setText(tr("%1").arg(absoluteUpperLim - absoluteLowerLim));
        break;
    case StageControlTraits::ReferenceType::UpperLimit :
        lowerLimitLabel->setText(tr("%1").arg(absoluteUpperLim - absoluteLowerLim));
        currPosLabel->setText(tr("%1").arg(absoluteUpperLim - absoluteCurrPos));
        break;
    }

}

void StageSetup::onInitButtonReleased()
{
    onStageStarted();
    bool ok;
    params_->initType = static_cast<StageControlTraits::InitType>(initComboBox->currentData().toInt(&ok));
    params_->range = rangeSpinBox->value();
    stageControl_->setStageRange(params_->range);
    if (stageControl_->motorRunning()) {
        QMessageBox::critical(this, tr("Stage is already running!"), tr("The stage is already running, stop it, please!"), QMessageBox::Ok);
    } else if (initStage || goToPosStage) {
        QMessageBox::critical(this, tr("Stage is controlled by another process!"), tr("The stage is controlled by another process, stop it, please!"), QMessageBox::Ok);
    } else {
        initStage = new StageTasks::Initialize(appState_, params_->initType, 0, this);
        connect(initStage, &StageTasks::Initialize::finished, this, &StageSetup::onInitStageFinished);
        connect(stopButton, &QPushButton::released, initStage, &StageTasks::Initialize::stop);
        initStage->start();
    }
}

void StageSetup::onStopButtonReleased()
{
    qDebug() << "StageSetup::onStopButtonReleased()";
    if (!initStage && !goToLimStage) {
        qDebug() << "StageSetup::onStopButtonReleased(): stopping stage";
        stageControl_->stop();
    }
}

void StageSetup::onInitComboboxCurrentIndexChanged(int index)
{
    bool ok;
    StageControlTraits::InitType initType =
            static_cast<StageControlTraits::InitType>(
                initComboBox->itemData(index).toInt(&ok));
    if (initType == (StageControlTraits::InitType::LowerLimit | StageControlTraits::InitType::UpperLimit)) {
        rangeSpinBox->setEnabled(false);
    } else {
        rangeSpinBox->setEnabled(true);
    }
}

void StageSetup::onPosRefComboboxCurrentIndexChanged(int index)
{
    bool ok;
    params_->currPosRefType =
            static_cast<StageControlTraits::ReferenceType>(
                posRefComboBox->itemData(index).toInt(&ok));
    switch (params_->currPosRefType) {
    case StageControlTraits::ReferenceType::Absolute :
        lowerLimitLabel->setText(tr("%1").arg(absoluteLowerLim));
        upperLimitLabel->setText(tr("%1").arg(absoluteUpperLim));
        currPosLabel->setText(tr("%1").arg(absoluteCurrPos));
        break;
    case StageControlTraits::ReferenceType::LowerLimit :
        lowerLimitLabel->setText(tr("%1").arg(0));
        upperLimitLabel->setText(tr("%1").arg(absoluteUpperLim - absoluteLowerLim));
        currPosLabel->setText(tr("%1").arg(absoluteCurrPos - absoluteLowerLim));
        break;
    case StageControlTraits::ReferenceType::UpperLimit :
        lowerLimitLabel->setText(tr("%1").arg(absoluteUpperLim - absoluteLowerLim));
        upperLimitLabel->setText(tr("%1").arg(0));
        currPosLabel->setText(tr("%1").arg(absoluteUpperLim - absoluteCurrPos));
        break;
    }
}

void StageSetup::onGoUpLimButtonReleased()
{
    onStageStarted();
    if (stageControl_->motorRunning()) {
        QMessageBox::critical(this, tr("Stage is already running!"), tr("The stage is already running, stop it, please!"), QMessageBox::Ok);
    } else if (initStage || goToLimStage || goToPosStage) {
        QMessageBox::critical(this, tr("Stage is controlled by another process!"), tr("The stage is controlled by another process, stop it, please!"), QMessageBox::Ok);
    } else {
        qDebug() << "StageSetup::onGoUpLimButtonReleased()";
        goToLimStage = new StageTasks::GoToLimWaitList(appState_, StageControlTraits::LimType::UpperLimit, this);
        qDebug() << "StageSetup::onGoUpLimButtonReleased(): 2";
        connect(goToLimStage, &StageTasks::GoToLimWaitList::finished, this, &StageSetup::onGoToLimStageFinished);
        qDebug() << "StageSetup::onGoUpLimButtonReleased(): 3";
        connect(stopButton, &QPushButton::released, goToLimStage, &StageTasks::GoToLimWaitList::stop);
        qDebug() << "StageSetup::onGoUpLimButtonReleased(): 4";
        goToLimStage->start();
        qDebug() << "StageSetup::onGoUpLimButtonReleased(): finished";
    }
}

void StageSetup::onGoLowLimButtonReleased()
{
    onStageStarted();
    if (stageControl_->motorRunning()) {
        QMessageBox::critical(this, tr("Stage is already running!"), tr("The stage is already running, stop it, please!"), QMessageBox::Ok);
    } else if (initStage || goToLimStage || goToPosStage) {
        QMessageBox::critical(this, tr("Stage is controlled by another process!"), tr("The stage is controlled by another process, stop it, please!"), QMessageBox::Ok);
    } else {
        goToLimStage = new StageTasks::GoToLimWaitList(appState_, StageControlTraits::LimType::LowerLimit, this);
        connect(goToLimStage, &StageTasks::GoToLimWaitList::finished, this, &StageSetup::onGoToLimStageFinished);
        connect(stopButton, &QPushButton::released, goToLimStage, &StageTasks::GoToLimWaitList::stop);
        goToLimStage->start();
    }
}

void StageSetup::onMeasGoButtonReleased()
{
    bool ok;
    params_->measPos = measPosSpinBox->value();
    params_->measRefType = static_cast<StageControlTraits::ReferenceType>(
                measRefComboBox->currentData().toInt(&ok));
    stageControl_->setMeasPos(params_->measPos, params_->measRefType);
    onStageStarted();
    if (stageControl_->motorRunning()) {
        QMessageBox::critical(this, tr("Stage is already running!"), tr("The stage is already running, stop it, please!"), QMessageBox::Ok);
    } else if (initStage || goToLimStage || goToPosStage) {
        QMessageBox::critical(this, tr("Stage is controlled by another process!"), tr("The stage is controlled by another process, stop it, please!"), QMessageBox::Ok);
    } else {
        goToPosStage = new StageTasks::GoToPos(appState_, StageControlTraits::PosType::Measurement, this);
        connect(goToPosStage, &StageTasks::GoToPos::finished, this, &StageSetup::onGoToPosStageFinished);
        goToPosStage->start();
    }
}

void StageSetup::onCalGoButtonReleased()
{
    bool ok;
    params_->calPos = calPosSpinBox->value();
    params_->calRefType = static_cast<StageControlTraits::ReferenceType>(
                calRefComboBox->currentData().toInt(&ok));
    stageControl_->setCalPos(params_->calPos, params_->calRefType);
    onStageStarted();
    if (stageControl_->motorRunning()) {
        QMessageBox::critical(this, tr("Stage is already running!"), tr("The stage is already running, stop it, please!"), QMessageBox::Ok);
    } else if (initStage || goToLimStage || goToPosStage) {
        QMessageBox::critical(this, tr("Stage is controlled by another process!"), tr("The stage is controlled by another process, stop it, please!"), QMessageBox::Ok);
    } else {
        goToPosStage = new StageTasks::GoToPos(appState_, StageControlTraits::PosType::Calibration, this);
        connect(goToPosStage, &StageTasks::GoToPos::finished, this, &StageSetup::onGoToPosStageFinished);
        goToPosStage->start();
    }
}

StageSetup::~StageSetup()
{
    delete params_;
    delete initStage;
    delete goToLimStage;
    delete goToPosStage;
}

