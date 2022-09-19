#ifndef _STRING_HPP
#define _STRING_HPP

#include <cstddef>
#include <cstdint>
#include <cstr.hpp>
#include <memory/heap.hpp>
#include <memory/memory.hpp>

constexpr uint64_t STRING_INITIAL_LENGTH = 10;

class String {
public:
    // Default constructor
    String() : Length(STRING_INITIAL_LENGTH) {
        Buffer = new uint8_t[Length + 1];
        memset(Buffer, 0, Length);
    }

    // Copy constructor
    String(const String& original) {
        Length = original.length();
        Buffer = new uint8_t[Length + 1];
        Buffer[Length] = '\0';
        memcpy(original.bytes(), Buffer, Length);
    }

    String(const char* cstr) {
        Length = strlen(cstr) - 1;
        Buffer = new uint8_t[Length + 1];
        memcpy((void*)cstr, Buffer, Length + 1);
    }

    String(const char* cstr, uint64_t byteCount) {
        Length = byteCount;
        Buffer = new uint8_t[Length + 1];
        memcpy((void*)cstr, Buffer, Length);
        Buffer[Length] = '\0';
    }

    ~String() { delete[] Buffer; }

    uint64_t length() { return Length; }
    uint64_t length() const { return Length; }

    uint8_t* bytes() const { return Buffer; }

    enum class Side {
        Left,
        Right,
    };

    /**
     * @note Character at index is included in right side.
     */
    String& chop(uint64_t index, Side side) {
        if (index > length() - 1) return *this;

        if (side == String::Side::Left) {
            Length = index;
            uint8_t* oldBuffer = Buffer;
            Buffer = new uint8_t[Length + 1];
            memcpy((void*)oldBuffer, Buffer, Length);
            Buffer[Length] = '\0';
            delete[] oldBuffer;
        } else {
            Length = strlen(&data()[index]) - 1;
            uint8_t* oldBuffer = Buffer;
            Buffer = new uint8_t[Length + 1];
            memcpy((void*)&oldBuffer[index], Buffer, Length + 1);
            delete[] oldBuffer;
        }
        return *this;
    }

    const char* data() const { return (const char*)Buffer; }

    /**
     * @brief Make a copy of the current contents of the string on the heap
     *
     * @note Free responsiblity is transferred to the caller upon returning.
     */
    const char* data_copy() const {
        uint8_t* copy = new uint8_t[Length + 1];
        memcpy(Buffer, copy, Length);
        copy[Length] = '\0';

        return (const char*)copy;
    }

    // Copy assignment
    String& operator=(const String& other) {
        if (this == &other) return *this;

        Length = other.Length;

        delete[] Buffer;

        Buffer = new uint8_t[Length + 1];
        Buffer[Length] = '\0';
        memcpy(other.Buffer, Buffer, Length);

        return *this;
    }

    // Move assignment
    String& operator=(String&& other) noexcept {
        if (this == &other) return *this;

        delete[] Buffer;

        // FIXME: Need atomic exchange here for Buffer pointer swap...
        Buffer = other.Buffer;
        other.Buffer = nullptr;
        Length = other.Length;
        other.Length = 0;

        return *this;
    }

    String& operator+(const String& other) {
        if (this == &other) return *this;

        uint64_t oldLength = Length;
        Length += other.Length;
        uint8_t* newBuffer = new uint8_t[Length + 1];

        memcpy(&Buffer[0], &newBuffer[0], oldLength);
        memcpy(&other.Buffer[0], &newBuffer[oldLength], other.Length);

        uint8_t* oldBuffer = Buffer;
        Buffer = newBuffer;
        delete[] oldBuffer;

        return *this;
    }

    String& operator+(const char* cstr) {
        if (cstr == nullptr) return *this;

        uint64_t stringLength = strlen(cstr);

        if (stringLength <= 1) return *this;

        uint64_t oldLength = Length;
        Length += stringLength - 1;
        uint8_t* newBuffer = new uint8_t[Length + 1];

        memcpy(&Buffer[0], &newBuffer[0], oldLength);
        memcpy((void*)cstr, &newBuffer[oldLength], stringLength - 1);

        newBuffer[Length] = '\0';
        uint8_t* oldBuffer = Buffer;
        Buffer = newBuffer;

        delete[] oldBuffer;

        return *this;
    }

    String& operator+=(const String& rhs) {
        this->operator+(rhs);
        return *this;
    }

    String& operator+=(const char* cstr) {
        this->operator+(cstr);
        return *this;
    }

    uint8_t& operator[](uint64_t index) const {
        if (index >= Length) return Buffer[Length - 1];

        return Buffer[index];
    }

    uint8_t& operator[](uint64_t index) {
        if (index >= Length) return Buffer[Length - 1];

        return Buffer[index];
    }

private:
    uint8_t* Buffer{nullptr};
    uint64_t Length{0};
};

inline String& operator<<(String& lhs, const String& rhs) {
    lhs += rhs;
    return lhs;
}

inline bool operator==(const String& lhs, const String& rhs) {
    if (lhs.length() != rhs.length()) return false;

    return !memcmp(lhs.bytes(), rhs.bytes(), lhs.length());
}

inline bool operator!=(const String& lhs, const String& rhs) { return !(lhs == rhs); }

#endif  // !_STRING_HPP