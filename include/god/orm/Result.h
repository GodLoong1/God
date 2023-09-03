#ifndef GOD_ORM_RESULT_H
#define GOD_ORM_RESULT_H

#include <memory>
#include <string>
#include <string_view>
#include <vector>
#include <charconv>
#include <type_traits>

#include "god/utils/NonCopyable.h"

namespace god
{

class Result;
class Row;
class Field;
class ResultIterator;
class RowIterator;

using ResultPtr = std::shared_ptr<Result>;

class Result : NonCopyable
{
public:
    virtual ~Result() noexcept = default;

    virtual size_t affectedRows() const noexcept = 0;

    virtual size_t size() const noexcept = 0;

    virtual size_t columns() const noexcept = 0;

    virtual bool empty() const noexcept = 0;

    virtual const char* columnName(size_t column) const noexcept = 0;

    virtual size_t columnNumber(const char* name) const noexcept = 0;

    virtual const char* getValue(size_t row, size_t column) const noexcept = 0;

    virtual size_t getLength(size_t row, size_t column) const noexcept = 0;

    virtual bool isNull(size_t row, size_t column) const noexcept = 0;

    Row operator[](size_t row) const noexcept;

    ResultIterator begin() const noexcept;
    ResultIterator end() const noexcept;
};

class Row : NonCopyable
{
public:
    Row(const Result* result, size_t row) noexcept
    : result_(result), row_(row)
    { }

    size_t size() const noexcept
    {
        return result_->columns();
    }

    bool empty() const noexcept
    {
        return size() == 0;
    }

    const char* getValue(size_t column) const noexcept
    {
        return result_->getValue(row_, column);
    }

    size_t getLength(size_t column) const noexcept
    {
        return result_->getLength(row_, column);
    }

    bool isNull(size_t column) const noexcept
    {
        return result_->isNull(row_, column);
    }

    const char* columnName(size_t column) const noexcept
    {
        return result_->columnName(column);
    }

    size_t columnNumber(const char* name) const noexcept
    {
        return result_->columnNumber(name);
    }

    Field operator[](const char* columnName) const noexcept;
    Field operator[](size_t column) const noexcept;

    RowIterator begin() const noexcept;
    RowIterator end() const noexcept;

private:
    const Result* result_;

protected:
    size_t row_;
};

class Field : NonCopyable
{
public:
    Field(const Row* row, size_t column) noexcept
    : row_(row), column_(column)
    { }

    const char* name() const noexcept
    {
        return row_->columnName(column_);
    }

    const char* value() const noexcept
    {
        return row_->getValue(column_);
    }

    size_t length() const noexcept
    {
        return row_->getLength(column_);
    }

    bool isNull() const noexcept
    {
        return row_->isNull(column_);
    }

    template<typename T = const char*>
    T as() const noexcept
    {
        if (isNull())
        {
            return T();
        }

        if constexpr (std::is_arithmetic_v<T>)
        {
            const char* val = value();
            T data{};
            std::from_chars(val, val + length(), data);
            return data;
        }
        else if constexpr (std::is_same_v<T, const char*>)
        {
            return T(value());
        }
        else if constexpr (std::is_same_v<T, std::string> ||
                           std::is_same_v<T, std::string_view>)
        {
            return T(value(), length());
        }
        else if constexpr (std::is_same_v<T, std::vector<char>>)
        {
            const char* val = value();
            return T(val, val + length());
        }
    }

private:
    const Row* row_;

protected:
    size_t column_;
};

class ResultIterator : protected Row
{
public:
    ResultIterator(const Result* result, size_t row) noexcept
    : Row(result, row)
    { }

    const Row& operator*() const noexcept
    {
        return *this;
    }

    ResultIterator& operator++() noexcept
    {
        ++Row::row_;
        return *this;
    }

    friend bool operator!=(const ResultIterator& rhs,
                           const ResultIterator& lhs) noexcept
    {
        return rhs.row_ != lhs.row_;
    }
};

class RowIterator : protected Field
{
public:
    RowIterator(const Row* row, size_t column)
    : Field(row, column)
    { }

    const Field& operator*() const noexcept
    {
        return *this;
    }

    RowIterator& operator++() noexcept
    {
        ++Field::column_;
        return *this;
    }

    friend bool operator!=(const RowIterator& lhs,
                           const RowIterator& rhs) noexcept
    {
        return lhs.column_ != rhs.column_;
    }
};

} // namespace god

#endif