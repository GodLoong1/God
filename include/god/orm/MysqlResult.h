#ifndef GOD_ORM_MYSQLRESULT_H
#define GOD_ORM_MYSQLRESULT_H

#include <mariadb/mysql.h>

#include <memory>
#include <unordered_map>
#include <vector>
#include <utility>
#include <string>
#include <string_view>

#include "god/orm/Result.h"

namespace god
{

class MysqlResult : public Result
{
public:
    MysqlResult(MYSQL_RES* result, size_t affectedRow) noexcept;

    size_t affectedRows() const noexcept override
    {
        return affectedRow_;
    }

    size_t size() const noexcept override
    {
        return rowNum_;
    }

    size_t columns() const noexcept override
    {
        return fieldNum_;
    }

    bool empty() const noexcept override
    {
        return size() == 0;
    }

    const char* columnName(size_t column) const noexcept override;
    size_t columnNumber(const char* name) const noexcept override;

    const char* getValue(size_t row, size_t column) const noexcept override;
    size_t getLength(size_t row, size_t column) const noexcept override;

    bool isNull(size_t row, size_t column) const noexcept override
    {
        return getValue(row, column) == nullptr ||
               getLength(row, column) == 0;
    }

private:
    const std::unique_ptr<MYSQL_RES, void(*)(MYSQL_RES*)> result_;
    // 影响行数
    const size_t affectedRow_;
    // 行数
    const size_t rowNum_;
    // 字段集
    const MYSQL_FIELD* fields_;
    // 字段数
    const size_t fieldNum_;
    // 字段名称 下标
    std::unordered_map<std::string_view, size_t> fieldMap_;
    // 行数 字符串数组 下表数组
    std::vector<std::pair<MYSQL_ROW, std::vector<size_t>>> rowVec_;
};

} // namespace god

#endif