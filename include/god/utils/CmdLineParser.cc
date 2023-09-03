#include "god/utils/CmdLineParser.h"

namespace god
{

CmdLineParser::CmdLineParser(int argc, char** argv) noexcept
{
    const char* curr = nullptr;

    for (int i = 1; i < argc; ++i)
    {
        if (argv[i][0] == '-')
        {
            curr = argv[i];
            params_[curr] = std::nullopt;
        }
        else if (curr)
        {
            params_[curr] = argv[i];
            curr = nullptr;
        }
    }
}

std::optional<std::optional<std::string>>
CmdLineParser::get(const std::string& opt) noexcept
{
    auto it = params_.find(opt);
    if (it != params_.end())
    {
        return it->second;
    }
    return std::nullopt;
}

} // namespace god