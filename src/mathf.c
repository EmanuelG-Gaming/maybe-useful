#include "mathf.h"

///////////////////////////////////////////////////////////////////////////////
// Low-level operations
///////////////////////////////////////////////////////////////////////////////
#ifdef MATHF_USE_SSE

void vec4_zero(Vec4f* out)
{
    _mm_store_ps(&out->x, _mm_set_ps1(0));
}
void vec4_copy(Vec4f* out, const Vec4f* v)
{
    _mm_store_ps(&out->x, _mm_load_ps(&v->x));
}
Vec4f* vec4_add(Vec4f* out, const Vec4f* a, const Vec4f* b)
{
    __m128 v = _mm_add_ps(_mm_load_ps(&a->x), _mm_load_ps(&b->x));
    _mm_store_ps(&out->x, v);
    return out;
}
Vec4f* vec4_sub(Vec4f* out, const Vec4f* a, const Vec4f* b)
{
    __m128 v = _mm_sub_ps(_mm_load_ps(&a->x), _mm_load_ps(&b->x));
    _mm_store_ps(&out->x, v);
    return out;
}
Vec4f* vec4_mul(Vec4f* out, const Vec4f* a, const Vec4f* b)
{
    __m128 v = _mm_mul_ps(_mm_load_ps(&a->x), _mm_load_ps(&b->x));
    _mm_store_ps(&out->x, v);
    return out;
}
Vec4f* vec4_div(Vec4f* out, const Vec4f* a, const Vec4f* b)
{
    __m128 v = _mm_div_ps(_mm_load_ps(&a->x), _mm_load_ps(&b->x));
    _mm_store_ps(&out->x, v);
    return out;
}
Vec4f* vec4_add_scalar(Vec4f* out, const Vec4f* a, float s)
{
    __m128 v = _mm_add_ps(_mm_load_ps(&a->x), _mm_set_ps1(s));
    _mm_store_ps(&out->x, v);
    return out;
}
Vec4f* vec4_sub_scalar(Vec4f* out, const Vec4f* a, float s)
{
    __m128 v = _mm_sub_ps(_mm_load_ps(&a->x), _mm_set_ps1(s));
    _mm_store_ps(&out->x, v);
    return out;
}
Vec4f* vec4_mul_scalar(Vec4f* out, const Vec4f* a, float s)
{
    __m128 v = _mm_mul_ps(_mm_load_ps(&a->x), _mm_set_ps1(s));
    _mm_store_ps(&out->x, v);
    return out;
}
Vec4f* vec4_div_scalar(Vec4f* out, const Vec4f* a, float s)
{
    __m128 v = _mm_div_ps(_mm_load_ps(&a->x), _mm_set_ps1(s));
    _mm_store_ps(&out->x, v);
    return out;
}

// a * (1-t) + b*t
Vec4f* vec4_lerp(Vec4f* out, const Vec4f* a, const Vec4f*b, float t)
{
    __m128 t2 = splat_x(_mm_set_ss(t));
    __m128 t3 = _mm_sub_ps(_mm_set_ps1(1.0f), t2);

    __m128 a2 = _mm_mul_ps(t3, _mm_load_ps(&a->x));
    __m128 b2 = _mm_mul_ps(t2, _mm_load_ps(&b->x));
    _mm_store_ps(&out->x, _mm_add_ps(a2, b2));
    return out;
}

// Use 4 SIMD Instructions for 4 vectors in a matrix
Mat4f* mat_copy(Mat4f* out, const Mat4f* a)
{
    _mm_store_ps(&out->c[0], _mm_load_ps(&a->c[0]));
    _mm_store_ps(&out->c[4], _mm_load_ps(&a->c[4]));
    _mm_store_ps(&out->c[8], _mm_load_ps(&a->c[8]));
    _mm_store_ps(&out->c[12], _mm_load_ps(&a->c[12]));
    return out;
}

