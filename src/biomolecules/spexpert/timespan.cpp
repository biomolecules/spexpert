#include "timespan.h"
#include <QDateTime>
//#include <QString>

TimeSpan::TimeSpan() :
    TimeSpan(0, 0, 0, 0)
{
}

TimeSpan::TimeSpan(int h, int m, int s, int ms)
{
    msecs_ = ms % 1000;
    secs_ = (s + ms/1000) % 60;
    mins_ = (m + (s + ms/1000) / 60) % 60;
    hours_ = h + (m + (s + ms/1000) / 60) / 60;
}

TimeSpan & TimeSpan::reset(int h, int m, int s, int ms)
{
    msecs_ = ms % 1000;
    secs_ = (s + ms/1000) % 60;
    mins_ = (m + (s + ms/1000) / 60) % 60;
    hours_ = h + (m + (s + ms/1000) / 60) / 60;
    return *this;
}

TimeSpan & TimeSpan::fromMSec(int ms)
{
    hours_ = ms / 3600000;
    msecs_ = ms % 3600000;
    mins_ = msecs_ / 60000;
    msecs_ %= 60000;
    secs_ = msecs_ / 1000;
    msecs_ %= 1000;
    return *this;
}

TimeSpan & TimeSpan::fromSec(int s)
{
    hours_ = s / 3600;
    secs_ = s % 3600;
    mins_ = secs_ / 60;
    secs_ %= 60;
    msecs_ = 0;
    return *this;
}

int TimeSpan::toMSec() const
{
    return hours_ * 3600000 + mins_ * 60000 + secs_ * 1000 + msecs_;
}

int TimeSpan::toSec() const
{
    return hours_ * 3600 + mins_ * 60 + secs_;
}

QString TimeSpan::toString() const
{
    QString timeSpan;
    bool blAnyTime = false;
    if (hours_ != 0)
    {
        timeSpan.append(QString("%1 h").arg(hours_));
        blAnyTime = true;
    }
    if (!blAnyTime && mins_ != 0)
    {
        timeSpan.append(QString("%1 min").arg(mins_));
        blAnyTime = true;
    }
    else if (blAnyTime)
    {
        timeSpan.append(QString(" %1 min").arg(mins_));
    }
    if (!blAnyTime)
    {
        timeSpan.append(QString("%1 s").arg(secs_));
    }
    else
    {
        timeSpan.append(QString(" %1 s").arg(secs_));
    }
    return timeSpan;
}


TimeSpan operator-(const QDateTime &a, const QDateTime &b)
{
    TimeSpan timeSpan;
    return timeSpan.fromMSec(int(a.toMSecsSinceEpoch() - b.toMSecsSinceEpoch()));
}
