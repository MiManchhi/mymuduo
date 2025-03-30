#include<time.h>
#include<string>

#include"Timestamp.h"

mymuduo::Timestamp::Timestamp() : m_microSecondSinceEpoch(0) {}
mymuduo::Timestamp::Timestamp(int64_t microSecondSinceEpoch) : m_microSecondSinceEpoch(microSecondSinceEpoch) {}
mymuduo::Timestamp mymuduo::Timestamp::now()
{
    return mymuduo::Timestamp(time(NULL));
}
std::string mymuduo::Timestamp::toString() const
{
    char buf[128] = {0};
    tm *tm_time = localtime(&m_microSecondSinceEpoch);
    snprintf(buf, 128, "%4d/%02d/%02d %02d:%02d:%02d",
             tm_time->tm_year + 1900,
             tm_time->tm_mon + 1,
             tm_time->tm_mday,
             tm_time->tm_hour,
             tm_time->tm_min,
             tm_time->tm_sec);
    return buf;
}