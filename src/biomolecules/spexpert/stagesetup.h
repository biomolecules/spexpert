#ifndef STAGECONTROLDIALOG_H
#define STAGECONTROLDIALOG_H

#include <QDialog>

// forward declarations
class AppState;
class StageControl;
class QPushButton;
class QDialogButtonBox;
class QGroupBox;
class QComboBox;
class QLabel;
class QSpinBox;
namespace StageControlTraits
{
class Params;
}
namespace StageTasks
{
class Initialize;
class GoToLimWaitList;
class GoToPos;
}

class StageSetup : public QDialog
{
    Q_OBJECT
public:
    explicit StageSetup(AppState *appState, QWidget *parent = 0);
    virtual ~StageSetup();

    virtual void keyPressEvent(QKeyEvent *e);

signals:

public slots:
    virtual void accept();
    virtual void reject();
    void onInitialized();
    void onFailed();
    void onStageStarted();
    void onStageFinished();
    void onInitStageFinished();
    void onGoToLimStageFinished();
    void onGoToPosStageFinished();
    void onCurrPosChanged(int pos);
    void onLowerLimChanged(int lim);
    void onUpperLimChanged(int lim);

private slots:
    void onInitButtonReleased();
    void onStopButtonReleased();
    void onInitComboboxCurrentIndexChanged(int index);
    void onPosRefComboboxCurrentIndexChanged(int index);
    void onGoUpLimButtonReleased();
    void onGoLowLimButtonReleased();
    void onMeasGoButtonReleased();
    void onCalGoButtonReleased();

private:
    QPushButton *initButton;
    QPushButton *goUpLimButton;
    QPushButton *goLowLimButton;
    QPushButton *stopButton;
    QGroupBox *posGroupBox;
    QComboBox *posRefComboBox;
    QLabel *upperLimitLabel;
    QLabel *lowerLimitLabel;
    QLabel *currPosLabel;
    QGroupBox *initGroupBox;
    QComboBox *initComboBox;
    QSpinBox *rangeSpinBox;
    QGroupBox *measGroupBox;
    QSpinBox *measPosSpinBox;
    QComboBox *measRefComboBox;
//    QComboBox *measInitComboBox;
    QPushButton *measGoButton;
    QGroupBox *calGroupBox;
    QSpinBox *calPosSpinBox;
    QComboBox *calRefComboBox;
//    QComboBox *calInitComboBox;
    QPushButton *calGoButton;
    QDialogButtonBox *dialogButtonBox;

    AppState *appState_;
    StageControl *stageControl_;
    StageControlTraits::Params *params_;
    StageTasks::Initialize *initStage;
    StageTasks::GoToLimWaitList *goToLimStage;
    StageTasks::GoToPos *goToPosStage;

    int absoluteLowerLim;
    int absoluteUpperLim;
    int absoluteCurrPos;
};

#endif // STAGECONTROLDIALOG_H