Mat4f* mat4_mul(Mat4f *out, const Mat4f *a, const Mat4f *b)
{
    // Temporary SIMD registers
    __m128 l, r0, r1, r2, r3,
              v0, v1, v2, v3;

    r0 = _mm_load_ps(&b->c[0]);
    r1 = _mm_load_ps(&b->c[4]);
    r2 = _mm_load_ps(&b->c[8]);
    r3 = _mm_load_ps(&b->c[12]);

    l = _mm_load_ps(&a->c[0]);
    v0 = _mm_mul_ps(l, splat_x(r0));
    v1 = _mm_mul_ps(l, splat_x(r1));
    v2 = _mm_mul_ps(l, splat_x(r2));
    v3 = _mm_mul_ps(l, splat_x(r3));

    l = _mm_load_ps(&a->c[4]);
    v0 = _mm_mul_ps(l, _mm_mul_ps(l, splat_y(r0)));
    v1 = _mm_mul_ps(l, _mm_mul_ps(l, splat_y(r1)));
    v2 = _mm_mul_ps(l, _mm_mul_ps(l, splat_y(r2)));
    v3 = _mm_mul_ps(l, _mm_mul_ps(l, splat_y(r3)));

    l = _mm_load_ps(&a->c[8]);
    v0 = _mm_mul_ps(l, _mm_mul_ps(l, splat_z(r0)));
    v1 = _mm_mul_ps(l, _mm_mul_ps(l, splat_z(r1)));
    v2 = _mm_mul_ps(l, _mm_mul_ps(l, splat_z(r2)));
    v3 = _mm_mul_ps(l, _mm_mul_ps(l, splat_z(r3)));

    l = _mm_load_ps(&a->c[12]);
    v0 = _mm_mul_ps(l, _mm_mul_ps(l, splat_z(r0)));
    v1 = _mm_mul_ps(l, _mm_mul_ps(l, splat_z(r1)));
    v2 = _mm_mul_ps(l, _mm_mul_ps(l, splat_z(r2)));
    v3 = _mm_mul_ps(l, _mm_mul_ps(l, splat_z(r3)));

    _mm_store_ps(&out->c[0], v0);
    _mm_store_ps(&out->c[4], v1);
    _mm_store_ps(&out->c[8], v2);
    _mm_store_ps(&out->c[12], v3);
    return out;
}

Vec4f* mat4_mul_vec4(Vec4f* out, const Mat4f* a, const Vec4f* b)
{
    __m128 r = _mm_loadu_ps(&b->x);

    __m128 l0 = _mm_load_ps(&a->c[0]); 
    __m128 l1 = _mm_load_ps(&a->c[4]); 
    __m128 l2 = _mm_load_ps(&a->c[8]); 
    __m128 l3 = _mm_load_ps(&a->c[12]); 

    __m128 v0 = _mm_mul_ps(l0, splat_x(r));
    v0 = _mm_add_ps(v0, _mm_mul_ps(l1, splat_y(r)));
    v0 = _mm_add_ps(v0, _mm_mul_ps(l2, splat_z(r)));
    v0 = _mm_add_ps(v0, _mm_mul_ps(l3, splat_w(r)));

    _mm_storeu_ps(&out->x, v0);
    return out;
}

Vec4f* mat4_mul_vec3(Vec4f* out, const Mat4f* a, Vec4f* b)
{
    b->w = 1.0f;
    return mat4_mul_vec4(out, a, b);
}

#else
#error "No SSE found for compatibility"
#endif

///////////////////////////////////////////////////////////////////////////////
// Front-end operations
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
// Vectors
Vec4f* vec4_set4(Vec4f* out, float x, float y, float z, float w)
{
    out->x = x;
    out->y = y;
    out->z = z;
    out->w = w;
    return out;
}
Vec4f* vec4_set3(Vec4f* out, float x, float y, float z)
{
    return vec4_set4(out, x, y, z, 1);
}
Vec4f* vec4_set2(Vec4f* out, float x, float y)
{
    return vec4_set4(out, x, y, 1, 1);
}
Vec4f* vec4_set1(Vec4f* out, float x)
{
    return vec4_set4(out, x, 1, 1, 1);
}

Vec4f* vec4_setV4(Vec4f* out, const Vec4f* src)
{
    return vec4_set4(out, src->x, src->y, src->z, src->w);
}
Vec4f* vec4_setV3(Vec4f* out, const Vec4f* src)
{
    return vec4_set3(out, src->x, src->y, src->z);
}
Vec4f* vec4_setV2(Vec4f* out, const Vec4f* src)
{
    return vec4_set2(out, src->x, src->y);
}
Vec4f* vec4_setV1(Vec4f* out, const Vec4f* src)
{
    return vec4_set1(out, src->x);
}


float vec4_dot3(const Vec4f* a, const Vec4f* b)
{
    return a->x*b->x + a->y*b->y + a->z*b->z;
}
float vec4_len3(const Vec4f* v)
{
    return sqrtf(v->x*v->x + v->y*v->y + v->z*v->z);
}
float vec4_len3_squared(const Vec4f* v)
{
    return v->x*v->x + v->y*v->y + v->z*v->z; 
}
float vec4_dst3(const Vec4f* v1, const Vec4f* v2)
{
    Vec4f d; vec4_sub(&d, v1, v2);
    return vec4_len3(&d);
}

