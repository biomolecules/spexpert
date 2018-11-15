#ifndef TIMESPAN_H
#define TIMESPAN_H

// forward declarations
class QDateTime;
class QString;

class TimeSpan
{
public:
    TimeSpan();
    TimeSpan(int h, int m = 0, int s = 0, int ms = 0);
    TimeSpan &reset(int h, int m = 0, int s = 0, int ms = 0);
    TimeSpan &fromMSec(int ms);
    TimeSpan &fromSec(int s);
    int toMSec() const;
    int toSec() const;
    QString toString() const;
    inline int hours() const { return hours_; }
    inline int mins() const { return mins_; }
    inline int secs() const { return secs_; }
    inline int msecs() const { return msecs_; }

private:
    int hours_;
    int mins_;
    int secs_;
    int msecs_;
};

TimeSpan operator-(const QDateTime & a, const QDateTime & b);

#endif // TIMESPAN_H
