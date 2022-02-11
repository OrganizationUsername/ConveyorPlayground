#pragma once

#include <cstdint>
#include <array>
#include <ostream>
#include <istream>
#include "Enums.h"
#include "Rotation.h"

template<int32_t TLength, typename TType, typename TCrtp>
struct Vector
{
    std::array<TType, TLength> m_Data;

    template <typename ... TTypes>
    Vector(TTypes... args) : m_Data{ args... }
    {
    }

    TCrtp operator+(const TCrtp& other) const
    {
        TCrtp out;
        for (int32_t i = 0; i < TLength; ++i)
        {
            out.m_Data[i] = m_Data[i] + other.m_Data[i];
        }

        return out;
    }

    TCrtp operator-(const TCrtp& other) const
    {
        TCrtp out;
        for (int32_t i = 0; i < TLength; ++i)
        {
            out.m_Data[i] = m_Data[i] - other.m_Data[i];
        }

        return out;
    }

    TCrtp operator*(const TCrtp& other) const
    {
        TCrtp out;
        for (int32_t i = 0; i < TLength; ++i)
        {
            out.m_Data[i] = m_Data[i] * other.m_Data[i];
        }

        return out;
    }

    TCrtp operator/(const TCrtp& other) const
    {
        TCrtp out;
        for (int32_t i = 0; i < TLength; ++i)
        {
            out.m_Data[i] = m_Data[i] / other.m_Data[i];
        }

        return out;
    }

    TCrtp operator*(const TType& other) const
    {
        TCrtp out;
        for (int32_t i = 0; i < TLength; ++i)
        {
            out.m_Data[i] = m_Data[i] * other;
        }

        return out;
    }

    TCrtp operator/(const TType& other) const
    {
        TCrtp out;
        for (int32_t i = 0; i < TLength; ++i)
        {
            out.m_Data[i] = m_Data[i] / other;
        }

        return out;
    }

    TCrtp operator+(const TType& other) const
    {
        TCrtp out;
        for (int32_t i = 0; i < TLength; ++i)
        {
            out.m_Data[i] = m_Data[i] + other;
        }

        return out;
    }

    TCrtp operator-(const TType& other) const
    {
        TCrtp out;
        for (int32_t i = 0; i < TLength; ++i)
        {
            out.m_Data[i] = m_Data[i] - other;
        }

        return out;
    }

    TCrtp& operator+=(const TCrtp& other)
    {
        for (int32_t i = 0; i < TLength; ++i)
        {
            m_Data[i] += other.m_Data[i];
        }

        return *(reinterpret_cast<TCrtp*>(this));
    }

    TCrtp& operator-=(const TCrtp& other)
    {
        for (int32_t i = 0; i < TLength; ++i)
        {
            m_Data[i] -= other.m_Data[i];
        }

        return *(reinterpret_cast<TCrtp*>(this));
    }

    bool operator==(const TCrtp& other) const
    {
        for (int32_t i = 0; i < TLength; ++i)
        {
            if (m_Data[i] != other.m_Data[i])
            {
                return false;
            }
        }

        return true;
    }

    friend std::istream& operator>>(std::istream& os, TCrtp& vec)
    {
        for (int32_t i = 0; i < TLength; ++i)
        {
            TType tmpData;
            os >> tmpData;
            vec.m_Data[i] = tmpData;
        }

        return os;
    }

    TCrtp Abs()
    {
        TCrtp out = {};
        for (int32_t i = 0; i < TLength; ++i)
        {
            out.m_Data[i] = std::abs(m_Data[i]);
        }

        return out;
    }
};

struct Vector2 : public Vector<2, int32_t, Vector2>
{
    Vector2() : Vector(0, 0) {}
    Vector2(int32_t x, int32_t y) : Vector(x, y) {}

    [[nodiscard]] int32_t GetX() const { return m_Data[0]; }
    [[nodiscard]] int32_t GetY() const { return m_Data[1]; }
    void SetX(int32_t value) { m_Data[0] = value; }
    void SetY(int32_t value) { m_Data[1] = value; }

    [[nodiscard]] Vector2 Rotate(Rotation rotation, Vector2 size) const
    {
        const Vector2 c_offset(1, 1);

        size -= c_offset;
        switch (rotation)
        {
        case Rotation::Deg90: return { size.GetY() - GetY(), GetX() };
        case Rotation::Deg180: return { size.GetX() - GetX(), size.GetY() - GetY() };
        case Rotation::Deg270: return { size.GetY() - GetY(), size.GetX() - GetX() };
        }

        return *this;
    }
};

struct Vector2F : public Vector<2, float, Vector2F>
{
    Vector2F() : Vector2F(0.0f, 0.0f) {}
    Vector2F(float x, float y) : Vector(x, y) {}

    [[nodiscard]] float GetX() const { return m_Data[0]; }
    [[nodiscard]] float GetY() const { return m_Data[1]; }
    void SetX(float value) { m_Data[0] = value; }
    void SetY(float value) { m_Data[1] = value; }

    [[nodiscard]] Vector2F Rotate(Rotation rotation, Vector2F size) const
    {
        switch (rotation)
        {
        case Rotation::Deg90: return { size.GetY() - GetY() - 1, GetX() };
        case Rotation::Deg180:
            return Rotate(Rotation::Deg90, size)
                .Rotate(Rotation::Deg90, size);
        case Rotation::Deg270:
            return Rotate(Rotation::Deg90, size)
                .Rotate(Rotation::Deg90, size)
                .Rotate(Rotation::Deg90, size);
        }

        return *this;
    }
};

struct Vector3 : public Vector<3, int32_t, Vector3>
{
    Vector3() : Vector(0, 0, 0) {}
    Vector3(int32_t x, int32_t y, int32_t z) : Vector(x, y, z) {}
    Vector3(Vector2 xy, int32_t z) : Vector(xy.GetX(), xy.GetY(), z) {}

    [[nodiscard]] int32_t GetX() const { return m_Data[0]; }
    [[nodiscard]] int32_t GetY() const { return m_Data[1]; }
    [[nodiscard]] int32_t GetZ() const { return m_Data[2]; }
    int32_t& GetX() { return m_Data[0]; }
    int32_t& GetY() { return m_Data[1]; }
    int32_t& GetZ() { return m_Data[2]; }
    void SetX(int32_t value) { m_Data[0] = value; }
    void SetY(int32_t value) { m_Data[1] = value; }
    void SetZ(int32_t value) { m_Data[2] = value; }

    [[nodiscard]] Vector2 GetXY() const { return { m_Data[0], m_Data[1] }; }
    void SetXY(Vector2 value)
    {
        m_Data[0] = value.m_Data[0];
        m_Data[1] = value.m_Data[1];
    }
};
