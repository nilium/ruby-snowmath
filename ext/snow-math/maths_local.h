/*
  3D math types & macros
  Written by Noel Cower

  See COPYING for license information
*/

#ifndef __SNOW__MATHS_H__
#define __SNOW__MATHS_H__

#ifdef __cplusplus
#include <cmath>
#else
#include <math.h>
#endif

#define S_STATIC_INLINE
#ifndef S_STATIC_INLINE
#ifdef __SNOW__MATHS_C__
#define S_INLINE
#else
#define S_INLINE extern inline
#endif
#else
#define S_INLINE static
#endif

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */

/* Typedefs and macros for specific floating point types */
#ifdef USE_FLOAT
typedef float s_float_t;
#define s_cos(X)  (cosf((X)))
#define s_sin(X)  (sinf((X)))
#define s_tan(X)  (tanf((X)))
#define s_acos(X) (acosf((X)))
#define s_asin(X) (asinf((X)))
#define s_atan(X) (atanf((X)))
#define s_fabs(X) (fabsf((X)))
#define s_sqrt(X) (sqrtf((X)))
#define s_float_lit(X) (X##f)
#else
typedef double s_float_t;
#define s_cos(X)  (cos((X)))
#define s_sin(X)  (sin((X)))
#define s_tan(X)  (tan((X)))
#define s_acos(X) (acos((X)))
#define s_asin(X) (asin((X)))
#define s_atan(X) (atan((X)))
#define s_fabs(X) (fabs((X)))
#define s_sqrt(X) (sqrt((X)))
#define s_float_lit(X) (X)
#endif

typedef s_float_t mat4_t[16];
typedef s_float_t mat3_t[9];
typedef s_float_t vec4_t[4];
typedef s_float_t vec3_t[3];
typedef s_float_t vec2_t[2];
typedef s_float_t quat_t[4];


/*!
 * Floating point epsilon for double comparisons.  This is, more or less, the
 * limit to accuracy.  If the difference between two floating point values is
 * less than the epsilon, they are for all intents and purposes the same.
 *
 * It should be stressed that this is absolutely not an accurate epsilon.
 */
#define S_FLOAT_EPSILON s_float_lit(1.0e-9)

#define S_DEG2RAD s_float_lit(0.01745329)
#define S_RAD2DEG s_float_lit(57.2957795)



/* Float comparison functions */

S_INLINE int  float_is_zero(const s_float_t x)
{
  return (s_fabs(x) < S_FLOAT_EPSILON);
}

S_INLINE int  float_equals(const s_float_t x, const s_float_t y)
{
  return float_is_zero(x - y);
}


/*==============================================================================

  2-Component Vector (vec2_t)

==============================================================================*/

extern const vec2_t g_vec2_zero;
extern const vec2_t g_vec2_one;

void          vec2_copy(const vec2_t in, vec2_t out);
void          vec2_set(s_float_t x, s_float_t y, vec2_t v);

/*!
 * Gets the squared length of a vector.  Useful for approximations and when
 * you don't need the actual magnitude.
 */
s_float_t     vec2_length_squared(const vec2_t v);
/*!
 * Gets the length/magnitude of a vector.
 */
s_float_t     vec2_length(const vec2_t v);
void          vec2_normalize(const vec2_t in, vec2_t out);

void          vec2_subtract(const vec2_t left, const vec2_t right, vec2_t out);
void          vec2_add(const vec2_t left, const vec2_t right, vec2_t out);
void          vec2_multiply(const vec2_t left, const vec2_t right, vec2_t out);
void          vec2_negate(const vec2_t v, vec2_t out);
void          vec2_inverse(const vec2_t v, vec2_t out);

void          vec2_project(const vec2_t in, const vec2_t normal, vec2_t out);
void          vec2_reflect(const vec2_t in, const vec2_t normal, vec2_t out);
s_float_t     vec2_dot_product(const vec2_t left, const vec2_t right);

void          vec2_scale(const vec2_t v, s_float_t scalar, vec2_t out);
int           vec2_divide(const vec2_t v, s_float_t divisor, vec2_t out);

int           vec2_equals(const vec2_t left, const vec2_t right);



/*==============================================================================

  3-Component Vector (vec3_t)

==============================================================================*/

extern const vec3_t g_vec3_zero;
extern const vec3_t g_vec3_one;

void          vec3_copy(const vec3_t in, vec3_t out);
void          vec3_set(s_float_t x, s_float_t y, s_float_t z, vec3_t v);

/*!
 * Gets the squared length of a vector.  Useful for approximations and when
 * you don't need the actual magnitude.
 */
s_float_t     vec3_length_squared(const vec3_t v);
/*!
 * Gets the length/magnitude of a vector.
 */
s_float_t     vec3_length(const vec3_t v);
void          vec3_normalize(const vec3_t in, vec3_t out);

