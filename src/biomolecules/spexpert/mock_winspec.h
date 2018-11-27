#ifndef BIOMOLECULES_SPEXPERT_MOCK_WINSPEC_H_
#define BIOMOLECULES_SPEXPERT_MOCK_WINSPEC_H_

#include <memory>

#include <QObject>
#include <QString>
#include <QVariantList>


// forward declarations
class QTimer;


namespace biomolecules {
namespace spexpert {
namespace core {

class MockWinSpec
{
public:
    MockWinSpec();
    ~MockWinSpec();

    bool Start_2(double expo, int acc, int frm, const QString& file_name);
    void StopRunning();
    bool Running();
    int GetAccum();
    int GetFrame();
    QVariantList GetSpectrum(int& frm, int& x, int& y, bool& status);
    QVariantList GetLastFrame(int& frm, int& x, int& y, bool& status);
    void GetWinSpecAcqParams(double& expo, int& acc, int& frm);
    void GetWinSpecAcqParams_2(double& expo, int& acc);
    void SetWinSpecAcqParams(double expo, int acc, int frm);
    void SetWinSpecAcqParams_2(double expo, int acc);
    void GetFilePath(QString& file_name);
    void SetFilePath(const QString& file_name);
    bool Save();
    bool SaveAs(const QString& file_name);
    bool ActivateWindow();
    void CloseAllWindows();
    void CloseWindow();
    bool QuitWinSpec();
    bool ShowWinSpecWin();
    bool WinSpecConnectionFailed();

private:
    const int kXSize_ = 1024;
    const int kYSize_ = 1;
    const int kMaxCounts_ = 2047;
    double expo_;
    int accums_;
    int frames_;
    QString file_path_;
    std::unique_ptr<QTimer> measurement_timer_;
};

}  // namespace core
}  // namespace spexpert
}  // namespace biomolecules

#endif  // BIOMOLECULES_SPEXPERT_MOCK_WINSPEC_H_
