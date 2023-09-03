#include <iostream>
#include <regex>
#include <string>

void test(const std::string& sub, const std::regex& pattern)
{
    std::string str(sub);
    std::smatch results;
    while (std::regex_search(str, results, pattern))
    {
        if (results.size() > 1)
        {
            std::cout << results[0].str() << std::endl;
        }
        str = results.suffix();
    }
}

int main() {
    // std::string str = "{}, {name}, {age}, {fasdfa1321:L432423sdf}";
    const std::regex pattern("\\{([^/]*)\\}");

    // for (int i = 0; i != 1; ++i)
    // {
    //     test(str, pattern);
    // }

    // std::cout << sizeof(std::regex) << std::endl;

    return 0;
}