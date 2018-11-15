#ifndef CENTRALWIDGET_H
#define CENTRALWIDGET_H

#include <QWidget>
#include <QDebug>

// forward declarations
namespace WinSpecVB
{
    class WinSpecVB;
}
class QPushButton;
class QVBoxLayout;
class QHBoxLayout;
class AppCore;
class PlotProxy;

class CentralWidget : public QWidget
{
    Q_OBJECT
public:
    explicit CentralWidget(QWidget *parent = 0);
    virtual ~CentralWidget();
    virtual QSize sizeHint() const;

signals:
    void stopStage();
    void startExperiment();
    void stopExperiment();
    void goToMeas();
    void goToCal();

public slots:
    void onExperimentStarted();
    void onExperimentFinished();
    void onStageMovementStarted();
    void onStageMovementFinished();

private slots:
    void onStartButtonReleased();
    void onStopButtonReleased();
    void onGoToMeasButtonReleased();
    void onGoToCalButtonReleased();
    void onStopStageButtonReleased();

private:
    friend class AppCore;
    QPushButton *startButton;
    QPushButton *stopButton;
    QPushButton *goToMeasButton;
    QPushButton *goToCalButton;
    QPushButton *stopStageButton;
    QWidget *buttonsWidget;

    PlotProxy *plotProxy;
};

#endif // CENTRALWIDGET_H
