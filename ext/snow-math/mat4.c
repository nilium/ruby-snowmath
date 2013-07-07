/*
  Transformation matrix
  Written by Noel Cower

  See COPYING for license information
*/

#define __SNOW__MAT4_C__

#include "maths_local.h"

#if defined(__cplusplus)
extern "C"
{
#endif /* __cplusplus */

/* reference:
  x y z w
  0 1 2 3
  4 5 6 7
  8 9 10  11
  12  13  14  15
*/

const mat4_t g_mat4_identity = {
  1.0, 0.0, 0.0, 0.0,
  0.0, 1.0, 0.0, 0.0,
  0.0, 0.0, 1.0, 0.0,
  0.0, 0.0, 0.0, 1.0
};

static s_float_t mat4_cofactor(const mat4_t m, int r0, int r1, int r2, int c0, int c1, int c2);

void mat4_identity(mat4_t out)
{
  out[0] = out[5] = out[10] = out[15] = 1.0;
  out[1] = out[2] = out[3] =
  out[4] = out[6] = out[7] =
  out[8] = out[9] = out[11] =
  out[12] = out[13] = out[14] = 0.0;
}

void mat4_copy(const mat4_t in, mat4_t out)
{
  out[0 ] = in[0 ];
  out[1 ] = in[1 ];
  out[2 ] = in[2 ];
  out[3 ] = in[3 ];
  out[4 ] = in[4 ];
  out[5 ] = in[5 ];
  out[6 ] = in[6 ];
  out[7 ] = in[7 ];
  out[8 ] = in[8 ];
  out[9 ] = in[9 ];
  out[10] = in[10];
  out[11] = in[11];
  out[12] = in[12];
  out[13] = in[13];
  out[14] = in[14];
  out[15] = in[15];
}

void mat4_set(
  s_float_t m00, s_float_t m01, s_float_t m02, s_float_t m03,
  s_float_t m04, s_float_t m05, s_float_t m06, s_float_t m07,
  s_float_t m08, s_float_t m09, s_float_t m10, s_float_t m11,
  s_float_t m12, s_float_t m13, s_float_t m14, s_float_t m15,
  mat4_t out)
{
  out[0 ] = m00;
  out[1 ] = m01;
  out[2 ] = m02;
  out[3 ] = m03;
  out[4 ] = m04;
  out[5 ] = m05;
  out[6 ] = m06;
  out[7 ] = m07;
  out[8 ] = m08;
  out[9 ] = m09;
  out[10] = m10;
  out[11] = m11;
  out[12] = m12;
  out[13] = m13;
  out[14] = m14;
  out[15] = m15;
}

void mat4_set_axes3(const vec3_t x, const vec3_t y, const vec3_t z, const vec3_t w, mat4_t out)
{
  out[0] = x[0];
  out[4] = x[1];
  out[8] = x[2];

  out[1] = y[0];
  out[5] = y[1];
  out[9] = y[2];

  out[2] = z[0];
  out[6] = z[1];
  out[10] = z[2];

  out[3] = w[0];
  out[7] = w[1];
  out[11] = w[2];

  out[12] = out[13] = out[14] = 0.0;
  out[15] = 1.0;
}

void mat4_get_axes3(const mat4_t m, vec3_t x, vec3_t y, vec3_t z, vec3_t w)
{
  if (x) {
    x[0] = m[0];
    x[1] = m[4];
    x[2] = m[8];
  }

  if (y) {
    y[0] = m[1];
    y[1] = m[5];
    y[2] = m[9];
  }

  if (z) {
    z[0] = m[2];
    z[1] = m[6];
    z[2] = m[10];
  }

  if (w) {
    w[0] = m[3];
    w[1] = m[7];
    w[2] = m[11];
  }
}

void mat4_set_axes4(const vec4_t x, const vec4_t y, const vec4_t z, const vec4_t w, mat4_t out)
{
  out[0] = x[0];
  out[4] = x[1];
  out[8] = x[2];
  out[12] = x[3];

  out[1] = y[0];
  out[5] = y[1];
  out[9] = y[2];
  out[13] = y[3];

  out[2] = z[0];
  out[6] = z[1];
  out[10] = z[2];
  out[14] = z[3];

  out[3] = w[0];
  out[7] = w[1];
  out[11] = w[2];
  out[15] = w[3];
}

void mat4_get_axes4(const mat4_t m, vec4_t x, vec4_t y, vec4_t z, vec4_t w)
{
  if (x) {
    x[0] = m[0];
    x[1] = m[4];
    x[2] = m[8];
    x[3] = m[12];
  }

  if (y) {
    y[0] = m[1];
    y[1] = m[5];
    y[2] = m[9];
    y[3] = m[13];
  }

  if (z) {
    z[0] = m[2];
    z[1] = m[6];
    z[2] = m[10];
    z[3] = m[14];
  }

  if (w) {
    w[0] = m[3];
    w[1] = m[7];
    w[2] = m[11];
    w[3] = m[15];
  }
}


/*! Builds a rotation matrix with the given angle and axis. */
void mat4_rotation(s_float_t angle, s_float_t x, s_float_t y, s_float_t z, mat4_t out)
{
  const s_float_t angle_rad = angle * S_DEG2RAD;
  const s_float_t c = s_cos(angle_rad);
  const s_float_t s = s_sin(angle_rad);
  const s_float_t ic = 1.0 - c;
  const s_float_t xy = x * y * ic;
  const s_float_t yz = y * z * ic;
  const s_float_t xz = x * z * ic;
  const s_float_t xs = s * x;
  const s_float_t ys = s * y;
  const s_float_t zs = s * z;

  out[0] = ((x * x) * ic) + c;
  out[1] = xy + zs;
  out[2] = xz - ys;
  out[4] = xy - zs;
  out[5] = ((y * y) * ic) + c;
  out[6] = yz + xs;
  out[8] = xz + ys;
  out[9] = yz - xs;
  out[10] = ((z * z) * ic) + c;
  out[3] = out[7] = out[11] =
  out[12] = out[13] = out[14] = 0.0;
  out[15] = 1.0;
}

void mat4_frustum(s_float_t left, s_float_t right, s_float_t bottom, s_float_t top, s_float_t near, s_float_t far, mat4_t out)
{
  const s_float_t xdelta = right - left;
  const s_float_t ydelta = top - bottom;
  const s_float_t zdelta = far - near;
  const s_float_t neardouble = 2.0 * near;

  out[0] = neardouble / xdelta;
  out[2] = (right + left) / xdelta;
  out[5] = neardouble / ydelta;
  out[6] = (top + bottom) / ydelta;
  out[10] = -((far + near) / zdelta);
  out[11] = -1.0;
  out[14] = -((neardouble * far) / zdelta);
  out[1] = out[3] = out[4] =
  out[7] = out[8] = out[9] =
  out[12] = out[13] = out[15] = 0.0;
}

void mat4_orthographic(s_float_t left, s_float_t right, s_float_t bottom, s_float_t top, s_float_t near, s_float_t far, mat4_t out)
{
  const s_float_t xdelta = right - left;
  const s_float_t ydelta = top - bottom;
  const s_float_t zdelta = far - near;

  out[0] = 2.0 / xdelta;
  out[5] = 2.0 / ydelta;
  out[10] = -2.0 / zdelta;
  out[12] = -((right + left) / xdelta);
  out[13] = -((top + bottom) / ydelta);
  out[14] = -((far + near) / zdelta);
  out[15] = 1.0;
  out[1] = out[2] = out[3] =
  out[4] = out[6] = out[7] =
  out[8] = out[9] = out[11] = 0.0;
}

void mat4_perspective(s_float_t fov_y, s_float_t aspect, s_float_t near, s_float_t far, mat4_t out)
{
  const s_float_t r = s_tan(fov_y * 0.5 * S_DEG2RAD);
  const s_float_t left = -r * aspect;
  const s_float_t right = r * aspect;
  const s_float_t bottom = -r;
  const s_float_t top = r;
  const s_float_t two_near = 2 * near;
  const s_float_t zdelta = 1.0 / (near - far);

  out[0] = two_near * (right - left);
  out[5] = two_near / (top - bottom);
  out[10] = (far + near) * zdelta;
  out[11] = -1.0;
  out[14] = (two_near * far) * zdelta;
  out[1] = out[2] = out[3] =
  out[4] = out[6] = out[7] =
  out[8] = out[9] = out[12] =
  out[13] = out[15] = 0.0;
}

void mat4_look_at(const vec3_t eye, const vec3_t center, const vec3_t up, mat4_t out)
{
  mat4_t r;
  vec3_t facing_norm, up_norm, s;
  /* facing_norm */
  vec3_subtract(center, eye, facing_norm);
  vec3_normalize(facing_norm, facing_norm);
  /* up_norm */
  vec3_normalize(up, up_norm);
  /* s */
  vec3_cross_product(facing_norm, up_norm, s);
  vec3_normalize(s, s);
  /* up_norm rejigged */
  vec3_cross_product(s, facing_norm, up_norm);
  up_norm[1] = -up_norm[1];
  facing_norm[0] = -facing_norm[0];
  facing_norm[1] = -facing_norm[1];
  facing_norm[2] = -facing_norm[2];

  mat4_set_axes3(s, up_norm, facing_norm, g_vec3_zero, r);
  mat4_translate(-eye[0], -eye[1], -eye[2], r, out);
}

void mat4_from_quat(const quat_t quat, mat4_t out)
{
  s_float_t tx, ty, tz, xx, xy, xz, yy, yz, zz, wx, wy, wz;

  tx = 2.0 * quat[0];
  ty = 2.0 * quat[1];
  tz = 2.0 * quat[2];

  xx = tx * quat[0];
  xy = tx * quat[1];
  xz = tx * quat[2];

  yy = ty * quat[1];
  yz = tz * quat[1];

  zz = tz * quat[3];

  wx = tx * quat[3];
  wy = ty * quat[3];
  wz = tz * quat[3];

  out[0 ] = 1.0 - (yy + zz);
  out[1 ] = xy - wz;
  out[2 ] = xz + wy;
  out[4 ] = xy + wz;
  out[5 ] = 1.0 - (xx + zz);
  out[6 ] = yz - wx;
  out[8 ] = xz - wy;
  out[9 ] = yz + wx;
  out[10] = 1.0 - (xx + yy);

  out[7 ] = 0.0;
  out[3 ] = 0.0;
  out[11] = 0.0;
  out[12] = 0.0;
  out[13] = 0.0;
  out[14] = 0.0;

  out[15] = 1.0;
}

void mat4_get_row4(const mat4_t in, int row, vec4_t out)
{
  if (0 <= row && row < 4) {
    const s_float_t *rowvec = in + (row * 4);
    out[0] = rowvec[0];
    out[1] = rowvec[1];
    out[2] = rowvec[2];
    out[3] = rowvec[3];
  }
}

void mat4_get_row3(const mat4_t in, int row, vec3_t out)
{
  if (0 <= row && row < 4) {
    const s_float_t *rowvec = in + (row * 4);
    out[0] = rowvec[0];
    out[1] = rowvec[1];
    out[2] = rowvec[2];
  }
}

void mat4_get_column4(const mat4_t in, int column, vec4_t out)
{
  if (0 <= column && column < 4) {
    const s_float_t *colvec = in + column;
    out[0] = colvec[0];
    out[1] = colvec[4];
    out[2] = colvec[8];
    out[3] = colvec[12];
  }
}

void mat4_get_column3(const mat4_t in, int column, vec3_t out)
{
  if (0 <= column && column < 4) {
    const s_float_t *colvec = in + column;
    out[0] = colvec[0];
    out[1] = colvec[4];
    out[2] = colvec[8];
  }
}

void mat4_set_row4(int row, const vec4_t value, mat4_t inout)
{
  if (0 <= row && row < 4) {
    s_float_t *rowvec = inout + (row * 4);
    rowvec[0] = value[0];
    rowvec[1] = value[1];
    rowvec[2] = value[2];
    rowvec[3] = value[3];
  }
}

void mat4_set_row3(int row, const vec3_t value, mat4_t inout)
{
  if (0 <= row && row < 4) {
    s_float_t *rowvec = inout + (row * 4);
    rowvec[0] = value[0];
    rowvec[1] = value[1];
    rowvec[2] = value[2];
  }
}

void mat4_set_column4(int column, const vec4_t value, mat4_t inout)
{
  if (0 <= column && column < 4) {
    s_float_t *colvec = inout + column;
    colvec[0] = value[0];
    colvec[4] = value[1];
    colvec[8] = value[2];
    colvec[12] = value[3];
  }
}

void mat4_set_column3(int column, const vec3_t value, mat4_t inout)
{
  if (0 <= column && column < 4) {
    s_float_t *colvec = inout + column;
    colvec[0] = value[0];
    colvec[4] = value[1];
    colvec[8] = value[2];
  }
}

int mat4_equals(const mat4_t left, const mat4_t right)
{
  /*
  compare the XYZ components of all axes first, since they are the most likely
  to vary between checks
  */
  return (
    float_equals(left[0],  right[0])  &&
    float_equals(left[1],  right[1])  &&
    float_equals(left[2],  right[2])  &&

    float_equals(left[4],  right[4])  &&
    float_equals(left[5],  right[5])  &&
    float_equals(left[6],  right[6])  &&

    float_equals(left[8],  right[8])  &&
    float_equals(left[9],  right[9])  &&
    float_equals(left[10], right[10]) &&

    float_equals(left[12], right[12]) &&
    float_equals(left[13], right[13]) &&
    float_equals(left[14], right[14]) &&

    float_equals(left[3],  right[3])  &&
    float_equals(left[7],  right[7])  &&
    float_equals(left[11], right[11]) &&
    float_equals(left[15], right[15])
    );
}

void mat4_transpose(const mat4_t in, mat4_t out)
{
  s_float_t temp;
  #define swap_elem(X, Y) temp = in[(X)]; out[(X)] = in[(Y)]; out[(Y)] = temp
  swap_elem(1, 4);
  swap_elem(8, 2);
  swap_elem(9, 6);
  swap_elem(12, 3);
  swap_elem(13, 7);
  swap_elem(14, 11);
  out[0] = in[0];
  out[5] = in[5];
  out[10] = in[10];
  out[15] = in[15];
}

void mat4_inverse_orthogonal(const mat4_t in, mat4_t out)
{
  const s_float_t m12 = in[12];
  const s_float_t m13 = in[13];
  const s_float_t m14 = in[14];
  mat4_t temp = {
    in[0],
    in[4],
    in[8],
    0.0,
    in[1],
    in[5],
    in[9],
    0.0,
    in[2],
    in[6],
    in[10],
    0.0
  };

  temp[12] = -((m12 * temp[0]) + (m13 * temp[4]) + (m14 * temp[8]));
  temp[13] = -((m12 * temp[1]) + (m13 * temp[5]) + (m14 * temp[9]));
  temp[14] = -((m12 * temp[2]) + (m13 * temp[6]) + (m14 * temp[10]));
  temp[15] = 1.0;

  mat4_copy(temp, out);
}

/*!
 * Writes the inverse affine of the input matrix to the output matrix.
 * \returns Non-zero if an inverse affine matrix can be created, otherwise
 * zero if not.  If zero, the output matrix is the identity matrix.
 */
int mat4_inverse_affine(const mat4_t in, mat4_t out)
{
  mat4_t temp;
  s_float_t det;
  s_float_t m12, m13, m14;

  temp[0 ] = (in[5 ] * in[10]) - (in[6 ] * in[9 ]);
  temp[1 ] = (in[2 ] * in[9 ]) - (in[1 ] * in[10]);
  temp[2 ] = (in[1 ] * in[6 ]) - (in[2 ] * in[5 ]);

  temp[4 ] = (in[6 ] * in[ 8]) - (in[4 ] * in[10]);
  temp[5 ] = (in[0 ] * in[10]) - (in[2 ] * in[ 8]);
  temp[6 ] = (in[2 ] * in[ 4]) - (in[0 ] * in[ 6]);

  temp[8 ] = (in[4 ] * in[ 9]) - (in[5 ] * in[ 8]);
  temp[9 ] = (in[1 ] * in[ 8]) - (in[0 ] * in[ 9]);
  temp[10] = (in[0 ] * in[ 5]) - (in[1 ] * in[ 4]);

  det = (in[0] * temp[0]) + (in[1] * temp[4]) + (in[2] * temp[8]);
  if (s_fabs(det) < S_FLOAT_EPSILON) {
    mat4_identity(out);
    return 0;
  }

  det = 1.0 / det;

  out[0] = temp[0] * det;
  out[1] = temp[1] * det;
  out[2] = temp[2] * det;
  out[4] = temp[4] * det;
  out[5] = temp[5] * det;
  out[6] = temp[6] * det;
  out[8 ] = temp[8 ] * det;
  out[9 ] = temp[9 ] * det;
  out[10] = temp[10] * det;

  m12 = in[12];
  m13 = in[13];
  m14 = in[14];

  out[12] = -((m12 * temp[0]) + (m13 * temp[4]) + (m14 * temp[8 ]));
  out[13] = -((m12 * temp[1]) + (m13 * temp[5]) + (m14 * temp[9 ]));
  out[14] = -((m12 * temp[2]) + (m13 * temp[6]) + (m14 * temp[10]));

  out[3] = out[7] = out[11] = 0.0;
  out[15] = 1.0;

  return 1;
}

static s_float_t mat4_cofactor(const mat4_t m, int r0, int r1, int r2, int c0, int c1, int c2)
{
  #define cofactor_addr(l, r) (m[l*4+r])
  return (
    (cofactor_addr(r0,c0) * ((cofactor_addr(r1, c1) * cofactor_addr(r2, c2)) - (cofactor_addr(r2, c1) * cofactor_addr(r1, c2)))) -
    (cofactor_addr(r0,c1) * ((cofactor_addr(r1, c0) * cofactor_addr(r2, c2)) - (cofactor_addr(r2, c0) * cofactor_addr(r1, c2)))) +
    (cofactor_addr(r0,c2) * ((cofactor_addr(r1, c0) * cofactor_addr(r2, c1)) - (cofactor_addr(r2, c0) * cofactor_addr(r1, c1))))
    );
  #undef cofactor_addr
}

void mat4_adjoint(const mat4_t in, mat4_t out)
{
  if (in == out) {
    mat4_t temp = {
       mat4_cofactor(in, 1, 2, 3, 1, 2, 3),
      -mat4_cofactor(in, 0, 2, 3, 1, 2, 3),
       mat4_cofactor(in, 0, 1, 3, 1, 2, 3),
      -mat4_cofactor(in, 0, 1, 2, 1, 2, 3),

      -mat4_cofactor(in, 1, 2, 3, 0, 2, 3),
       mat4_cofactor(in, 0, 2, 3, 0, 2, 3),
      -mat4_cofactor(in, 0, 1, 3, 0, 2, 3),
       mat4_cofactor(in, 0, 1, 2, 0, 2, 3),

       mat4_cofactor(in, 1, 2, 3, 0, 1, 3),
      -mat4_cofactor(in, 0, 2, 3, 0, 1, 3),
       mat4_cofactor(in, 0, 1, 3, 0, 1, 3),
      -mat4_cofactor(in, 0, 1, 2, 0, 1, 3),

      -mat4_cofactor(in, 1, 2, 3, 0, 1, 2),
       mat4_cofactor(in, 0, 2, 3, 0, 1, 2),
      -mat4_cofactor(in, 0, 1, 3, 0, 1, 2),
       mat4_cofactor(in, 0, 1, 2, 0, 1, 2)
    };
    mat4_copy(temp, out);
  } else {
    out[0 ] =  mat4_cofactor(in, 1, 2, 3, 1, 2, 3);
    out[1 ] = -mat4_cofactor(in, 0, 2, 3, 1, 2, 3);
    out[2 ] =  mat4_cofactor(in, 0, 1, 3, 1, 2, 3);
    out[3 ] = -mat4_cofactor(in, 0, 1, 2, 1, 2, 3);

    out[4 ] = -mat4_cofactor(in, 1, 2, 3, 0, 2, 3);
    out[5 ] =  mat4_cofactor(in, 0, 2, 3, 0, 2, 3);
    out[6 ] = -mat4_cofactor(in, 0, 1, 3, 0, 2, 3);
    out[7 ] =  mat4_cofactor(in, 0, 1, 2, 0, 2, 3);

    out[8 ] =  mat4_cofactor(in, 1, 2, 3, 0, 1, 3);
    out[9 ] = -mat4_cofactor(in, 0, 2, 3, 0, 1, 3);
    out[10] =  mat4_cofactor(in, 0, 1, 3, 0, 1, 3);
    out[11] = -mat4_cofactor(in, 0, 1, 2, 0, 1, 3);

    out[12] = -mat4_cofactor(in, 1, 2, 3, 0, 1, 2);
    out[13] =  mat4_cofactor(in, 0, 2, 3, 0, 1, 2);
    out[14] = -mat4_cofactor(in, 0, 1, 3, 0, 1, 2);
    out[15] =  mat4_cofactor(in, 0, 1, 2, 0, 1, 2);
  }
}

s_float_t mat4_determinant(const mat4_t m)
{
  return (
    (m[0] * mat4_cofactor(m, 1, 2, 3, 1, 2, 3)) -
    (m[1] * mat4_cofactor(m, 1, 2, 3, 0, 2, 3)) +
    (m[2] * mat4_cofactor(m, 1, 2, 3, 0, 1, 3)) -
    (m[3] * mat4_cofactor(m, 1, 2, 3, 0, 1, 2))
    );
}

int mat4_inverse_general(const mat4_t in, mat4_t out)
{
  int index;
  s_float_t det = mat4_determinant(in);

  if (s_fabs(det) < S_FLOAT_EPSILON) {
    return 0;
  }

  mat4_adjoint(in, out);
  det = 1.0 / det;
  for (index = 0; index < 16; ++index) {
    out[index] *= det;
  }

  return 1;
}

/*! Translates the given matrix by <X, Y, Z>. */
void mat4_translate(s_float_t x, s_float_t y, s_float_t z, const mat4_t in, mat4_t out)
{
  s_float_t m12 = in[12] + ((x * in[0]) + (y * in[4]) + (z * in[8]));
  s_float_t m13 = in[13] + ((x * in[1]) + (y * in[5]) + (z * in[9]));
  s_float_t m14 = in[14] + ((x * in[2]) + (y * in[6]) + (z * in[10]));
  s_float_t m15 = in[15] + ((x * in[3]) + (y * in[7]) + (z * in[11]));

  if (in != out)
    mat4_copy(in, out);

  out[12] = m12;
  out[13] = m13;
  out[14] = m14;
  out[15] = m15;
}

void mat4_translation(s_float_t x, s_float_t y, s_float_t z, mat4_t out) {
  s_float_t m12 = x;
  s_float_t m13 = y;
  s_float_t m14 = z;
  s_float_t m15 = 1;

  mat4_copy(g_mat4_identity, out);

  out[12] = m12;
  out[13] = m13;
  out[14] = m14;
  out[15] = m15;
}

void mat4_multiply(const mat4_t left, const mat4_t right, mat4_t out)
{
  mat4_t temp;
  int index;

  // Transpose the left matrix so we can be a little more cache-friendly.
  mat4_transpose(left, temp);

  for (index = 0; index < 4; ++index) {
    const int inner_index = index * 4;
    s_float_t cx, cy, cz, cw;

    cx = temp[inner_index];
    cy = temp[inner_index + 1];
    cz = temp[inner_index + 2];
    cw = temp[inner_index + 3];

    temp[inner_index]     = (cx * right[0 ]) + (cy * right[1 ]) + (cz * right[2 ]) + (cw * right[3 ]);
    temp[inner_index + 1] = (cx * right[4 ]) + (cy * right[5 ]) + (cz * right[6 ]) + (cw * right[7 ]);
    temp[inner_index + 2] = (cx * right[8 ]) + (cy * right[9 ]) + (cz * right[10]) + (cw * right[11]);
    temp[inner_index + 3] = (cx * right[12]) + (cy * right[13]) + (cz * right[14]) + (cw * right[15]);
  }

  mat4_transpose(temp, out);
}

void mat4_multiply_vec4(const mat4_t left, const vec4_t right, vec4_t out)
{
  const s_float_t x = right[0], y = right[1], z = right[2], w = right[3];
  out[0] = (x * left[0]) + (y * left[4]) + (z * left[8 ]) + (w * left[12]);
  out[1] = (x * left[1]) + (y * left[5]) + (z * left[9 ]) + (w * left[13]);
  out[2] = (x * left[2]) + (y * left[6]) + (z * left[10]) + (w * left[14]);
  out[3] = (x * left[3]) + (y * left[7]) + (z * left[11]) + (w * left[15]);
}

void mat4_transform_vec3(const mat4_t left, const vec3_t right, vec3_t out)
{
  const s_float_t x = right[0], y = right[1], z = right[2];
  out[0] = (x * left[0]) + (y * left[4]) + (z * left[8 ]) + left[12];
  out[1] = (x * left[1]) + (y * left[5]) + (z * left[9 ]) + left[13];
  out[2] = (x * left[2]) + (y * left[6]) + (z * left[10]) + left[14];
}

void mat4_rotate_vec3(const mat4_t left, const vec3_t right, vec3_t out)
{
  const s_float_t x = right[0], y = right[1], z = right[2];
  out[0] = (x * left[0]) + (y * left[4]) + (z * left[8 ]);
  out[1] = (x * left[1]) + (y * left[5]) + (z * left[9 ]);
  out[2] = (x * left[2]) + (y * left[6]) + (z * left[10]);
}

void mat4_inv_rotate_vec3(const mat4_t left, const vec3_t right, vec3_t out)
{
  const s_float_t x = right[0], y = right[1], z = right[2];
  out[0] = (x * left[0]) + (y * left[1]) + (z * left[2 ]);
  out[1] = (x * left[4]) + (y * left[5]) + (z * left[6 ]);
  out[2] = (x * left[8]) + (y * left[9]) + (z * left[10]);
}

void mat4_scale(const mat4_t in, s_float_t x, s_float_t y, s_float_t z, mat4_t out)
{
  mat4_copy(in, out);
  out[0] *= x;
  out[4] *= x;
  out[8] *= x;

  out[1] *= y;
  out[5] *= y;
  out[9] *= y;

  out[2 ] *= z;
  out[6 ] *= z;
  out[10] *= z;
}

#if defined(__cplusplus)
}
#endif /* __cplusplus */

