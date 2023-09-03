#ifndef GOD_UTILS_CMDLINEPARSER_H
#define GOD_UTILS_CMDLINEPARSER_H

#include <string>
#include <unordered_map>
#include <optional>

namespace god
{

/// 命令行解析
class CmdLineParser
{
public:
    CmdLineParser(int argc, char** argv) noexcept;

    std::optional<std::optional<std::string>>
    get(const std::string& opt) noexcept;

private:
    std::unordered_map<std::string, std::optional<std::string>> params_;
};

} // namespace god

#endif