void          vec3_subtract(const vec3_t left, const vec3_t right, vec3_t out);
void          vec3_add(const vec3_t left, const vec3_t right, vec3_t out);
void          vec3_multiply(const vec3_t left, const vec3_t right, vec3_t out);
void          vec3_negate(const vec3_t v, vec3_t out);
void          vec3_inverse(const vec3_t v, vec3_t out);

void          vec3_project(const vec3_t in, const vec3_t normal, vec3_t out);
void          vec3_reflect(const vec3_t in, const vec3_t normal, vec3_t out);
void          vec3_cross_product(const vec3_t left, const vec3_t right, vec3_t out);
s_float_t     vec3_dot_product(const vec3_t left, const vec3_t right);

void          vec3_scale(const vec3_t v, s_float_t scalar, vec3_t out);
int           vec3_divide(const vec3_t v, s_float_t divisor, vec3_t out);

int           vec3_equals(const vec3_t left, const vec3_t right);



/*==============================================================================

  4-Component Vector (vec4_t)

==============================================================================*/

extern const vec4_t g_vec4_zero;
extern const vec4_t g_vec4_one;
extern const vec4_t g_vec4_identity;

void          vec4_copy(const vec4_t in, vec4_t out);
void          vec4_set(s_float_t x, s_float_t y, s_float_t z, s_float_t w, vec4_t v);

/*!
 * Gets the squared length of a vector.  Useful for approximations and when
 * you don't need the actual magnitude.
 */
s_float_t     vec4_length_squared(const vec4_t v);
/*!
 * Gets the length/magnitude of a vector.
 */
s_float_t     vec4_length(const vec4_t v);
void          vec4_normalize(const vec4_t in, vec4_t out);

void          vec4_subtract(const vec4_t left, const vec4_t right, vec4_t out);
void          vec4_add(const vec4_t left, const vec4_t right, vec4_t out);
void          vec4_multiply(const vec4_t left, const vec4_t right, vec4_t out);
void          vec4_negate(const vec4_t v, vec4_t out);
void          vec4_inverse(const vec4_t v, vec4_t out);

void          vec4_project(const vec4_t in, const vec4_t normal, vec4_t out);
void          vec4_reflect(const vec4_t in, const vec4_t normal, vec4_t out);
s_float_t     vec4_dot_product(const vec4_t left, const vec4_t right);

void          vec4_scale(const vec4_t v, s_float_t scalar, vec4_t out);
int           vec4_divide(const vec4_t v, s_float_t divisor, vec4_t out);

int           vec4_equals(const vec4_t left, const vec4_t right);



/*==============================================================================

  3x3 Matrix (mat3_t)

==============================================================================*/

extern const mat3_t g_mat3_identity;

void          mat3_identity(mat3_t out);
void          mat3_copy(const mat3_t in, mat3_t out);
void          mat3_set(s_float_t m00, s_float_t m10, s_float_t m20,
                       s_float_t m01, s_float_t m11, s_float_t m21,
                       s_float_t m02, s_float_t m12, s_float_t m22,
                       mat3_t out);
void          mat3_to_mat4(const mat3_t in, mat4_t out);
void          mat3_rotation(s_float_t angle, s_float_t x, s_float_t y, s_float_t z, mat3_t out);
void          mat3_from_quat(const quat_t in, mat3_t out);
void          mat3_transpose(const mat3_t in, mat3_t out);
void          mat3_scale(const mat3_t in, s_float_t x, s_float_t y, s_float_t z, mat3_t out);
void          mat3_orthogonal(const mat3_t in, mat3_t out);
void          mat3_multiply(const mat3_t lhs, const mat3_t rhs, mat3_t out);
void          mat3_rotate_vec3(const mat3_t lhs, const vec3_t rhs, vec3_t out);
void          mat3_inv_rotate_vec3(const mat3_t lhs, const vec3_t rhs, vec3_t out);
void          mat3_cofactor(const mat3_t in, mat3_t out);
void          mat3_adjoint(const mat3_t in, mat3_t out);
void          mat3_get_row3(const mat3_t in, int row, vec3_t out);
void          mat3_get_column3(const mat3_t in, int column, vec3_t out);
void          mat3_set_row3(int row, const vec3_t in, mat3_t out);
void          mat3_set_column3(int column, const vec3_t in, mat3_t out);
s_float_t     mat3_determinant(const mat3_t in);
int           mat3_equals(const mat3_t lhs, const mat3_t rhs);
int           mat3_inverse(const mat3_t in, mat3_t out);


/*==============================================================================

  4x4 Matrix (mat4_t)

==============================================================================*/

extern const mat4_t g_mat4_identity;

void          mat4_identity(mat4_t out);
void          mat4_copy(const mat4_t in, mat4_t out);
void          mat4_set(
  s_float_t m00, s_float_t m01, s_float_t m02, s_float_t m03,
  s_float_t m04, s_float_t m05, s_float_t m06, s_float_t m07,
  s_float_t m08, s_float_t m09, s_float_t m10, s_float_t m11,
  s_float_t m12, s_float_t m13, s_float_t m14, s_float_t m15,
  mat4_t out);
