#pragma once

#include <cstdint>
#include <string>

namespace tl {

struct Span {
    std::size_t start;
    std::size_t end;
    std::size_t line;
    std::size_t column;

    Span() : start(0), end(0), line(1), column(1) {}
    Span(std::size_t start, std::size_t end, std::size_t line, std::size_t column)
        : start(start), end(end), line(line), column(column) {}

    std::size_t length() const { return end - start; }
    bool empty() const { return start == end; }
};

} // namespace tl
