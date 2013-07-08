/*
  Quaternion maths
  Written by Noel Cower

  See COPYING for license information
*/

#define __SNOW__QUAT_C__

#include "maths_local.h"

#if defined(__cplusplus)
extern "C"
{
#endif /* __cplusplus */

const quat_t g_quat_identity = { s_float_lit(0.0), s_float_lit(0.0), s_float_lit(0.0), s_float_lit(1.0) };

void quat_set(s_float_t x, s_float_t y, s_float_t z, s_float_t w, quat_t out)
{
  out[0] = x;
  out[1] = y;
  out[2] = z;
  out[3] = w;
}

void quat_copy(const quat_t in, quat_t out)
{
  if (in == out) return;

  out[0] = in[0];
  out[1] = in[1];
  out[2] = in[2];
  out[3] = in[3];
}

void quat_identity(quat_t q)
{
  q[0] = q[1] = q[2] = s_float_lit(0.0);
  q[3] = s_float_lit(1.0);
}

void quat_inverse(const quat_t in, quat_t out)
{
  out[0] = -in[0];
  out[1] = -in[1];
  out[2] = -in[2];
  out[3] = in[3];
}

void quat_negate(const quat_t in, quat_t out)
{
  out[0] = -in[0];
  out[1] = -in[1];
  out[2] = -in[2];
  out[3] = -in[3];
}

void quat_multiply(const quat_t left, const quat_t right, quat_t out)
{
  s_float_t w;
  s_float_t w1, w2;
  vec3_t t1, t2;
  w1 = left[3];
  w2 = right[3];

  w = (w1 * w2) - vec3_dot_product(left, right);
  vec3_copy(right, t1);
  vec3_scale(t1, w1, t1);
  vec3_copy(left, t2);
  vec3_scale(t2, w2, t2);
  vec3_add(t1, t2, t1);
  vec3_cross_product(right, left, t2);
  vec3_add(t1, t2, t1);
  vec3_copy(t1, out);
  out[3] = w;
}

void quat_multiply_vec3(const quat_t left, const vec3_t right, vec3_t out)
{
  vec3_t lxr_cross, lxlr_cross;
  vec3_cross_product(left, right, lxr_cross);
  vec3_cross_product(left, lxr_cross, lxlr_cross);
  vec3_scale(lxr_cross, s_float_lit(2.0) * left[3], lxr_cross);
  vec3_scale(lxlr_cross, s_float_lit(2.0), lxlr_cross);
  vec3_add(lxr_cross, lxlr_cross, lxr_cross);
  vec3_add(right, lxr_cross, out);
}

void quat_from_angle_axis(s_float_t angle, s_float_t x, s_float_t y, s_float_t z, quat_t out)
{
  vec3_t v = {x, y, z};
  vec3_normalize(v, v);

  angle *= (S_DEG2RAD * s_float_lit(0.5));
  const s_float_t s = s_sin(angle);

  out[0] = v[0] * s;
  out[1] = v[1] * s;
  out[2] = v[2] * s;
  out[3] = s_cos(angle);
}

void quat_from_mat4(const mat4_t mat, quat_t out)
{
  s_float_t trace, r;
  int index;

  trace = mat[0] + mat[5] + mat[10];
  if (trace > 0) {
    r = s_sqrt(trace + s_float_lit(1.0));
    out[3] = r * s_float_lit(0.5);
    r = s_float_lit(0.5) / r;
    out[0] = (mat[9] - mat[6]) * r;
    out[1] = (mat[2] - mat[8]) * r;
    out[2] = (mat[4] - mat[1]) * r;
  } else {
    index = 0;
    if (mat[10] > mat[index * 5]) {
      index = 2;
    } else if (mat[5] > mat[0]) {
      index = 1;
    }

    r = s_sqrt(mat[index * 5] - (mat[((index + 1)) % 3 * 5] + mat[((index + 2) % 3) * 5]) + s_float_lit(1.0));
    out[index] = r * s_float_lit(0.5);

    if (r) r = s_float_lit(0.5) / r;

    switch (index)
    {
    case 0:
      out[1] = (mat[1] + mat[4]) * r;
      out[2] = (mat[2] + mat[8]) * r;
      out[3] = (mat[9] - mat[6]) * r;
    break;

    case 1:
      out[3] = (mat[2] + mat[8]) * r;
      out[2] = (mat[9] + mat[6]) * r;
      out[0] = (mat[1] - mat[4]) * r;
    break;

    case 2:
      out[3] = (mat[4] + mat[1]) * r;
      out[0] = (mat[2] + mat[8]) * r;
      out[1] = (mat[6] - mat[9]) * r;
    break;
    }
  }
}

void quat_from_mat3(const mat3_t mat, quat_t out)
{
  s_float_t trace, r;
  const s_float_t *m = m;
  s_float_t m01 = mat[3], m02 = mat[6], m10 = mat[1],
            m12 = mat[7], m20 = mat[2], m21 = mat[5];

  trace = mat[0] + mat[4] + mat[8];

  if (trace > s_float_lit(0.0)) {
    r = s_sqrt(trace + s_float_lit(1.0));
    out[3] = r * s_float_lit(0.5);
    r = s_float_lit(0.5) / r;
    out[0] = (m12 - m21) * r;
    out[1] = (m20 - m02) * r;
    out[2] = (m01 - m10) * r;
  } else {
    int index = 0;
    if (mat[8] > (index ? mat[4] : mat[0])) {
      index = 2;
    } else if (mat[4] > mat[0]) {
      index = 1;
    }

    switch (index) {
    default:
    case 0:
      r = out[0] = s_sqrt(mat[0] - (mat[4] + mat[8]) + s_float_lit(1.0)) * s_float_lit(0.5);
      if (!float_is_zero(r)) {
        r = s_float_lit(0.5) / r;
      }
      out[1] = (m10 + m01) * r;
      out[2] = (m20 + m02) * r;
      out[3] = (m12 - m21) * r;
      break;

    case 1:
      r = out[1] = s_sqrt(mat[4] - (mat[8] + mat[0]) + s_float_lit(1.0)) * s_float_lit(0.5);
      if (!float_is_zero(r)) {
        r = s_float_lit(0.5) / r;
      }
      out[0] = (m10 + m01) * r;
      out[2] = (m12 + m21) * r;
      out[3] = (m20 - m02) * r;
      break;

    case 2:
      r = out[2] = s_sqrt(mat[4] - (mat[0] + mat[4]) + s_float_lit(1.0)) * s_float_lit(0.5);
      if (!float_is_zero(r)) {
        r = s_float_lit(0.5) / r;
      }
      out[0] = (m20 + m02) * r;
      out[1] = (m21 + m12) * r;
      out[3] = (m01 - m10) * r;
      break;
    }
  }
}

void quat_slerp(const quat_t from, const quat_t to, s_float_t delta, quat_t out)
{
  s_float_t dot, scale0, scale1, angle, inverse_sin;
  s_float_t dx, dy, dz, dw;

  dot = vec4_dot_product((const s_float_t *)from, (const s_float_t *)to);

  if (dot < s_float_lit(0.0)) {
    dot = -dot;
    dx = -to[0];
    dy = -to[1];
    dz = -to[2];
    dw = -to[3];
  } else {
    dx = to[0];
    dy = to[1];
    dz = to[2];
    dw = to[3];
  }

  delta = fminf(fmaxf(delta, s_float_lit(0.0)), s_float_lit(1.0));

  angle = s_acos(dot);
  inverse_sin = s_float_lit(1.0) / s_sin(dot);

  scale0 = s_sin((s_float_lit(1.0) - delta) * angle) * inverse_sin;
  scale1 = s_sin(delta * angle) * inverse_sin;

  out[0] = (from[0] * scale0) + (dx * scale1);
  out[1] = (from[1] * scale0) + (dy * scale1);
  out[2] = (from[2] * scale0) + (dz * scale1);
  out[3] = (from[3] * scale0) + (dw * scale1);
}

#if defined(__cplusplus)
}
#endif /* __cplusplus */