float vec4_dst3_squared(const Vec4f* v1, const Vec4f* v2)
{
    Vec4f d; vec4_sub(&d, v1, v2);
    return vec4_len3_squared(&d);
}
Vec4f* vec4_nor3(Vec4f* out, const Vec4f* v)
{
    return vec4_mul_scalar(out, v, 1 / vec4_len3(v));
}


float vec4_dot4(const Vec4f* a, const Vec4f* b)
{
    return a->x*b->x + a->y*b->y + a->z*b->z + a->w*b->w;
}
float vec4_len4(const Vec4f* v)
{
    return sqrtf(v->x*v->x + v->y*v->y + v->z*v->z + v->w*v->w);
}
float vec4_len4_squared(const Vec4f* v)
{
    return (v->x*v->x + v->y*v->y + v->z*v->z + v->w*v->w);
}

float vec4_dst4(const Vec4f* v1, const Vec4f* v2)
{
    Vec4f d; vec4_sub(&d, v1, v2);
    return vec4_len4(&d);
}

float vec4_dst4_squared(const Vec4f* v1, const Vec4f* v2)
{
    Vec4f d; vec4_sub(&d, v1, v2);
    return vec4_len4_squared(&d);
}
Vec4f* vec4_nor4(Vec4f* out, const Vec4f* v)
{
    return vec4_mul_scalar(out, v, 1 / vec4_len4(v));
}



Vec4f* vec4_round3(Vec4f* out, const Vec4f* v)
{
    out->x = roundf(v->x);
    out->y = roundf(v->y);
    out->z = roundf(v->z);
    return out;
}
Vec4f* vec4_round4(Vec4f* out, const Vec4f* v)
{
    out->x = roundf(v->x);
    out->y = roundf(v->y);
    out->z = roundf(v->z);
    out->w = roundf(v->w);
    return out;
}

Vec4f* vec4_cross3(Vec4f* out, const Vec4f* a, const Vec4f* b)
{
    out->x = a->y * b->z - a->z * b->y;
	out->y = a->z * b->x - a->x * b->z;
	out->z = a->x * b->y - a->y * b->x;
	return out;
}
Vec4f* vec4_rotate_axis_angle3(Vec4f* v, const Vec4f* axis, float angle)
{
Vec4f nor_axis; vec4_nor3(&nor_axis, axis);
    
    angle /= 2.0f;
    float a = sinf(angle);
    float b = nor_axis.x*a;
    float c = nor_axis.y*a;
    float d = nor_axis.z*a;
    a = cosf(angle);
    Vec4f w = (Vec4f){ b, c, d, 0 };

    Vec4f wv; vec4_cross3(&wv, &w, v);

    Vec4f wwv; vec4_cross3(&wwv, &w, &wv);

	vec4_mul_scalar(&wwv, &wv, 2*a);
	vec4_mul_scalar(&wwv, &wwv, 2);

	// Then add them together
	vec4_add(v, v, &wv);
	vec4_add(v, v, &wwv);
    return v;
}

float vec4_angle3(const Vec4f* v1, const Vec4f* v2)
{
    float result = 0.0f;

	Vec4f cross; vec4_cross3(&cross, v1, v2);
	float len = vec4_len3(&cross);
	float dot = vec4_dot3(v1, v2);
    result = atan2f(len, dot);

    return result;
}

///////////////////////////////////////////////////////////////////////////////
// Matrices

void mat4_load(Mat4f *out, float data[16])
{
    for (int i = 0; i < 16; ++i) {
        out->c[16] = data[i];
    }
}

Mat4f* mat4_ident(Mat4f *out, float ident)
{
	out->c[0] = ident; out->c[1] = 0; out->c[2] = 0; out->c[3] = 0;
	out->c[4] = 0; out->c[5] = ident; out->c[6] = 0; out->c[7] = 0;
	out->c[8] = 0; out->c[9] = 0; out->c[10] = ident; out->c[11] = 0;
	out->c[12] = 0; out->c[13] = 0; out->c[14] = 0; out->c[15] = ident;
	return out;
}

Mat4f* mat4_from_translation(Mat4f *out, const Vec4f *a)
{
	mat4_ident(out, 1.0f);
	out->c[12] = a->x;
	out->c[13] = a->y;
	out->c[14] = a->z;
	return out;
}

Mat4f* mat4_from_scale(Mat4f *out, const Vec4f *a)
{
	mat4_ident(out, 1.0f);
	out->c[0] = a->x;
	out->c[5] = a->y;
	out->c[10] = a->z;
	return out;
}

