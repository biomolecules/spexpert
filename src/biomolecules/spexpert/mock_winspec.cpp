#include "mock_winspec.h"

#ifdef __MINGW32__  // random device on MinGW does not work, a seed is always same so we need chrono for seed.
#include <chrono>
#endif
#include <random>

#include <QTimer>

namespace biomolecules {
namespace spexpert {
namespace core {

// generator of random numbers which is used throughout the class to produce random undefined responses (active timer
// delay query when the timer is not active) and random response delays from the card

// random device on MinGW does not work, a seed is always same.
#ifdef __MINGW32__
static std::mt19937_64& get_random_generator()
{
    thread_local std::mt19937_64 random_generator{
        static_cast<std::uint_fast64_t>(std::chrono::system_clock::now().time_since_epoch().count())};
    return random_generator;
}
#else   // ifdef __MINGW32__
static std::mt19937_64& get_random_generator()
{
    thread_local std::random_device random_device;
    thread_local std::mt19937_64 random_generator{random_device()};
    return random_generator;
}
#endif  // ifdef __MINGW32__


MockWinSpec::MockWinSpec()
    : expo_{1.0},
      accums_{1},
      frames_{1},
      measurement_timer_{new QTimer}
{
    measurement_timer_->setSingleShot(true);
}

MockWinSpec::~MockWinSpec()
{}

bool MockWinSpec::Start_2(double expo, int acc, int frm, const QString& file_name)
{
    file_path_ = file_name;
    expo_ = expo;
    frames_ = frm;
    accums_ = acc;
    measurement_timer_->start(static_cast<int>(1000 * expo * acc * frm));
    return true;
}


void MockWinSpec::StopRunning()
{
    measurement_timer_->stop();
}


bool MockWinSpec::Running()
{
    return measurement_timer_->isActive();
}


int MockWinSpec::GetAccum()
{
    return 0;
}


int MockWinSpec::GetFrame()
{
    return 0;
}


QVariantList MockWinSpec::GetSpectrum(int& frm, int& x, int& y, bool& status)
{
    QVariantList out;
    frm = frames_;
    x = kXSize_;
    y = kYSize_;
    out.reserve(frames_ * kXSize_ * kYSize_);
    std::uniform_int_distribution<int> distribution{
        0, kMaxCounts_};
    const int num_elems = frm * x * y;
    for (int i = 0; i < num_elems; ++i) {
        out.append(distribution(get_random_generator()));
    }
    status = true;
    return out;
}


QVariantList MockWinSpec::GetLastFrame(int& frm, int& x, int& y, bool& status)
{
    QVariantList out;
    frm = 0;
    x = kXSize_;
    y = kYSize_;
    out.reserve(kXSize_ * kYSize_);
    std::uniform_int_distribution<int> distribution{
        0, kMaxCounts_};
    const int num_elems = x * y;
    for (int i = 0; i < num_elems; ++i) {
        out.append(distribution(get_random_generator()));
    }
    status = true;
    return out;
}


void MockWinSpec::GetWinSpecAcqParams(double& expo, int& acc, int& frm)
{
    expo = expo_;
    acc = accums_;
    frm = frames_;
}


void MockWinSpec::GetWinSpecAcqParams_2(double& expo, int& acc)
{
    expo = expo_;
    acc = accums_;
}


void MockWinSpec::SetWinSpecAcqParams(double expo, int acc, int frm)
{
    expo_ = expo;
    accums_ = acc;
    frames_ = frm;
}


void MockWinSpec::SetWinSpecAcqParams_2(double expo, int acc)
{
    expo_ = expo;
    accums_ = acc;
}


void MockWinSpec::GetFilePath(QString& file_name)
{
    file_name = file_path_;
}


void MockWinSpec::SetFilePath(const QString& file_name)
{
    file_path_ = file_name;
}


bool MockWinSpec::Save()
{
    return true;
}


bool MockWinSpec::SaveAs(const QString& file_name)
{
    file_path_ = file_name;
    return true;
}


bool MockWinSpec::ActivateWindow()
{
    return true;
}


void MockWinSpec::CloseAllWindows()
{}


void MockWinSpec::CloseWindow()
{}


bool MockWinSpec::QuitWinSpec()
{
    return true;
}


bool MockWinSpec::ShowWinSpecWin()
{
    return true;
}


bool MockWinSpec::WinSpecConnectionFailed()
{
    return false;
}

}  // namespace core
}  // namespace spexpert
}  // namespace biomolecules
