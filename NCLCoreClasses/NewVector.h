/*
Part of Newcastle University's Game Engineering source code.

Use as you see fit!

Comments and queries to: richard-gordon.davison AT ncl.ac.uk
https://research.ncl.ac.uk/game/
*/
#pragma once

namespace NCL::Maths {

    template <typename T, uint32_t n>
    struct NewVector    {
        T array[n];

        template <typename T, uint32_t n>
        inline T operator[](int i) {
            return ((T*)this)[i];
        }

        template <typename T, uint32_t n>
        inline T& operator[](int i) {
            return ((T*)this)[i];
        }

    };

    using NewVector2 = NewVector<float, 2>;
    using NewVector3 = NewVector<float, 3>;
    using NewVector4 = NewVector<float, 4>;

    using NewVector2i = NewVector<int, 2>;
    using NewVector3i = NewVector<int, 3>;
    using NewVector4i = NewVector<int, 4>;

    using NewVector2d = NewVector<double, 2>;
    using NewVector3d = NewVector<double, 3>;
    using NewVector4d = NewVector<double, 4>;

    template <typename T>
    struct NewVector<T, 2> {
        union {
            T array[2];
            struct {
                T x;
                T y;
            };
        };

        NewVector<T, 2>() : x(0),y(0) {
        }

        NewVector<T, 2>(T inX, T inY) {
            x = inX;
            y = inY;
        }

        T operator[](int i) const {
            return ((T*)this)[i];
        }
        T& operator[](int i) {
            return ((T*)this)[i];
        }
    };

    template <typename T>
    struct NewVector<T, 3> {
        union {
            T array[3];
            struct {
                T x;
                T y;
                T z;
            };
        };

        NewVector<T, 3>() : x(0), y(0), z(0) {
        }

        NewVector<T, 3>(T inX, T inY, T inZ) {
            x = inX;
            y = inY;
            z = inZ;
        }

        NewVector<T, 3>(NewVector<T, 2> v, T inZ) {
            x = v.array[0];
            y = v.array[1];
            z = inZ;
        }


        T operator[](int i) const {
            return ((T*)this)[i];
        }
        T& operator[](int i) {
            return ((T*)this)[i];
        }
    };

    template <typename T>
    struct NewVector<T, 4> {
        union {
            T array[4];
            struct {
                T x;
                T y;
                T z;
                T w;
            };
        };

        NewVector<T, 4>() : x(0), y(0), z(0), w(0) {
        }

        NewVector<T, 4>(T inX, T inY, T inZ, T inW) {
            x = inX;
            y = inY;
            z = inZ;
            w = inW;
        }

        NewVector<T, 4>(NewVector<T, 2> v, T inZ, T inW) {
            x = v.array[0];
            y = v.array[1];
            z = inZ;
            w = inW;
        }

        NewVector<T, 4>(NewVector<T, 3> v, T inW) {
            x = v.array[0];
            y = v.array[1];
            z = v.array[2];
            w = inW;
        }

        T operator[](int i) const {
            return ((T*)this)[i];
        }
        T& operator[](int i) {
            return ((T*)this)[i];
        }
    };

    template <typename T>
    NewVector<T, 3> Cross(const NewVector<T, 3>& a, const NewVector<T, 3>& b) {
        NewVector<T, 3> result;
        result.x = (a.y * b.z) - (a.z * b.y);
        result.y = (a.z * b.x) - (a.x * b.z);
        result.z = (a.x * b.y) - (a.y * b.x);
        return result;
    }

    template <typename T, uint32_t n>
    T Dot(const NewVector<T,n>& a, const NewVector<T,n>& b)   {
        T result(0);
        for (int i = 0; i < n; ++i) {
            result += a[i] * b[i];
        }
        return result;
    }

    template <typename T, uint32_t n>
    constexpr T LengthSquared(const NewVector<T, n>& a)  {
        T result(0);
        for (int i = 0; i < n; ++i) {
            result += a[i] * a[i];
        }
        return result;
    }

    template <typename T, uint32_t n>
    constexpr NewVector<T, n> Normalise(const NewVector<T, n>& a) {
        NewVector<T, n> result;
        T l = Length(a);
        if (l > 0.0f) {
            T r = T(1.0) / l;
            for (int i = 0; i < n; ++i) {
                result.array[i] = a.array[i] * r;
            }
        }
        return result;
    }

    template <typename T, uint32_t n>
    T Length(const NewVector<T, n>& a) {
        return std::sqrt(LengthSquared(a));
    }

    template <typename T, uint32_t n>
    constexpr T		GetMinElement(const NewVector<T, n>& a) {
        float v = a.array[0];
        for (int i = 1; i < n; ++i) {
            v = std::min(v, a.array[i]);
        }
        return v;
    }

