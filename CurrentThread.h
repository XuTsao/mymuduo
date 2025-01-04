#pragma once

#include <unistd.h>
#include <sys/syscall.h>

namespace CurrentThread
{
    extern __thread int t_cachedTid; // __thread是GCC内置的{线程局部存储}设施

    void cacheTid();

    inline int tid()
    {
        if (__builtin_expect(t_cachedTid == 0, 0))
        {
            cacheTid();
        }
        return t_cachedTid;
    }
}