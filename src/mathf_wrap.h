#include "mathf.h"
#include "mathf.c"

#ifdef __cplusplus

///////////////////////////////////////////////////////////////////////////////
// Vectors
///////////////////////////////////////////////////////////////////////////////


inline Vec4f::Vec4f()
{
    vec4_zero(this);
}

inline Vec4f::Vec4f(float all)
{
    x = y = z = w = all;
}
inline Vec4f::Vec4f(float *v)
{
    x = v[0]; y = v[1]; z = v[2]; w = v[3];
}

inline Vec4f::Vec4f(float x, float y)
{
    vec4_set2(this, x, y);
}
inline Vec4f::Vec4f(float x, float y, float z)
{
    vec4_set3(this, x, y, z);
}
inline Vec4f::Vec4f(float x, float y, float z, float w)
{
    vec4_set4(this, x, y, z, w);
}
inline Vec4f::Vec4f(Vec4f v, float w) {
    vec4_copy(this, &v);
    this->w = w;
}


///////////////////////////////////////////////////////////////////////////////
// Vector operations

inline Vec4f Vec4f::Nor() const
{
    Vec4f v;
    vec4_nor4(&v, this);
    return v;
}
inline Vec4f Vec4f::operator-() const
{
    Vec4f v;
    vec4_mul_scalar(&v, this, -1);
    return v;
}

inline Vec4f Vec4f::operator+(const Vec4f& v2) const
{
    Vec4f v;
    vec4_add(&v, this, &v2);
    return v;
}
inline Vec4f Vec4f::operator-(const Vec4f& v2) const
{
    Vec4f v;
    vec4_sub(&v, this, &v2);
    return v;
}

inline Vec4f Vec4f::operator*(float s) const
{
    Vec4f v;
    vec4_mul_scalar(&v, this, s);
    return v;
}

inline Vec4f Vec4f::operator*(const Vec4f& v2) const
{
    Vec4f v;
    vec4_mul(&v, this, &v2);
    return v;
}

inline float Vec4f::Dot(const Vec4f& v1, const Vec4f& v2)
{
    return vec4_dot3(&v1, &v2);
}
inline Vec4f Vec4f::Cross(const Vec4f& v1, const Vec4f& v2)
{
    Vec4f v;
    vec4_cross3(&v, &v1, &v2);
    return v;
}

///////////////////////////////////////////////////////////////////////////////
// Matrices
///////////////////////////////////////////////////////////////////////////////

inline Mat4f::Mat4f()
{
    mat4_ident(this, 1);
}
inline Mat4f::Mat4f(float ident)
{
    mat4_ident(this, ident);
}

inline Mat4f::Mat4f(float *v)
{
    for (int i = 0; i < 16; ++i) c[i] = v[i];
}

inline Mat4f Mat4f::operator*(const Mat4f &m2) const
{
    Mat4f mat;
    mat4_mul(&mat, this, &m2);
    return mat;
}

inline Vec4f Mat4f::operator*(const Vec4f &v) const
{
    Vec4f vec;
    mat4_mul_vec4(&vec, this, &v);
    return vec;
}

inline Mat4f Mat4f::Transpose() const
{
    Mat4f mat;
    mat4_transpose3(&mat, this);
    return mat;
}

inline void Mat4f::Translate(const Vec4f &v)
{
    mat4_translate(this, this, &v);
}

inline void Mat4f::Scale(const Vec4f &v)
{
    mat4_scale(this, this, &v);
}

inline void Mat4f::Rotate(const Vec4f &v)
{
    mat4_rotate(this, this, &v);
}

inline void Mat4f::Inverse3()
{
    mat4_inverse3(this, this);
}

inline void Mat4f::TransposeSave3(float *dat)
{
    Mat4f mat;
    mat4_transpose3(&mat, this);
    for (int i = 0; i < 16; ++i) {
        c[i] = mat.c[i];
    }
}

///////////////////////////////////////////////////////////////////////////////
// Static functions used for notation
///////////////////////////////////////////////////////////////////////////////

inline Mat4f Mat4f::Lookat(const Vec4f &eye, const Vec4f &center, const Vec4f &up)
{
    Mat4f mat;
    mat4_lookAt(&mat, &eye, &center, &up);
    return mat;
}
inline Mat4f Mat4f::Look(const Vec4f &eye, const Vec4f &dir, const Vec4f &up)
{
    Mat4f mat;
    mat4_look(&mat, &eye, &dir, &up);
    return mat;
}

inline Mat4f Mat4f::FromRotation(const Vec4f &r)
{
    Mat4f mat;
    mat4_from_rotation(&mat, &r);
    return mat;
}
inline Mat4f Mat4f::FromTranslation(const Vec4f &v)
{
    Mat4f mat;
    mat4_from_translation(&mat, &v);
    return mat;
}
inline Mat4f Mat4f::FromScale(const Vec4f &v)
{
    Mat4f mat;
    mat4_from_scale(&mat, &v);
    return mat;
}

inline Mat4f Mat4f::Perspective(float fovy, float aspect, float fnear, float ffar)
{
    Mat4f mat;
    mat4_perspective(&mat, fovy, aspect, fnear, ffar);
    return mat;
}

inline Mat4f Mat4f::Ortho(float left, float right, float bottom, float top, float fnear, float ffar)
{
    Mat4f mat;
    mat4_ortho(&mat, left, right, bottom, top, fnear, ffar);
    return mat;
}

#endif // __cplusplus
