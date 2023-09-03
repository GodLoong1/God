#include "god/orm/Result.h"

namespace god
{

Row Result::operator[](size_t row) const noexcept
{
    return Row(this, row);
}

ResultIterator Result::begin() const noexcept
{
    return ResultIterator(this, 0);
}

ResultIterator Result::end() const noexcept
{
    return ResultIterator(this, size());
}

Field Row::operator[](const char* columnName) const noexcept
{
    size_t column = result_->columnNumber(columnName);
    return Field(this, column);
}

Field Row::operator[](size_t column) const noexcept
{
    return Field(this, column);
}

RowIterator Row::begin() const noexcept
{
    return RowIterator(this, 0);
}

RowIterator Row::end() const noexcept
{
    return RowIterator(this, size());
}

} // namespace god