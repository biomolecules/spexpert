#include "centralwidget.h"
#include "appcore.h"

#include <QPushButton>
#include <QLayout>
#include "plotproxy.h"

CentralWidget::CentralWidget(QWidget *parent) :
    QWidget(parent)
{
//    appCore = new AppCore(this, reinterpret_cast<MainWindow*>(parent), this);
    startButton = new QPushButton("Start", this);
    stopButton = new QPushButton("Stop", this);
    goToMeasButton = new QPushButton("GoToMeas", this);
    goToCalButton = new QPushButton("GoToCal", this);
    stopStageButton = new QPushButton("Stop stage", this);
    buttonsWidget = new QWidget(this);
    buttonsWidget->setFixedWidth(100);

    plotProxy = new PlotProxy(this);

    connect(startButton, &QPushButton::released, this, &CentralWidget::onStartButtonReleased);
    connect(stopButton, &QPushButton::released, this, &CentralWidget::onStopButtonReleased);
    connect(goToMeasButton, &QPushButton::released, this, &CentralWidget::onGoToMeasButtonReleased);
    connect(goToCalButton, &QPushButton::released, this, &CentralWidget::onGoToCalButtonReleased);
    connect(stopStageButton, &QPushButton::released, this, &CentralWidget::onStopStageButtonReleased);

    QHBoxLayout *horizontalLayout = new QHBoxLayout;
    this->setLayout(horizontalLayout);
    horizontalLayout->addWidget(buttonsWidget);

    QVBoxLayout *buttonsLayout = new QVBoxLayout(buttonsWidget);
    buttonsLayout->addWidget(startButton);
    buttonsLayout->addWidget(stopButton);
    buttonsLayout->addWidget(goToMeasButton);
    buttonsLayout->addWidget(goToCalButton);
    buttonsLayout->addWidget(stopStageButton);
    buttonsLayout->addStretch();
    // horizontalLayout->addStretch();
    // horizontalLayout->addLayout(buttonsLayout);

    horizontalLayout->addWidget(plotProxy->getMainPlotWidget());
}

QSize CentralWidget::sizeHint() const
{
    return QSize(600, 400);
}

void CentralWidget::onExperimentStarted()
{
    startButton->setEnabled(false);
    stopButton->setEnabled(true);
    goToMeasButton->setEnabled(false);
    goToCalButton->setEnabled(false);
    stopStageButton->setEnabled(false);
}

void CentralWidget::onExperimentFinished()
{
    startButton->setEnabled(true);
    stopButton->setEnabled(true);
    goToMeasButton->setEnabled(true);
    goToCalButton->setEnabled(true);
    stopStageButton->setEnabled(true);
}

void CentralWidget::onStageMovementStarted()
{
    startButton->setEnabled(false);
    stopButton->setEnabled(false);
    goToMeasButton->setEnabled(false);
    goToCalButton->setEnabled(false);
    stopStageButton->setEnabled(true);
}

void CentralWidget::onStageMovementFinished()
{
    startButton->setEnabled(true);
    stopButton->setEnabled(false);
    goToMeasButton->setEnabled(true);
    goToCalButton->setEnabled(true);
    stopStageButton->setEnabled(true);
}

void CentralWidget::onStartButtonReleased()
{
    qDebug() << "Start";
    emit startExperiment();
//    appCore->startExperiment();
}

void CentralWidget::onStopButtonReleased()
{
    qDebug() << "Stop";
    emit stopExperiment();
//    appCore->stopExperiment();
}

void CentralWidget::onGoToMeasButtonReleased()
{
    emit goToMeas();
//    appCore->onGoToMeas();
}

void CentralWidget::onGoToCalButtonReleased()
{
    emit goToCal();
//    appCore->onGoToCal();
}

void CentralWidget::onStopStageButtonReleased()
{
    emit stopStage();
}

CentralWidget::~CentralWidget()
{
}
