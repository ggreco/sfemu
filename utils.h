#ifndef UTILS_H

#define UTILS_H

#include <string>
#include <sstream>
#include <sys/time.h>
#include <iomanip>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>

template <typename T>
inline std::string to_string(T val)
{
	std::ostringstream s;
    s << val;
    return s.str();
}

inline std::string to_lower(const std::string &orig)
{
    std::string result = orig;

    for (std::string::iterator it = result.begin(); it != result.end(); ++it)
        if (::isupper(*it))
            *it = ::tolower(*it);

    return result;
}

inline int to_int(const std::string &str)   { return atoi(str.c_str()); }
inline long to_long(const std::string &str) { return atol(str.c_str()); }
inline long long to_longlong(const std::string &str) { return atoll(str.c_str()); }
inline double to_double(const std::string &str) { return atof(str.c_str()); }

inline int64_t gettime() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return ((int64_t)tv.tv_sec) * 1000000LL + (int64_t)tv.tv_usec;
}


inline long long to_ts(const std::string &datetime) {
    long long ts;
    int y, m, d, H, M, S, ss;

    // FORMATO: AAAA-MM-GGTHH:MM:SS.SS
    if (::sscanf(datetime.c_str(), "%4d-%2d-%2dT%2d:%2d:%2d.%2d", 
                &y, &m, &d, &H, &M, &S, &ss) != 7) {
        return -1;
    }
    struct tm t;
    t.tm_year = y - 1900;
    t.tm_mon = m - 1;
    t.tm_mday = d;
    t.tm_hour = H;
    t.tm_min = M;
    t.tm_sec = S;
    t.tm_isdst = -1;
    ts = ::mktime(&t);

    ts *= 1000LL;    // converto in millisecondi
    ts += (ss * 10); // aggiungo i centesimi

    return ts;
}
#endif
