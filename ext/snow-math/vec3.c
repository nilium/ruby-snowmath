/*
  3D vector maths
  Written by Noel Cower

  See COPYING for license information
*/

#define __SNOW__VEC3_C__

#include "maths_local.h"

#if defined(__cplusplus)
extern "C"
{
#endif /* __cplusplus */

const vec3_t g_vec3_zero = {s_float_lit(0.0), s_float_lit(0.0), s_float_lit(0.0)};
const vec3_t g_vec3_one = {s_float_lit(1.0), s_float_lit(1.0), s_float_lit(1.0)};

void vec3_copy(const vec3_t in, vec3_t out)
{
  out[0] = in[0];
  out[1] = in[1];
  out[2] = in[2];
}

void vec3_set(s_float_t x, s_float_t y, s_float_t z, vec3_t v)
{
  v[0] = x;
  v[1] = y;
  v[2] = z;
}

/*!
 * Gets the squared length of a vector.  Useful for approximations and when
 * you don't need the actual magnitude.
 */
s_float_t vec3_length_squared(const vec3_t v)
{
  return (v[0] * v[0]) + (v[1] * v[1]) + (v[2] * v[2]);
}

/*!
 * Gets the length/magnitude of a vector.
 */
s_float_t vec3_length(const vec3_t v)
{
  return s_sqrt((v[0] * v[0]) + (v[1] * v[1]) + (v[2] * v[2]));
}

void vec3_normalize(const vec3_t in, vec3_t out)
{
  s_float_t mag = vec3_length(in);
  if (mag) mag = s_float_lit(1.0) / mag;
  out[0] = in[0] * mag;
  out[1] = in[1] * mag;
  out[2] = in[2] * mag;
}

void vec3_subtract(const vec3_t left, const vec3_t right, vec3_t out)
{
  out[0] = left[0] - right[0];
  out[1] = left[1] - right[1];
  out[2] = left[2] - right[2];
}

void vec3_add(const vec3_t left, const vec3_t right, vec3_t out)
{
  out[0] = left[0] + right[0];
  out[1] = left[1] + right[1];
  out[2] = left[2] + right[2];
}

void vec3_multiply(const vec3_t left, const vec3_t right, vec3_t out)
{
  out[0] = left[0] * right[0];
  out[1] = left[1] * right[1];
  out[2] = left[2] * right[2];
}

void vec3_negate(const vec3_t v, vec3_t out)
{
  out[2] = -v[2];
  out[1] = -v[1];
  out[0] = -v[0];
}

void vec3_inverse(const vec3_t v, vec3_t out)
{
  out[2] = (v[2] != s_float_lit(0.0) && v[2] != s_float_lit(-0.0)) ? (s_float_lit(1.0) / v[2]) : v[2];
  out[1] = (v[1] != s_float_lit(0.0) && v[1] != s_float_lit(-0.0)) ? (s_float_lit(1.0) / v[1]) : v[1];
  out[0] = (v[0] != s_float_lit(0.0) && v[0] != s_float_lit(-0.0)) ? (s_float_lit(1.0) / v[0]) : v[0];
}

void vec3_project(const vec3_t in, const vec3_t normal, vec3_t out)
{
  vec3_scale(normal, vec3_dot_product(in, normal), out);
}

void vec3_reflect(const vec3_t in, const vec3_t normal, vec3_t out)
{
  vec3_t temp;
  vec3_scale(normal, s_float_lit(2.0) * vec3_dot_product(in, normal), temp);
  vec3_subtract(in, temp, out);
}

void vec3_cross_product(const vec3_t left, const vec3_t right, vec3_t out)
{
  s_float_t x, y, z;
  x = (left[1] * right[2]) - (left[2] * right[1]);
  y = (left[0] * right[2]) - (left[2] * right[0]);
  z = (left[0] * right[1]) - (left[1] * right[0]);
  out[0] = x;
  out[1] = y;
  out[2] = z;
}

s_float_t vec3_dot_product(const vec3_t left, const vec3_t right)
{
  return ((left[0] * right[0]) + (left[1] * right[1]) + (left[2] * right[2]));
}

void vec3_scale(const vec3_t v, s_float_t scalar, vec3_t out)
{
  out[2] = v[2] * scalar;
  out[1] = v[1] * scalar;
  out[0] = v[0] * scalar;
}

/*!
 * Divides the given vector by the divisor.
 * \returns Zero if successful, otherwise nonzero if the result is undefined.
 */
int vec3_divide(const vec3_t v, s_float_t divisor, vec3_t out)
{
  if (divisor) {
    divisor = s_float_lit(1.0) / divisor;
    out[2] = v[2] * divisor;
    out[1] = v[1] * divisor;
    out[0] = v[0] * divisor;
    return 0;
  }
  return 1;
}

int vec3_equals(const vec3_t left, const vec3_t right)
{
  return float_equals(left[0], right[0]) &&
    float_equals(left[1], right[1]) &&
    float_equals(left[2], right[2]);
}

#if defined(__cplusplus)
}
#endif /* __cplusplus */

