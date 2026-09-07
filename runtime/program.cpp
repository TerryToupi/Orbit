#include <base/base.hpp>

using namespace Orbit;

int main(void)
{
    Pool<u64> pool;
    pool.emplace(43);
    return 0;
}