void          mat4_to_mat3(const mat4_t in, mat3_t out);

void          mat4_set_axes3(const vec3_t x, const vec3_t y, const vec3_t z, const vec3_t w, mat4_t out);
void          mat4_get_axes3(const mat4_t m, vec3_t x, vec3_t y, vec3_t z, vec3_t w);
void          mat4_set_axes4(const vec4_t x, const vec4_t y, const vec4_t z, const vec4_t w, mat4_t out);
void          mat4_get_axes4(const mat4_t m, vec4_t x, vec4_t y, vec4_t z, vec4_t w);

/*! Builds a rotation matrix with the given angle and axis. */
void          mat4_rotation(s_float_t angle, s_float_t x, s_float_t y, s_float_t z, mat4_t out);
void          mat4_frustum(s_float_t left, s_float_t right, s_float_t bottom, s_float_t top, s_float_t near, s_float_t far, mat4_t out);
void          mat4_orthographic(s_float_t left, s_float_t right, s_float_t bottom, s_float_t top, s_float_t near, s_float_t far, mat4_t out);
void          mat4_perspective(s_float_t fov_y, s_float_t aspect, s_float_t near, s_float_t far, mat4_t out);
void          mat4_look_at(const vec3_t eye, const vec3_t center, const vec3_t up, mat4_t out);
void          mat4_from_quat(const quat_t quat, mat4_t out);

void          mat4_get_row4(const mat4_t in, int row, vec4_t out);
void          mat4_get_row3(const mat4_t in, int row, vec3_t out);
void          mat4_get_column4(const mat4_t in, int column, vec4_t out);
void          mat4_get_column3(const mat4_t in, int column, vec3_t out);

void          mat4_set_row4(int row, const vec4_t value, mat4_t inout);
void          mat4_set_row3(int row, const vec3_t value, mat4_t inout);
void          mat4_set_column4(int column, const vec4_t value, mat4_t inout);
void          mat4_set_column3(int column, const vec3_t value, mat4_t inout);

int           mat4_equals(const mat4_t left, const mat4_t right);

void          mat4_transpose(const mat4_t in, mat4_t out);
void          mat4_inverse_orthogonal(const mat4_t in, mat4_t out);
/*!
 * Writes the inverse affine of the input matrix to the output matrix.
 * \returns Non-zero if an inverse affine matrix can be created, otherwise
 * zero if not.  If zero, the output matrix is the identity matrix.
 */
int           mat4_inverse_affine(const mat4_t in, mat4_t out);
void          mat4_adjoint(const mat4_t in, mat4_t out);
s_float_t     mat4_determinant(const mat4_t m);
int           mat4_inverse_general(const mat4_t in, mat4_t out);

/*! Translates the given matrix by <X, Y, Z>. */
void          mat4_translate(s_float_t x, s_float_t y, s_float_t z, const mat4_t in, mat4_t out);
void          mat4_translation(s_float_t x, s_float_t y, s_float_t z, mat4_t out);
void          mat4_multiply(const mat4_t left, const mat4_t right, mat4_t out);
void          mat4_multiply_vec4(const mat4_t left, const vec4_t right, vec4_t out);
void          mat4_transform_vec3(const mat4_t left, const vec3_t right, vec3_t out);
void          mat4_rotate_vec3(const mat4_t left, const vec3_t right, vec3_t out);
void          mat4_inv_rotate_vec3(const mat4_t left, const vec3_t right, vec3_t out);
void          mat4_scale(const mat4_t in, s_float_t x, s_float_t y, s_float_t z, mat4_t out);



/*==============================================================================

  Quaternion (quat_t)

==============================================================================*/

extern const quat_t g_quat_identity;

/* note: all methods assume that input quaternions are unit quaternions */

void          quat_set(s_float_t x, s_float_t y, s_float_t z, s_float_t w, quat_t out);
void          quat_copy(const quat_t in, quat_t out);
void          quat_identity(quat_t q);

void          quat_inverse(const quat_t in, quat_t out);
void          quat_negate(const quat_t in, quat_t out);

void          quat_multiply(const quat_t left, const quat_t right, quat_t out);
void          quat_multiply_vec3(const quat_t left, const vec3_t right, vec3_t out);

void          quat_from_angle_axis(s_float_t angle, s_float_t x, s_float_t y, s_float_t z, quat_t out);
void          quat_from_mat4(const mat4_t mat, quat_t out);
void          quat_from_mat3(const mat3_t mat, quat_t out);

void          quat_slerp(const quat_t from, const quat_t to, s_float_t delta, quat_t out);

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* end of include guard: __SNOW__MATHS_H__ */

