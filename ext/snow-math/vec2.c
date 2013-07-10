/*
  2D vector maths
  Written by Noel Cower

  See COPYING for license information
*/

#define __SNOW__VEC2_C__

#include "maths_local.h"

#if defined(__cplusplus)
extern "C"
{
#endif /* __cplusplus */

const vec2_t g_vec2_zero = {s_float_lit(0.0), s_float_lit(0.0)};
const vec2_t g_vec2_one = {s_float_lit(1.0), s_float_lit(1.0)};

void vec2_copy(const vec2_t in, vec2_t out)
{
  out[1] = in[1];
  out[0] = in[0];
}

void vec2_set(s_float_t x, s_float_t y, vec2_t v)
{
  v[0] = x;
  v[1] = y;
}

/*!
 * Gets the squared length of a vector.  Useful for approximations and when
 * you don't need the actual magnitude.
 */
s_float_t vec2_length_squared(const vec2_t v)
{
  return (v[0] * v[0]) + (v[1] * v[1]);
}

/*!
 * Gets the length/magnitude of a vector.
 */
s_float_t vec2_length(const vec2_t v)
{
  return s_sqrt((v[0] * v[0]) + (v[1] * v[1]));
}

void vec2_normalize(const vec2_t in, vec2_t out)
{
  s_float_t mag = vec2_length(in);
  if (mag) mag = s_float_lit(1.0) / mag;
  out[1] = in[1] * mag;
  out[0] = in[0] * mag;
}

void vec2_subtract(const vec2_t left, const vec2_t right, vec2_t out)
{
  out[1] = left[1] - right[1];
  out[0] = left[0] - right[0];
}

void vec2_add(const vec2_t left, const vec2_t right, vec2_t out)
{
  out[1] = left[1] + right[1];
  out[0] = left[0] + right[0];
}

void vec2_multiply(const vec2_t left, const vec2_t right, vec2_t out)
{
  out[1] = left[1] * right[1];
  out[0] = left[0] * right[0];
}

void vec2_negate(const vec2_t v, vec2_t out)
{
  out[1] = -v[1];
  out[0] = -v[0];
}

void vec2_inverse(const vec2_t v, vec2_t out)
{
  out[1] = (!float_is_zero(v[1])) ? (s_float_lit(1.0) / v[1]) : v[1];
  out[0] = (!float_is_zero(v[0])) ? (s_float_lit(1.0) / v[0]) : v[0];
}

void vec2_project(const vec2_t in, const vec2_t normal, vec2_t out)
{
  vec2_scale(normal, vec2_dot_product(in, normal), out);
}

void vec2_reflect(const vec2_t in, const vec2_t normal, vec2_t out)
{
  vec2_t temp;
  vec2_scale(normal, s_float_lit(2.0) * vec2_dot_product(in, normal), temp);
  vec2_subtract(in, temp, out);
}

s_float_t vec2_dot_product(const vec2_t left, const vec2_t right)
{
  return ((left[0] * right[0]) + (left[1] * right[1]));
}

void vec2_scale(const vec2_t v, s_float_t scalar, vec2_t out)
{
   out[1] = v[1] * scalar;
   out[0] = v[0] * scalar;
}

/*!
 * Divides the given vector by the divisor.
 * \returns Zero if successful, otherwise nonzero if the result is undefined.
 */
int vec2_divide(const vec2_t v, s_float_t divisor, vec2_t out)
{
  if (divisor) {
    divisor = s_float_lit(1.0) / divisor;
    out[1] = v[1] * divisor;
    out[0] = v[0] * divisor;
    return 1;
  }
  return 0;
}

int vec2_equals(const vec2_t left, const vec2_t right)
{
  return float_equals(left[0], right[0]) &&
         float_equals(left[1], right[1]);
}

#if defined(__cplusplus)
}
#endif /* __cplusplus */

