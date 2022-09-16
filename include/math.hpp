#ifndef _MATH_HPP
#define _MATH_HPP

#include <cstddef>
#include <cstdint>

template <typename T> struct Vector2 {
    T x;
    T y;
};

template <> struct Vector2<uint64_t> {
    uint64_t x;
    uint64_t y;

    Vector2() {
        x = 0;
        y = 0;
    }

    Vector2(uint64_t _x, uint64_t _y) {
        x = _x;
        y = _y;
    }

    friend inline bool operator==(const Vector2& lhs, const Vector2& rhs) {
        return (lhs.x == rhs.x && lhs.y == rhs.y);
    }

    friend inline bool operator!=(const Vector2& lhs, const Vector2& rhs) { return !(lhs == rhs); }
    
    friend inline Vector2 operator+(const Vector2& lhs, const Vector2& rhs) {
        return {lhs.x + rhs.x, lhs.y + rhs.y};
    }
    
    friend inline Vector2 operator-(const Vector2& lhs, const Vector2& rhs) {
        return {lhs.x - rhs.x, lhs.y - rhs.y};
    }
    
    friend inline Vector2 operator*(const Vector2& lhs, const Vector2& rhs) {
        return {lhs.x * rhs.x, lhs.y * rhs.y};
    }
    
    friend inline Vector2 operator/(const Vector2& lhs, const Vector2& rhs) {
        return {lhs.x / rhs.x, lhs.y / rhs.y};
    }
};

#endif  // !_MATH_HPP