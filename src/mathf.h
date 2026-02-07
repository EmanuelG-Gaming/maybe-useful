#ifndef MATHF_H_
#define MATHF_H_ 1

#ifdef __cplusplus
extern "C" {
#endif

// Math library, mostly used for linear algebra

#include <math.h>
#include <stdio.h>

///////////////////////////////////////////////////////////////////////////////
// SIMD support
///////////////////////////////////////////////////////////////////////////////

// We use SSE by default
#define MATHF_USE_SSE
#ifdef MATHF_USE_SSE
// Intel intrinsics for SIMD (x86_-64)
#include <immintrin.h>
#define splat_x(r) _mm_shuffle_ps(r, r, 0)
#define splat_y(r) _mm_shuffle_ps(r, r, 0x55)
#define splat_z(r) _mm_shuffle_ps(r, r, 0xAA)
#define splat_w(r) _mm_shuffle_ps(r, r, 0xFF)
#endif

#ifdef _MSC_BER
#define ALIGN_MAT __declspec(align(16))
#else
#define ALIGN_MAT __attribute((aligned(16)))
#endif

#ifdef __cplusplus
#define ALIGN_MAT_DECL alignas(16)
#else
#define ALIGN_MAT_DECL
#endif



///////////////////////////////////////////////////////////////////////////////
// Constants
///////////////////////////////////////////////////////////////////////////////

#define PI 3.14159265359
#define PI2 6.28318530718
#define TAU 6.28318530718

///////////////////////////////////////////////////////////////////////////////
// Utility macros
///////////////////////////////////////////////////////////////////////////////

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#define CLAMP(i, v, x) (MAX(MIN(v, x), i))
#define ABS(a, b) (((a) > 0) ? (a) : -(a))

#define cosf(_x) ((float)cos(_x))
#define sinf(_x) ((float)sin(_x))
#define radians(_x) ((_x) * (__typeof__(_x)) (PI / 180.0f))
#define degrees(_x) ((_x) * (__typeof__(_x)) (180.0f / PI))
#define floori(_x) ((int) (floor((double) (_x))))
#define sign(_x) ({ __typeof__(_x) _xx = (_x);\
    ((__typeof__(_x)) ( (((__typeof__(_x)) 0) < _xx) - (_xx < ((__typeof__(_x)) 0))));})
#define lerpf(_a, _b, _t) ({ __typeof__(_t) _u = (_t); ((_a) * (1 - _u)) + ((_b) * _u); })
#define safe_expf(_x, _e) ({ __typeof__(_x) __x = (_x); __typeof__(_e) __e = (_e); sign(__x) * fabsf(powf(fabsf(__x), __e)); })


///////////////////////////////////////////////////////////////////////////////
// Vectors
///////////////////////////////////////////////////////////////////////////////

typedef struct Vec2f {
    float x, y;
#ifdef __cplusplus
#endif
} Vec2f;
typedef Vec2f Vec2;

typedef struct Vec3f {
    float x, y, z;
#ifdef __cplusplus
#endif
} Vec3f;
typedef Vec3f Vec3;

// Can also be a Quaternion
typedef struct ALIGN_MAT_DECL Vec4f {
    float x, y, z, w;
#ifdef __cplusplus
    // Constructors
    Vec4f();

    Vec4f(float all);
    Vec4f(float *v);
    Vec4f(float x, float y);
    Vec4f(float x, float y, float z);
    Vec4f(float x, float y, float z, float w);
    Vec4f(Vec4f v, float w);

    // Assignment operator
    Vec4f operator=(const Vec4f& v2) const;

    Vec4f Nor() const;
    Vec4f operator-() const;

    Vec4f operator+(const Vec4f& v2) const;
    Vec4f operator-(const Vec4f& v2) const;

    Vec4f operator*(float s) const;
    Vec4f operator*(const Vec4f& v2) const;

    static float Dot(const Vec4f& v1, const Vec4f& v2);
    static Vec4f Cross(const Vec4f& v1, const Vec4f& v2);
#endif
} Vec4f;
typedef Vec4f Quat;
typedef Vec4f Vec4;
typedef Vec4f Vec;


///////////////////////////////////////////////////////////////////////////////
// Matrices
///////////////////////////////////////////////////////////////////////////////

typedef union ALIGN_MAT_DECL Mat4f {
    // Multiple ways to define a matrix in the same region
    /*
    struct {
        float _11, _12, _13, _14;
        float _21, _22, _23, _24;
        float _31, _32, _33, _34;
        float _41, _42, _43, _44;
    };
    */
    Vec4f v[4];
    float m[4][4];
    float c[16];
#ifdef __cplusplus
    Mat4f();
	Mat4f(float ident);
	Mat4f(float *v);

	Mat4f operator*(const Mat4f &m2) const;
	Vec4f operator*(const Vec4f &v) const;

	Mat4f Transpose() const;
	void Translate(const Vec4f &v);
	void Scale(const Vec4f &v);
	void Rotate(const Vec4f &v);
	void Inverse3();
	void TransposeSave3(float *dat);

	static Mat4f Lookat(const Vec4f &eye, const Vec4f &center, const Vec4f &up);
	static Mat4f Look(const Vec4f &eye, const Vec4f &dir, const Vec4f &up);
	static Mat4f FromRotation(const Vec4f &r);
	static Mat4f FromTranslation(const Vec4f &v);
	static Mat4f FromScale(const Vec4f &v);
	
    static Mat4f Perspective(float fovy, float aspect, float fnear, float ffar);
    static Mat4f Ortho(float left, float right, float bottom, float top, float fnear, float ffar);

#endif
} Mat4f;
typedef Mat4f Mat4;
typedef Mat4f Mat;

