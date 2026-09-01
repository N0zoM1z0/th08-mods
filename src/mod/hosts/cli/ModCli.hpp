#ifndef TH_MOD_HOSTS_CLI_MOD_CLI_HPP
#define TH_MOD_HOSTS_CLI_MOD_CLI_HPP

#include <stdio.h>

namespace th_mod
{
namespace cli
{

enum Result
{
    kRunGame,
    kExitSuccess,
    kExitFailure
};

Result ConfigureFromArguments(int argc, char *const *argv,
                              FILE *output, FILE *errors);

} // namespace cli
} // namespace th_mod

#endif
