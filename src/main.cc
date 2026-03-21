// include headers previously here are moved to scenario.h
#include "scenario.h"

int
main(int argc, char** argv)
{
    struct rlimit file_limits;
    file_limits.rlim_cur = 65536;
    file_limits.rlim_max = 65536;

    setrlimit(RLIMIT_NOFILE, &file_limits);

    ns3::Scenario s(argc, argv);
    s(); // run the scenario as soon as it is ready

    return 0;
}
