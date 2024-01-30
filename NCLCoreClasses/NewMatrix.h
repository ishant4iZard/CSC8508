/*
Part of Newcastle University's Game Engineering source code.

Use as you see fit!

Comments and queries to: richard-gordon.davison AT ncl.ac.uk
https://research.ncl.ac.uk/game/
*/
#pragma once

namespace NCL::Maths {

    template <typename T, uint32_t r, uint32_t c>
    struct NewMatrix    {
        T array[c][r];

        NewMatrix() {
            for (int i = 0; i < c; ++i) {
                for (int j = 0; j < r; ++j) {
                    T a = (i == j) ? T(1) : T(0);
                    array[i][j] = a;
                }
            }
        }
    };

    using NewMatrix2 = NewMatrix<float, 2, 2>;
    using NewMatrix3 = NewMatrix<float, 3, 3>;
    using NewMatrix4 = NewMatrix<float, 4, 4>;

    template <typename T, uint32_t r, uint32_t c>
    constexpr NewMatrix<T,r,c> operator*(const NewMatrix<T, r, c>& a, const NewMatrix<T, r, c>& b) {
        NewMatrix<T, r, c> out;
        for (unsigned int cc = 0; cc < c; ++cc) {
            for (unsigned int rr = 0; rr < r; ++rr) {
                out.array[cc][rr] = T(0);
                for (unsigned int i = 0; i < r; ++i) {
                    out.array[cc][rr] += a.array[i][rr] * b.array[cc][i];
                }
            }
        }
        return out;
    }

    //Matrix4 Specialisations
    template <typename T>
    constexpr NewMatrix<T, 4, 4> Translation(const NewVector<T, 3>& v) {
        NewMatrix<T, 4, 4> mat;

        mat.array[3][0] = T(v.x);
        mat.array[3][1] = T(v.y);
        mat.array[3][2] = T(v.z);

        return mat;
    }

    template <typename T>
    constexpr NewMatrix<T, 4, 4> Scale(const NewVector<T, 3>& v) {
        NewMatrix<T, 4, 4> mat;

        mat.array[0][0] = T(v.x);
        mat.array[1][1] = T(v.y);
        mat.array[2][2] = T(v.z);
        mat.array[2][3] = T(1);

        return mat;
    }

    template <typename T>
    constexpr NewMatrix<T, 4, 4> Rotation(T degrees, const NewVector<T, 3>& inAxis) {
        NewMatrix<T, 4, 4> mat;

        const NewVector<T, 3> axis = Maths::Normalise(inAxis);
       
        float c = cos((float)Maths::DegreesToRadians(degrees));
        float s = sin((float)Maths::DegreesToRadians(degrees));

        mat.array[0][0] = (axis.x * axis.x) * (1.0f - c) + c;
        mat.array[0][1] = (axis.y * axis.x) * (1.0f - c) + (axis.z * s);
        mat.array[0][2] = (axis.z * axis.x) * (1.0f - c) - (axis.y * s);
        
        mat.array[1][0] = (axis.x * axis.y) * (1.0f - c) - (axis.z * s);
        mat.array[1][1] = (axis.y * axis.y) * (1.0f - c) + c;
        mat.array[1][2] = (axis.z * axis.y) * (1.0f - c) + (axis.x * s);
        
        mat.array[2][0] = (axis.x * axis.z) * (1.0f - c) + (axis.y * s);
        mat.array[2][1] = (axis.y * axis.z) * (1.0f - c) - (axis.x * s);
        mat.array[2][2] = (axis.z * axis.z) * (1.0f - c) + c;

        return mat;
    }
}