#include "plotproxy.h"

#include "appstate.h"
#include "lockableqvector.h"

#include <QMutex>
#include <QMutexLocker>

#include "qcustomplot/qcustomplot.h"


PlotProxy::PlotProxy(QWidget *parent) :
    QObject(parent)
{
    mainPlot = new QCustomPlot(parent);
}

QCustomPlot *PlotProxy::getMainPlot()
{
    return mainPlot;
}

QWidget *PlotProxy::getMainPlotWidget()
{
    return mainPlot;
}

void PlotProxy::updatePlot(AppState *appState)
{
    switch (appState->plotStyle())
    {
    case AppStateTraits::PlotStyle::Spectra :
        switch (appState->getPlotType())
        {
        case AppStateTraits::PlotType::Frame :
            if (appState->lastFrameChanged())
            {
                QMutexLocker lockerY(appState->getLastFrame().toMutexY());
                QMutexLocker lockerX(appState->getLastFrame().toMutexX());
                mainPlot->graph(0)->setData(appState->getLastFrame().toXQVector(), appState->getLastFrame().toYQVector());
                lockerY.unlock();
                lockerX.unlock();
                appState->setLastFrameChanged(false);
                mainPlot->rescaleAxes();
                mainPlot->replot();
            }
            break;
        case AppStateTraits::PlotType::Spectrum :
            if (appState->spectrumChanged())
            {
                mainPlot->clearGraphs();
                ConstSpectrumIterator it;
                ConstXSpectrumIterator itX;
                int ii;
                QMutexLocker lockerY(appState->getSpectrum().toMutexY());
                QMutexLocker lockerX(appState->getSpectrum().toMutexX());
                for (ii = 0, it = appState->getPlotSpectrum().toYQVector().begin(), itX = appState->getPlotSpectrum().toXQVector().begin();
                     it != appState->getPlotSpectrum().toYQVector().end(); ++ii, ++it, ++itX)
                {
                    mainPlot->addGraph();
                    mainPlot->graph(ii)->setData(*itX, *it);
                }
                lockerY.unlock();
                lockerX.unlock();
                appState->setSpectrumChanged(false);
                double lims[4];
                appState->getPlotSpectrum().getLimsSafe(lims);
                double dX = (lims[1] - lims[0]) / 10;
                double dY = (lims[3] - lims[2]) / 10;
                mainPlot->xAxis->setRange(lims[0] - dX, lims[1] + dX);
                mainPlot->yAxis->setRange(lims[2] - dY, lims[3] + dY);
                mainPlot->replot();
            }
            break;
        case AppStateTraits::PlotType::None :
            mainPlot->replot();
            break;
        }
        break;
    default :
        break;
    }
}

void PlotProxy::plotTypeChanged(AppState *appState)
{
    qDebug() << "PlotProxy::plotTypeChanged()";
    if (appState->plotStyle() == AppStateTraits::PlotStyle::Spectra) {
        mainPlot->clearGraphs();

        switch(appState->getPlotType())
        {
        case AppStateTraits::PlotType::Frame :
            mainPlot->addGraph();
            break;
        default :
            break;
        }
        updatePlot(appState);
    }
}

void PlotProxy::onPlotTemperatures()
{
    qDebug() << "PlotProxy::onPlotTemperatures()";
    mainPlot->clearGraphs();
    mainPlot->addGraph();
    mainPlot->graph(0)->setPen(QPen(Qt::blue));
    mainPlot->graph(0)->setLineStyle(QCPGraph::lsLine);
    mainPlot->graph(0)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssNone));
    mainPlot->addGraph();
    mainPlot->graph(1)->setPen(QPen(Qt::red));
    mainPlot->graph(1)->setLineStyle(QCPGraph::lsNone);
    mainPlot->graph(1)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 6));
    mainPlot->xAxis->setLabel("time (s)");
    mainPlot->yAxis->setLabel("temperature (°C)");
    mainPlot->replot();
//    mainPlot->xAxis->setTickLabelType(QCPAxis::ltDateTime);
//    mainPlot->xAxis->setDateTimeFormat("dd'd'hh:mm:ss");
}

void PlotProxy::onUpdateMeasT(AppState *appState)
{
    mainPlot->graph(1)->setData(appState->tMeasTs(), appState->measTs());
    xmax = appState->tReadTs().last();
    xmin = appState->tReadTs().first();
    ymax = appState->tYMax();
    ymin = appState->tYMin();
    if (xmax == xmin) {
        mainPlot->xAxis->setRange(xmin - 0.5, xmax + 0.5);
    } else {
        mainPlot->xAxis->setRange(xmin - (xmax - xmin) / 20.0 , xmax + (xmax - xmin) / 20.0 );
    }
    if (ymax == ymin) {
        mainPlot->yAxis->setRange(ymin - 0.5, ymax + 0.5);
    } else {
        mainPlot->yAxis->setRange(ymin - (ymax - ymin) / 20.0 , ymax + (ymax - ymin) / 20.0 );
    }
    mainPlot->replot();
}

void PlotProxy::onUpdateReadT(AppState *appState)
{
    xmax = appState->tReadTs().last();
    xmin = appState->tReadTs().first();
    ymax = appState->tYMax();
    ymin = appState->tYMin();
    if (xmax == xmin) {
        mainPlot->xAxis->setRange(xmin - 0.5, xmax + 0.5);
    } else {
        mainPlot->xAxis->setRange(xmin - (xmax - xmin) / 20.0 , xmax + (xmax - xmin) / 20.0 );
    }
    if (ymax == ymin) {
        mainPlot->yAxis->setRange(ymin - 0.5, ymax + 0.5);
    } else {
        mainPlot->yAxis->setRange(ymin - (ymax - ymin) / 20.0 , ymax + (ymax - ymin) / 20.0 );
    }
    mainPlot->replot();mainPlot->graph(0)->setData(appState->tReadTs(), appState->readTs());
    mainPlot->replot();
}

PlotProxy::~PlotProxy()
{

}