///////////////////////////////////////////////////////////////////////////////
// Low-level functions that can be optimized through SIMD
///////////////////////////////////////////////////////////////////////////////

void vec4_zero(Vec4f* out);
void vec4_copy(Vec4f* out, const Vec4f* v);
Vec4f* vec4_add(Vec4f* out, const Vec4f* a, const Vec4f* b);
Vec4f* vec4_sub(Vec4f* out, const Vec4f* a, const Vec4f* b);
Vec4f* vec4_mul(Vec4f* out, const Vec4f* a, const Vec4f* b);
Vec4f* vec4_div(Vec4f* out, const Vec4f* a, const Vec4f* b);

Vec4f* vec4_add_scalar(Vec4f* out, const Vec4f* a, float b);
Vec4f* vec4_sub_scalar(Vec4f* out, const Vec4f* a, float s);
Vec4f* vec4_mul_scalar(Vec4f* out, const Vec4f* a, float s);
Vec4f* vec4_div_scalar(Vec4f* out, const Vec4f* a, float s);

Vec4f* vec4_lerp(Vec4f* out, const Vec4f* a, const Vec4f* b, float t);

Mat4f* mat4_copy(Mat4f* out, const Mat4f* a);

Mat4f* mat4_mul(Mat4f* out, const Mat4f* a, const Mat4f* b);
Vec4f* mat4_mul_vec4(Vec4f* out, const Mat4f* a, const Vec4f* b);
Vec4f* mat4_mul_vec3(Vec4f* out, const Mat4f* a, Vec4f* b);

///////////////////////////////////////////////////////////////////////////////
// Frontend functions
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Vectors

Vec4f* vec4_set4(Vec4f* out, float x, float y, float z, float w);
Vec4f* vec4_set3(Vec4f* out, float x, float y, float z);
Vec4f* vec4_set2(Vec4f* out, float x, float y);
Vec4f* vec4_set1(Vec4f* out, float x);
#define vec4_set vec4_set4

Vec4f* vec4_setV4(Vec4f* out, const Vec4f* src);
Vec4f* vec4_setV3(Vec4f* out, const Vec4f* src);
Vec4f* vec4_setV2(Vec4f* out, const Vec4f* src);
Vec4f* vec4_setV1(Vec4f* out, const Vec4f* src);
#define vec4_setV vec4_setV4


float vec4_dot3(const Vec4f* a, const Vec4f* b);
float vec4_len3(const Vec4f* v);
float vec4_len3_squared(const Vec4f* v);

float vec4_dst3(const Vec4f* v1, const Vec4f* v2);
float vec4_dst3_squared(const Vec4f* v1, const Vec4f* v2);
Vec4f* vec4_nor3(Vec4f* out, const Vec4f* v);


float vec4_dot4(const Vec4f* a, const Vec4f* b);
float vec4_len4(const Vec4f* v);
float vec4_len4_squared(const Vec4f* v);

float vec4_dst4(const Vec4f* v1, const Vec4f* v2);
float vec4_dst4_squared(const Vec4f* v1, const Vec4f* v2);
Vec4f* vec4_nor4(Vec4f* out, const Vec4f* v);

Vec4f* vec4_round3(Vec4f* out, const Vec4f* v);
Vec4f* vec4_round4(Vec4f* out, const Vec4f* v);

Vec4f* vec4_cross3(Vec4f* out, const Vec4f* a, const Vec4f* b);
Vec4f* vec4_rotate_axis_angle3(Vec4f* v, const Vec4f* axis, float angle);
float vec4_angle3(const Vec4f* v1, const Vec4f* v2);


///////////////////////////////////////////////////////////////////////////////
// Matrices

void mat4_load(Mat4f *out, float data[16]);
Mat4f* mat4_ident(Mat4f *out, float ident);

Mat4f* mat4_from_translation(Mat4f *out, const Vec4f *a);
Mat4f* mat4_from_scale(Mat4f *out, const Vec4f *a);
Mat4f* mat4_from_rotation(Mat4f *out, const Vec4f *a);

Mat4f* mat4_translate(Mat4f *out, const Mat4f *a, const Vec4f *b);
Mat4f* mat4_rotate(Mat4f *out, const Mat4f *a, const Vec4f *b);

Mat4f *mat4_inverse3(Mat4f *out, const Mat4f *m);
Mat4f *mat4_transpose3(Mat4f *out, const Mat4f *m);

Mat4f *mat4_lookAt(Mat4f *out, const Vec4f *eye, const Vec4f *center, const Vec4f *up);
Mat4f* mat4_look(Mat4f* out, const Vec4f* eye, const Vec4f* dir, const Vec4f* up);