    template <typename T, uint32_t n>
    constexpr T		GetMaxElement(const NewVector<T, n>& a)  {
        float v = a.array[0];
        for (int i = 1; i < n; ++i) {
            v = std::max(v, a.array[i]);
        }
        return v;
    }

    template <typename T, uint32_t n>
    constexpr T		GetAbsMaxElement(const NewVector<T, n>& a) {
        float v = std::abs(a.array[0]);
        for (int i = 1; i < n; ++i) {
            v = std::max(v, std::abs(a.array[i]));
        }
        return v;
    }

    template <typename T, uint32_t n>
    constexpr NewVector<T, n>		Clamp(NewVector<T, n>& input, NewVector<T, n>& mins, NewVector<T, n>& maxs) {
        NewVector<T, n> output;
        for (int i = 1; i < n; ++i) {
            output.array[i] = std::clamp(input.array[i], mins.array[i], maxs.array[i]);
        }
        return output;
    }

    template <typename T, uint32_t n>
    inline NewVector<T, n>  operator+(const NewVector<T, n>& a, const NewVector<T, n>& b) {
        NewVector<T, n> answer;
        for (int i = 0; i < n; ++i) {
            answer[i] = a[i] + b[i];
        }
        return answer;
    }

    template <typename T, uint32_t n>
    inline NewVector<T, n>  operator-(const NewVector<T, n>& a, const NewVector<T, n>& b) {
        NewVector<T, n> answer;
        for (int i = 0; i < n; ++i) {
            answer[i] = a[i] - b[i];
        }
        return answer;
    }

    template <typename T, uint32_t n>
    inline NewVector<T, n>  operator-(const NewVector<T, n>& a) {
        NewVector<T, n> answer;
        for (int i = 0; i < n; ++i) {
            answer[i] = -a[i];
        }
        return answer;
    }

    template <typename T, uint32_t n>
    inline NewVector<T, n>  operator*(const NewVector<T, n>& a, const NewVector<T, n>& b) {
        NewVector<T, n> answer;
        for (int i = 0; i < n; ++i) {
            answer[i] = a[i] * b[i];
        }
        return answer;
    }

    template <typename T, uint32_t n>
    inline NewVector<T, n>  operator/(const NewVector<T, n>& a, const NewVector<T, n>& b) {
        NewVector<T, n> answer;
        for (int i = 0; i < n; ++i) {
            answer[i] = a[i] / b[i];
        }
        return answer;
    }

    template <typename T, uint32_t n>
    inline NewVector<T, n>  operator*(const NewVector<T, n>& a, const T& b) {
        NewVector<T, n> answer;
        for (int i = 0; i < n; ++i) {
            answer[i] = a[i] * b[i];
        }
        return answer;
    }

    template <typename T, uint32_t n>
    inline NewVector<T, n>  operator/(const NewVector<T, n>& a, const T& b) {
        NewVector<T, n> answer;
        for (int i = 0; i < n; ++i) {
            answer[i] = a[i] / b[i];
        }
        return answer;
    }

    template <typename T, uint32_t n>
    inline NewVector<T, n>&  operator+=(NewVector<T, n>& a, const NewVector<T, n>& b) {
        for (int i = 0; i < n; ++i) {
            a[i] = a[i] + b[i];
        }
        return a;
    }

    template <typename T, uint32_t n>
    inline NewVector<T, n>& operator-=(NewVector<T, n>& a, const NewVector<T, n>& b) {
        for (int i = 0; i < n; ++i) {
            a[i] = a[i] - b[i];
        }
        return a;
    }

    template <typename T, uint32_t n>
    inline NewVector<T, n>& operator*=(NewVector<T, n>& a, const NewVector<T, n>& b) {
        for (int i = 0; i < n; ++i) {
            a[i] = a[i] * b[i];
        }
        return a;
    }

    template <typename T, uint32_t n>
    inline NewVector<T, n>& operator/=(NewVector<T, n>& a, const NewVector<T, n>& b) {
        for (int i = 0; i < n; ++i) {
            a[i] = a[i] / b[i];
        }
        return a;
    }

    template <typename T, uint32_t n>
    inline NewVector<T, n>& operator*=(NewVector<T, n>& a, const T& b) {
        for (int i = 0; i < n; ++i) {
            a[i] = a[i] * b;
        }
        return a;
    }

    template <typename T, uint32_t n>
    inline NewVector<T, n>& operator/=(NewVector<T, n>& a, const T& b) {
        for (int i = 0; i < n; ++i) {
            a[i] = a[i] / b;
        }
        return a;
    }
}