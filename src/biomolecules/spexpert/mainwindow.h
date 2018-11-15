#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

// forward declarations
class CentralWidget;
class AppCore;
class QAction;
class QActionGroup;
class StageSetup;
namespace NeslabusWidgets
{
class MainDialogWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = 0);
    virtual ~MainWindow();

signals:

public slots:
    void onStartReadingTemperature();
    void onFinishReadingTemperature();

protected:
    void closeEvent(QCloseEvent *event);

private slots:
    void onLanguageChanged(QAction * action);
    void onExperimentActionTrigered();
    void onStageSetupActionTrigered();
    void onNeslabSetupActionTrigered();
    void onExperimentStarted();
    void onExperimentFinished();


private:
    void createActions();
    void createMenus();
    void createLanguageMenu();
    void readSettings();

    CentralWidget *centralWidget;
    NeslabusWidgets::MainDialogWindow *neslabDialogWindow;
    AppCore *appCore;

    QAction *exitAction;

    QAction *experimentAction;
    QAction *stageSetupAction;
    QAction *neslabSetupAction;

    QActionGroup * langActionGroup;

    QMenu *fileMenu;
    QMenu *setupMenu;
    QMenu *languageMenu;

    QString langPath;
    QString defaultLanguage;

    QString neslabComPortName;
};

#endif // MAINWINDOW_H