Mat4f* mat4_perspective(Mat4f *out, float fovy, float aspect, float fnear, float ffar);
Mat4f* mat4_ortho(Mat4f *out, float left, float right, float bottom, float top, float fnear, float ffar);

Vec4f *mat4_get_rotation(Vec4f *out, const Mat4f *mat);

///////////////////////////////////////////////////////////////////////////////
// Formatting
///////////////////////////////////////////////////////////////////////////////

// Cursed macros
#define F "%.2f"
#define A ", "

#define FORMAT_VEC2F_STR F A F
#define FORMAT_VEC3F_STR F A F A F
#define FORMAT_VEC4F_STR F A F A F A F

#define FORMAT_VEC2F_ARGS(v) v.x, v.y
#define FORMAT_VEC3F_ARGS(v) v.x, v.y, v.z
#define FORMAT_VEC4F_ARGS(v) v.x, v.y, v.z, v.w

#define FORMAT_MAT2F_STR F A F ",\n"\
                         F A F

#define FORMAT_MAT3F_STR F A F A F ",\n"\
                         F A F A F ",\n"\
                         F A F A F

#define FORMAT_MAT4F_STR F A F A F A F ",\n"\
                         F A F A F A F ",\n"\
                         F A F A F A F ",\n"\
                         F A F A F A F

#define FORMAT_MAT4F_2_RM_ARGS(m)\
                             m.c[0], m.c[1],\
                             m.c[4], m.c[5]\

#define FORMAT_MAT4F_3_RM_ARGS(m)\
                             m.c[0], m.c[1], m.c[2],\
                             m.c[4], m.c[5], m.c[6],\
                             m.c[8], m.c[9], m.c[10]\

#define FORMAT_MAT4F_4_RM_ARGS(m)\
                             m.c[0], m.c[1], m.c[2], m.c[3],\
                             m.c[4], m.c[5], m.c[6], m.c[7],\
                             m.c[8], m.c[9], m.c[10], m.c[11],\
                             m.c[12], m.c[13], m.c[14], m.c[15]

#define FORMAT_MAT4F_2_CM_ARGS(m)\
                             m.c[0], m.c[4],\
                             m.c[1], m.c[5]\

#define FORMAT_MAT4F_3_CM_ARGS(m)\
                             m.c[0], m.c[4], m.c[8],\
                             m.c[1], m.c[5], m.c[9],\
                             m.c[2], m.c[6], m.c[10]\

#define FORMAT_MAT4F_4_CM_ARGS(m)\
                             m.c[0], m.c[4], m.c[8], m.c[12],\
                             m.c[1], m.c[5], m.c[9], m.c[13],\
                             m.c[2], m.c[6], m.c[10], m.c[14],\
                             m.c[3], m.c[7], m.c[11], m.c[15]


static inline void print_vec4_2(Vec4f vec)
{
    printf("Vec" FORMAT_VEC2F_STR ")\n", FORMAT_VEC2F_ARGS(vec));
}
static inline void print_vec4_3(Vec4f vec)
{
    printf("Vec" FORMAT_VEC3F_STR ")\n", FORMAT_VEC3F_ARGS(vec));
}
static inline void print_vec4_4(Vec4f vec)
{
    printf("Vec(" FORMAT_VEC4F_STR ")\n", FORMAT_VEC4F_ARGS(vec));
}

///////////////////////////////////////////////////////////////////////////////
// Row-major matrix print

static inline void print_mat4_2_rm(Mat4f mat)
{
    printf("Mat(\n" FORMAT_MAT2F_STR ")\n", FORMAT_MAT4F_2_RM_ARGS(mat));
}
static inline void print_mat4_3_rm(Mat4f mat)
{
    printf("Mat(\n" FORMAT_MAT3F_STR ")\n", FORMAT_MAT4F_3_RM_ARGS(mat));
}
static inline void print_mat4_4_rm(Mat4f mat)
{
    printf("Mat(\n" FORMAT_MAT4F_STR ")\n", FORMAT_MAT4F_4_RM_ARGS(mat));
}

///////////////////////////////////////////////////////////////////////////////
// Column-major matrix print

static inline void print_mat4_2_cm(Mat4f mat)
{
    printf("Mat(\n" FORMAT_MAT2F_STR ")\n", FORMAT_MAT4F_2_CM_ARGS(mat));
}
static inline void print_mat4_3_cm(Mat4f mat)
{
    printf("Mat(\n" FORMAT_MAT3F_STR ")\n", FORMAT_MAT4F_3_CM_ARGS(mat));
}
static inline void print_mat4_4_cm(Mat4f mat)
{
    printf("Mat(\n" FORMAT_MAT4F_STR ")\n", FORMAT_MAT4F_4_CM_ARGS(mat));
}


#define print_mat2(m) print_mat4_3_rm(m)
#define print_mat3(m) print_mat4_3_rm(m)
#define print_mat4(m) print_mat4_4_rm(m)

#undef A
#undef F

#ifdef __cplusplus
} // extern "C"
#endif

#endif // MATHF_H_
