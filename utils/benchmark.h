#ifndef __TEST_BENCHMARK__
#define __TEST_BENCHMARK__

#include "system.h"
#include "log.h"

namespace test {

using ns = std::chrono::nanoseconds;
using us = std::chrono::microseconds;
using ms = std::chrono::milliseconds;
using s = std::chrono::seconds;

template <typename T>
class BenchMark
{
public:
    typedef std::chrono::high_resolution_clock _clock;

    BenchMark(std::string str)
        : point(_clock::now())
        , m_str(str)
    { }

    ~BenchMark()
    {
        LOG("%s: cost (%ld). ",
            m_str.c_str(),
            std::chrono::duration_cast<T>(_clock::now() - point).count());
    }

private:
    _clock::time_point point;
    std::string m_str;


};

} // namespace test

#endif // __TEST_BENCHMARK__
