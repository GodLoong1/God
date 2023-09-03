#include "god/orm/MysqlResult.h"

#include <cassert>

namespace god
{

MysqlResult::MysqlResult(MYSQL_RES* result,
                         size_t affectedRow) noexcept
: result_(result, mysql_free_result),
  affectedRow_(affectedRow),
  rowNum_(mysql_num_rows(result_.get())),
  fields_(mysql_fetch_fields(result_.get())),
  fieldNum_(mysql_num_fields(result_.get()))
{
    for (size_t i = 0; i != fieldNum_; ++i)
    {
        fieldMap_[fields_[i].name] = i;
    }

    MYSQL_ROW row;
    while ((row = mysql_fetch_row(result_.get())))
    {
        size_t* lengths = mysql_fetch_lengths(result_.get());
        std::vector lens(lengths, lengths + fieldNum_);
        rowVec_.emplace_back(row, std::move(lens));
    }
}

const char* MysqlResult::columnName(size_t column) const noexcept
{
    assert(column < columns());
    return fields_[column].name;
}

size_t MysqlResult::columnNumber(const char* name) const noexcept
{
    assert(fieldMap_.count(name));
    return const_cast<MysqlResult*>(this)->fieldMap_[name];
}

const char* MysqlResult::getValue(size_t row, size_t column) const noexcept
{
    assert(row < size());
    assert(column < columns());
    return rowVec_[row].first[column];
}

size_t MysqlResult::getLength(size_t row, size_t column) const noexcept
{
    assert(row < size());
    assert(column < columns());
    return rowVec_[row].second[column];
}

} // namespace god