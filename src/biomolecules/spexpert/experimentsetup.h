#ifndef EXPERIMENTSETUP_H
#define EXPERIMENTSETUP_H

#include <QDialog>
#include <QAbstractTableModel>
#include <QTableView>

// forward declarations
class AppState;
class StageControl;
class Neslab;
class QTabWidget;
class QDialogButtonBox;
class QLineEdit;
class QDoubleSpinBox;
class QSpinBox;
class QGroupBox;
class QCheckBox;

namespace AppStateTraits
{
class ExpWinSpecParams;
class CalWinSpecParams;
class InitWinSpecParams;
}

namespace StageTasks
{
class Initialize;
}

class ExpSettingsTab : public QWidget
{
    Q_OBJECT
public:
    explicit ExpSettingsTab(AppStateTraits::InitWinSpecParams &params, AppState *appState, bool extRange, int expNumber, QWidget *parent = 0);
    virtual ~ExpSettingsTab();

    bool saveParams();

signals:
    void initStage();

public slots:
    void onInitStageFinished();

private slots:
    void onFilenameButtonClicked();
    void onFilenameEditEditingFinished();
    void onAutoCalGroupBoxClicked(bool checked);

private:
    QLineEdit *filenameEdit;
    QPushButton *filenameButton;
    QGroupBox *expGroupBox;
    QDoubleSpinBox *expoSpinBox;
    QSpinBox *frmSpinBox;
    QSpinBox *accSpinBox;

    QSpinBox *grPosSpinBox;
    QGroupBox *autoCalGroupBox;
//    QSpinBox *eachMeasSpinBox;
    QDoubleSpinBox *calExpoSpinBox;
    QSpinBox *calFrmSpinBox;
    QSpinBox *calAccSpinBox;

    AppStateTraits::InitWinSpecParams &params_;
    AppState *appState_;
    bool extRange_;
    int expNumber_;
};

class BatchExpTab : public QWidget
{
    Q_OBJECT
public:
    explicit BatchExpTab(AppStateTraits::InitWinSpecParams &params, QWidget *parent = 0);

    bool saveParams();

signals:
    void batchExpStateChanged(bool state);

private slots:
    void onBatchExpGroupBoxClicked(bool checked);

private:
    QGroupBox *batchExpGroupBox;
    QSpinBox *numSpectraSpinBox;
    QSpinBox *firstNumSpinBox;
    QSpinBox *stepSpinBox;
    QSpinBox *numDigitsSpinBox;

    QGroupBox *delayGroupBox;
    QSpinBox *hoursSpinBox;
    QSpinBox *minsSpinBox;
    QSpinBox *secsSpinBox;

    AppStateTraits::InitWinSpecParams &params_;
};

class TSettingsTab : public QWidget
{
    Q_OBJECT
public:
    TSettingsTab(AppStateTraits::InitWinSpecParams &params, Neslab *neslab, QWidget *parent = 0);

    bool saveParams();

signals:
    void tMeasStateChanged(bool state);

private slots:
    void onNeslabConnected();
    void onReadLowTemperatureLimitFinished(double T, bool enhancedPrec);
    void onReadHighTemperatureLimitFinished(double T, bool enhancedPrec);
    void onTMeasGroupBoxClicked(bool checked);
    void onSameAsTCheckBoxStateChanged(int state);
    void onSetStartTButtonReleased();

private:
    void enableTMeas();

    QGroupBox *tMeasGroupBox;
    QGroupBox *startTGroupBox;
    QDoubleSpinBox *startTSpinBox;
    QPushButton *setStartTPushButton;
    QDoubleSpinBox *stepTSpinBox;
    QDoubleSpinBox *endTSpinBox;
    QGroupBox *initDelayGroupBox;
    QSpinBox *initHoursSpinBox;
    QSpinBox *initMinsSpinBox;
    QSpinBox *initSecsSpinBox;
    QGroupBox *delayGroupBox;
    QSpinBox *hoursSpinBox;
    QSpinBox *minsSpinBox;
    QSpinBox *secsSpinBox;
    QGroupBox *loopGroupBox;
    QSpinBox *loopHoursSpinBox;
    QSpinBox *loopMinsSpinBox;
    QSpinBox *loopSecsSpinBox;
    QGroupBox *afterMeasGroupBox;
    QDoubleSpinBox *afterMeasSpinBox;
    QGroupBox *filenamesGroupBox;
    QCheckBox *sameAsTCheckBox;
    QSpinBox *firstNumSpinBox;
    QSpinBox *stepSpinBox;
    QSpinBox *numDigitsSpinBox;

