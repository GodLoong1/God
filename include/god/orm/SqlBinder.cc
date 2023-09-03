#include "god/orm/SqlBinder.h"

namespace god
{

std::string SqlBinder::sql()
{
    if (paramVec_.empty())
    {
        return std::move(sql_);
    }

    std::string ret;
    size_t pos = 0, seekPos = 0;

    for (size_t i = 0; i != paramVec_.size(); ++i)
    {
        seekPos = sql_.find('?', pos);
        if (seekPos == std::string::npos)
        {
            ret.append(sql_, pos);
            pos = seekPos;
            break;
        }
        else
        {
            ret.append(sql_, pos, seekPos - pos);
            pos = seekPos + 1;
            ret.append(paramVec_[i]);
        }
    }
    return ret;
}

} // namespace god