Mat4f* mat4_from_rotation(Mat4f *out, const Vec4f *a)
{
	float x = a->x;
	float y = a->y;
	float z = a->z;
	float w = a->w;

	float tx = x + x; // Note: Using x + x instead of 2.0f * x to force this function to return the same value as the SSE4.1 version across platforms.
	float ty = y + y;
	float tz = z + z;

	float xx = tx * x;
	float yy = ty * y;
	float zz = tz * z;
	float xy = tx * y;
	float xz = tx * z;
	float xw = tx * w;
	float yz = ty * z;
	float yw = ty * w;
	float zw = tz * w;

	out->c[0] = (1.0f - yy) - zz;
	out->c[1] = xy + zw;
	out->c[2] = xz - yw;
	out->c[3] = 0;
	out->c[4] = xy - zw;
	out->c[5] = (1.0f - zz) - xx;
	out->c[6] = yz + xw;
    out->c[7] = 0;
	out->c[8] = xz + yw;
	out->c[9] = yz - xw;
	out->c[10] = (1.0f - xx) - yy;
	out->c[11] = 0;
	out->c[12] = 0;
	out->c[13] = 0;
	out->c[14] = 0;
	out->c[15] = 1.0f;
	return out;
}

Mat4f* mat4_translate(Mat4f *out, const Mat4f *a, const Vec4f *b)
{
	out->c[12] += a->c[0] * b->x + a->c[4] * b->y + a->c[8] * b->z;
	out->c[13] += a->c[1] * b->x + a->c[5] * b->y + a->c[9] * b->z;
	out->c[14] += a->c[2] * b->x + a->c[6] * b->y + a->c[10] * b->z;
	return out;
}

Mat4f* mat4_scale(Mat4f *out, const Mat4f *a, const Vec4f *b)
{
	for (int i = 0; i < 3; i++) {
		out->c[0 + i] = a->c[0 + i] * b->x;
		out->c[4 + i] = a->c[4 + i] * b->y;
		out->c[8 + i] = a->c[8 + i] * b->z;
	}
	return out;
}

Mat4f* mat4_rotate(Mat4f *out, const Mat4f *a, const Vec4f *b)
{
	Mat4f rot;
	mat4_from_rotation(&rot, b);
    // Rotation is accumulative
	return mat4_mul(out, a, &rot);
}

Mat4f *mat4_inverse3(Mat4f *out, const Mat4f *m)
{
	float det;
	float a = m->c[0], b = m->c[1], c = m->c[2],
		  d = m->c[4], e = m->c[5], f = m->c[6],
		  g = m->c[8], h = m->c[9], i = m->c[10];

	out->c[0] = e * i - f * h;
	out->c[1] = -(b * i - h * c);
	out->c[2] = b * f - e * c;
	out->c[3] = 0;
	out->c[4] = -(d * i - g * f);
	out->c[5] = a * i - c * g;
	out->c[6] = -(a * f - d * c);
	out->c[7] = 0;
	out->c[8] = d * h - g * e;
	out->c[9] = -(a * h - g * b);
	out->c[10] = a * e - b * d;
	out->c[11] = 0;
	out->c[12] = 0;
	out->c[13] = 0;
	out->c[14] = 0;
	out->c[15] = 0;

    // Calculate inverse determinant
	det = 1.0f / (a * out->c[0] + b * out->c[4] + c * out->c[8]);
	for (int i = 0; i < 16; ++i) {
		out->c[i] *= det;
	}
	return out;
}

Mat4f *mat4_transpose3(Mat4f *out, const Mat4f *m)
{
	out->c[0] = m->c[0];
	out->c[1] = m->c[4];
	out->c[2] = m->c[8];
	out->c[3] = 0;
	out->c[4] = m->c[1];
	out->c[5] = m->c[5];
	out->c[6] = m->c[9];
	out->c[7] = 0;
	out->c[8] = m->c[2];
	out->c[9] = m->c[6];
	out->c[10] = m->c[10];
	out->c[11] = 0;
	out->c[12] = 0;
	out->c[13] = 0;
	out->c[14] = 0;
	out->c[15] = 1;
	return out;
}

