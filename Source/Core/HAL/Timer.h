//
//  Timer.h
//  Platform
//
//  Created by Yongsik Im on 2/8/25.
//

#ifndef __HS_TIMER_H__
#define __HS_TIMER_H__

#include "Precompile.h"

#include <stack>

HS_NS_BEGIN

class Timer
{
public:
    static bool Initialize();

    static void Start();
    static double Stop();
    static void Reset();
    static double GetElapsedSeconds();
    static double GetElapsedMilliseconds();
    static double GetElapsedMicroseconds();

private:
    static double getTimeSinceInit();
    static std::stack<double> _laps;

    static double _initTime;
};

HS_NS_END

#endif /* __HS_TIMER_H__ */
