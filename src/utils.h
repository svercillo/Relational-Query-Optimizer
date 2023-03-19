#ifndef UTILS_H
#define UTILS_H
#pragma once

struct vector_hash {
    std::size_t operator()(const std::vector<std::string>& v) const {
        std::hash<std::string> hasher;
        std::size_t seed = v.size();
        for (const auto& str : v) {
            seed ^= hasher(str) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        }
        return seed;
    }
};

struct vector_equal {
    bool operator()(const std::vector<std::string>& a, const std::vector<std::string>& b) const {
        return a == b;
    }
};

#endif // UTILS_H