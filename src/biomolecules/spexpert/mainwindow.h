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
namespace biomolecules {
namespace spexpert {
namespace gui {
class RelayControlPanel;
}  // namespace gui
}  // namespace spexpert
}  // namespace biomolecules

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
    void onRelayControlPanelActionTrigered();
    void onExperimentStarted();
    void onExperimentFinished();
    void onK8090Connected();


private:
    void createActions();
    void createMenus();
    void createLanguageMenu();
    void readSettings();

    CentralWidget *centralWidget;
    NeslabusWidgets::MainDialogWindow *neslabDialogWindow;
    biomolecules::spexpert::gui::RelayControlPanel* relayControlPanel_;
    AppCore *appCore;

    QAction *exitAction;

    QAction *experimentAction;
    QAction *stageSetupAction;
    QAction *neslabSetupAction;
    QAction *relayControlPanelAction;

    QActionGroup * langActionGroup;

    QMenu *fileMenu;
    QMenu *setupMenu;
    QMenu *languageMenu;

    QString langPath;
    QString defaultLanguage;

    QString neslabComPortName;
    QString k8090ComPortName;
};

#endif // MAINWINDOW_H