Mat4f *mat4_lookAt(Mat4f *out, const Vec4f *eye, const Vec4f *center, const Vec4f *up)
{
	Vec4f f, s, u;
	vec4_nor3(&f, vec4_sub(&f, center, eye));
	vec4_nor3(&s, vec4_cross3(&s, &f, up));
	vec4_cross3(&u, &s, &f);

	out->c[0] = s.x;
	out->c[1] = u.x;
	out->c[2] = -f.x;
	out->c[3] = 0;
	out->c[4] = s.y;
	out->c[5] = u.y;
	out->c[6] = -f.y;
	out->c[7] = 0;
	out->c[8] = s.z;
	out->c[9] = u.z;
	out->c[10] = -f.z;
	out->c[11] = 0;
	out->c[12] = -vec4_dot3(&s, eye);
	out->c[13] = -vec4_dot3(&u, eye);
	out->c[14] = vec4_dot3(&f, eye);
	out->c[15] = 1;
	return out;
}
Mat4f* mat4_look(Mat4f* out, const Vec4f* eye, const Vec4f* dir, const Vec4f* up)
{
	Vec4f center;
	vec4_add(&center, eye, dir);
	return mat4_lookAt(out, eye, &center, up);
}

Mat4f* mat4_perspective(Mat4f *out, float fovy, float aspect, float fnear, float ffar)
{
	float f = 1.0f / tanf(fovy * 0.5f);
	float fn = 1.0f / (fnear - ffar);
	mat4_ident(out, 0.0f);
	out->c[0] = f / aspect;
	out->c[5] = f;
	out->c[10] = ffar * fn;
	out->c[11] = -1.0f;
	out->c[14] = fnear * ffar * fn;
	return out;
}

Mat4f* mat4_ortho(Mat4f *out, float left, float right, float bottom, float top, float fnear, float ffar)
{

    float rl = (float)(right - left);
    float tb = (float)(top - bottom);
    float fn = (float)(ffar - fnear);

	mat4_ident(out, 0);
    out->c[0 ] = 2/rl;
    out->c[1 ] = 0;
    out->c[2 ] = 0;
    out->c[3 ] = 0;
    out->c[4 ] = 0;
    out->c[5 ] = 2/tb;
    out->c[6 ] = 0;
    out->c[7 ] = 0;
    out->c[8 ] = 0;
    out->c[9 ] = 0;
    out->c[10] = -2/fn;
    out->c[11] = 0;
    out->c[12] = -((float)left + (float)right)/rl;
    out->c[13] = -((float)top + (float)bottom)/tb;
    out->c[14] = -((float)ffar + (float)fnear)/fn;
    out->c[15] = 1;

    return out;
}


Vec4f *mat4_get_rotation(Vec4f *out, const Mat4f *mat)
{
	const float *m = mat->c;

	float tr = m[0] + m[5] + m[10];
	if (tr >= 0.0f) {
		float s = sqrt(tr + 1.0f);
		float is = 0.5f / s;
		out->x = (m[1 * 4 + 2] - m[2 * 4 + 1]) * is;
		out->y = (m[2 * 4 + 0] - m[0 * 4 + 2]) * is;
		out->z = (m[0 * 4 + 1] - m[1 * 4 + 0]) * is;
		out->w = 0.5f * s;
		return out;
	}

	int i = 0;
	if (m[1 * 4 + 1] > m[0 * 4 + 0]) i = 1;
	if (m[2 * 4 + 2] > m[i * 4 + i]) i = 2;

	if (i == 0) {
		float s = sqrt(m[0] - (m[5] + m[10]) + 1);
		float is = 0.5f / s;
		out->x = 0.5f * s;
		out->y = (m[1 * 4 + 0] + m[0 * 4 + 1]) * is;
		out->z = (m[0 * 4 + 2] + m[2 * 4 + 0]) * is;
		out->w = (m[1 * 4 + 2] - m[2 * 4 + 1]) * is;
		return out;
	} else if (i == 1) {
		float s = sqrt(m[5] - (m[10] + m[0]) + 1);
		float is = 0.5f / s;
		out->x = (m[1 * 4 + 0] + m[0 * 4 + 1]) * is;
		out->y = 0.5f * s;
		out->z = (m[2 * 4 + 1] + m[1 * 4 + 2]) * is;
		out->w = (m[2 * 4 + 0] - m[0 * 4 + 2]) * is;
		return out;
	} else {
		float s = sqrt(m[10] - (m[0] + m[5]) + 1);
		float is = 0.5f / s;
		out->x = (m[0 * 4 + 2] + m[2 * 4 + 0]) * is;
		out->y = (m[2 * 4 + 1] + m[1 * 4 + 2]) * is;
		out->z = 0.5f * s;
		out->w = (m[0 * 4 + 1] - m[1 * 4 + 0]) * is;
	}
	return out;
}

///////////////////////////////////////////////////////////////////////////////
// Formatting
///////////////////////////////////////////////////////////////////////////////