    Neslab *neslab_;
    AppStateTraits::InitWinSpecParams &params_;
    bool gotLowTLim;
    bool gotHighTLim;
};

namespace ExtRanTabTraits
{

struct Params
{
    QString fileName;
    int grPos;
    double expo;
    int acc;
    int frm;
};

class Model : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit Model(
            AppStateTraits::InitWinSpecParams &initWinSpecParams,
            QObject *parent = 0);
    Qt::ItemFlags flags(const QModelIndex &index) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role) const;
    bool setData(const QModelIndex &index, const QVariant &value,
                 int role = Qt::DisplayRole);
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const;
    bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex());
    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex());

    bool moveRows(const QModelIndex &sourceParent, int sourceRow, int count, const QModelIndex &destinationParent, int destinationChild);

private:
    AppStateTraits::InitWinSpecParams &initWinSpecParams_;
    QList<Params> params_;
};

class Dialog : public QDialog
{
    Q_OBJECT

public:
    Dialog(AppStateTraits::InitWinSpecParams &params, AppState *appState, int expNumber,
                    QWidget *parent = 0);
    ~Dialog();

    virtual void keyPressEvent(QKeyEvent *e);

    void accept();

private:
    ExpSettingsTab *expSettings;

    AppStateTraits::InitWinSpecParams &saveParams_;
    AppStateTraits::InitWinSpecParams *params_;
    QDialogButtonBox *dialogButtonBox;
};

class ViewEventFilter : public QObject
{
    Q_OBJECT

public:
    explicit ViewEventFilter(QObject *parent = 0);

signals:
    void pressedKey(int key);

protected:
    bool eventFilter(QObject *obj, QEvent *event);
};

} // ExtRanTabTraits

class ExtRanTab : public QWidget
{
    Q_OBJECT
public:
    explicit ExtRanTab(AppStateTraits::InitWinSpecParams &params, AppState *appState, QWidget *parent = 0);

    bool saveParams();

signals:
    void extRanStateChanged(bool state);

public slots:
    void autorenameFilenames();

private slots:
    void onExtRanGroupBoxClicked(bool state);
    void onAutoFilenamesGroupBoxClicked(bool state);
    void onFilenameButtonReleased();
    void onFilenameEditEditingFinished();
    void onAddButtonClicked();
    void onEditButtonClicked();
    void onRemoveButtonClicked();
    void onMoveUpButtonClicked();
    void onMoveDownButtonClicked();
    void onViewDoubleClicked(const QModelIndex &index);
    void onViewPressedKey(int key);

private:
    void addItem(int expNumber);
    void editItem(int expNumber);
    void removeItem(int expNumber);

    QGroupBox *extRanGroupBox;
    QGroupBox *autoFilenamesGroupBox;
    QLineEdit *filenameEdit;
    QPushButton *filenameButton;

    QAbstractTableModel *model;
    QTableView *view;
    QPushButton *addButton;
    QPushButton *editButton;
    QPushButton *removeButton;
    QPushButton *moveUpButton;
    QPushButton *moveDownButton;

    AppStateTraits::InitWinSpecParams &params_;
    AppState *appState_;
};

class ExperimentSetup : public QDialog
{
    Q_OBJECT
public:
    explicit ExperimentSetup(AppState *appState, QWidget *parent = 0);
    virtual ~ExperimentSetup();

    virtual void keyPressEvent(QKeyEvent *e);

    static int countNumDigits(int number);

signals:
    void initStage();

public slots:
    virtual void accept();
    void onExpSettingsTabInitStage();
    void onInitStageFinished();

private slots:
    void onExtRanStateChanged(bool state);

private:
    ExpSettingsTab *expSettingsTab;
    BatchExpTab *batchExpTab;
    TSettingsTab *tSettingsTab;
    ExtRanTab *extRanTab;

    QTabWidget *tabWidget;
    QDialogButtonBox *dialogButtonBox;

    AppState *appState_;
    Neslab *neslab_;
    AppStateTraits::InitWinSpecParams *params_;
};

#endif // EXPERIMENTSETUP_H
