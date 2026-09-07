#ifndef __ORBIT_APP__
#define __ORBIT_APP__

#include <utils/types.hpp>

namespace Orbit::App
{

enum class Status : u32
{
    CONTINUE = 0,   // keep running
    SUCCESS,        // stop, report success to the OS
    FAILURE,        // stop, report failure to the OS
};

Status startup();
Status update(u64 tick);
void   shutdown();

}

#endif
