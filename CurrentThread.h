#pragma once

#include <unistd.h>
#include <sys/syscall.h>

namespace mymuduo
{
namespace CurrentThread
{
    // __thread 关键字声明线程局部变量t_cachedTid，
    //确保每个线程拥有独立的副本，无需锁或其他同步机制
    extern __thread int t_cachedTid;
    void cachedTid();

    inline int tid()
    {
        //__builtin_expect提示编译器优化条件分支，
        //将t_cachedTid == 0视为低概率事件，提高指令流水线效率
        if(__builtin_expect(t_cachedTid == 0,0))
        {
            cachedTid();
        }
        return t_cachedTid;
    }
} // namespace CurrentThread

} // namespace mymuduo