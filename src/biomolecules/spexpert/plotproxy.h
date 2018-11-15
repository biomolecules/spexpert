#ifndef PLOTPROXY_H
#define PLOTPROXY_H

#include <QObject>

// forward declarations
class QCustomPlot;
class AppState;

class PlotProxy : public QObject
{
    Q_OBJECT
public:
    explicit PlotProxy(QWidget *parent = 0);
    virtual ~PlotProxy();

    QCustomPlot* getMainPlot();
    QWidget *getMainPlotWidget();
signals:

public slots:
    void updatePlot(AppState *appState);
    void plotTypeChanged(AppState *appState);
    void onPlotTemperatures();
    void onUpdateMeasT(AppState *appState);
    void onUpdateReadT(AppState *appState);

private:
    QCustomPlot *mainPlot;

    double xmin;
    double xmax;
    double ymin;
    double ymax;
};

#endif // PLOTPROXY_H
