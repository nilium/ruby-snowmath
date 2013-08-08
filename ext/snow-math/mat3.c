/*
  3x3 (rotation / scaling) matrix
  Written by Noel Cower

  See COPYING for license information
*/

#define __SNOW__MAT3_C__

#include "maths_local.h"

#if defined(__cplusplus)
extern "C"
{
#endif /* __cplusplus */



const mat3_t g_mat3_identity = {
  s_float_lit(1.0), s_float_lit(0.0), s_float_lit(0.0),
  s_float_lit(0.0), s_float_lit(1.0), s_float_lit(0.0),
  s_float_lit(0.0), s_float_lit(0.0), s_float_lit(1.0)
};



/* Rows */
#define S_MR 0
#define S_MS 3
#define S_MT 6

/* row components */
#define S_MR_X 0
#define S_MR_Y 1
#define S_MR_Z 2
#define S_MS_X 3
#define S_MS_Y 4
#define S_MS_Z 5
#define S_MT_X 6
#define S_MT_Y 7
#define S_MT_Z 8

/* columns */
#define S_MIR_X 0
#define S_MIR_Y 3
#define S_MIR_Z 6
#define S_MIS_X 1
#define S_MIS_Y 4
#define S_MIS_Z 7
#define S_MIT_X 2
#define S_MIT_Y 5
#define S_MIT_Z 8



void mat3_identity(mat3_t out)
{
  out[0] =
  out[4] =
  out[8] = s_float_lit(1.0);

  out[1] =
  out[2] =
  out[3] =
  out[5] =
  out[6] =
  out[7] = s_float_lit(0.0);
}



void mat3_to_mat4(const mat3_t in, mat4_t out)
{
  out[0] = in[0];
  out[1] = in[1];
  out[2] = in[2];
  out[3] = s_float_lit(0.0);
  out[4] = in[3];
  out[5] = in[4];
  out[6] = in[5];
  out[7] = s_float_lit(0.0);
  out[8] = in[6];
  out[9] = in[7];
  out[10] = in[8];
  out[14] =
  out[13] =
  out[12] =
  out[11] = s_float_lit(0.0);
  out[15] = s_float_lit(1.0);
}



void mat3_set(s_float_t m00, s_float_t m10, s_float_t m20,
              s_float_t m01, s_float_t m11, s_float_t m21,
              s_float_t m02, s_float_t m12, s_float_t m22,
              mat3_t out)
{
  out[0] = m00;
  out[1] = m10;
  out[2] = m20;
  out[3] = m01;
  out[4] = m11;
  out[5] = m21;
  out[6] = m02;
  out[7] = m12;
  out[8] = m22;
}



void mat3_rotation(s_float_t angle, s_float_t x, s_float_t y, s_float_t z, mat3_t out)
{
  const s_float_t angle_rad = angle * S_DEG2RAD;
  const s_float_t c  = s_cos(angle_rad);
  const s_float_t s  = s_sin(angle_rad);
  const s_float_t ic = s_float_lit(1.0) - c;
  const s_float_t xy = x * y * ic;
  const s_float_t yz = y * z * ic;
  const s_float_t xz = x * z * ic;
  const s_float_t xs = s * x;
  const s_float_t ys = s * y;
  const s_float_t zs = s * z;
  const s_float_t xx = x * x;
  const s_float_t yy = y * y;
  const s_float_t zz = z * z;

  out[0] = xx + c * (s_float_lit(1.0) - xx);
  out[1] = xy * ic - zs;
  out[2] = z * x * ic + ys;

  out[3] = xy * ic + zs;
  out[4] = yy + c * (s_float_lit(1.0) - yy);
  out[5] = yz * ic - xs;

  out[6] = xz * ic - ys;
  out[7] = yz * ic + xs;
  out[8] = zz + c * (s_float_lit(1.0) - zz);
}



void mat3_from_quat(const quat_t in, mat3_t out)
{
  s_float_t xx, xy, xz, yy, yz, zz, wx, wy, wz;

  xx = in[0] * in[0];
  xy = in[0] * in[1];
  xz = in[0] * in[2];

  yy = in[1] * in[1];
  yz = in[2] * in[2];

  zz = in[2] * in[2];

  wx = in[0] * in[3];
  wy = in[1] * in[3];
  wz = in[2] * in[3];

  out[0] = s_float_lit(1.0) - s_float_lit(2.0) * (yy + zz);
  out[1] = s_float_lit(2.0) * (xy - wz);
  out[2] = s_float_lit(2.0) * (xz + wy);
  out[3] = s_float_lit(2.0) * (xy + wz);
  out[4] = s_float_lit(1.0) - s_float_lit(2.0) * (xx + zz);
  out[5] = s_float_lit(2.0) * (yz - wx);
  out[6] = s_float_lit(2.0) * (xz - wy);
  out[7] = s_float_lit(2.0) * (yz + wx);
  out[8] = s_float_lit(1.0) - s_float_lit(2.0) * (xx + yy);
}



void mat3_transpose(const mat3_t in, mat3_t out)
{
  s_float_t temp;
  #define S_SWAP_ELEM(LHS, RHS) temp = in[LHS]; out[LHS] = in[RHS]; out[RHS] = temp
  out[0] = in[0];
  out[4] = in[4];
  out[8] = in[8];
  S_SWAP_ELEM(1, 3);
  S_SWAP_ELEM(2, 6);
  S_SWAP_ELEM(5, 7);
  #undef S_SWAP_ELEM
}



void mat3_copy(const mat3_t in, mat3_t out)
{
  out[0] = in[0];
  out[1] = in[1];
  out[2] = in[2];
  out[3] = in[3];
  out[4] = in[4];
  out[5] = in[5];
  out[6] = in[6];
  out[7] = in[7];
  out[8] = in[8];
}



int mat3_equals(const mat3_t lhs, const mat3_t rhs)
{
  return
    float_equals(rhs[0], lhs[0]) &&
    float_equals(rhs[1], lhs[1]) &&
    float_equals(rhs[2], lhs[2]) &&
    float_equals(rhs[3], lhs[3]) &&
    float_equals(rhs[4], lhs[4]) &&
    float_equals(rhs[5], lhs[5]) &&
    float_equals(rhs[6], lhs[6]) &&
    float_equals(rhs[7], lhs[7]) &&
    float_equals(rhs[8], lhs[8]);
}



void mat3_scale(const mat3_t in, s_float_t x, s_float_t y, s_float_t z, mat3_t out)
{
  out[0] = in[0] * x;
  out[1] = in[1] * y;
  out[2] = in[2] * z;
  out[3] = in[3] * x;
  out[4] = in[4] * y;
  out[5] = in[5] * z;
  out[6] = in[6] * x;
  out[7] = in[7] * y;
  out[8] = in[8] * z;
}



void mat3_orthogonal(const mat3_t in, mat3_t out)
{
  vec3_normalize(&in[S_MT], &out[S_MT]);
  vec3_cross_product(&in[S_MS], &in[S_MT], &out[S_MR]);
  vec3_normalize(&in[S_MR], &out[S_MR]);
  vec3_cross_product(&in[S_MT], &in[S_MR], &out[S_MS]);
}



void mat3_multiply(const mat3_t lhs, const mat3_t rhs, mat3_t out)
{
  mat3_t temp;
  s_float_t cx, cy, cz;

  mat3_transpose(lhs, temp);

  cx      = temp[0];
  cy      = temp[1];
  cz      = temp[2];

  temp[0] = (cx * rhs[0 ]) + (cy * rhs[1 ]) + (cz * rhs[2 ]);
  temp[1] = (cx * rhs[3 ]) + (cy * rhs[4 ]) + (cz * rhs[5 ]);
  temp[2] = (cx * rhs[6 ]) + (cy * rhs[7 ]) + (cz * rhs[8]);

  cx      = temp[3];
  cy      = temp[4];
  cz      = temp[5];

  temp[3] = (cx * rhs[0 ]) + (cy * rhs[1 ]) + (cz * rhs[2 ]);
  temp[4] = (cx * rhs[3 ]) + (cy * rhs[4 ]) + (cz * rhs[5 ]);
  temp[5] = (cx * rhs[6 ]) + (cy * rhs[7 ]) + (cz * rhs[8]);

  cx      = temp[6];
  cy      = temp[7];
  cz      = temp[8];

  temp[6] = (cx * rhs[0 ]) + (cy * rhs[1 ]) + (cz * rhs[2 ]);
  temp[7] = (cx * rhs[3 ]) + (cy * rhs[4 ]) + (cz * rhs[5 ]);
  temp[8] = (cx * rhs[6 ]) + (cy * rhs[7 ]) + (cz * rhs[8]);

  mat3_transpose(temp, out);
}



void mat3_rotate_vec3(const mat3_t lhs, const vec3_t rhs, vec3_t out)
{
  mat3_t temp;
  s_float_t x;
  s_float_t y;
  s_float_t z;

  mat3_transpose(lhs, temp);

  x = temp[0] * rhs[0] + temp[1] * rhs[1] + temp[2] * rhs[2];
  y = temp[3] * rhs[0] + temp[4] * rhs[1] + temp[5] * rhs[2];
  z = temp[6] * rhs[0] + temp[7] * rhs[1] + temp[8] * rhs[2];

  out[0] = x;
  out[1] = y;
  out[2] = z;
}



void mat3_inv_rotate_vec3(const mat3_t lhs, const vec3_t rhs, vec3_t out)
{
  s_float_t x;
  s_float_t y;
  s_float_t z;

  x = lhs[0] * rhs[0] + lhs[1] * rhs[1] + lhs[2] * rhs[2];
  y = lhs[3] * rhs[0] + lhs[4] * rhs[1] + lhs[5] * rhs[2];
  z = lhs[6] * rhs[0] + lhs[7] * rhs[1] + lhs[8] * rhs[2];

  out[0] = x;
  out[1] = y;
  out[2] = z;
}



void mat3_cofactor(const mat3_t in, mat3_t out)
{
  const mat3_t temp = {
       (in[S_MS_Y] * in[S_MT_Z] - in[S_MS_Z] * in[S_MT_Y]),
      -(in[S_MS_X] * in[S_MT_Z] - in[S_MS_Z] * in[S_MT_X]),
       (in[S_MS_X] * in[S_MT_Y] - in[S_MS_Y] * in[S_MT_X]),

      -(in[S_MR_Y] * in[S_MT_Z] - in[S_MR_Z] * in[S_MT_Y]),
       (in[S_MR_X] * in[S_MT_Z] - in[S_MR_Z] * in[S_MT_X]),
      -(in[S_MR_X] * in[S_MT_Y] - in[S_MR_Y] * in[S_MT_X]),

       (in[S_MR_Y] * in[S_MS_Z] - in[S_MR_Z] * in[S_MS_Y]),
      -(in[S_MR_X] * in[S_MS_Z] - in[S_MR_Z] * in[S_MS_X]),
       (in[S_MR_X] * in[S_MS_Y] - in[S_MR_Y] * in[S_MS_X])
  };
  mat3_copy(temp, out);
}



s_float_t mat3_determinant(const mat3_t in)
{
  return in[S_MR_X] * (in[S_MS_Y] * in[S_MT_Z] - in[S_MS_Z] * in[S_MT_Y]) +
         in[S_MR_Y] * (in[S_MS_Z] * in[S_MT_X] - in[S_MS_X] * in[S_MT_Z]) +
         in[S_MR_Z] * (in[S_MS_X] * in[S_MT_Y] - in[S_MS_Y] * in[S_MT_X]);
}



void mat3_adjoint(const mat3_t in, mat3_t out)
{
  mat3_cofactor(in, out);
  mat3_transpose(out, out);
}



int mat3_inverse(const mat3_t in, mat3_t out)
{
  s_float_t determinant = mat3_determinant(in);
  if (!float_is_zero(determinant)) {
    determinant = s_float_lit(1.0) / determinant;
  } else {
    return 0;
  }

  mat3_cofactor(in, out);
  mat3_transpose(out, out);
  out[0] *= determinant;
  out[1] *= determinant;
  out[2] *= determinant;
  out[3] *= determinant;
  out[4] *= determinant;
  out[5] *= determinant;
  out[6] *= determinant;
  out[7] *= determinant;
  out[8] *= determinant;

  return 1;
}



void mat3_get_row3(const mat3_t in, int row, vec3_t out)
{
  switch (row) {
  case 0: {
    out[0] = in[0];
    out[1] = in[1];
    out[2] = in[2];
    break;
  }
  case 1: {
    out[0] = in[3];
    out[1] = in[4];
    out[2] = in[5];
    break;
  }
  case 2: {
    out[0] = in[6];
    out[1] = in[7];
    out[2] = in[8];
    break;
  }
  default: { break; }
  }
}



void mat3_get_column3(const mat3_t in, int column, vec3_t out)
{
  switch (column) {
  case 0: {
    out[0] = in[0];
    out[1] = in[3];
    out[2] = in[6];
    break;
  }
  case 1: {
    out[0] = in[1];
    out[1] = in[4];
    out[2] = in[7];
    break;
  }
  case 2: {
    out[0] = in[2];
    out[1] = in[5];
    out[2] = in[8];
    break;
  }
  default: { break; }
  }
}



void mat3_set_row3(int row, const vec3_t in, mat3_t out)
{
  switch (row) {
  case 0: {
    out[0] = in[0];
    out[1] = in[1];
    out[2] = in[2];
    break;
  }
  case 1: {
    out[3] = in[0];
    out[4] = in[1];
    out[5] = in[2];
    break;
  }
  case 2: {
    out[6] = in[0];
    out[7] = in[1];
    out[8] = in[2];
    break;
  }
  default: { break; }
  }
}



void mat3_set_column3(int column, const vec3_t in, mat3_t out)
{
  switch (column) {
  case 0: {
    out[0] = in[0];
    out[3] = in[1];
    out[6] = in[2];
    break;
  }
  case 1: {
    out[3] = in[0];
    out[4] = in[1];
    out[5] = in[2];
    break;
  }
  case 2: {
    out[6] = in[0];
    out[7] = in[1];
    out[8] = in[2];
    break;
  }
  default: { break; }
  }
}





#if defined(__cplusplus)
}
#endif
