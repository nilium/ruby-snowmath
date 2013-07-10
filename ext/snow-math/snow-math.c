/*
Maths bindings for Ruby
Written by Noel Cower

See COPYING for license information
*/

#include "maths_local.h"
#include "ruby.h"

#define kSM_WANT_THREE_OR_FOUR_FORMAT_LIT ("Expected a Vec3, Vec4, or Quat, got %s")
#define kSM_WANT_FOUR_FORMAT_LIT ("Expected a Vec4 or Quat, got %s")

/*
  Generates a label within the current function with the given name. Should be
  as unique as anyone needs.
*/
#define SM_LABEL(NAME) __FUNCTION__##NAME

/*
  Returns the Ruby class value's identifier
*/
#define SM_KLASS(TYPE)              s_sm_##TYPE##_klass

/*
  Returns whether a given ruby value is a kind of RB_TYPE (a Ruby value for a
  class).
*/
#define SM_RB_IS_A(RB_VALUE, RB_TYPE) (RTEST(rb_obj_is_kind_of((RB_VALUE), (RB_TYPE))))

/*
  Wrapper around SM_RB_IS_A that checks for SM_RB_IS_A(value, SM_KLASS(type)).
  Vaguely convenient.
*/
#define SM_IS_A(SM_VALUE, SM_TYPE)  SM_RB_IS_A(SM_VALUE, SM_KLASS(SM_TYPE))

/*
  Convenience macro to raise an exception if a value isn't of a given type. Only
  useful for a few things, as vec3, vec4, and quat are semi-compatible due to
  having similar sizes. So any vec4/quat can be used as a vec3 in some cases and
  any quat can be used in place of a vec4 except in the case of a few binary ops
  and such (because the binary op macro doesn't check for much -- so that might
  need rewriting later).
*/
#define SM_RAISE_IF_NOT_TYPE(SM_VALUE, SM_TYPE) do {                                              \
  if (!SM_IS_A(SM_VALUE, SM_TYPE)) {                                                              \
    rb_raise(rb_eTypeError, "Expected %s, got %s",                                                \
      rb_class2name(SM_KLASS(SM_TYPE)),                                                           \
      rb_obj_classname((SM_VALUE)));                                                              \
  } } while (0)


/*
  Array types -- optional if BUILD_ARRAY_TYPE isn't defined.

  All array types are defined as something like this in Ruby:

  class TypeArray
    def fetch(index) -> Type
      Returns an  Object of Type that references array data (non-const)

    def store(index, value) -> value
      Copies value object of Type's data to the array's data. This is a nop if
      the value already references the array's data.
  end

  Array implemnetations are defined below.
*/
#ifndef BUILD_ARRAY_TYPE
#define BUILD_ARRAY_TYPE 1
#endif
#if BUILD_ARRAY_TYPE

static ID kRB_IVAR_MATHARRAY_LENGTH;
static ID kRB_IVAR_MATHARRAY_CACHE;
static ID kRB_IVAR_MATHARRAY_SOURCE;

/*
 * Returns the array's length.
 *
 * call-seq: length -> fixnum
 */
static VALUE sm_mathtype_array_length(VALUE sm_self)
{
  return rb_ivar_get(sm_self, kRB_IVAR_MATHARRAY_LENGTH);
}

#endif


/*==============================================================================

  Static Ruby class / module values and function decls

==============================================================================*/

static VALUE s_sm_snowmath_mod = Qnil;
static VALUE s_sm_vec3_klass = Qnil;
static VALUE s_sm_vec4_klass = Qnil;
static VALUE s_sm_quat_klass = Qnil;
static VALUE s_sm_mat3_klass = Qnil;
static VALUE s_sm_mat4_klass = Qnil;


/*
  Note about wrap / unwrap functions:
  Neither function actually cares about its input. It will not verify that a
  quat is passed to sm_unwrap_quat and you can similarly pass a vec4 to
  sm_wrap_vec3 or sm_wrap_quat, so the types are mostly interchangeable provided
  they're of the same size or larger.

  This allows some flexibility when passing Quats to Vec4 functions and so on,
  though it's a better idea to simply create a new Vec4 or Quat when you need to
  pass one to another's function. The conversion is easy enough, so it's not a
  huge deal.
*/
static VALUE    sm_wrap_vec3(const vec3_t value, VALUE klass);
static vec3_t * sm_unwrap_vec3(VALUE sm_value, vec3_t store);
static VALUE    sm_wrap_vec4(const vec4_t value, VALUE klass);
static vec4_t * sm_unwrap_vec4(VALUE sm_value, vec4_t store);
static VALUE    sm_wrap_quat(const quat_t value, VALUE klass);
static quat_t * sm_unwrap_quat(VALUE sm_value, quat_t store);
static VALUE    sm_wrap_mat3(const mat3_t value, VALUE klass);
static mat3_t * sm_unwrap_mat3(VALUE sm_value, mat3_t store);
static VALUE    sm_wrap_mat4(const mat4_t value, VALUE klass);
static mat4_t * sm_unwrap_mat4(VALUE sm_value, mat4_t store);



/*==============================================================================

  Array types

==============================================================================*/

#if BUILD_ARRAY_TYPE

/*==============================================================================

  Snow::Vec3Array methods (s_sm_vec3_array_klass)

==============================================================================*/

static VALUE s_sm_vec3_array_klass = Qnil;

/*
 * In the first form, a new typed array of Vec3 elements is allocated and
 * returned. In the second form, a copy of a typed array of Vec3 objects is
 * made and returned. Copied arrays do not share data.
 *
 * call-seq:
 *    new(size)       -> new vec3_array
 *    new(vec3_array) -> copy of vec3_array
 */
static VALUE sm_vec3_array_new(VALUE sm_self, VALUE sm_length_or_copy)
{
  size_t length = 0;
  vec3_t *arr;
  VALUE sm_type_array;
  int copy_array = 0;
  if ((copy_array = SM_IS_A(sm_length_or_copy, vec3_array))) {
    length = NUM2SIZET(sm_mathtype_array_length(sm_length_or_copy));
  } else {
    length = NUM2SIZET(sm_length_or_copy);
  }
  if (length <= 0) {
    return Qnil;
  }
  arr = ALLOC_N(vec3_t, length);
  if (copy_array) {
    const vec3_t *source;
    Data_Get_Struct(sm_length_or_copy, vec3_t, source);
    MEMCPY(arr, source, vec3_t, length);
    sm_length_or_copy = sm_mathtype_array_length(sm_length_or_copy);
    sm_self = rb_obj_class(sm_length_or_copy);
  }
  sm_type_array = Data_Wrap_Struct(sm_self, 0, free, arr);
  rb_ivar_set(sm_type_array, kRB_IVAR_MATHARRAY_LENGTH, sm_length_or_copy);
  rb_ivar_set(sm_type_array, kRB_IVAR_MATHARRAY_CACHE, rb_ary_new2((long)length));
  rb_obj_call_init(sm_type_array, 0, 0);
  return sm_type_array;
}



/*
 * Resizes the array to new_length and returns self.
 *
 * If resizing to a length smaller than the previous length, excess array
 * elements are discarded and the array is truncated. Otherwise, when resizing
 * the array to a greater length than previous, new elements in the array will
 * contain garbage values.
 *
 * If new_length is equal to self.length, the call does nothing to the array.
 *
 * Attempting to resize an array to a new length of zero or less will raise a
 * RangeError. Do not try to resize arrays to zero or less. Do not be that
 * person.
 *
 * call-seq:
 *    resize!(new_length) -> self
 */
static VALUE sm_vec3_array_resize(VALUE sm_self, VALUE sm_new_length)
{
  size_t new_length;
  size_t old_length;

  old_length = NUM2SIZET(sm_mathtype_array_length(sm_self));
  new_length = NUM2SIZET(sm_new_length);

  if (old_length == new_length) {
    /* No change, done */
    return sm_self;
  } else if (new_length < 1) {
    /* Someone decided to be that person. */
    rb_raise(rb_eRangeError,
      "Cannot resize array to length less than or equal to 0.");
    return sm_self;
  }

  REALLOC_N(RDATA(sm_self)->data, vec3_t, new_length);
  rb_ivar_set(sm_self, kRB_IVAR_MATHARRAY_LENGTH, sm_new_length);
  rb_ary_clear(rb_ivar_get(sm_self, kRB_IVAR_MATHARRAY_CACHE));

  return sm_self;
}



/*
 * Fetches a Vec3 from the array at the index and returns it. The returned Vec3
 * may be a cached object. In all cases, values returned from a typed array are
 * associated with the memory of the array and not given their own memory. So,
 * modifying a Vec3 fetched from an array modifies the array's data.
 *
 * As a result, objects returned by a Vec3Array should not be considered
 * thread-safe, nor should manipulating a Vec3Array be considered thread-safe
 * either. If you want to work with data returned from an array without altering
 * the array data, you should call Vec3#dup or Vec3#copy to get a new Vec3 with a
 * copy of the array object's data.
 *
 * call-seq: fetch(index) -> vec3
 */
static VALUE sm_vec3_array_fetch(VALUE sm_self, VALUE sm_index)
{
  vec3_t *arr;
  size_t length = NUM2SIZET(sm_mathtype_array_length(sm_self));
  size_t index = NUM2SIZET(sm_index);
  VALUE sm_inner;
  VALUE sm_cache;
  if (index >= length) {
    rb_raise(rb_eRangeError,
      "Index %zu out of bounds for array with length %zu",
      index, length);
  }

  sm_cache = rb_ivar_get(sm_self, kRB_IVAR_MATHARRAY_CACHE);
  if (!RTEST(sm_cache)) {
    rb_raise(rb_eRuntimeError, "No cache available");
  }
  sm_inner = rb_ary_entry(sm_cache, (long)index);

  if (!RTEST(sm_inner)) {
    /* No cached value, create one. */
    Data_Get_Struct(sm_self, vec3_t, arr);
    sm_inner = Data_Wrap_Struct(s_sm_vec3_klass, 0, 0, arr[index]);
    rb_ivar_set(sm_inner, kRB_IVAR_MATHARRAY_SOURCE, sm_self);
    /* Store the Vec3 in the cache */
    rb_ary_store(sm_cache, (long)index, sm_inner);
  }

  return sm_inner;
}



/*
 * Stores a Vec3 at the given index. If the provided Vec3 is a member of the
 * array and stored at the index, then no copy is done, otherwise the Vec3 is
 * copied to the array.
 *
 * call-seq: store(index, value) -> value
 */
static VALUE sm_vec3_array_store(VALUE sm_self, VALUE sm_index, VALUE sm_value)
{
  vec3_t *arr;
  vec3_t *value;
  size_t length = NUM2SIZET(sm_mathtype_array_length(sm_self));
  size_t index = NUM2SIZET(sm_index);

  if (index >= length) {
    rb_raise(rb_eRangeError,
      "Index %zu out of bounds for array with length %zu",
      index, length);
  } else if (!SM_IS_A(sm_value, vec3) && !SM_IS_A(sm_value, vec4) && !SM_IS_A(sm_value, quat)) {
    rb_raise(rb_eTypeError,
      "Invalid value to store: expected Vec3, Vec4, or Quat, got %s",
      rb_obj_classname(sm_value));
  }

  Data_Get_Struct(sm_self, vec3_t, arr);
  value = sm_unwrap_vec3(sm_value, NULL);

  if (value == &arr[index]) {
    /* The object's part of the array, don't bother copying */
    return sm_value;
  }

  vec3_copy(*value, arr[index]);
  return sm_value;
}



/*
 * Returns the length of the array.
 *
 * call-seq: length -> fixnum
 */
static VALUE sm_vec3_array_size(VALUE sm_self)
{
  size_t length = NUM2SIZET(sm_mathtype_array_length(sm_self));
  return SIZET2NUM(length * sizeof(vec3_t));
}



/*==============================================================================

  Snow::Vec4Array methods (s_sm_vec4_array_klass)

==============================================================================*/

static VALUE s_sm_vec4_array_klass = Qnil;

/*
 * In the first form, a new typed array of Vec4 elements is allocated and
 * returned. In the second form, a copy of a typed array of Vec4 objects is
 * made and returned. Copied arrays do not share data.
 *
 * call-seq:
 *    new(size)       -> new vec4_array
 *    new(vec4_array) -> copy of vec4_array
 */
static VALUE sm_vec4_array_new(VALUE sm_self, VALUE sm_length_or_copy)
{
  size_t length = 0;
  vec4_t *arr;
  VALUE sm_type_array;
  int copy_array = 0;
  if ((copy_array = SM_IS_A(sm_length_or_copy, vec4_array))) {
    length = NUM2SIZET(sm_mathtype_array_length(sm_length_or_copy));
  } else {
    length = NUM2SIZET(sm_length_or_copy);
  }
  if (length <= 0) {
    return Qnil;
  }
  arr = ALLOC_N(vec4_t, length);
  if (copy_array) {
    const vec4_t *source;
    Data_Get_Struct(sm_length_or_copy, vec4_t, source);
    MEMCPY(arr, source, vec4_t, length);
    sm_length_or_copy = sm_mathtype_array_length(sm_length_or_copy);
    sm_self = rb_obj_class(sm_length_or_copy);
  }
  sm_type_array = Data_Wrap_Struct(sm_self, 0, free, arr);
  rb_ivar_set(sm_type_array, kRB_IVAR_MATHARRAY_LENGTH, sm_length_or_copy);
  rb_ivar_set(sm_type_array, kRB_IVAR_MATHARRAY_CACHE, rb_ary_new2((long)length));
  rb_obj_call_init(sm_type_array, 0, 0);
  return sm_type_array;
}



/*
 * Resizes the array to new_length and returns self.
 *
 * If resizing to a length smaller than the previous length, excess array
 * elements are discarded and the array is truncated. Otherwise, when resizing
 * the array to a greater length than previous, new elements in the array will
 * contain garbage values.
 *
 * If new_length is equal to self.length, the call does nothing to the array.
 *
 * Attempting to resize an array to a new length of zero or less will raise a
 * RangeError. Do not try to resize arrays to zero or less. Do not be that
 * person.
 *
 * call-seq:
 *    resize!(new_length) -> self
 */
static VALUE sm_vec4_array_resize(VALUE sm_self, VALUE sm_new_length)
{
  size_t new_length;
  size_t old_length;

  old_length = NUM2SIZET(sm_mathtype_array_length(sm_self));
  new_length = NUM2SIZET(sm_new_length);

  if (old_length == new_length) {
    /* No change, done */
    return sm_self;
  } else if (new_length < 1) {
    /* Someone decided to be that person. */
    rb_raise(rb_eRangeError,
      "Cannot resize array to length less than or equal to 0.");
    return sm_self;
  }

  REALLOC_N(RDATA(sm_self)->data, vec4_t, new_length);
  rb_ivar_set(sm_self, kRB_IVAR_MATHARRAY_LENGTH, sm_new_length);
  rb_ary_clear(rb_ivar_get(sm_self, kRB_IVAR_MATHARRAY_CACHE));

  return sm_self;
}



/*
 * Fetches a Vec4 from the array at the index and returns it. The returned Vec4
 * may be a cached object. In all cases, values returned from a typed array are
 * associated with the memory of the array and not given their own memory. So,
 * modifying a Vec4 fetched from an array modifies the array's data.
 *
 * As a result, objects returned by a Vec4Array should not be considered
 * thread-safe, nor should manipulating a Vec4Array be considered thread-safe
 * either. If you want to work with data returned from an array without altering
 * the array data, you should call Vec4#dup or Vec4#copy to get a new Vec4 with a
 * copy of the array object's data.
 *
 * call-seq: fetch(index) -> vec4
 */
static VALUE sm_vec4_array_fetch(VALUE sm_self, VALUE sm_index)
{
  vec4_t *arr;
  size_t length = NUM2SIZET(sm_mathtype_array_length(sm_self));
  size_t index = NUM2SIZET(sm_index);
  VALUE sm_inner;
  VALUE sm_cache;
  if (index >= length) {
    rb_raise(rb_eRangeError,
      "Index %zu out of bounds for array with length %zu",
      index, length);
  }

  sm_cache = rb_ivar_get(sm_self, kRB_IVAR_MATHARRAY_CACHE);
  if (!RTEST(sm_cache)) {
    rb_raise(rb_eRuntimeError, "No cache available");
  }
  sm_inner = rb_ary_entry(sm_cache, (long)index);

  if (!RTEST(sm_inner)) {
    /* No cached value, create one. */
    Data_Get_Struct(sm_self, vec4_t, arr);
    sm_inner = Data_Wrap_Struct(s_sm_vec4_klass, 0, 0, arr[index]);
    rb_ivar_set(sm_inner, kRB_IVAR_MATHARRAY_SOURCE, sm_self);
    /* Store the Vec4 in the cache */
    rb_ary_store(sm_cache, (long)index, sm_inner);
  }

  return sm_inner;
}



/*
 * Stores a Vec4 at the given index. If the provided Vec4 is a member of the
 * array and stored at the index, then no copy is done, otherwise the Vec4 is
 * copied to the array.
 *
 * call-seq: store(index, value) -> value
 */
static VALUE sm_vec4_array_store(VALUE sm_self, VALUE sm_index, VALUE sm_value)
{
  vec4_t *arr;
  vec4_t *value;
  size_t length = NUM2SIZET(sm_mathtype_array_length(sm_self));
  size_t index = NUM2SIZET(sm_index);

  if (index >= length) {
    rb_raise(rb_eRangeError,
      "Index %zu out of bounds for array with length %zu",
      index, length);
  } else if (!SM_IS_A(sm_value, vec4) || !(SM_IS_A(sm_value, quat))) {
    rb_raise(rb_eTypeError,
      "Invalid value to store: expected Quat or Vec4, got %s",
      rb_obj_classname(sm_value));
  }

  Data_Get_Struct(sm_self, vec4_t, arr);
  value = sm_unwrap_vec4(sm_value, NULL);

  if (value == &arr[index]) {
    /* The object's part of the array, don't bother copying */
    return sm_value;
  }

  vec4_copy(*value, arr[index]);
  return sm_value;
}



/*
 * Returns the length of the array.
 *
 * call-seq: length -> fixnum
 */
static VALUE sm_vec4_array_size(VALUE sm_self)
{
  size_t length = NUM2SIZET(sm_mathtype_array_length(sm_self));
  return SIZET2NUM(length * sizeof(vec4_t));
}



/*==============================================================================

  Snow::QuatArray methods (s_sm_quat_array_klass)

==============================================================================*/

static VALUE s_sm_quat_array_klass = Qnil;

/*
 * In the first form, a new typed array of Quat elements is allocated and
 * returned. In the second form, a copy of a typed array of Quat objects is
 * made and returned. Copied arrays do not share data.
 *
 * call-seq:
 *    new(size)       -> new quat_array
 *    new(quat_array) -> copy of quat_array
 */
static VALUE sm_quat_array_new(VALUE sm_self, VALUE sm_length_or_copy)
{
  size_t length = 0;
  quat_t *arr;
  VALUE sm_type_array;
  int copy_array = 0;
  if ((copy_array = SM_IS_A(sm_length_or_copy, quat_array))) {
    length = NUM2SIZET(sm_mathtype_array_length(sm_length_or_copy));
  } else {
    length = NUM2SIZET(sm_length_or_copy);
  }
  if (length <= 0) {
    return Qnil;
  }
  arr = ALLOC_N(quat_t, length);
  if (copy_array) {
    const quat_t *source;
    Data_Get_Struct(sm_length_or_copy, quat_t, source);
    MEMCPY(arr, source, quat_t, length);
    sm_length_or_copy = sm_mathtype_array_length(sm_length_or_copy);
    sm_self = rb_obj_class(sm_length_or_copy);
  }
  sm_type_array = Data_Wrap_Struct(sm_self, 0, free, arr);
  rb_ivar_set(sm_type_array, kRB_IVAR_MATHARRAY_LENGTH, sm_length_or_copy);
  rb_ivar_set(sm_type_array, kRB_IVAR_MATHARRAY_CACHE, rb_ary_new2((long)length));
  rb_obj_call_init(sm_type_array, 0, 0);
  return sm_type_array;
}



/*
 * Resizes the array to new_length and returns self.
 *
 * If resizing to a length smaller than the previous length, excess array
 * elements are discarded and the array is truncated. Otherwise, when resizing
 * the array to a greater length than previous, new elements in the array will
 * contain garbage values.
 *
 * If new_length is equal to self.length, the call does nothing to the array.
 *
 * Attempting to resize an array to a new length of zero or less will raise a
 * RangeError. Do not try to resize arrays to zero or less. Do not be that
 * person.
 *
 * call-seq:
 *    resize!(new_length) -> self
 */
static VALUE sm_quat_array_resize(VALUE sm_self, VALUE sm_new_length)
{
  size_t new_length;
  size_t old_length;

  old_length = NUM2SIZET(sm_mathtype_array_length(sm_self));
  new_length = NUM2SIZET(sm_new_length);

  if (old_length == new_length) {
    /* No change, done */
    return sm_self;
  } else if (new_length < 1) {
    /* Someone decided to be that person. */
    rb_raise(rb_eRangeError,
      "Cannot resize array to length less than or equal to 0.");
    return sm_self;
  }

  REALLOC_N(RDATA(sm_self)->data, quat_t, new_length);
  rb_ivar_set(sm_self, kRB_IVAR_MATHARRAY_LENGTH, sm_new_length);
  rb_ary_clear(rb_ivar_get(sm_self, kRB_IVAR_MATHARRAY_CACHE));

  return sm_self;
}



/*
 * Fetches a Quat from the array at the index and returns it. The returned Quat
 * may be a cached object. In all cases, values returned from a typed array are
 * associated with the memory of the array and not given their own memory. So,
 * modifying a Quat fetched from an array modifies the array's data.
 *
 * As a result, objects returned by a QuatArray should not be considered
 * thread-safe, nor should manipulating a QuatArray be considered thread-safe
 * either. If you want to work with data returned from an array without altering
 * the array data, you should call Quat#dup or Quat#copy to get a new Quat with a
 * copy of the array object's data.
 *
 * call-seq: fetch(index) -> quat
 */
static VALUE sm_quat_array_fetch(VALUE sm_self, VALUE sm_index)
{
  quat_t *arr;
  size_t length = NUM2SIZET(sm_mathtype_array_length(sm_self));
  size_t index = NUM2SIZET(sm_index);
  VALUE sm_inner;
  VALUE sm_cache;
  if (index >= length) {
    rb_raise(rb_eRangeError,
      "Index %zu out of bounds for array with length %zu",
      index, length);
  }

  sm_cache = rb_ivar_get(sm_self, kRB_IVAR_MATHARRAY_CACHE);
  if (!RTEST(sm_cache)) {
    rb_raise(rb_eRuntimeError, "No cache available");
  }
  sm_inner = rb_ary_entry(sm_cache, (long)index);

  if (!RTEST(sm_inner)) {
    /* No cached value, create one. */
    Data_Get_Struct(sm_self, quat_t, arr);
    sm_inner = Data_Wrap_Struct(s_sm_quat_klass, 0, 0, arr[index]);
    rb_ivar_set(sm_inner, kRB_IVAR_MATHARRAY_SOURCE, sm_self);
    /* Store the Quat in the cache */
    rb_ary_store(sm_cache, (long)index, sm_inner);
  }

  return sm_inner;
}



/*
 * Stores a Quat at the given index. If the provided Quat is a member of the
 * array and stored at the index, then no copy is done, otherwise the Quat is
 * copied to the array.
 *
 * call-seq: store(index, value) -> value
 */
static VALUE sm_quat_array_store(VALUE sm_self, VALUE sm_index, VALUE sm_value)
{
  quat_t *arr;
  quat_t *value;
  size_t length = NUM2SIZET(sm_mathtype_array_length(sm_self));
  size_t index = NUM2SIZET(sm_index);

  if (index >= length) {
    rb_raise(rb_eRangeError,
      "Index %zu out of bounds for array with length %zu",
      index, length);
  } else if (!SM_IS_A(sm_value, vec4) || !(SM_IS_A(sm_value, quat))) {
    rb_raise(rb_eTypeError,
      "Invalid value to store: expected Quat or Vec4, got %s",
      rb_obj_classname(sm_value));
  }

  Data_Get_Struct(sm_self, quat_t, arr);
  value = sm_unwrap_quat(sm_value, NULL);

  if (value == &arr[index]) {
    /* The object's part of the array, don't bother copying */
    return sm_value;
  }

  quat_copy(*value, arr[index]);
  return sm_value;
}



/*
 * Returns the length of the array.
 *
 * call-seq: length -> fixnum
 */
static VALUE sm_quat_array_size(VALUE sm_self)
{
  size_t length = NUM2SIZET(sm_mathtype_array_length(sm_self));
  return SIZET2NUM(length * sizeof(quat_t));
}



/*==============================================================================

  Snow::Mat3Array methods (s_sm_mat3_array_klass)

==============================================================================*/

static VALUE s_sm_mat3_array_klass = Qnil;

/*
 * In the first form, a new typed array of Mat3 elements is allocated and
 * returned. In the second form, a copy of a typed array of Mat3 objects is
 * made and returned. Copied arrays do not share data.
 *
 * call-seq:
 *    new(size)       -> new mat3_array
 *    new(mat3_array) -> copy of mat3_array
 */
static VALUE sm_mat3_array_new(VALUE sm_self, VALUE sm_length_or_copy)
{
  size_t length = 0;
  mat3_t *arr;
  VALUE sm_type_array;
  int copy_array = 0;
  if ((copy_array = SM_IS_A(sm_length_or_copy, mat3_array))) {
    length = NUM2SIZET(sm_mathtype_array_length(sm_length_or_copy));
  } else {
    length = NUM2SIZET(sm_length_or_copy);
  }
  if (length <= 0) {
    return Qnil;
  }
  arr = ALLOC_N(mat3_t, length);
  if (copy_array) {
    const mat3_t *source;
    Data_Get_Struct(sm_length_or_copy, mat3_t, source);
    MEMCPY(arr, source, mat3_t, length);
    sm_length_or_copy = sm_mathtype_array_length(sm_length_or_copy);
    sm_self = rb_obj_class(sm_length_or_copy);
  }
  sm_type_array = Data_Wrap_Struct(sm_self, 0, free, arr);
  rb_ivar_set(sm_type_array, kRB_IVAR_MATHARRAY_LENGTH, sm_length_or_copy);
  rb_ivar_set(sm_type_array, kRB_IVAR_MATHARRAY_CACHE, rb_ary_new2((long)length));
  rb_obj_call_init(sm_type_array, 0, 0);
  return sm_type_array;
}



/*
 * Resizes the array to new_length and returns self.
 *
 * If resizing to a length smaller than the previous length, excess array
 * elements are discarded and the array is truncated. Otherwise, when resizing
 * the array to a greater length than previous, new elements in the array will
 * contain garbage values.
 *
 * If new_length is equal to self.length, the call does nothing to the array.
 *
 * Attempting to resize an array to a new length of zero or less will raise a
 * RangeError. Do not try to resize arrays to zero or less. Do not be that
 * person.
 *
 * call-seq:
 *    resize!(new_length) -> self
 */
static VALUE sm_mat3_array_resize(VALUE sm_self, VALUE sm_new_length)
{
  size_t new_length;
  size_t old_length;

  old_length = NUM2SIZET(sm_mathtype_array_length(sm_self));
  new_length = NUM2SIZET(sm_new_length);

  if (old_length == new_length) {
    /* No change, done */
    return sm_self;
  } else if (new_length < 1) {
    /* Someone decided to be that person. */
    rb_raise(rb_eRangeError,
      "Cannot resize array to length less than or equal to 0.");
    return sm_self;
  }

  REALLOC_N(RDATA(sm_self)->data, mat3_t, new_length);
  rb_ivar_set(sm_self, kRB_IVAR_MATHARRAY_LENGTH, sm_new_length);
  rb_ary_clear(rb_ivar_get(sm_self, kRB_IVAR_MATHARRAY_CACHE));

  return sm_self;
}



/*
 * Fetches a Mat3 from the array at the index and returns it. The returned Mat3
 * may be a cached object. In all cases, values returned from a typed array are
 * associated with the memory of the array and not given their own memory. So,
 * modifying a Mat3 fetched from an array modifies the array's data.
 *
 * As a result, objects returned by a Mat3Array should not be considered
 * thread-safe, nor should manipulating a Mat3Array be considered thread-safe
 * either. If you want to work with data returned from an array without altering
 * the array data, you should call Mat3#dup or Mat3#copy to get a new Mat3 with a
 * copy of the array object's data.
 *
 * call-seq: fetch(index) -> mat3
 */
static VALUE sm_mat3_array_fetch(VALUE sm_self, VALUE sm_index)
{
  mat3_t *arr;
  size_t length = NUM2SIZET(sm_mathtype_array_length(sm_self));
  size_t index = NUM2SIZET(sm_index);
  VALUE sm_inner;
  VALUE sm_cache;
  if (index >= length) {
    rb_raise(rb_eRangeError,
      "Index %zu out of bounds for array with length %zu",
      index, length);
  }

  sm_cache = rb_ivar_get(sm_self, kRB_IVAR_MATHARRAY_CACHE);
  if (!RTEST(sm_cache)) {
    rb_raise(rb_eRuntimeError, "No cache available");
  }
  sm_inner = rb_ary_entry(sm_cache, (long)index);

  if (!RTEST(sm_inner)) {
    /* No cached value, create one. */
    Data_Get_Struct(sm_self, mat3_t, arr);
    sm_inner = Data_Wrap_Struct(s_sm_mat3_klass, 0, 0, arr[index]);
    rb_ivar_set(sm_inner, kRB_IVAR_MATHARRAY_SOURCE, sm_self);
    /* Store the Mat3 in the cache */
    rb_ary_store(sm_cache, (long)index, sm_inner);
  }

  return sm_inner;
}



/*
 * Stores a Mat3 at the given index. If the provided Mat3 is a member of the
 * array and stored at the index, then no copy is done, otherwise the Mat3 is
 * copied to the array.
 *
 * If the value stored is a Mat4, it will be converted to a Mat3 for storage,
 * though this will not modify the value directly.
 *
 * call-seq: store(index, value) -> value
 */
static VALUE sm_mat3_array_store(VALUE sm_self, VALUE sm_index, VALUE sm_value)
{
  mat3_t *arr;
  size_t length = NUM2SIZET(sm_mathtype_array_length(sm_self));
  size_t index = NUM2SIZET(sm_index);
  int is_mat3 = 0;

  if (index >= length) {
    rb_raise(rb_eRangeError,
      "Index %zu out of bounds for array with length %zu",
      index, length);
  } else if (!(is_mat3 = SM_IS_A(sm_value, mat3)) && !SM_IS_A(sm_value, mat4)) {
    rb_raise(rb_eTypeError,
      "Invalid value to store: expected Mat3 or Mat4, got %s",
      rb_obj_classname(sm_value));
  }

  Data_Get_Struct(sm_self, mat3_t, arr);

  if (is_mat3) {
    mat3_t *value = sm_unwrap_mat3(sm_value, NULL);
    if (value == &arr[index]) {
      /* The object's part of the array, don't bother copying */
      return sm_value;
    }
    mat3_copy(*value, arr[index]);
  } else {
    mat4_to_mat3(*sm_unwrap_mat4(sm_value, NULL), arr[index]);
  }
  return sm_value;
}



/*
 * Returns the length of the array.
 *
 * call-seq: length -> fixnum
 */
static VALUE sm_mat3_array_size(VALUE sm_self)
{
  size_t length = NUM2SIZET(sm_mathtype_array_length(sm_self));
  return SIZET2NUM(length * sizeof(mat3_t));
}



/*==============================================================================

  Snow::Mat4Array methods (s_sm_mat4_array_klass)

==============================================================================*/

static VALUE s_sm_mat4_array_klass = Qnil;

/*
 * In the first form, a new typed array of Mat4 elements is allocated and
 * returned. In the second form, a copy of a typed array of Mat4 objects is
 * made and returned. Copied arrays do not share data.
 *
 * call-seq:
 *    new(size)       -> new mat4_array
 *    new(mat4_array) -> copy of mat4_array
 */
static VALUE sm_mat4_array_new(VALUE sm_self, VALUE sm_length_or_copy)
{
  size_t length = 0;
  mat4_t *arr;
  VALUE sm_type_array;
  int copy_array = 0;
  if ((copy_array = SM_IS_A(sm_length_or_copy, mat4_array))) {
    length = NUM2SIZET(sm_mathtype_array_length(sm_length_or_copy));
  } else {
    length = NUM2SIZET(sm_length_or_copy);
  }
  if (length <= 0) {
    return Qnil;
  }
  arr = ALLOC_N(mat4_t, length);
  if (copy_array) {
    const mat4_t *source;
    Data_Get_Struct(sm_length_or_copy, mat4_t, source);
    MEMCPY(arr, source, mat4_t, length);
    sm_length_or_copy = sm_mathtype_array_length(sm_length_or_copy);
    sm_self = rb_obj_class(sm_length_or_copy);
  }
  sm_type_array = Data_Wrap_Struct(sm_self, 0, free, arr);
  rb_ivar_set(sm_type_array, kRB_IVAR_MATHARRAY_LENGTH, sm_length_or_copy);
  rb_ivar_set(sm_type_array, kRB_IVAR_MATHARRAY_CACHE, rb_ary_new2((long)length));
  rb_obj_call_init(sm_type_array, 0, 0);
  return sm_type_array;
}



/*
 * Resizes the array to new_length and returns self.
 *
 * If resizing to a length smaller than the previous length, excess array
 * elements are discarded and the array is truncated. Otherwise, when resizing
 * the array to a greater length than previous, new elements in the array will
 * contain garbage values.
 *
 * If new_length is equal to self.length, the call does nothing to the array.
 *
 * Attempting to resize an array to a new length of zero or less will raise a
 * RangeError. Do not try to resize arrays to zero or less. Do not be that
 * person.
 *
 * call-seq:
 *    resize!(new_length) -> self
 */
static VALUE sm_mat4_array_resize(VALUE sm_self, VALUE sm_new_length)
{
  size_t new_length;
  size_t old_length;

  old_length = NUM2SIZET(sm_mathtype_array_length(sm_self));
  new_length = NUM2SIZET(sm_new_length);

  if (old_length == new_length) {
    /* No change, done */
    return sm_self;
  } else if (new_length < 1) {
    /* Someone decided to be that person. */
    rb_raise(rb_eRangeError,
      "Cannot resize array to length less than or equal to 0.");
    return sm_self;
  }

  REALLOC_N(RDATA(sm_self)->data, mat4_t, new_length);
  rb_ivar_set(sm_self, kRB_IVAR_MATHARRAY_LENGTH, sm_new_length);
  rb_ary_clear(rb_ivar_get(sm_self, kRB_IVAR_MATHARRAY_CACHE));

  return sm_self;
}



/*
 * Fetches a Mat4 from the array at the index and returns it. The returned Mat4
 * may be a cached object. In all cases, values returned from a typed array are
 * associated with the memory of the array and not given their own memory. So,
 * modifying a Mat4 fetched from an array modifies the array's data.
 *
 * As a result, objects returned by a Mat4Array should not be considered
 * thread-safe, nor should manipulating a Mat4Array be considered thread-safe
 * either. If you want to work with data returned from an array without altering
 * the array data, you should call Mat4#dup or Mat4#copy to get a new Mat4 with a
 * copy of the array object's data.
 *
 * call-seq: fetch(index) -> mat4
 */
static VALUE sm_mat4_array_fetch(VALUE sm_self, VALUE sm_index)
{
  mat4_t *arr;
  size_t length = NUM2SIZET(sm_mathtype_array_length(sm_self));
  size_t index = NUM2SIZET(sm_index);
  VALUE sm_inner;
  VALUE sm_cache;
  if (index >= length) {
    rb_raise(rb_eRangeError,
      "Index %zu out of bounds for array with length %zu",
      index, length);
  }

  sm_cache = rb_ivar_get(sm_self, kRB_IVAR_MATHARRAY_CACHE);
  if (!RTEST(sm_cache)) {
    rb_raise(rb_eRuntimeError, "No cache available");
  }
  sm_inner = rb_ary_entry(sm_cache, (long)index);

  if (!RTEST(sm_inner)) {
    /* No cached value, create one. */
    Data_Get_Struct(sm_self, mat4_t, arr);
    sm_inner = Data_Wrap_Struct(s_sm_mat4_klass, 0, 0, arr[index]);
    rb_ivar_set(sm_inner, kRB_IVAR_MATHARRAY_SOURCE, sm_self);
    /* Store the Mat4 in the cache */
    rb_ary_store(sm_cache, (long)index, sm_inner);
  }

  return sm_inner;
}



/*
 * Stores a Mat4 at the given index. If the provided Mat4 is a member of the
 * array and stored at the index, then no copy is done, otherwise the Mat4 is
 * copied to the array.
 *
 * If the value stored is a Mat3, it will be converted to a Mat4 for storage,
 * though this will not modify the value directly.
 *
 * call-seq: store(index, value) -> value
 */
static VALUE sm_mat4_array_store(VALUE sm_self, VALUE sm_index, VALUE sm_value)
{
  mat4_t *arr;
  size_t length = NUM2SIZET(sm_mathtype_array_length(sm_self));
  size_t index = NUM2SIZET(sm_index);
  int is_mat4 = 0;

  if (index >= length) {
    rb_raise(rb_eRangeError,
      "Index %zu out of bounds for array with length %zu",
      index, length);
  } else if (!(is_mat4 = SM_IS_A(sm_value, mat4)) && !SM_IS_A(sm_value, mat3)) {
    rb_raise(rb_eTypeError,
      "Invalid value to store: expected Mat3 or Mat4, got %s",
      rb_obj_classname(sm_value));
  }

  Data_Get_Struct(sm_self, mat4_t, arr);

  if (is_mat4) {
    mat4_t *value = sm_unwrap_mat4(sm_value, NULL);
    if (value == &arr[index]) {
      /* The object's part of the array, don't bother copying */
      return sm_value;
    }
    mat4_copy(*value, arr[index]);
  } else {
    mat3_to_mat4(*sm_unwrap_mat3(sm_value, NULL), arr[index]);
  }
  return sm_value;
}



/*
 * Returns the length of the array.
 *
 * call-seq: length -> fixnum
 */
static VALUE sm_mat4_array_size(VALUE sm_self)
{
  size_t length = NUM2SIZET(sm_mathtype_array_length(sm_self));
  return SIZET2NUM(length * sizeof(mat4_t));
}


#endif /* BUILD_ARRAY_TYPE */



/*==============================================================================

  vec3_t functions

==============================================================================*/

static VALUE sm_wrap_vec3(const vec3_t value, VALUE klass)
{
  vec3_t *copy;
  VALUE sm_wrapped = Qnil;
  if (!RTEST(klass)) {
    klass = s_sm_vec3_klass;
  }
  sm_wrapped = Data_Make_Struct(klass, vec3_t, 0, free, copy);
  if (value) {
    vec3_copy(value, *copy);
  }
  return sm_wrapped;
}



static vec3_t *sm_unwrap_vec3(VALUE sm_value, vec3_t store)
{
  vec3_t *value;
  Data_Get_Struct(sm_value, vec3_t, value);
  if(store) vec3_copy(*value, store);
  return value;
}



/*
 * Gets the component of the Vec3 at the given index.
 *
 * call-seq: fetch(index) -> float
 */
static VALUE sm_vec3_fetch (VALUE sm_self, VALUE sm_index)
{
  static const int max_index = sizeof(vec3_t) / sizeof(s_float_t);
  const vec3_t *self = sm_unwrap_vec3(sm_self, NULL);
  int index = NUM2INT(sm_index);
  if (index < 0 || index >= max_index) {
    rb_raise(rb_eRangeError,
      "Index %d is out of bounds, must be from 0 through %d", index, max_index - 1);
  }
  return rb_float_new(self[0][NUM2INT(sm_index)]);
}



/*
 * Sets the Vec3's component at the index to the value.
 *
 * call-seq: store(index, value) -> value
 */
static VALUE sm_vec3_store (VALUE sm_self, VALUE sm_index, VALUE sm_value)
{
  static const int max_index = sizeof(vec3_t) / sizeof(s_float_t);
  vec3_t *self = sm_unwrap_vec3(sm_self, NULL);
  int index = NUM2INT(sm_index);
  if (index < 0 || index >= max_index) {
    rb_raise(rb_eRangeError,
      "Index %d is out of bounds, must be from 0 through %d", index, max_index - 1);
  }
  self[0][index] = (s_float_t)rb_num2dbl(sm_value);
  return sm_value;
}



/*
 * Returns the length in bytes of the Vec3. When compiled to use doubles as the
 * base type, this is always 24. Otherwise, when compiled to use floats, it's
 * always 12.
 *
 * call-seq: size -> fixnum
 */
static VALUE sm_vec3_size (VALUE self)
{
  return SIZET2NUM(sizeof(vec3_t));
}



/*
 * Returns the length of the Vec3 in components. Result is always 3.
 *
 * call-seq: length -> fixnum
 */
static VALUE sm_vec3_length (VALUE self)
{
  return SIZET2NUM(sizeof(vec3_t) / sizeof(s_float_t));
}



/*
 * Returns a copy of self.
 *
 * call-seq:
 *    copy(output = nil) -> output or new vec3
 */
 static VALUE sm_vec3_copy(int argc, VALUE *argv, VALUE sm_self)
 {
   VALUE sm_out;
   vec3_t *self;
   rb_scan_args(argc, argv, "01", &sm_out);
   self = sm_unwrap_vec3(sm_self, NULL);
   if (argc == 1) {
     if (!RTEST(sm_out)) {
       goto SM_LABEL(skip_output);
     }{
     if (!SM_IS_A(sm_out, vec3) && !SM_IS_A(sm_out, vec4) && !SM_IS_A(sm_out, quat)) {
       rb_raise(rb_eTypeError,
         kSM_WANT_THREE_OR_FOUR_FORMAT_LIT,
         rb_obj_classname(sm_out));
       return Qnil;
     }
     vec3_t *output = sm_unwrap_vec3(sm_out, NULL);
     vec3_copy (*self, *output);
   }} else if (argc == 0) {
 SM_LABEL(skip_output): {
     vec3_t output;
     vec3_copy (*self, output);
     sm_out = sm_wrap_vec3(output, rb_obj_class(sm_self));
     rb_obj_call_init(sm_out, 0, 0);
   }} else {
     rb_raise(rb_eArgError, "Invalid number of arguments to copy");
   }
   return sm_out;
 }



/*
 * Returns a vector whose components are the multiplicative inverse of this
 * vector's.
 *
 * call-seq:
 *    normalize(output = nil) -> output or new vec3
 */
 static VALUE sm_vec3_normalize(int argc, VALUE *argv, VALUE sm_self)
 {
   VALUE sm_out;
   vec3_t *self;
   rb_scan_args(argc, argv, "01", &sm_out);
   self = sm_unwrap_vec3(sm_self, NULL);
   if (argc == 1) {
     if (!RTEST(sm_out)) {
       goto SM_LABEL(skip_output);
     }{
     if (!SM_IS_A(sm_out, vec3) && !SM_IS_A(sm_out, vec4) && !SM_IS_A(sm_out, quat)) {
       rb_raise(rb_eTypeError,
         kSM_WANT_THREE_OR_FOUR_FORMAT_LIT,
         rb_obj_classname(sm_out));
       return Qnil;
     }
     vec3_t *output = sm_unwrap_vec3(sm_out, NULL);
     vec3_normalize (*self, *output);
   }} else if (argc == 0) {
 SM_LABEL(skip_output): {
     vec3_t output;
     vec3_normalize (*self, output);
     sm_out = sm_wrap_vec3(output, rb_obj_class(sm_self));
     rb_obj_call_init(sm_out, 0, 0);
   }} else {
     rb_raise(rb_eArgError, "Invalid number of arguments to normalize");
   }
   return sm_out;
 }



/*
 * Returns a vector whose components are the multiplicative inverse of this
 * vector's.
 *
 * call-seq:
 *    inverse(output = nil) -> output or new vec3
 */
 static VALUE sm_vec3_inverse(int argc, VALUE *argv, VALUE sm_self)
 {
   VALUE sm_out;
   vec3_t *self;
   rb_scan_args(argc, argv, "01", &sm_out);
   self = sm_unwrap_vec3(sm_self, NULL);
   if (argc == 1) {
     if (!RTEST(sm_out)) {
       goto SM_LABEL(skip_output);
     }{
     if (!SM_IS_A(sm_out, vec3) && !SM_IS_A(sm_out, vec4) && !SM_IS_A(sm_out, quat)) {
       rb_raise(rb_eTypeError,
         kSM_WANT_THREE_OR_FOUR_FORMAT_LIT,
         rb_obj_classname(sm_out));
       return Qnil;
     }
     vec3_t *output = sm_unwrap_vec3(sm_out, NULL);
     vec3_inverse (*self, *output);
   }} else if (argc == 0) {
 SM_LABEL(skip_output): {
     vec3_t output;
     vec3_inverse (*self, output);
     sm_out = sm_wrap_vec3(output, rb_obj_class(sm_self));
     rb_obj_call_init(sm_out, 0, 0);
   }} else {
     rb_raise(rb_eArgError, "Invalid number of arguments to inverse");
   }
   return sm_out;
 }



/*
 * Negates this vector's components and returns the result.
 *
 * call-seq:
 *    negate(output = nil) -> output or new vec3
 */
 static VALUE sm_vec3_negate(int argc, VALUE *argv, VALUE sm_self)
 {
   VALUE sm_out;
   vec3_t *self;
   rb_scan_args(argc, argv, "01", &sm_out);
   self = sm_unwrap_vec3(sm_self, NULL);
   if (argc == 1) {
     if (!RTEST(sm_out)) {
       goto SM_LABEL(skip_output);
     }{
     if (!SM_IS_A(sm_out, vec3) && !SM_IS_A(sm_out, vec4) && !SM_IS_A(sm_out, quat)) {
       rb_raise(rb_eTypeError,
         kSM_WANT_THREE_OR_FOUR_FORMAT_LIT,
         rb_obj_classname(sm_out));
       return Qnil;
     }
     vec3_t *output = sm_unwrap_vec3(sm_out, NULL);
     vec3_negate (*self, *output);
   }} else if (argc == 0) {
 SM_LABEL(skip_output): {
     vec3_t output;
     vec3_negate (*self, output);
     sm_out = sm_wrap_vec3(output, rb_obj_class(sm_self));
     rb_obj_call_init(sm_out, 0, 0);
   }} else {
     rb_raise(rb_eArgError, "Invalid number of arguments to negate");
   }
   return sm_out;
 }



/*
 * Projects this vector onto a normal vector and returns the result.
 *
 * call-seq:
 *    project(normal, output = nil) -> output or new vec3
 */
static VALUE sm_vec3_project(int argc, VALUE *argv, VALUE sm_self)
{
  VALUE sm_rhs;
  VALUE sm_out;
  vec3_t *self;
  vec3_t *rhs;
  rb_scan_args(argc, argv, "11", &sm_rhs, &sm_out);
  self = sm_unwrap_vec3(sm_self, NULL);
  if (!SM_IS_A(sm_rhs, vec3) && !SM_IS_A(sm_rhs, vec4) && !SM_IS_A(sm_rhs, quat)) {
    rb_raise(rb_eTypeError,
      kSM_WANT_THREE_OR_FOUR_FORMAT_LIT,
      rb_obj_classname(sm_rhs));
    return Qnil;
  }
  rhs = sm_unwrap_vec3(sm_rhs, NULL);
  if (argc == 2) {
    if (!RTEST(sm_out)) {
      goto SM_LABEL(skip_output);
    }{
    if (!SM_IS_A(sm_out, vec3) && !SM_IS_A(sm_out, vec4) && !SM_IS_A(sm_out, quat)) {
      rb_raise(rb_eTypeError,
        kSM_WANT_THREE_OR_FOUR_FORMAT_LIT,
        rb_obj_classname(sm_out));
      return Qnil;
    }
    vec3_t *output = sm_unwrap_vec3(sm_out, NULL);
    vec3_project(*self, *rhs, *output);
  }} else if (argc == 1) {
SM_LABEL(skip_output): {
    vec3_t output;
    vec3_project(*self, *rhs, output);
    sm_out = sm_wrap_vec3(output, rb_obj_class(sm_self));
    rb_obj_call_init(sm_out, 0, 0);
  }} else {
    rb_raise(rb_eArgError, "Invalid number of arguments to project");
  }
  return sm_out;
}



/*
 * Reflects this vector against a normal vector and returns the result.
 *
 * call-seq:
 *    reflect(normal, output = nil) -> output or new vec3
 */
static VALUE sm_vec3_reflect(int argc, VALUE *argv, VALUE sm_self)
{
  VALUE sm_rhs;
  VALUE sm_out;
  vec3_t *self;
  vec3_t *rhs;
  rb_scan_args(argc, argv, "11", &sm_rhs, &sm_out);
  self = sm_unwrap_vec3(sm_self, NULL);
  if (!SM_IS_A(sm_rhs, vec3) && !SM_IS_A(sm_rhs, vec4) && !SM_IS_A(sm_rhs, quat)) {
    rb_raise(rb_eTypeError,
      kSM_WANT_THREE_OR_FOUR_FORMAT_LIT,
      rb_obj_classname(sm_rhs));
    return Qnil;
  }
  rhs = sm_unwrap_vec3(sm_rhs, NULL);
  if (argc == 2) {
    if (!RTEST(sm_out)) {
      goto SM_LABEL(skip_output);
    }{
    if (!SM_IS_A(sm_out, vec3) && !SM_IS_A(sm_out, vec4) && !SM_IS_A(sm_out, quat)) {
      rb_raise(rb_eTypeError,
        kSM_WANT_THREE_OR_FOUR_FORMAT_LIT,
        rb_obj_classname(sm_out));
      return Qnil;
    }
    vec3_t *output = sm_unwrap_vec3(sm_out, NULL);
    vec3_reflect(*self, *rhs, *output);
  }} else if (argc == 1) {
SM_LABEL(skip_output): {
    vec3_t output;
    vec3_reflect(*self, *rhs, output);
    sm_out = sm_wrap_vec3(output, rb_obj_class(sm_self));
    rb_obj_call_init(sm_out, 0, 0);
  }} else {
    rb_raise(rb_eArgError, "Invalid number of arguments to reflect");
  }
  return sm_out;
}



/*
 * Returns the cross product of this vector and another Vec3.
 *
 * call-seq:
 *    cross_product(vec3, output = nil) -> output or new vec3
 */
static VALUE sm_vec3_cross_product(int argc, VALUE *argv, VALUE sm_self)
{
  VALUE sm_rhs;
  VALUE sm_out;
  vec3_t *self;
  vec3_t *rhs;
  rb_scan_args(argc, argv, "11", &sm_rhs, &sm_out);
  self = sm_unwrap_vec3(sm_self, NULL);
  if (!SM_IS_A(sm_rhs, vec3) && !SM_IS_A(sm_rhs, vec4) && !SM_IS_A(sm_rhs, quat)) {
    rb_raise(rb_eTypeError,
      kSM_WANT_THREE_OR_FOUR_FORMAT_LIT,
      rb_obj_classname(sm_rhs));
    return Qnil;
  }
  rhs = sm_unwrap_vec3(sm_rhs, NULL);
  if (argc == 2) {
    if (!RTEST(sm_out)) {
      goto SM_LABEL(skip_output);
    }{
    if (!SM_IS_A(sm_out, vec3) && !SM_IS_A(sm_out, vec4) && !SM_IS_A(sm_out, quat)) {
      rb_raise(rb_eTypeError,
        kSM_WANT_THREE_OR_FOUR_FORMAT_LIT,
        rb_obj_classname(sm_out));
      return Qnil;
    }
    vec3_t *output = sm_unwrap_vec3(sm_out, NULL);
    vec3_cross_product(*self, *rhs, *output);
  }} else if (argc == 1) {
SM_LABEL(skip_output): {
    vec3_t output;
    vec3_cross_product(*self, *rhs, output);
    sm_out = sm_wrap_vec3(output, rb_obj_class(sm_self));
    rb_obj_call_init(sm_out, 0, 0);
  }} else {
    rb_raise(rb_eArgError, "Invalid number of arguments to cross_product");
  }
  return sm_out;
}



/*
 * Multiplies this and another vector's components together and returns the
 * result.
 *
 * call-seq:
 *    multiply(vec3, output = nil) -> output or new vec3
 */
static VALUE sm_vec3_multiply(int argc, VALUE *argv, VALUE sm_self)
{
  VALUE sm_rhs;
  VALUE sm_out;
  vec3_t *self;
  vec3_t *rhs;
  rb_scan_args(argc, argv, "11", &sm_rhs, &sm_out);
  self = sm_unwrap_vec3(sm_self, NULL);
  if (!SM_IS_A(sm_rhs, vec3) && !SM_IS_A(sm_rhs, vec4) && !SM_IS_A(sm_rhs, quat)) {
    rb_raise(rb_eTypeError,
      kSM_WANT_THREE_OR_FOUR_FORMAT_LIT,
      rb_obj_classname(sm_rhs));
    return Qnil;
  }
  rhs = sm_unwrap_vec3(sm_rhs, NULL);
  if (argc == 2) {
    if (!RTEST(sm_out)) {
      goto SM_LABEL(skip_output);
    }{
    if (!SM_IS_A(sm_out, vec3) && !SM_IS_A(sm_out, vec4) && !SM_IS_A(sm_out, quat)) {
      rb_raise(rb_eTypeError,
        kSM_WANT_THREE_OR_FOUR_FORMAT_LIT,
        rb_obj_classname(sm_out));
      return Qnil;
    }
    vec3_t *output = sm_unwrap_vec3(sm_out, NULL);
    vec3_multiply(*self, *rhs, *output);
  }} else if (argc == 1) {
SM_LABEL(skip_output): {
    vec3_t output;
    vec3_multiply(*self, *rhs, output);
    sm_out = sm_wrap_vec3(output, rb_obj_class(sm_self));
    rb_obj_call_init(sm_out, 0, 0);
  }} else {
    rb_raise(rb_eArgError, "Invalid number of arguments to multiply_vec3");
  }
  return sm_out;
}



/*
 * Adds this and another vector's components together and returns the result.
 *
 * call-seq:
 *    add(vec3, output = nil) -> output or new vec3
 */
static VALUE sm_vec3_add(int argc, VALUE *argv, VALUE sm_self)
{
  VALUE sm_rhs;
  VALUE sm_out;
  vec3_t *self;
  vec3_t *rhs;
  rb_scan_args(argc, argv, "11", &sm_rhs, &sm_out);
  self = sm_unwrap_vec3(sm_self, NULL);
  if (!SM_IS_A(sm_rhs, vec3) && !SM_IS_A(sm_rhs, vec4) && !SM_IS_A(sm_rhs, quat)) {
    rb_raise(rb_eTypeError,
      kSM_WANT_THREE_OR_FOUR_FORMAT_LIT,
      rb_obj_classname(sm_rhs));
    return Qnil;
  }
  rhs = sm_unwrap_vec3(sm_rhs, NULL);
  if (argc == 2) {
    if (!RTEST(sm_out)) {
      goto SM_LABEL(skip_output);
    }{
    if (!SM_IS_A(sm_out, vec3) && !SM_IS_A(sm_out, vec4) && !SM_IS_A(sm_out, quat)) {
      rb_raise(rb_eTypeError,
        kSM_WANT_THREE_OR_FOUR_FORMAT_LIT,
        rb_obj_classname(sm_out));
      return Qnil;
    }
    vec3_t *output = sm_unwrap_vec3(sm_out, NULL);
    vec3_add(*self, *rhs, *output);
  }} else if (argc == 1) {
SM_LABEL(skip_output): {
    vec3_t output;
    vec3_add(*self, *rhs, output);
    sm_out = sm_wrap_vec3(output, rb_obj_class(sm_self));
    rb_obj_call_init(sm_out, 0, 0);
  }} else {
    rb_raise(rb_eArgError, "Invalid number of arguments to add");
  }
  return sm_out;
}



/*
 * Subtracts another vector's components from this vector's and returns the
 * result.
 *
 * call-seq:
 *    subtract(vec3, output = nil) -> output or new vec3
 */
static VALUE sm_vec3_subtract(int argc, VALUE *argv, VALUE sm_self)
{
  VALUE sm_rhs;
  VALUE sm_out;
  vec3_t *self;
  vec3_t *rhs;
  rb_scan_args(argc, argv, "11", &sm_rhs, &sm_out);
  self = sm_unwrap_vec3(sm_self, NULL);
  if (!SM_IS_A(sm_rhs, vec3) && !SM_IS_A(sm_rhs, vec4) && !SM_IS_A(sm_rhs, quat)) {
    rb_raise(rb_eTypeError,
      kSM_WANT_THREE_OR_FOUR_FORMAT_LIT,
      rb_obj_classname(sm_rhs));
    return Qnil;
  }
  rhs = sm_unwrap_vec3(sm_rhs, NULL);
  if (argc == 2) {
    if (!RTEST(sm_out)) {
      goto SM_LABEL(skip_output);
    }{
    if (!SM_IS_A(sm_out, vec3) && !SM_IS_A(sm_out, vec4) && !SM_IS_A(sm_out, quat)) {
      rb_raise(rb_eTypeError,
        kSM_WANT_THREE_OR_FOUR_FORMAT_LIT,
        rb_obj_classname(sm_out));
      return Qnil;
    }
    vec3_t *output = sm_unwrap_vec3(sm_out, NULL);
    vec3_subtract(*self, *rhs, *output);
  }} else if (argc == 1) {
SM_LABEL(skip_output): {
    vec3_t output;
    vec3_subtract(*self, *rhs, output);
    sm_out = sm_wrap_vec3(output, rb_obj_class(sm_self));
    rb_obj_call_init(sm_out, 0, 0);
  }} else {
    rb_raise(rb_eArgError, "Invalid number of arguments to subtract");
  }
  return sm_out;
}



/*
 * Returns the dot product of this and another Vec3 or the XYZ components of a
 * Vec4 or Quat.
 *
 * call-seq:
 *    dot_product(vec3) -> float
 *    dot_product(vec4) -> float
 *    dot_product(quat) -> float
 */
static VALUE sm_vec3_dot_product(VALUE sm_self, VALUE sm_other)
{
  if (!SM_IS_A(sm_other, vec3) &&
      !SM_IS_A(sm_other, vec4) &&
      !SM_IS_A(sm_other, quat)) {
    rb_raise(rb_eArgError,
      "Expected a Quat, Vec3, or Vec4, got %s",
      rb_obj_classname(sm_other));
    return Qnil;
  }
  return rb_float_new(
    vec3_dot_product(
      *sm_unwrap_vec3(sm_self, NULL),
      *sm_unwrap_vec3(sm_other, NULL)));
}



/*
 * Allocates a Vec3.
 *
 * call-seq:
 *    new()          -> vec3 with components [0, 0, 0]
 *    new(x, y, z)   -> vec3 with components [x, y, z]
 *    new([x, y, z]) -> vec3 with components [x, y, z]
 *    new(vec3)      -> copy of vec3
 *    new(vec4)      -> vec3 of vec4's x, y, and z components
 *    new(quat)      -> vec3 of quat's x, y, and z components
 */
static VALUE sm_vec3_new(int argc, VALUE *argv, VALUE self)
{
  VALUE sm_vec = sm_wrap_vec3(g_vec3_zero, self);
  rb_obj_call_init(sm_vec, argc, argv);
  return sm_vec;
}



/*
 * Sets the Vec3's components.
 *
 * call-seq:
 *    set(x, y, z)   -> vec3 with components [x, y, z]
 *    set([x, y, z]) -> vec3 with components [x, y, z]
 *    set(vec3)      -> copy of vec3
 *    set(vec4)      -> vec3 of vec4's x, y, and z components
 *    set(quat)      -> vec3 of quat's x, y, and z components
 */
static VALUE sm_vec3_init(int argc, VALUE *argv, VALUE sm_self)
{
  vec3_t *self = sm_unwrap_vec3(sm_self, NULL);
  size_t arr_index = 0;

  switch(argc) {

  // Default value
  case 0: { break; }

  // Copy or by-array
  case 1: {
    if (SM_IS_A(argv[0], vec3) ||
        SM_IS_A(argv[0], vec4) ||
        SM_IS_A(argv[0], quat)) {
      sm_unwrap_vec3(argv[0], *self);
      break;
    }

    // Optional offset into array provided
    if (0) {
      case 2:
      arr_index = NUM2SIZET(argv[1]);
    }

    // Array of values
    if (SM_RB_IS_A(argv[0], rb_cArray)) {
      VALUE arrdata = argv[0];
      const size_t arr_end = arr_index + 3;
      s_float_t *vec_elem = *self;
      for (; arr_index < arr_end; ++arr_index, ++vec_elem) {
        *vec_elem = (s_float_t)rb_num2dbl(rb_ary_entry(arrdata, (long)arr_index));
      }
      break;
    }

    rb_raise(rb_eArgError, "Expected either an array of Numerics or a Vec3");
    break;
  }

  // X, Y, Z
  case 3: {
    self[0][0] = (s_float_t)rb_num2dbl(argv[0]);
    self[0][1] = (s_float_t)rb_num2dbl(argv[1]);
    self[0][2] = (s_float_t)rb_num2dbl(argv[2]);
    break;
  }

  default: {
    rb_raise(rb_eArgError, "Invalid arguments to Vec3.initialize");
    break;
  }
  } // switch (argc)

  return sm_self;
}



/*
 * Returns a string representation of self.
 *
 *    Vec3[].to_s     # => "{ 0.0, 0.0, 0.0 }"
 *
 * call-seq:
 *    to_s -> string
 */
static VALUE sm_vec3_to_s(VALUE self)
{
  const s_float_t *v;
  v = (const s_float_t *)*sm_unwrap_vec3(self, NULL);
  return rb_sprintf(
    "{ "
    "%f, %f, %f"
    " }",
    v[0], v[1], v[2]);
}



/*
 * Returns the squared magnitude of self.
 *
 * call-seq:
 *    magnitude_squared -> float
 */
static VALUE sm_vec3_magnitude_squared(VALUE sm_self)
{
  return rb_float_new(vec3_length_squared(*sm_unwrap_vec3(sm_self, NULL)));
}



/*
 * Returns the magnitude of self.
 *
 * call-seq:
 *    magnitude -> float
 */
static VALUE sm_vec3_magnitude(VALUE sm_self)
{
  return rb_float_new(vec3_length(*sm_unwrap_vec3(sm_self, NULL)));
}



/*
 * Scales this vector's components by a scalar value and returns the result.
 *
 * call-seq:
 *    scale(scalar, output = nil) -> output or new vec3
 */
static VALUE sm_vec3_scale(int argc, VALUE *argv, VALUE sm_self)
{
  VALUE sm_out;
  VALUE sm_scalar;
  s_float_t scalar;
  vec3_t *self = sm_unwrap_vec3(sm_self, NULL);

  rb_scan_args(argc, argv, "11", &sm_scalar, &sm_out);
  scalar = rb_num2dbl(sm_scalar);

  if (SM_IS_A(sm_out, vec3)) {
    vec3_scale(*self, scalar, *sm_unwrap_vec3(sm_out, NULL));
  } else {
    vec3_t out;
    vec3_scale(*self, scalar, out);
    sm_out = sm_wrap_vec3(out, rb_obj_class(sm_self));
    rb_obj_call_init(sm_out, 0, 0);
  }

  return sm_out;
}



/*
 * Divides this vector's components by a scalar value and returns the result.
 *
 * call-seq:
 *    divide(scalar, output = nil) -> output or new vec3
 */
static VALUE sm_vec3_divide(int argc, VALUE *argv, VALUE sm_self)
{
  VALUE sm_out;
  VALUE sm_scalar;
  s_float_t scalar;
  vec3_t *self = sm_unwrap_vec3(sm_self, NULL);

  rb_scan_args(argc, argv, "11", &sm_scalar, &sm_out);
  scalar = rb_num2dbl(sm_scalar);

  if (SM_IS_A(sm_out, vec3)) {
    vec3_divide(*self, scalar, *sm_unwrap_vec3(sm_out, NULL));
  } else {
    vec3_t out;
    vec3_divide(*self, scalar, out);
    sm_out = sm_wrap_vec3(out, rb_obj_class(sm_self));
    rb_obj_call_init(sm_out, 0, 0);
  }

  return sm_out;
}



/*
 * Tests whether a Vec3 is equivalent to another Vec3, a Vec4, or a Quat. When
 * testing for equivalency against 4-component objects, only the first three
 * components are compared.
 *
 * call-seq:
 *    vec3 == other_vec3 -> bool
 *    vec3 == vec4       -> bool
 *    vec3 == quat       -> bool
 */
static VALUE sm_vec3_equals(VALUE sm_self, VALUE sm_other)
{
  if (!RTEST(sm_other) || (!SM_IS_A(sm_other, vec3) && !SM_IS_A(sm_other, vec4) && !SM_IS_A(sm_other, quat))) {
    return Qfalse;
  }

  return vec3_equals(*sm_unwrap_vec3(sm_self, NULL), *sm_unwrap_vec3(sm_other, NULL)) ? Qtrue : Qfalse;
}



/*==============================================================================

  vec4_t functions

==============================================================================*/

static VALUE sm_wrap_vec4(const vec4_t value, VALUE klass)
{
  vec4_t *copy;
  VALUE sm_wrapped = Qnil;
  if (!RTEST(klass)) {
    klass = s_sm_vec4_klass;
  }
  sm_wrapped = Data_Make_Struct(klass, vec4_t, 0, free, copy);
  if (value) {
    vec4_copy(value, *copy);
  }
  return sm_wrapped;
}



static vec4_t *sm_unwrap_vec4(VALUE sm_value, vec4_t store)
{
  vec4_t *value;
  Data_Get_Struct(sm_value, vec4_t, value);
  if(store) vec4_copy(*value, store);
  return value;
}



/*
 * Gets the component of the Vec4 at the given index.
 *
 * call-seq: fetch(index) -> float
 */
static VALUE sm_vec4_fetch (VALUE sm_self, VALUE sm_index)
{
  static const int max_index = sizeof(vec4_t) / sizeof(s_float_t);
  const vec4_t *self = sm_unwrap_vec4(sm_self, NULL);
  int index = NUM2INT(sm_index);
  if (index < 0 || index >= max_index) {
    rb_raise(rb_eRangeError,
      "Index %d is out of bounds, must be from 0 through %d", index, max_index - 1);
  }
  return rb_float_new(self[0][NUM2INT(sm_index)]);
}



/*
 * Sets the Vec4's component at the index to the value.
 *
 * call-seq: store(index, value) -> value
 */
static VALUE sm_vec4_store (VALUE sm_self, VALUE sm_index, VALUE sm_value)
{
  static const int max_index = sizeof(vec4_t) / sizeof(s_float_t);
  vec4_t *self = sm_unwrap_vec4(sm_self, NULL);
  int index = NUM2INT(sm_index);
  if (index < 0 || index >= max_index) {
    rb_raise(rb_eRangeError,
      "Index %d is out of bounds, must be from 0 through %d", index, max_index - 1);
  }
  self[0][index] = (s_float_t)rb_num2dbl(sm_value);
  return sm_value;
}



/*
 * Returns the length in bytes of the Vec4. When compiled to use doubles as the
 * base type, this is always 32. Otherwise, when compiled to use floats, it's
 * always 16.
 *
 * call-seq: size -> fixnum
 */
static VALUE sm_vec4_size (VALUE self)
{
  return SIZET2NUM(sizeof(vec4_t));
}



/*
 * Returns the length of the Vec4 in components. Result is always 4.
 *
 * call-seq: length -> fixnum
 */
static VALUE sm_vec4_length (VALUE self)
{
  return SIZET2NUM(sizeof(vec4_t) / sizeof(s_float_t));
}



/*
 * Returns a copy of self.
 *
 * call-seq:
 *    copy(output = nil) -> output or new vec4 / quat
 */
 static VALUE sm_vec4_copy(int argc, VALUE *argv, VALUE sm_self)
 {
   VALUE sm_out;
   vec4_t *self;
   rb_scan_args(argc, argv, "01", &sm_out);
   self = sm_unwrap_vec4(sm_self, NULL);
   if (argc == 1) {
     if (!RTEST(sm_out)) {
       goto SM_LABEL(skip_output);
     }{
     if (!SM_IS_A(sm_out, vec4) && !SM_IS_A(sm_out, quat)) {
       rb_raise(rb_eTypeError,
         kSM_WANT_FOUR_FORMAT_LIT,
         rb_obj_classname(sm_out));
       return Qnil;
     }
     vec4_t *output = sm_unwrap_vec4(sm_out, NULL);
     vec4_copy (*self, *output);
   }} else if (argc == 0) {
 SM_LABEL(skip_output): {
     vec4_t output;
     vec4_copy (*self, output);
     sm_out = sm_wrap_vec4(output, rb_obj_class(sm_self));
     rb_obj_call_init(sm_out, 0, 0);
   }} else {
     rb_raise(rb_eArgError, "Invalid number of arguments to copy");
   }
   return sm_out;
 }



/*
 * Returns a normalized Vec4 or Quat, depending on the type of the receiver and
 * output.
 *
 * call-seq:
 *    normalize(output = nil) -> output or new vec4 / quat
 */
 static VALUE sm_vec4_normalize(int argc, VALUE *argv, VALUE sm_self)
 {
   VALUE sm_out;
   vec4_t *self;
   rb_scan_args(argc, argv, "01", &sm_out);
   self = sm_unwrap_vec4(sm_self, NULL);
   if (argc == 1) {
     if (!RTEST(sm_out)) {
       goto SM_LABEL(skip_output);
     }{
     if (!SM_IS_A(sm_out, vec4) && !SM_IS_A(sm_out, quat)) {
      rb_raise(rb_eTypeError,
        kSM_WANT_FOUR_FORMAT_LIT,
        rb_obj_classname(sm_out));
     }
     vec4_t *output = sm_unwrap_vec4(sm_out, NULL);
     vec4_normalize (*self, *output);
   }} else if (argc == 0) {
 SM_LABEL(skip_output): {
     vec4_t output;
     vec4_normalize (*self, output);
     sm_out = sm_wrap_vec4(output, rb_obj_class(sm_self));
     rb_obj_call_init(sm_out, 0, 0);
   }} else {
     rb_raise(rb_eArgError, "Invalid number of arguments to normalize");
   }
   return sm_out;
 }



/*
 * Returns a vector whose components are the multiplicative inverse of this
 * vector's.
 *
 * call-seq:
 *    inverse(output = nil) -> output or new vec4
 */
 static VALUE sm_vec4_inverse(int argc, VALUE *argv, VALUE sm_self)
 {
   VALUE sm_out;
   vec4_t *self;
   rb_scan_args(argc, argv, "01", &sm_out);
   self = sm_unwrap_vec4(sm_self, NULL);
   if (argc == 1) {
     if (!RTEST(sm_out)) {
       goto SM_LABEL(skip_output);
     }{
     if (!SM_IS_A(sm_out, vec4) && !SM_IS_A(sm_out, quat)) {
       rb_raise(rb_eTypeError,
         kSM_WANT_FOUR_FORMAT_LIT,
         rb_obj_classname(sm_out));
       return Qnil;
     }
     vec4_t *output = sm_unwrap_vec4(sm_out, NULL);
     vec4_inverse (*self, *output);
   }} else if (argc == 0) {
 SM_LABEL(skip_output): {
     vec4_t output;
     vec4_inverse (*self, output);
     sm_out = sm_wrap_vec4(output, rb_obj_class(sm_self));
     rb_obj_call_init(sm_out, 0, 0);
   }} else {
     rb_raise(rb_eArgError, "Invalid number of arguments to inverse");
   }
   return sm_out;
 }



/*
 * Negates this vector or quaternions's components and returns the result.
 *
 * call-seq:
 *    negate(output = nil) -> output or new vec4 or quat
 */
 static VALUE sm_vec4_negate(int argc, VALUE *argv, VALUE sm_self)
 {
   VALUE sm_out;
   vec4_t *self;
   rb_scan_args(argc, argv, "01", &sm_out);
   self = sm_unwrap_vec4(sm_self, NULL);
   if (argc == 1) {
     if (!RTEST(sm_out)) {
       goto SM_LABEL(skip_output);
     }{
     if (!SM_IS_A(sm_out, vec4) && !SM_IS_A(sm_out, quat)) {
       rb_raise(rb_eTypeError,
         kSM_WANT_FOUR_FORMAT_LIT,
         rb_obj_classname(sm_out));
       return Qnil;
     }
     vec4_t *output = sm_unwrap_vec4(sm_out, NULL);
     vec4_negate (*self, *output);
   }} else if (argc == 0) {
 SM_LABEL(skip_output): {
     vec4_t output;
     vec4_negate (*self, output);
     sm_out = sm_wrap_vec4(output, rb_obj_class(sm_self));
     rb_obj_call_init(sm_out, 0, 0);
   }} else {
     rb_raise(rb_eArgError, "Invalid number of arguments to negate");
   }
   return sm_out;
 }



/*
 * Projects this vector onto a normal vector and returns the result.
 *
 * call-seq:
 *    project(normal, output = nil) -> output or new vec4
 */
static VALUE sm_vec4_project(int argc, VALUE *argv, VALUE sm_self)
{
  VALUE sm_rhs;
  VALUE sm_out;
  vec4_t *self;
  vec4_t *rhs;
  rb_scan_args(argc, argv, "11", &sm_rhs, &sm_out);
  self = sm_unwrap_vec4(sm_self, NULL);
  if (!SM_IS_A(sm_rhs, vec4) && !SM_IS_A(sm_rhs, quat)) {
    rb_raise(rb_eTypeError,
      kSM_WANT_FOUR_FORMAT_LIT,
      rb_obj_classname(sm_rhs));
    return Qnil;
  }
  rhs = sm_unwrap_vec4(sm_rhs, NULL);
  if (argc == 2) {
    if (!RTEST(sm_out)) {
      goto SM_LABEL(skip_output);
    }{
    if (!SM_IS_A(sm_out, vec4) && !SM_IS_A(sm_out, quat)) {
      rb_raise(rb_eTypeError,
        kSM_WANT_FOUR_FORMAT_LIT,
        rb_obj_classname(sm_out));
      return Qnil;
    }
    vec4_t *output = sm_unwrap_vec4(sm_out, NULL);
    vec4_project(*self, *rhs, *output);
  }} else if (argc == 1) {
SM_LABEL(skip_output): {
    vec4_t output;
    vec4_project(*self, *rhs, output);
    sm_out = sm_wrap_vec4(output, rb_obj_class(sm_self));
    rb_obj_call_init(sm_out, 0, 0);
  }} else {
    rb_raise(rb_eArgError, "Invalid number of arguments to project");
  }
  return sm_out;
}



/*
 * Reflects this vector against a normal vector and returns the result.
 *
 * call-seq:
 *    reflect(normal, output = nil) -> output or new vec4
 */
static VALUE sm_vec4_reflect(int argc, VALUE *argv, VALUE sm_self)
{
  VALUE sm_rhs;
  VALUE sm_out;
  vec4_t *self;
  vec4_t *rhs;
  rb_scan_args(argc, argv, "11", &sm_rhs, &sm_out);
  self = sm_unwrap_vec4(sm_self, NULL);
  if (!SM_IS_A(sm_rhs, vec4) && !SM_IS_A(sm_rhs, quat)) {
    rb_raise(rb_eTypeError,
      kSM_WANT_FOUR_FORMAT_LIT,
      rb_obj_classname(sm_rhs));
    return Qnil;
  }
  rhs = sm_unwrap_vec4(sm_rhs, NULL);
  if (argc == 2) {
    if (!RTEST(sm_out)) {
      goto SM_LABEL(skip_output);
    }{
    if (!SM_IS_A(sm_out, vec4) && !SM_IS_A(sm_out, quat)) {
      rb_raise(rb_eTypeError,
        kSM_WANT_FOUR_FORMAT_LIT,
        rb_obj_classname(sm_out));
      return Qnil;
    }
    vec4_t *output = sm_unwrap_vec4(sm_out, NULL);
    vec4_reflect(*self, *rhs, *output);
  }} else if (argc == 1) {
SM_LABEL(skip_output): {
    vec4_t output;
    vec4_reflect(*self, *rhs, output);
    sm_out = sm_wrap_vec4(output, rb_obj_class(sm_self));
    rb_obj_call_init(sm_out, 0, 0);
  }} else {
    rb_raise(rb_eArgError, "Invalid number of arguments to reflect");
  }
  return sm_out;
}



/*
 * Multiplies this and another vector's components together and returns the
 * result.
 *
 * call-seq:
 *    multiply(vec4, output = nil) -> output or new vec4
 */
static VALUE sm_vec4_multiply(int argc, VALUE *argv, VALUE sm_self)
{
  VALUE sm_rhs;
  VALUE sm_out;
  vec4_t *self;
  vec4_t *rhs;
  rb_scan_args(argc, argv, "11", &sm_rhs, &sm_out);
  self = sm_unwrap_vec4(sm_self, NULL);
  if (!SM_IS_A(sm_rhs, vec4) && !SM_IS_A(sm_rhs, quat)) {
    rb_raise(rb_eTypeError,
      kSM_WANT_FOUR_FORMAT_LIT,
      rb_obj_classname(sm_rhs));
    return Qnil;
  }
  rhs = sm_unwrap_vec4(sm_rhs, NULL);
  if (argc == 2) {
    if (!RTEST(sm_out)) {
      goto SM_LABEL(skip_output);
    }{
    if (!SM_IS_A(sm_out, vec4) && !SM_IS_A(sm_out, quat)) {
      rb_raise(rb_eTypeError,
        kSM_WANT_FOUR_FORMAT_LIT,
        rb_obj_classname(sm_out));
      return Qnil;
    }
    vec4_t *output = sm_unwrap_vec4(sm_out, NULL);
    vec4_multiply(*self, *rhs, *output);
  }} else if (argc == 1) {
SM_LABEL(skip_output): {
    vec4_t output;
    vec4_multiply(*self, *rhs, output);
    sm_out = sm_wrap_vec4(output, rb_obj_class(sm_self));
    rb_obj_call_init(sm_out, 0, 0);
  }} else {
    rb_raise(rb_eArgError, "Invalid number of arguments to multiply_vec4");
  }
  return sm_out;
}



/*
 * Adds this and another vector or quaternion's components together and returns
 * the result. The result type is that of the receiver.
 *
 * call-seq:
 *    add(vec4, output = nil) -> output or new vec4 or quat
 */
static VALUE sm_vec4_add(int argc, VALUE *argv, VALUE sm_self)
{
  VALUE sm_rhs;
  VALUE sm_out;
  vec4_t *self;
  vec4_t *rhs;
  rb_scan_args(argc, argv, "11", &sm_rhs, &sm_out);
  self = sm_unwrap_vec4(sm_self, NULL);
  if (!SM_IS_A(sm_rhs, vec4) && !SM_IS_A(sm_rhs, quat)) {
    rb_raise(rb_eTypeError,
      kSM_WANT_FOUR_FORMAT_LIT,
      rb_obj_classname(sm_rhs));
    return Qnil;
  }
  rhs = sm_unwrap_vec4(sm_rhs, NULL);
  if (argc == 2) {
    if (!RTEST(sm_out)) {
      goto SM_LABEL(skip_output);
    }{
    if (!SM_IS_A(sm_rhs, vec4) && !SM_IS_A(sm_rhs, quat)) {
      rb_raise(rb_eTypeError,
        kSM_WANT_FOUR_FORMAT_LIT,
        rb_obj_classname(sm_rhs));
      return Qnil;
    }
    vec4_t *output = sm_unwrap_vec4(sm_out, NULL);
    vec4_add(*self, *rhs, *output);
  }} else if (argc == 1) {
SM_LABEL(skip_output): {
    vec4_t output;
    vec4_add(*self, *rhs, output);
    sm_out = sm_wrap_vec4(output, rb_obj_class(sm_self));
    rb_obj_call_init(sm_out, 0, 0);
  }} else {
    rb_raise(rb_eArgError, "Invalid number of arguments to add");
  }
  return sm_out;
}



/*
 * Subtracts another vector or quaternion's components from this vector's and
 * returns the result. The return type is that of the receiver.
 *
 * call-seq:
 *    subtract(vec4, output = nil) -> output or new vec4
 */
static VALUE sm_vec4_subtract(int argc, VALUE *argv, VALUE sm_self)
{
  VALUE sm_rhs;
  VALUE sm_out;
  vec4_t *self;
  vec4_t *rhs;
  rb_scan_args(argc, argv, "11", &sm_rhs, &sm_out);
  self = sm_unwrap_vec4(sm_self, NULL);
  if (!SM_IS_A(sm_rhs, vec4) && !SM_IS_A(sm_rhs, quat)) {
    rb_raise(rb_eTypeError,
      kSM_WANT_FOUR_FORMAT_LIT,
      rb_obj_classname(sm_rhs));
    return Qnil;
  }
  rhs = sm_unwrap_vec4(sm_rhs, NULL);
  if (argc == 2) {
    if (!RTEST(sm_out)) {
      goto SM_LABEL(skip_output);
    }{
    if (!SM_IS_A(sm_rhs, vec4) && !SM_IS_A(sm_rhs, quat)) {
      rb_raise(rb_eTypeError,
        kSM_WANT_FOUR_FORMAT_LIT,
        rb_obj_classname(sm_rhs));
      return Qnil;
    }
    vec4_t *output = sm_unwrap_vec4(sm_out, NULL);
    vec4_subtract(*self, *rhs, *output);
  }} else if (argc == 1) {
SM_LABEL(skip_output): {
    vec4_t output;
    vec4_subtract(*self, *rhs, output);
    sm_out = sm_wrap_vec4(output, rb_obj_class(sm_self));
    rb_obj_call_init(sm_out, 0, 0);
  }} else {
    rb_raise(rb_eArgError, "Invalid number of arguments to subtract");
  }
  return sm_out;
}




/*
 * Returns the dot product of self and another Vec4 or Quat.
 *
 * call-seq:
 *    dot_product(vec4) -> float
 *    dot_product(quat) -> float
 */
static VALUE sm_vec4_dot_product(VALUE sm_self, VALUE sm_other)
{
  if (!SM_IS_A(sm_other, vec4) &&
      !SM_IS_A(sm_other, quat)) {
    rb_raise(rb_eArgError,
      kSM_WANT_FOUR_FORMAT_LIT,
      rb_obj_classname(sm_other));
    return Qnil;
  }
  return rb_float_new(
    vec4_dot_product(
      *sm_unwrap_vec4(sm_self, NULL),
      *sm_unwrap_vec4(sm_other, NULL)));
}



/*
 * Allocates a new Vec4.
 *
 * call-seq:
 *     new()               -> new vec4 with components [0, 0, 0, 1]
 *     new(x, y, z, w = 1) -> new vec4 with components [x, y, z, w]
 *     new([x, y, z, w])   -> new vec4 with components [x, y, z, w]
 *     new(vec4)           -> copy of vec4
 *     new(vec3)           -> copy of vec3 with w component of 1
 *     new(quat)           -> copy of quat as vec4
 */
static VALUE sm_vec4_new(int argc, VALUE *argv, VALUE self)
{
  VALUE sm_vec = sm_wrap_vec4(g_vec4_identity, self);
  rb_obj_call_init(sm_vec, argc, argv);
  return sm_vec;
}



/*
 * Sets the Vec4's components.
 *
 * call-seq:
 *    set(x, y, z, w = 1) -> new vec4 with components [x, y, z, w]
 *    set([x, y, z, w])   -> new vec4 with components [x, y, z, w]
 *    set(vec4)           -> copy of vec4
 *    set(vec3)           -> copy of vec3 with w component of 1
 *    set(quat)           -> copy of quat as vec4
 */
static VALUE sm_vec4_init(int argc, VALUE *argv, VALUE sm_self)
{
  vec4_t *self = sm_unwrap_vec4(sm_self, NULL);
  size_t arr_index = 0;

  switch(argc) {

  // Default value
  case 0: { break; }

  // Copy or by-array
  case 1: {
    if (SM_IS_A(argv[0], quat) ||
        SM_IS_A(argv[0], vec4)) {
      sm_unwrap_quat(argv[0], *self);
      break;
    }

    if (SM_IS_A(argv[0], vec3)) {
      sm_unwrap_vec3(argv[0], *self);
      break;
    }

    // Optional offset into array provided
    if (0) {
      case 2:
      arr_index = NUM2SIZET(argv[1]);
    }

    // Array of values
    if (SM_RB_IS_A(argv[0], rb_cArray)) {
      VALUE arrdata = argv[0];
      const size_t arr_end = arr_index + 4;
      s_float_t *vec_elem = *self;
      for (; arr_index < arr_end; ++arr_index, ++vec_elem) {
        *vec_elem = (s_float_t)rb_num2dbl(rb_ary_entry(arrdata, (long)arr_index));
      }
      break;
    }

    rb_raise(rb_eArgError, "Expected either an array of Numerics or a Vec4");
    break;
  }

  // W
  case 4: {
    self[0][3] = (s_float_t)rb_num2dbl(argv[3]);
    case 3: // X, Y, Z
    self[0][0] = (s_float_t)rb_num2dbl(argv[0]);
    self[0][1] = (s_float_t)rb_num2dbl(argv[1]);
    self[0][2] = (s_float_t)rb_num2dbl(argv[2]);
    break;
  }

  default: {
    rb_raise(rb_eArgError, "Invalid arguments to Vec4.initialize");
    break;
  }
  } // switch (argc)

  return sm_self;
}



/*
 * Returns a string representation of self.
 *
 *    Vec4[].to_s     # => "{ 0.0, 0.0, 0.0, 1.0 }"
 *
 * call-seq:
 *    to_s -> string
 */
static VALUE sm_vec4_to_s(VALUE self)
{
  const s_float_t *v;
  v = (const s_float_t *)*sm_unwrap_vec4(self, NULL);
  return rb_sprintf(
    "{ "
    "%f, %f, %f, %f"
    " }",
    v[0], v[1], v[2], v[3]);
}



/*
 * Returns the squared magnitude of self.
 *
 * call-seq:
 *    magnitude_squared -> float
 */
static VALUE sm_vec4_magnitude_squared(VALUE sm_self)
{
  return rb_float_new(vec4_length_squared(*sm_unwrap_vec4(sm_self, NULL)));
}



/*
 * Returns the magnitude of self.
 *
 * call-seq:
 *    magnitude -> float
 */
static VALUE sm_vec4_magnitude(VALUE sm_self)
{
  return rb_float_new(vec4_length(*sm_unwrap_vec4(sm_self, NULL)));
}



/*
 * Scales this vector or quaternion's components by a scalar value and returns
 * the result. The return type is that of the receiver.
 *
 * call-seq:
 *    scale(scalar, output = nil) -> output or new vec4
 */
static VALUE sm_vec4_scale(int argc, VALUE *argv, VALUE sm_self)
{
  VALUE sm_out;
  VALUE sm_scalar;
  s_float_t scalar;
  vec4_t *self = sm_unwrap_vec4(sm_self, NULL);

  rb_scan_args(argc, argv, "11", &sm_scalar, &sm_out);
  scalar = rb_num2dbl(sm_scalar);

  if ((SM_IS_A(sm_out, vec4) || SM_IS_A(sm_out, quat))) {
    vec4_scale(*self, scalar, *sm_unwrap_vec4(sm_out, NULL));
  } else {
    vec4_t out;
    vec4_scale(*self, scalar, out);
    sm_out = sm_wrap_vec4(out, rb_obj_class(sm_self));
    rb_obj_call_init(sm_out, 0, 0);
  }

  return sm_out;
}



/*
 * Divides this vector or quaternion's components by a scalar value and returns
 * the result. The return type is that of the receiver.
 *
 * call-seq:
 *    divide(scalar, output = nil) -> output or new vec4
 */
static VALUE sm_vec4_divide(int argc, VALUE *argv, VALUE sm_self)
{
  VALUE sm_out;
  VALUE sm_scalar;
  s_float_t scalar;
  vec4_t *self = sm_unwrap_vec4(sm_self, NULL);

  rb_scan_args(argc, argv, "11", &sm_scalar, &sm_out);
  scalar = rb_num2dbl(sm_scalar);

  if ((SM_IS_A(sm_out, vec4) || SM_IS_A(sm_out, quat))) {
    vec4_divide(*self, scalar, *sm_unwrap_vec4(sm_out, NULL));
  } else {
    vec4_t out;
    vec4_divide(*self, scalar, out);
    sm_out = sm_wrap_vec4(out, rb_obj_class(sm_self));
    rb_obj_call_init(sm_out, 0, 0);
  }

  return sm_out;
}



/*
 * Tests this Vec4 or Quat and another Vec4 or Quat for equivalency.
 *
 * call-seq:
 *    quat == other_quat -> bool
 *    vec4 == other_vec4 -> bool
 *    quat == vec4       -> bool
 *    vec4 == quat       -> bool
 */
static VALUE sm_vec4_equals(VALUE sm_self, VALUE sm_other)
{
  if (!RTEST(sm_other) || (!SM_IS_A(sm_other, vec4) && !SM_IS_A(sm_other, quat))) {
    return Qfalse;
  }

  return vec4_equals(*sm_unwrap_vec4(sm_self, NULL), *sm_unwrap_vec4(sm_other, NULL)) ? Qtrue : Qfalse;
}



/*==============================================================================

  quat_t functions

==============================================================================*/

static VALUE sm_wrap_quat(const quat_t value, VALUE klass)
{
  quat_t *copy;
  VALUE sm_wrapped = Qnil;
  if (!RTEST(klass)) {
    klass = s_sm_quat_klass;
  }
  sm_wrapped = Data_Make_Struct(klass, quat_t, 0, free, copy);
  if (value) {
    quat_copy(value, *copy);
  }
  return sm_wrapped;
}



static quat_t *sm_unwrap_quat(VALUE sm_value, quat_t store)
{
  quat_t *value;
  Data_Get_Struct(sm_value, quat_t, value);
  if(store) quat_copy(*value, store);
  return value;
}



/*
 * Gets the component of the Quat at the given index.
 *
 * call-seq: fetch(index) -> float
 */
static VALUE sm_quat_fetch (VALUE sm_self, VALUE sm_index)
{
  static const int max_index = sizeof(quat_t) / sizeof(s_float_t);
  const quat_t *self = sm_unwrap_quat(sm_self, NULL);
  int index = NUM2INT(sm_index);
  if (index < 0 || index >= max_index) {
    rb_raise(rb_eRangeError,
      "Index %d is out of bounds, must be from 0 through %d", index, max_index - 1);
  }
  return rb_float_new(self[0][NUM2INT(sm_index)]);
}



/*
 * Sets the Quat's component at the index to the value.
 *
 * call-seq: store(index, value) -> value
 */
static VALUE sm_quat_store (VALUE sm_self, VALUE sm_index, VALUE sm_value)
{
  static const int max_index = sizeof(quat_t) / sizeof(s_float_t);
  quat_t *self = sm_unwrap_quat(sm_self, NULL);
  int index = NUM2INT(sm_index);
  if (index < 0 || index >= max_index) {
    rb_raise(rb_eRangeError,
      "Index %d is out of bounds, must be from 0 through %d", index, max_index - 1);
  }
  self[0][index] = (s_float_t)rb_num2dbl(sm_value);
  return sm_value;
}



/*
 * Returns the length in bytes of the Quat. When compiled to use doubles as the
 * base type, this is always 32. Otherwise, when compiled to use floats, it's
 * always 16.
 *
 * call-seq: size -> fixnum
 */
static VALUE sm_quat_size (VALUE self)
{
  return SIZET2NUM(sizeof(quat_t));
}



/*
 * Returns the length of the Quat in components. Result is always 4.
 *
 * call-seq: length -> fixnum
 */
static VALUE sm_quat_length (VALUE self)
{
  return SIZET2NUM(sizeof(quat_t) / sizeof(s_float_t));
}



/*
 * Returns the inverse of this Quat. Note that this is not the same as the
 * inverse of, for example, a Vec4.
 *
 * call-seq:
 *    inverse(output = nil) -> output or new quat
 */
 static VALUE sm_quat_inverse(int argc, VALUE *argv, VALUE sm_self)
 {
   VALUE sm_out;
   quat_t *self;
   rb_scan_args(argc, argv, "01", &sm_out);
   self = sm_unwrap_quat(sm_self, NULL);
   if (argc == 1) {
     if (!RTEST(sm_out)) {
       goto SM_LABEL(skip_output);
     }{
     if (!SM_IS_A(sm_out, vec4) && !SM_IS_A(sm_out, quat)) {
       rb_raise(rb_eTypeError,
         kSM_WANT_FOUR_FORMAT_LIT,
         rb_obj_classname(sm_out));
       return Qnil;
     }
     quat_t *output = sm_unwrap_quat(sm_out, NULL);
     quat_inverse (*self, *output);
   }} else if (argc == 0) {
 SM_LABEL(skip_output): {
     quat_t output;
     quat_inverse (*self, output);
     sm_out = sm_wrap_quat(output, rb_obj_class(sm_self));
     rb_obj_call_init(sm_out, 0, 0);
   }} else {
     rb_raise(rb_eArgError, "Invalid number of arguments to inverse");
   }
   return sm_out;
 }



/*
 * Concatenates this quaternion and another and returns the result.
 *
 * call-seq:
 *    multiply_quat(quat, output = nil) -> output or new quat
 */
static VALUE sm_quat_multiply(int argc, VALUE *argv, VALUE sm_self)
{
  VALUE sm_rhs;
  VALUE sm_out;
  quat_t *self;
  quat_t *rhs;
  rb_scan_args(argc, argv, "11", &sm_rhs, &sm_out);
  self = sm_unwrap_quat(sm_self, NULL);
  if (!!SM_IS_A(sm_rhs, vec4) && !SM_IS_A(sm_rhs, quat)) {
    rb_raise(rb_eTypeError,
      kSM_WANT_FOUR_FORMAT_LIT,
      rb_obj_classname(sm_rhs));
    return Qnil;
  }
  rhs = sm_unwrap_quat(sm_rhs, NULL);
  if (argc == 2) {
    if (!RTEST(sm_out)) {
      goto SM_LABEL(skip_output);
    }{
    if (!!SM_IS_A(sm_out, vec4) && !SM_IS_A(sm_out, quat)) {
      rb_raise(rb_eTypeError,
        kSM_WANT_FOUR_FORMAT_LIT,
        rb_obj_classname(sm_out));
      return Qnil;
    }
    quat_t *output = sm_unwrap_quat(sm_out, NULL);
    quat_multiply(*self, *rhs, *output);
  }} else if (argc == 1) {
SM_LABEL(skip_output): {
    quat_t output;
    quat_multiply(*self, *rhs, output);
    sm_out = sm_wrap_quat(output, rb_obj_class(sm_self));
    rb_obj_call_init(sm_out, 0, 0);
  }} else {
    rb_raise(rb_eArgError, "Invalid number of arguments to multiply_quat");
  }
  return sm_out;
}



/*
 * Multiplies a quaternion and vec3, returning the rotated vec3.
 *
 * call-seq:
 *    multiply_vec3(quat, output = nil) -> output or new quat
 */
static VALUE sm_quat_multiply_vec3(int argc, VALUE *argv, VALUE sm_self)
{
  VALUE sm_rhs;
  VALUE sm_out;
  quat_t *self;
  vec3_t *rhs;
  rb_scan_args(argc, argv, "11", &sm_rhs, &sm_out);
  self = sm_unwrap_quat(sm_self, NULL);
  if (!SM_IS_A(sm_rhs, vec3) && !SM_IS_A(sm_rhs, vec4) && !SM_IS_A(sm_rhs, quat)) {
    rb_raise(rb_eTypeError,
      kSM_WANT_THREE_OR_FOUR_FORMAT_LIT,
      rb_obj_classname(sm_rhs));
    return Qnil;
  }
  rhs = sm_unwrap_vec3(sm_rhs, NULL);
  if (argc == 2) {
    if (!RTEST(sm_out)) {
      goto SM_LABEL(skip_output);
    }{
    if (!SM_IS_A(sm_out, vec3) && !SM_IS_A(sm_out, vec4) && !SM_IS_A(sm_out, quat)) {
      rb_raise(rb_eTypeError,
        kSM_WANT_THREE_OR_FOUR_FORMAT_LIT,
        rb_obj_classname(sm_out));
      return Qnil;
    }
    vec3_t *output = sm_unwrap_vec3(sm_out, NULL);
    quat_multiply_vec3(*self, *rhs, *output);
  }} else if (argc == 1) {
SM_LABEL(skip_output): {
    vec3_t output;
    quat_multiply_vec3(*self, *rhs, output);
    sm_out = sm_wrap_vec3(output, rb_obj_class(sm_rhs));
    rb_obj_call_init(sm_out, 0, 0);
  }} else {
    rb_raise(rb_eArgError, "Invalid number of arguments to multiply_vec3");
  }
  return sm_out;
}



/*
 * Allocates a new Quat.
 *
 * call-seq:
 *    new()               -> new identity quaternion
 *    new(x, y, z, w = 1) -> new quaternion with components [x, y, z, w]
 *    new([x, y, z, w])   -> new quaternion with components [x, y, z, w]
 *    new(quat)           -> copy of quat
 *    new(vec3)           -> new quaternion with the components [vec3.xyz, 1]
 *    new(vec4)           -> new quaternion with the components of vec4
 *    new(mat3)           -> new quaternion from mat3
 *    new(mat4)           -> new quaternion from mat4
 */
static VALUE sm_quat_new(int argc, VALUE *argv, VALUE self)
{
  VALUE sm_quat = sm_wrap_quat(g_quat_identity, self);
  rb_obj_call_init(sm_quat, argc, argv);
  return sm_quat;
}



/*
 * Sets the Quat's components.
 *
 * call-seq:
 *    set(x, y, z, w = 1) -> new quaternion with components [x, y, z, w]
 *    set([x, y, z, w])   -> new quaternion with components [x, y, z, w]
 *    set(quat)           -> copy of quat
 *    set(vec3)           -> new quaternion with the components [vec3.xyz, 1]
 *    set(vec4)           -> new quaternion with the components of vec4
 *    set(mat3)           -> new quaternion from mat3
 *    set(mat4)           -> new quaternion from mat4
 */
static VALUE sm_quat_init(int argc, VALUE *argv, VALUE sm_self)
{
  quat_t *self = sm_unwrap_quat(sm_self, NULL);
  size_t arr_index = 0;

  switch(argc) {

  // Default value
  case 0: { break; }

  // Copy or by-array
  case 1: {
    if (SM_IS_A(argv[0], vec3)) {
      sm_unwrap_vec3(argv[0], *self);
      break;
    }

    if (SM_IS_A(argv[0], quat) ||
        SM_IS_A(argv[0], vec4)) {
      sm_unwrap_quat(argv[0], *self);
      break;
    }

    if (SM_IS_A(argv[0], mat4)) {
      const mat4_t *mat = sm_unwrap_mat4(argv[0], NULL);
      quat_from_mat4(*mat, *self);
      break;
    }

    if (SM_IS_A(argv[0], mat3)) {
      const mat3_t *mat = sm_unwrap_mat3(argv[0], NULL);
      quat_from_mat3(*mat, *self);
      break;
    }

    // Optional offset into array provided
    if (0) {
      case 2:
      arr_index = NUM2SIZET(argv[1]);
    }

    // Array of values
    if (SM_RB_IS_A(argv[0], rb_cArray)) {
      VALUE arrdata = argv[0];
      const size_t arr_end = arr_index + 3;
      s_float_t *vec_elem = *self;
      for (; arr_index < arr_end; ++arr_index, ++vec_elem) {
        *vec_elem = (s_float_t)rb_num2dbl(rb_ary_entry(arrdata, (long)arr_index));
      }
      break;
    }

    rb_raise(rb_eArgError, "Expected either an array of Numerics or a Quat");
    break;
  }

  // W
  case 4: {
    self[0][3] = (s_float_t)rb_num2dbl(argv[3]);
    case 3: // X, Y, Z
    self[0][0] = (s_float_t)rb_num2dbl(argv[0]);
    self[0][1] = (s_float_t)rb_num2dbl(argv[1]);
    self[0][2] = (s_float_t)rb_num2dbl(argv[2]);
    break;
  }

  default: {
    rb_raise(rb_eArgError, "Invalid arguments to Quat.initialize");
    break;
  }
  } // switch (argc)

  return sm_self;
}



/*
 * Returns a string representation of self.
 *
 *    Quat[].to_s     # => "{ 0.0, 0.0, 0.0, 1.0 }"
 *
 * call-seq:
 *    to_s -> string
 */
static VALUE sm_quat_to_s(VALUE self)
{
  const s_float_t *v;
  v = (const s_float_t *)*sm_unwrap_quat(self, NULL);
  return rb_sprintf(
    "{ "
    "%f, %f, %f, %f"
    " }",
    v[0], v[1], v[2], v[3]);
}



/*
 * Returns a quaternion describing a rotation around a given axis.
 *
 * call-seq:
 *    angle_axis(angle_degrees, axis_vec3, output = nil) -> output or new quat
 */
static VALUE sm_quat_angle_axis(int argc, VALUE *argv, VALUE self)
{
  VALUE sm_angle;
  VALUE sm_axis;
  VALUE sm_out;
  s_float_t angle;
  const vec3_t *axis;

  rb_scan_args(argc, argv, "21", &sm_angle, &sm_axis, &sm_out);
  if (!SM_IS_A(sm_axis, vec3) && !SM_IS_A(sm_axis, vec4) && !SM_IS_A(sm_axis, quat)) {
    rb_raise(rb_eTypeError,
      kSM_WANT_THREE_OR_FOUR_FORMAT_LIT,
      rb_obj_classname(sm_axis));
    return Qnil;
  }

  angle = (s_float_t)rb_num2dbl(sm_angle);
  axis = sm_unwrap_vec3(sm_axis, NULL);

  if (SM_IS_A(sm_out, quat) || SM_IS_A(sm_out, vec4)) {
    quat_t *out = sm_unwrap_quat(sm_out, NULL);
    quat_from_angle_axis(angle, (*axis)[0], (*axis)[1], (*axis)[2], *out);
  } else {
    quat_t out;
    quat_from_angle_axis(angle, (*axis)[0], (*axis)[1], (*axis)[2], out);
    sm_out = sm_wrap_quat(out, self);
    rb_obj_call_init(sm_out, 0, 0);
  }

  return sm_out;
}



/*
 * Sets self to the identity quaternion.
 *
 * call-seq:
 *    load_identity -> self
 */
static VALUE sm_quat_identity(VALUE sm_self)
{
  quat_t *self = sm_unwrap_quat(sm_self, NULL);
  quat_identity(*self);
  return sm_self;
}



/*
 * Returns a quaternion interpolated between self and destination using
 * spherical linear interpolation. Alpha is the interpolation value and must be
 * clamped from 0 to 1.
 *
 * call-seq:
 *    slerp(destination, alpha, output = nil) -> output or new quat
 */
static VALUE sm_quat_slerp(int argc, VALUE *argv, VALUE sm_self)
{
  VALUE sm_out;
  VALUE sm_destination;
  VALUE sm_alpha;
  quat_t *destination;
  quat_t *self = sm_unwrap_vec4(sm_self, NULL);
  s_float_t alpha;

  rb_scan_args(argc, argv, "21", &sm_destination, &sm_alpha, &sm_out);
  alpha = rb_num2dbl(sm_alpha);

  if (!SM_IS_A(sm_destination, vec4) && !SM_IS_A(sm_destination, quat)) {
    rb_raise(rb_eTypeError,
      kSM_WANT_FOUR_FORMAT_LIT,
      rb_obj_classname(sm_destination));
    return Qnil;
  }

  destination = sm_unwrap_quat(sm_destination, NULL);

  if ((SM_IS_A(sm_out, vec4) || SM_IS_A(sm_out, quat))) {
    quat_slerp(*self, *destination, alpha, *sm_unwrap_quat(sm_out, NULL));
  } else {
    quat_t out;
    quat_slerp(*self, *destination, alpha, out);
    sm_out = sm_wrap_quat(out, rb_obj_class(sm_self));
    rb_obj_call_init(sm_out, 0, 0);
  }

  return sm_out;
}



/*==============================================================================

  mat4_t functions

==============================================================================*/

static VALUE sm_wrap_mat4(const mat4_t value, VALUE klass)
{
  mat4_t *copy;
  VALUE sm_wrapped = Qnil;
  if (!RTEST(klass)) {
    klass = s_sm_mat4_klass;
  }
  sm_wrapped = Data_Make_Struct(klass, mat4_t, 0, free, copy);
  if (value) {
    mat4_copy(value, *copy);
  }
  return sm_wrapped;
}



static mat4_t *sm_unwrap_mat4(VALUE sm_value, mat4_t store)
{
  mat4_t *value;
  Data_Get_Struct(sm_value, mat4_t, value);
  if(store) mat4_copy(*value, store);
  return value;
}



/*
 * Gets the component of the Mat4 at the given index.
 *
 * call-seq: fetch(index) -> float
 */
static VALUE sm_mat4_fetch (VALUE sm_self, VALUE sm_index)
{
  static const int max_index = sizeof(mat4_t) / sizeof(s_float_t);
  const mat4_t *self = sm_unwrap_mat4(sm_self, NULL);
  int index = NUM2INT(sm_index);
  if (index < 0 || index >= max_index) {
    rb_raise(rb_eRangeError,
      "Index %d is out of bounds, must be from 0 through %d", index, max_index - 1);
  }
  return rb_float_new(self[0][NUM2INT(sm_index)]);
}



/*
 * Sets the Mat4's component at the index to the value.
 *
 * call-seq: store(index, value) -> value
 */
static VALUE sm_mat4_store (VALUE sm_self, VALUE sm_index, VALUE sm_value)
{
  static const int max_index = sizeof(mat4_t) / sizeof(s_float_t);
  mat4_t *self = sm_unwrap_mat4(sm_self, NULL);
  int index = NUM2INT(sm_index);
  if (index < 0 || index >= max_index) {
    rb_raise(rb_eRangeError,
      "Index %d is out of bounds, must be from 0 through %d", index, max_index - 1);
  }
  self[0][index] = (s_float_t)rb_num2dbl(sm_value);
  return sm_value;
}



/*
 * Returns the length in bytes of the Mat4. When compiled to use doubles as the
 * base type, this is always 128. Otherwise, when compiled to use floats, it's
 * always 64.
 *
 * call-seq: size -> fixnum
 */
static VALUE sm_mat4_size (VALUE self)
{
  return SIZET2NUM(sizeof(mat4_t));
}



/*
 * Returns the length of the Mat4 in components. Result is always 16.
 *
 * call-seq: length -> fixnum
 */
static VALUE sm_mat4_length (VALUE self)
{
  return SIZET2NUM(sizeof(mat4_t) / sizeof(s_float_t));
}



/*
 * Returns a copy of self.
 *
 * call-seq:
 *    copy(output = nil) -> output or new mat4
 */
 static VALUE sm_mat4_copy(int argc, VALUE *argv, VALUE sm_self)
 {
   VALUE sm_out;
   mat4_t *self;
   rb_scan_args(argc, argv, "01", &sm_out);
   self = sm_unwrap_mat4(sm_self, NULL);
   if (argc == 1) {
     if (!RTEST(sm_out)) {
       goto SM_LABEL(skip_output);
     }{
     SM_RAISE_IF_NOT_TYPE(sm_out, mat4);
     mat4_t *output = sm_unwrap_mat4(sm_out, NULL);
     mat4_copy (*self, *output);
   }} else if (argc == 0) {
 SM_LABEL(skip_output): {
     mat4_t output;
     mat4_copy (*self, output);
     sm_out = sm_wrap_mat4(output, rb_obj_class(sm_self));
     rb_obj_call_init(sm_out, 0, 0);
   }} else {
     rb_raise(rb_eArgError, "Invalid number of arguments to copy");
   }
   return sm_out;
 }



/*
 * Converts the Mat4 to a Mat3.
 *
 * call-seq:
 *    to_mat3(output = nil) -> output or new mat3
 */
 static VALUE sm_mat4_to_mat3(int argc, VALUE *argv, VALUE sm_self)
 {
   VALUE sm_out;
   mat4_t *self;
   rb_scan_args(argc, argv, "01", &sm_out);
   self = sm_unwrap_mat4(sm_self, NULL);
   if (argc == 1) {
     if (!RTEST(sm_out)) {
       goto SM_LABEL(skip_output);
     }{
     SM_RAISE_IF_NOT_TYPE(sm_out, mat3);
     mat3_t *output = sm_unwrap_mat3(sm_out, NULL);
     mat4_to_mat3 (*self, *output);
   }} else if (argc == 0) {
 SM_LABEL(skip_output): {
     mat3_t output;
     mat4_to_mat3 (*self, output);
     sm_out = sm_wrap_mat3(output, s_sm_mat4_klass);
     rb_obj_call_init(sm_out, 0, 0);
   }} else {
     rb_raise(rb_eArgError, "Invalid number of arguments to to_mat3");
   }
   return sm_out;
 }



/*
 * Transposes this matrix and returns the result.
 *
 * call-seq:
 *    transpose(output = nil) -> output or new mat4
 */
 static VALUE sm_mat4_transpose(int argc, VALUE *argv, VALUE sm_self)
 {
   VALUE sm_out;
   mat4_t *self;
   rb_scan_args(argc, argv, "01", &sm_out);
   self = sm_unwrap_mat4(sm_self, NULL);
   if (argc == 1) {
     if (!RTEST(sm_out)) {
       goto SM_LABEL(skip_output);
     }{
     SM_RAISE_IF_NOT_TYPE(sm_out, mat4);
     mat4_t *output = sm_unwrap_mat4(sm_out, NULL);
     mat4_transpose (*self, *output);
   }} else if (argc == 0) {
 SM_LABEL(skip_output): {
     mat4_t output;
     mat4_transpose (*self, output);
     sm_out = sm_wrap_mat4(output, rb_obj_class(sm_self));
     rb_obj_call_init(sm_out, 0, 0);
   }} else {
     rb_raise(rb_eArgError, "Invalid number of arguments to transpose");
   }
   return sm_out;
 }



/*
 * Returns an inverse orthogonal matrix.
 *
 * call-seq:
 *    inverse_orthogonal(output = nil) -> output or new mat4
 */
 static VALUE sm_mat4_inverse_orthogonal(int argc, VALUE *argv, VALUE sm_self)
 {
   VALUE sm_out;
   mat4_t *self;
   rb_scan_args(argc, argv, "01", &sm_out);
   self = sm_unwrap_mat4(sm_self, NULL);
   if (argc == 1) {
     if (!RTEST(sm_out)) {
       goto SM_LABEL(skip_output);
     }{
     SM_RAISE_IF_NOT_TYPE(sm_out, mat4);
     mat4_t *output = sm_unwrap_mat4(sm_out, NULL);
     mat4_inverse_orthogonal (*self, *output);
   }} else if (argc == 0) {
 SM_LABEL(skip_output): {
     mat4_t output;
     mat4_inverse_orthogonal (*self, output);
     sm_out = sm_wrap_mat4(output, rb_obj_class(sm_self));
     rb_obj_call_init(sm_out, 0, 0);
   }} else {
     rb_raise(rb_eArgError, "Invalid number of arguments to inverse_orthogonal");
   }
   return sm_out;
 }



/*
 * Returns an adjoint matrix.
 *
 * call-seq:
 *    adjoint(output = nil) -> output or new mat4
 */
 static VALUE sm_mat4_adjoint(int argc, VALUE *argv, VALUE sm_self)
 {
   VALUE sm_out;
   mat4_t *self;
   rb_scan_args(argc, argv, "01", &sm_out);
   self = sm_unwrap_mat4(sm_self, NULL);
   if (argc == 1) {
     if (!RTEST(sm_out)) {
       goto SM_LABEL(skip_output);
     }{
     SM_RAISE_IF_NOT_TYPE(sm_out, mat4);
     mat4_t *output = sm_unwrap_mat4(sm_out, NULL);
     mat4_adjoint (*self, *output);
   }} else if (argc == 0) {
 SM_LABEL(skip_output): {
     mat4_t output;
     mat4_adjoint (*self, output);
     sm_out = sm_wrap_mat4(output, rb_obj_class(sm_self));
     rb_obj_call_init(sm_out, 0, 0);
   }} else {
     rb_raise(rb_eArgError, "Invalid number of arguments to adjoint");
   }
   return sm_out;
 }



/*
 * Multiplies this and another Mat4 together and returns the result.
 *
 * call-seq:
 *    multiply_mat4(mat4, output = nil) -> output or new mat4
 */
static VALUE sm_mat4_multiply(int argc, VALUE *argv, VALUE sm_self)
{
  VALUE sm_rhs;
  VALUE sm_out;
  mat4_t *self;
  mat4_t *rhs;
  rb_scan_args(argc, argv, "11", &sm_rhs, &sm_out);
  self = sm_unwrap_mat4(sm_self, NULL);
  SM_RAISE_IF_NOT_TYPE(sm_rhs, mat4);
  rhs = sm_unwrap_mat4(sm_rhs, NULL);
  if (argc == 2) {
    if (!RTEST(sm_out)) {
      goto SM_LABEL(skip_output);
    }{
    SM_RAISE_IF_NOT_TYPE(sm_out, mat4);
    mat4_t *output = sm_unwrap_mat4(sm_out, NULL);
    mat4_multiply(*self, *rhs, *output);
  }} else if (argc == 1) {
SM_LABEL(skip_output): {
    mat4_t output;
    mat4_multiply(*self, *rhs, output);
    sm_out = sm_wrap_mat4(output, rb_obj_class(sm_self));
    rb_obj_call_init(sm_out, 0, 0);
  }} else {
    rb_raise(rb_eArgError, "Invalid number of arguments to multiply_mat4");
  }
  return sm_out;
}



/*
 * Transforms a Vec4 using self and returns the resulting vector.
 *
 * call-seq:
 *    multiply_vec4(vec4, output = nil) -> output or new vec4
 */
static VALUE sm_mat4_multiply_vec4(int argc, VALUE *argv, VALUE sm_self)
{
  VALUE sm_rhs;
  VALUE sm_out;
  mat4_t *self;
  vec4_t *rhs;
  rb_scan_args(argc, argv, "11", &sm_rhs, &sm_out);
  self = sm_unwrap_mat4(sm_self, NULL);
  if (!SM_IS_A(sm_rhs, vec4) && !SM_IS_A(sm_rhs, quat)) {
    rb_raise(rb_eTypeError,
      kSM_WANT_FOUR_FORMAT_LIT,
      rb_obj_classname(sm_rhs));
    return Qnil;
  }
  rhs = sm_unwrap_vec4(sm_rhs, NULL);
  if (argc == 2) {
    if (!RTEST(sm_out)) {
      goto SM_LABEL(skip_output);
    }{
    if (!SM_IS_A(sm_out, vec4) && !SM_IS_A(sm_out, quat)) {
      rb_raise(rb_eTypeError,
        kSM_WANT_FOUR_FORMAT_LIT,
        rb_obj_classname(sm_out));
      return Qnil;
    }
    vec4_t *output = sm_unwrap_vec4(sm_out, NULL);
    mat4_multiply_vec4(*self, *rhs, *output);
  }} else if (argc == 1) {
SM_LABEL(skip_output): {
    vec4_t output;
    mat4_multiply_vec4(*self, *rhs, output);
    sm_out = sm_wrap_vec4(output, rb_obj_class(sm_rhs));
    rb_obj_call_init(sm_out, 0, 0);
  }} else {
    rb_raise(rb_eArgError, "Invalid number of arguments to multiply_vec4");
  }
  return sm_out;
}



/*
 * Transforms a Vec3 using self and returns the resulting vector.
 *
 * call-seq:
 *    transform_vec3(vec3, output = nil) -> output or new vec3
 */
static VALUE sm_mat4_transform_vec3(int argc, VALUE *argv, VALUE sm_self)
{
  VALUE sm_rhs;
  VALUE sm_out;
  mat4_t *self;
  vec3_t *rhs;
  rb_scan_args(argc, argv, "11", &sm_rhs, &sm_out);
  self = sm_unwrap_mat4(sm_self, NULL);
  if (!SM_IS_A(sm_rhs, vec3) && !SM_IS_A(sm_rhs, vec4) && !SM_IS_A(sm_rhs, quat)) {
    rb_raise(rb_eTypeError,
      kSM_WANT_THREE_OR_FOUR_FORMAT_LIT,
      rb_obj_classname(sm_rhs));
    return Qnil;
  }
  rhs = sm_unwrap_vec3(sm_rhs, NULL);
  if (argc == 2) {
    if (!RTEST(sm_out)) {
      goto SM_LABEL(skip_output);
    }{
    if (!SM_IS_A(sm_out, vec3) && !SM_IS_A(sm_out, vec4) && !SM_IS_A(sm_out, quat)) {
      rb_raise(rb_eTypeError,
        kSM_WANT_THREE_OR_FOUR_FORMAT_LIT,
        rb_obj_classname(sm_out));
      return Qnil;
    }
    vec3_t *output = sm_unwrap_vec3(sm_out, NULL);
    mat4_transform_vec3(*self, *rhs, *output);
  }} else if (argc == 1) {
SM_LABEL(skip_output): {
    vec3_t output;
    mat4_transform_vec3(*self, *rhs, output);
    sm_out = sm_wrap_vec3(output, rb_obj_class(sm_rhs));
    rb_obj_call_init(sm_out, 0, 0);
  }} else {
    rb_raise(rb_eArgError, "Invalid number of arguments to transform_vec3");
  }
  return sm_out;
}



/*
 * Rotates a Vec3 by self, using only the inner 9x9 matrix to transform the
 * vector. Returns the rotated vector.
 *
 * call-seq:
 *    rotate_vec3(vec3, output = nil) -> output or new vec3
 */
static VALUE sm_mat4_rotate_vec3(int argc, VALUE *argv, VALUE sm_self)
{
  VALUE sm_rhs;
  VALUE sm_out;
  mat4_t *self;
  vec3_t *rhs;
  rb_scan_args(argc, argv, "11", &sm_rhs, &sm_out);
  self = sm_unwrap_mat4(sm_self, NULL);
  if (!SM_IS_A(sm_rhs, vec3) && !SM_IS_A(sm_rhs, vec4) && !SM_IS_A(sm_rhs, quat)) {
    rb_raise(rb_eTypeError,
      kSM_WANT_THREE_OR_FOUR_FORMAT_LIT,
      rb_obj_classname(sm_rhs));
    return Qnil;
  }
  rhs = sm_unwrap_vec3(sm_rhs, NULL);
  if (argc == 2) {
    if (!RTEST(sm_out)) {
      goto SM_LABEL(skip_output);
    }{
    if (!SM_IS_A(sm_out, vec3) && !SM_IS_A(sm_out, vec4) && !SM_IS_A(sm_out, quat)) {
      rb_raise(rb_eTypeError,
        kSM_WANT_THREE_OR_FOUR_FORMAT_LIT,
        rb_obj_classname(sm_out));
      return Qnil;
    }
    vec3_t *output = sm_unwrap_vec3(sm_out, NULL);
    mat4_rotate_vec3(*self, *rhs, *output);
  }} else if (argc == 1) {
SM_LABEL(skip_output): {
    vec3_t output;
    mat4_rotate_vec3(*self, *rhs, output);
    sm_out = sm_wrap_vec3(output, rb_obj_class(sm_rhs));
    rb_obj_call_init(sm_out, 0, 0);
  }} else {
    rb_raise(rb_eArgError, "Invalid number of arguments to rotate_vec3");
  }
  return sm_out;
}



/*
 * Convenience function to rotate a Vec3 using the inverse of self. Returns the
 * resulting vector.
 *
 * call-seq:
 *    inv_rotate_vec3(vec3, output = nil) -> output or new vec3
 */
static VALUE sm_mat4_inv_rotate_vec3(int argc, VALUE *argv, VALUE sm_self)
{
  VALUE sm_rhs;
  VALUE sm_out;
  mat4_t *self;
  vec3_t *rhs;
  rb_scan_args(argc, argv, "11", &sm_rhs, &sm_out);
  self = sm_unwrap_mat4(sm_self, NULL);
  if (!SM_IS_A(sm_rhs, vec3) && !SM_IS_A(sm_rhs, vec4) && !SM_IS_A(sm_rhs, quat)) {
    rb_raise(rb_eTypeError,
      kSM_WANT_THREE_OR_FOUR_FORMAT_LIT,
      rb_obj_classname(sm_rhs));
    return Qnil;
  }
  rhs = sm_unwrap_vec3(sm_rhs, NULL);
  if (argc == 2) {
    if (!RTEST(sm_out)) {
      goto SM_LABEL(skip_output);
    }{
    if (!SM_IS_A(sm_out, vec3) && !SM_IS_A(sm_out, vec4) && !SM_IS_A(sm_out, quat)) {
      rb_raise(rb_eTypeError,
        kSM_WANT_THREE_OR_FOUR_FORMAT_LIT,
        rb_obj_classname(sm_out));
      return Qnil;
    }
    vec3_t *output = sm_unwrap_vec3(sm_out, NULL);
    mat4_inv_rotate_vec3(*self, *rhs, *output);
  }} else if (argc == 1) {
SM_LABEL(skip_output): {
    vec3_t output;
    mat4_inv_rotate_vec3(*self, *rhs, output);
    sm_out = sm_wrap_vec3(output, rb_obj_class(sm_rhs));
    rb_obj_call_init(sm_out, 0, 0);
  }} else {
    rb_raise(rb_eArgError, "Invalid number of arguments to inverse_rotate_vec3");
  }
  return sm_out;
}



/*
 * Returns an inverse affine matrix if successful. Otherwise, returns nil.
 *
 * call-seq:
 *    inverse_affine(output = nil) -> output, new mat4, or nil
 */
static VALUE sm_mat4_inverse_affine(int argc, VALUE *argv, VALUE sm_self)
{
  VALUE sm_out = Qnil;
  mat4_t *self;

  rb_scan_args(argc, argv, "01", &sm_out);
  self = sm_unwrap_mat4(sm_self, NULL);

  if (argc == 1) {
    mat4_t *output;

    if (!RTEST(sm_out)) {
      goto SM_LABEL(output_lbl);
    }

    if (!SM_IS_A(sm_out, mat4)) {
      rb_raise(rb_eTypeError,
        "Invalid argument to output of inverse_affine: expected %s, got %s",
        rb_class2name(s_sm_mat4_klass),
        rb_obj_classname(sm_out));
      return Qnil;
    }

    output = sm_unwrap_mat4(sm_out, NULL);
    if (!mat4_inverse_affine(*self, *output)) {
      return Qnil;
    }

  } else if (argc == 0) {
    SM_LABEL(output_lbl): {
      mat4_t output;
      if (!mat4_inverse_affine(*self, output)) {
        return Qnil;
      }

      sm_out = sm_wrap_mat4(output, rb_obj_class(sm_self));
      rb_obj_call_init(sm_out, 0, 0);
    }
  } else {
    rb_raise(rb_eArgError, "Invalid number of arguments to inverse_affine");
  }

  return sm_out;
}



/*
 * Returns an generalized inverse matrix if successful. Otherwise, returns nil.
 *
 * call-seq:
 *    inverse_general(output = nil) -> output, new mat4, or nil
 */
static VALUE sm_mat4_inverse_general(int argc, VALUE *argv, VALUE sm_self)
{
  VALUE sm_out = Qnil;
  mat4_t *self;

  rb_scan_args(argc, argv, "01", &sm_out);
  self = sm_unwrap_mat4(sm_self, NULL);

  if (argc == 1) {
    mat4_t *output;

    if (!RTEST(sm_out)) {
      goto SM_LABEL(skip_output);
    }

    if (!SM_IS_A(sm_out, mat4)) {
      rb_raise(rb_eTypeError,
        "Invalid argument to output of inverse_general: expected %s, got %s",
        rb_class2name(s_sm_mat4_klass),
        rb_obj_classname(sm_out));
      return Qnil;
    }

    output = sm_unwrap_mat4(sm_out, NULL);
    if (!mat4_inverse_general(*self, *output)) {
      return Qnil;
    }

  } else if (argc == 0) {
    SM_LABEL(skip_output): {
      mat4_t output;
      if (!mat4_inverse_general(*self, output)) {
        return Qnil;
      }

      sm_out = sm_wrap_mat4(output, rb_obj_class(sm_self));
      rb_obj_call_init(sm_out, 0, 0);
    }
  } else {
    rb_raise(rb_eArgError, "Invalid number of arguments to inverse_general");
  }

  return sm_out;
}



/*
 * Returns the matrix determinant.
 *
 * call-seq:
 *    determinant -> float
 */
static VALUE sm_mat4_determinant(VALUE sm_self)
{
  return mat4_determinant(*sm_unwrap_mat4(sm_self, NULL));
}



/*
 * Translates this matrix by X, Y, and Z (or a Vec3's X, Y, and Z components)
 * and returns the result. Essentially the same as multiplying this matrix by a
 * translation matrix, but slightly more convenient.
 *
 * call-seq:
 *    translate(x, y, z, output = nil) -> output or new mat4
 *    translate(vec3, output = nil)    -> output or new mat4
 */
static VALUE sm_mat4_translate(int argc, VALUE *argv, VALUE sm_self)
{
  VALUE sm_out = Qnil;
  mat4_t *self = sm_unwrap_mat4(sm_self, NULL);
  vec3_t xyz;

  SM_LABEL(argc_reconfig):
  switch (argc) {
  case 2: case 4: {
    sm_out = argv[--argc];
    if (RTEST(sm_out)) {
      SM_RAISE_IF_NOT_TYPE(sm_out, mat4);
    }
    goto SM_LABEL(argc_reconfig);
  }

  case 1: {
    sm_unwrap_vec3(argv[0], xyz);
    goto SM_LABEL(get_output);
  }

  case 3: {
    xyz[0] = rb_num2dbl(argv[0]);
    xyz[1] = rb_num2dbl(argv[1]);
    xyz[2] = rb_num2dbl(argv[2]);

    SM_LABEL(get_output):
    if (RTEST(sm_out)) {
      mat4_t *out = sm_unwrap_mat4(sm_out, NULL);
      mat4_translate(xyz[0], xyz[1], xyz[2], *self, *out);
    } else {
      mat4_t out;
      mat4_translate(xyz[0], xyz[1], xyz[2], *self, out);
      sm_out = sm_wrap_mat4(out, rb_obj_class(sm_self));
      rb_obj_call_init(sm_out, 0, 0);
    }
  }
  }

  return sm_out;
}



/*
 * Returns a translation matrix for the given X, Y, and Z translations (or using
 * the vector's components as such).
 *
 * call-seq:
 *    translation(x, y, z, output = nil) -> output or new mat4
 *    translation(vec3, output = nil)    -> output or new mat4
 */
static VALUE sm_mat4_translation(int argc, VALUE *argv, VALUE sm_self)
{
  VALUE sm_out = Qnil;
  vec3_t xyz;

  SM_LABEL(argc_reconfig):
  switch (argc) {
  case 2: case 4: {
    sm_out = argv[--argc];
    if (RTEST(sm_out)) {
      SM_RAISE_IF_NOT_TYPE(sm_out, mat4);
    }
    goto SM_LABEL(argc_reconfig);
  }

  case 1: {
    sm_unwrap_vec3(argv[0], xyz);
    goto SM_LABEL(get_output);
  }

  case 3: {
    xyz[0] = rb_num2dbl(argv[0]);
    xyz[1] = rb_num2dbl(argv[1]);
    xyz[2] = rb_num2dbl(argv[2]);

    SM_LABEL(get_output):
    if (RTEST(sm_out)) {
      mat4_t *out = sm_unwrap_mat4(sm_out, NULL);
      mat4_translation(xyz[0], xyz[1], xyz[2], *out);
    } else {
      mat4_t out;
      mat4_translation(xyz[0], xyz[1], xyz[2], out);
      sm_out = sm_wrap_mat4(out, sm_self);
      rb_obj_call_init(sm_out, 0, 0);
    }
  }
  }

  return sm_out;
}



/*
 * Allocates a new Mat4.
 *
 * call-seq:
 *    new()                        -> identity mat4
 *    new(m1, m2, ..., m15, m16)   -> new mat4 with components
 *    new([m1, m2, ..., m15, m16]) -> new mat4 with components
 *    new(mat4)                    -> copy of mat4
 *    new(mat3)                    -> new mat4 with mat3's components
 *    new(quat)                    -> quat as mat4
 *    new(Vec4, Vec4, Vec4, Vec4)  -> new mat4 with given row vectors
 */
static VALUE sm_mat4_new(int argc, VALUE *argv, VALUE self)
{
  VALUE sm_mat = sm_wrap_mat4(g_mat4_identity, self);
  rb_obj_call_init(sm_mat, argc, argv);
  return sm_mat;
}



/*
 * Sets the Mat4's components.
 *
 * call-seq:
 *    set(m1, m2, ..., m15, m16)   -> new mat4 with components
 *    set([m1, m2, ..., m15, m16]) -> new mat4 with components
 *    set(mat4)                    -> copy of mat4
 *    set(mat3)                    -> new mat4 with mat3's components
 *    set(quat)                    -> quat as mat4
 *    set(Vec4, Vec4, Vec4, Vec4)  -> new mat4 with given row vectors
 */
static VALUE sm_mat4_init(int argc, VALUE *argv, VALUE sm_self)
{
  mat4_t *self = sm_unwrap_mat4(sm_self, NULL);
  size_t arr_index = 0;

  switch (argc) {

  case 0: {
    // Identity (handled in _new)
    break;
  }

  // Copy Mat4 or provided [Numeric..]
  case 1: {
    // Copy Mat4
    if (SM_IS_A(argv[0], mat4)) {
      sm_unwrap_mat4(argv[0], *self);
      break;
    }

    // Copy Mat3
    if (SM_IS_A(argv[0], mat3)) {
      mat3_to_mat4(*sm_unwrap_mat4(argv[0], NULL), *self);
      break;
    }

    // Build from Quaternion
    if (SM_IS_A(argv[0], quat)) {
      mat4_from_quat(*sm_unwrap_quat(argv[0], NULL), *self);
      break;
    }

    // Optional offset into array provided
    if (0) {
      case 2:
      arr_index = NUM2SIZET(argv[1]);
    }

    // Array of values
    if (SM_RB_IS_A(argv[0], rb_cArray)) {
      VALUE arrdata = argv[0];
      const size_t arr_end = arr_index + 16;
      s_float_t *mat_elem = *self;
      for (; arr_index < arr_end; ++arr_index, ++mat_elem) {
        *mat_elem = rb_num2dbl(rb_ary_entry(arrdata, (long)arr_index));
      }
      break;
    }

    rb_raise(rb_eArgError, "Expected either an array of Numerics or a Mat4");
    break;
  }

  // Mat4(Vec4, Vec4, Vec4, Vec4)
  case 4: {
    size_t arg_index;
    s_float_t *mat_elem = *self;
    for (arg_index = 0; arg_index < 4; ++arg_index, mat_elem += 4) {
      if (!SM_IS_A(argv[arg_index], vec4) && !SM_IS_A(argv[arg_index], quat)) {
        rb_raise(
          rb_eArgError,
          "Argument %d must be a Vec4 or Quat when supplying four arguments to Mat4.initialize/set",
          (int)(arg_index + 1));
      }

      sm_unwrap_vec4(argv[arg_index], mat_elem);
    }
    break;
  }

  // Mat4(Numeric m00 .. m16)
  case 16: {
    s_float_t *mat_elem = *self;
    VALUE *argv_p = argv;
    for (; argc; --argc, ++argv_p, ++mat_elem) {
      *mat_elem = (s_float_t)rb_num2dbl(*argv_p);
    }
    break;
  }

  default: {
    rb_raise(rb_eArgError, "Invalid arguments to Mat4.initialize");
    break;
  }
  } // swtich (argc)

  return sm_self;
}



/*
 * Returns a string representation of self.
 *
 *    Mat4[].to_s     # => "{ 1.0, 0.0, 0.0, 0.0,\n
 *                    #       0.0, 1.0, 0.0, 0.0,\n"
 *                    #       0.0, 0.0, 1.0, 0.0,\n"
 *                    #       0.0, 0.0, 0.0, 1.0 }"
 *
 * call-seq:
 *    to_s -> string
 */
static VALUE sm_mat4_to_s(VALUE self)
{
  const s_float_t *v;
  v = (const s_float_t *)*sm_unwrap_mat4(self, NULL);
  return rb_sprintf(
    "{ "
    "%f, %f, %f, %f" ",\n  "
    "%f, %f, %f, %f" ",\n  "
    "%f, %f, %f, %f" ",\n  "
    "%f, %f, %f, %f"
    " }",
    v[0], v[1], v[2], v[3],
    v[4], v[5], v[6], v[7],
    v[8], v[9], v[10], v[11],
    v[12], v[13], v[14], v[15]);
}



/*
 * Returns a Mat4 describing a rotation around an axis.
 *
 * call-seq:
 *    angle_axis(angle_degrees, axis_vec3, output = nil) -> output or new mat4
 */
static VALUE sm_mat4_angle_axis(int argc, VALUE *argv, VALUE self)
{
  VALUE sm_angle;
  VALUE sm_axis;
  VALUE sm_out;
  s_float_t angle;
  const vec3_t *axis;

  rb_scan_args(argc, argv, "21", &sm_angle, &sm_axis, &sm_out);
  if (!SM_IS_A(sm_axis, vec3) && !SM_IS_A(sm_axis, vec4) && !SM_IS_A(sm_axis, quat)) {
    rb_raise(rb_eTypeError,
      kSM_WANT_THREE_OR_FOUR_FORMAT_LIT,
      rb_obj_classname(sm_axis));
    return Qnil;
  }

  angle = (s_float_t)rb_num2dbl(sm_angle);
  axis = sm_unwrap_vec3(sm_axis, NULL);

  if (SM_IS_A(sm_out, mat4)) {
    mat4_t *out = sm_unwrap_mat4(sm_out, NULL);
    mat4_rotation(angle, (*axis)[0], (*axis)[1], (*axis)[2], *out);
  } else {
    mat4_t out;
    mat4_rotation(angle, (*axis)[0], (*axis)[1], (*axis)[2], out);
    sm_out = sm_wrap_mat4(out, self);
    rb_obj_call_init(sm_out, 0, 0);
  }

  return sm_out;
}



/*
 * Returns a Vec3 whose components are that of the row at the given index.
 *
 * call-seq:
 *    get_row3(index, output = nil) -> output or new vec3
 */
static VALUE sm_mat4_get_row3(int argc, VALUE *argv, VALUE sm_self)
{
  mat4_t *self;
  int index;
  VALUE sm_out;

  self = sm_unwrap_mat4(sm_self, NULL);
  index = NUM2INT(argv[0]);
  sm_out = Qnil;

  if (index < 0 || index > 3) {
    rb_raise(rb_eRangeError, "Index %d is out of range, must be (0 .. 3)", index);
    return Qnil;
  }

  switch (argc) {
  case 2: {
    vec3_t *out;

    sm_out = argv[1];

    if (RTEST(sm_out)) {
      if (!SM_IS_A(sm_out, vec3) && !SM_IS_A(sm_out, vec4) && !SM_IS_A(sm_out, quat)) {
        rb_raise(rb_eTypeError,
          kSM_WANT_THREE_OR_FOUR_FORMAT_LIT,
          rb_obj_classname(sm_out));
        return Qnil;
      }
    } else {
      goto SM_LABEL(no_output);
    }

    out = sm_unwrap_vec3(sm_out, NULL);
    mat4_get_row3(*self, index, *out);

    break;
  }

  case 1: SM_LABEL(no_output): {
    vec3_t out;
    mat4_get_row3(*self, index, out);
    sm_out = sm_wrap_vec3(out, Qnil);
    rb_obj_call_init(sm_out, 0, 0);
    break;
  }

  default: {
    rb_raise(rb_eArgError, "Invalid number of arguments to get_row3 - expected 1 or 2");
    break;
  }
  }

  return sm_out;
}



/*
 * Returns a Vec4 whose components are that of the row at the given index.
 *
 * call-seq:
 *    get_row4(index, output = nil) -> output or new vec4
 */
static VALUE sm_mat4_get_row4(int argc, VALUE *argv, VALUE sm_self)
{
  mat4_t *self;
  int index;
  VALUE sm_out;

  self = sm_unwrap_mat4(sm_self, NULL);
  index = NUM2INT(argv[0]);
  sm_out = Qnil;

  if (index < 0 || index > 3) {
    rb_raise(rb_eRangeError, "Index %d is out of range, must be (0 .. 3)", index);
    return Qnil;
  }

  switch (argc) {
  case 2: {
    vec4_t *out;

    sm_out = argv[1];

    if (RTEST(sm_out)) {
      if (!SM_IS_A(sm_out, vec4) && !SM_IS_A(sm_out, quat)) {
        rb_raise(rb_eTypeError,
          kSM_WANT_FOUR_FORMAT_LIT,
          rb_obj_classname(sm_out));
        return Qnil;
      }
    } else {
      goto SM_LABEL(no_output);
    }

    out = sm_unwrap_vec4(sm_out, NULL);
    mat4_get_row4(*self, index, *out);

    break;
  }

  case 1: SM_LABEL(no_output): {
    vec4_t out;
    mat4_get_row4(*self, index, out);
    sm_out = sm_wrap_vec4(out, Qnil);
    rb_obj_call_init(sm_out, 0, 0);
    break;
  }

  default: {
    rb_raise(rb_eArgError, "Invalid number of arguments to get_row4 - expected 1 or 2");
    break;
  }
  }

  return sm_out;
}



/*
 * Returns a Vec3 whose components are that of the column at the given index.
 *
 * call-seq:
 *    get_column3(index, output = nil) -> output or new vec3
 */
static VALUE sm_mat4_get_column3(int argc, VALUE *argv, VALUE sm_self)
{
  mat4_t *self;
  int index;
  VALUE sm_out;

  self = sm_unwrap_mat4(sm_self, NULL);
  index = NUM2INT(argv[0]);
  sm_out = Qnil;

  if (index < 0 || index > 3) {
    rb_raise(rb_eRangeError, "Index %d is out of range, must be (0 .. 3)", index);
    return Qnil;
  }

  switch (argc) {
  case 2: {
    vec3_t *out;

    sm_out = argv[1];

    if (RTEST(sm_out)) {
      if (!SM_IS_A(sm_out, vec3) && !SM_IS_A(sm_out, vec4) && !SM_IS_A(sm_out, quat)) {
        rb_raise(rb_eTypeError,
          kSM_WANT_THREE_OR_FOUR_FORMAT_LIT,
          rb_obj_classname(sm_out));
        return Qnil;
      }
    } else {
      goto SM_LABEL(no_output);
    }

    out = sm_unwrap_vec3(sm_out, NULL);
    mat4_get_column3(*self, index, *out);

    break;
  }

  case 1: SM_LABEL(no_output): {
    vec3_t out;
    mat4_get_column3(*self, index, out);
    sm_out = sm_wrap_vec3(out, Qnil);
    rb_obj_call_init(sm_out, 0, 0);
    break;
  }

  default: {
    rb_raise(rb_eArgError, "Invalid number of arguments to get_column3 - expected 1 or 2");
    break;
  }
  }

  return sm_out;
}



/*
 * Returns a Vec4 whose components are that of the column at the given index.
 *
 * call-seq:
 *    get_column4(index, output = nil) -> output or new vec4
 */
static VALUE sm_mat4_get_column4(int argc, VALUE *argv, VALUE sm_self)
{
  mat4_t *self;
  int index;
  VALUE sm_out;

  self = sm_unwrap_mat4(sm_self, NULL);
  index = NUM2INT(argv[0]);
  sm_out = Qnil;

  if (index < 0 || index > 3) {
    rb_raise(rb_eRangeError, "Index %d is out of range, must be (0 .. 3)", index);
    return Qnil;
  }

  switch (argc) {
  case 2: {
    vec4_t *out;

    sm_out = argv[1];

    if (RTEST(sm_out)) {
      if (!SM_IS_A(sm_out, vec4) && !SM_IS_A(sm_out, quat)) {
        rb_raise(rb_eTypeError,
          kSM_WANT_FOUR_FORMAT_LIT,
          rb_obj_classname(sm_out));
        return Qnil;
      }
    } else {
      goto SM_LABEL(no_output);
    }

    out = sm_unwrap_vec4(sm_out, NULL);
    mat4_get_column4(*self, index, *out);

    break;
  }

  case 1: SM_LABEL(no_output): {
    vec4_t out;
    mat4_get_column4(*self, index, out);
    sm_out = sm_wrap_vec4(out, Qnil);
    rb_obj_call_init(sm_out, 0, 0);
    break;
  }

  default: {
    rb_raise(rb_eArgError, "Invalid number of arguments to get_column4 - expected 1 or 2");
    break;
  }
  }

  return sm_out;
}



/*
 * Sets the matrix's row at the given index to the given vector.
 *
 * call-seq:
 *    set_row3(index, vec3) -> self
 */
static VALUE sm_mat4_set_row3(VALUE sm_self, VALUE sm_index, VALUE sm_value)
{
  const vec3_t *value;
  int index;
  mat4_t *self;

  if (!SM_IS_A(sm_value, vec3) && !SM_IS_A(sm_value, vec4) && !SM_IS_A(sm_value, quat)) {
    rb_raise(rb_eTypeError,
      kSM_WANT_THREE_OR_FOUR_FORMAT_LIT,
      rb_obj_classname(sm_value));
    return Qnil;
  }

  self = sm_unwrap_mat4(sm_self, NULL);
  value = sm_unwrap_vec3(sm_value, NULL);
  index = NUM2INT(sm_index);

  if (index < 0 || index > 3) {
    rb_raise(rb_eRangeError, "Index %d is out of range, must be (0 .. 3)", index);
    return Qnil;
  }

  mat4_set_row3(index, *value, *self);

  return sm_self;
}



/*
 * Sets the matrix's column at the given index to the given vector.
 *
 * call-seq:
 *    set_column3(index, vec3) -> self
 */
static VALUE sm_mat4_set_column3(VALUE sm_self, VALUE sm_index, VALUE sm_value)
{
  const vec3_t *value;
  int index;
  mat4_t *self;

  if (!SM_IS_A(sm_value, vec3) && !SM_IS_A(sm_value, vec4) && !SM_IS_A(sm_value, quat)) {
    rb_raise(rb_eTypeError,
      kSM_WANT_THREE_OR_FOUR_FORMAT_LIT,
      rb_obj_classname(sm_value));
    return Qnil;
  }

  self = sm_unwrap_mat4(sm_self, NULL);
  value = sm_unwrap_vec3(sm_value, NULL);
  index = NUM2INT(sm_index);

  if (index < 0 || index > 3) {
    rb_raise(rb_eRangeError, "Index %d is out of range, must be (0 .. 3)", index);
    return Qnil;
  }

  mat4_set_column3(index, *value, *self);

  return sm_self;
}



/*
 * Sets the matrix's row at the given index to the given vector.
 *
 * call-seq:
 *    set_row4(index, vec4) -> self
 */
static VALUE sm_mat4_set_row4(VALUE sm_self, VALUE sm_index, VALUE sm_value)
{
  const vec4_t *value;
  int index;
  mat4_t *self;

  if (!SM_IS_A(sm_value, vec4) && !SM_IS_A(sm_value, quat)) {
    rb_raise(rb_eTypeError,
      kSM_WANT_FOUR_FORMAT_LIT,
      rb_obj_classname(sm_value));
    return Qnil;
  }

  self = sm_unwrap_mat4(sm_self, NULL);
  value = sm_unwrap_vec4(sm_value, NULL);
  index = NUM2INT(sm_index);

  if (index < 0 || index > 3) {
    rb_raise(rb_eRangeError, "Index %d is out of range, must be (0 .. 3)", index);
    return Qnil;
  }

  mat4_set_row4(index, *value, *self);

  return sm_self;
}



/*
 * Sets the matrix's column at the given index to the given vector.
 *
 * call-seq:
 *    set_column4(index, vec4) -> self
 */
static VALUE sm_mat4_set_column4(VALUE sm_self, VALUE sm_index, VALUE sm_value)
{
  const vec4_t *value;
  int index;
  mat4_t *self;

  if (!SM_IS_A(sm_value, vec4) && !SM_IS_A(sm_value, quat)) {
      rb_raise(rb_eTypeError,
        kSM_WANT_FOUR_FORMAT_LIT,
        rb_obj_classname(sm_value));
      return Qnil;
    }

  self = sm_unwrap_mat4(sm_self, NULL);
  value = sm_unwrap_vec4(sm_value, NULL);
  index = NUM2INT(sm_index);

  if (index < 0 || index > 3) {
    rb_raise(rb_eRangeError, "Index %d is out of range, must be (0 .. 3)", index);
    return Qnil;
  }

  mat4_set_column4(index, *value, *self);

  return sm_self;
}



/*
 * Sets self to the identity matrix.
 *
 * call-seq:
 *    load_identity -> self
 */
static VALUE sm_mat4_identity(VALUE sm_self)
{
  mat4_t *self = sm_unwrap_mat4(sm_self, NULL);
  mat4_identity(*self);
  return sm_self;
}



/*
 * Returns a matrix describing a frustum perspective.
 *
 * call-seq:
 *    frustum(left, right, bottom, top, z_near, z_far, output = nil) -> output or new mat4
 */
static VALUE sm_mat4_frustum(int argc, VALUE *argv, VALUE self)
{
  VALUE sm_left;
  VALUE sm_right;
  VALUE sm_bottom;
  VALUE sm_top;
  VALUE sm_z_near;
  VALUE sm_z_far;
  VALUE sm_out;
  s_float_t left;
  s_float_t right;
  s_float_t bottom;
  s_float_t top;
  s_float_t z_near;
  s_float_t z_far;

  rb_scan_args(argc, argv, "61", &sm_left, &sm_right, &sm_bottom, &sm_top, &sm_z_near, &sm_z_far, &sm_out);

  left = (s_float_t)rb_num2dbl(sm_left);
  right = (s_float_t)rb_num2dbl(sm_right);
  bottom = (s_float_t)rb_num2dbl(sm_bottom);
  top = (s_float_t)rb_num2dbl(sm_top);
  z_near = (s_float_t)rb_num2dbl(sm_z_near);
  z_far = (s_float_t)rb_num2dbl(sm_z_far);

  if (SM_IS_A(sm_out, mat4)) {
    mat4_t *out = sm_unwrap_mat4(sm_out, NULL);
    mat4_frustum(left, right, bottom, top, z_near, z_far, *out);
  } else {
    mat4_t out;
    mat4_frustum(left, right, bottom, top, z_near, z_far, out);
    sm_out = sm_wrap_mat4(out, Qnil);
    rb_obj_call_init(sm_out, 0, 0);
  }

  return sm_out;
}



/*
 * Returns a matrix describing an orthographic projection.
 *
 * call-seq:
 *    orthographic(left, right, bottom, top, z_near, z_far, output = nil) -> output or new mat4
 */
static VALUE sm_mat4_orthographic(int argc, VALUE *argv, VALUE self)
{
  VALUE sm_left;
  VALUE sm_right;
  VALUE sm_bottom;
  VALUE sm_top;
  VALUE sm_z_near;
  VALUE sm_z_far;
  VALUE sm_out;
  s_float_t left;
  s_float_t right;
  s_float_t bottom;
  s_float_t top;
  s_float_t z_near;
  s_float_t z_far;

  rb_scan_args(argc, argv, "61", &sm_left, &sm_right, &sm_bottom, &sm_top, &sm_z_near, &sm_z_far, &sm_out);

  left = (s_float_t)rb_num2dbl(sm_left);
  right = (s_float_t)rb_num2dbl(sm_right);
  bottom = (s_float_t)rb_num2dbl(sm_bottom);
  top = (s_float_t)rb_num2dbl(sm_top);
  z_near = (s_float_t)rb_num2dbl(sm_z_near);
  z_far = (s_float_t)rb_num2dbl(sm_z_far);

  if (SM_IS_A(sm_out, mat4)) {
    mat4_t *out = sm_unwrap_mat4(sm_out, NULL);
    mat4_orthographic(left, right, bottom, top, z_near, z_far, *out);
  } else {
    mat4_t out;
    mat4_orthographic(left, right, bottom, top, z_near, z_far, out);
    sm_out = sm_wrap_mat4(out, self);
    rb_obj_call_init(sm_out, 0, 0);
  }

  return sm_out;
}



/*
 * Returns a matrix describing a perspective projection.
 *
 * call-seq:
 *    perspective(fov_y_degrees, aspect, z_near, z_far, output = nil) -> output or new mat4
 */
static VALUE sm_mat4_perspective(int argc, VALUE *argv, VALUE self)
{
  VALUE sm_fov_y;
  VALUE sm_aspect;
  VALUE sm_z_near;
  VALUE sm_z_far;
  VALUE sm_out;
  s_float_t fov_y;
  s_float_t aspect;
  s_float_t z_near;
  s_float_t z_far;

  rb_scan_args(argc, argv, "41", &sm_fov_y, &sm_aspect, &sm_z_near, &sm_z_far, &sm_out);

  fov_y = (s_float_t)rb_num2dbl(sm_fov_y);
  aspect = (s_float_t)rb_num2dbl(sm_aspect);
  z_near = (s_float_t)rb_num2dbl(sm_z_near);
  z_far = (s_float_t)rb_num2dbl(sm_z_far);

  if (SM_IS_A(sm_out, mat4)) {
    mat4_t *out = sm_unwrap_mat4(sm_out, NULL);
    mat4_perspective(fov_y, aspect, z_near, z_far, *out);
  } else {
    mat4_t out;
    mat4_perspective(fov_y, aspect, z_near, z_far, out);
    sm_out = sm_wrap_mat4(out, self);
    rb_obj_call_init(sm_out, 0, 0);
  }

  return sm_out;
}



/*
 * Returns a matrix describing a view transformation for an eye looking at
 * center with the given up vector.
 *
 * call-seq:
 *    look_at(eye, center, up, output = nil) -> output or new mat4
 */
static VALUE sm_mat4_look_at(int argc, VALUE *argv, VALUE self)
{
  VALUE sm_eye;
  VALUE sm_center;
  VALUE sm_up;
  VALUE sm_out;
  const vec3_t *eye;
  const vec3_t *center;
  const vec3_t *up;

  rb_scan_args(argc, argv, "31", &sm_eye, &sm_center, &sm_up, &sm_out);

  eye = sm_unwrap_vec3(sm_eye, NULL);
  center = sm_unwrap_vec3(sm_center, NULL);
  up = sm_unwrap_vec3(sm_up, NULL);

  if (SM_IS_A(sm_out, mat4)) {
    mat4_t *out = sm_unwrap_mat4(sm_out, NULL);
    mat4_look_at(*eye, *center, *up, *out);
  } else {
    mat4_t out;
    mat4_look_at(*eye, *center, *up, out);
    sm_out = sm_wrap_mat4(out, self);
    rb_obj_call_init(sm_out, 0, 0);
  }

  return sm_out;
}



/*
 * Scales the inner 9x9 matrix's columns by X, Y, and Z and returns the result.
 *
 * call-seq:
 *    scale(x, y, z, output = nil) -> output or new mat4
 */
static VALUE sm_mat4_scale(int argc, VALUE *argv, VALUE sm_self)
{
  VALUE sm_out;
  VALUE sm_x, sm_y, sm_z;
  s_float_t x, y, z;
  mat4_t *self = sm_unwrap_mat4(sm_self, NULL);

  rb_scan_args(argc, argv, "31", &sm_x, &sm_y, &sm_z, &sm_out);
  x = rb_num2dbl(sm_x);
  y = rb_num2dbl(sm_y);
  z = rb_num2dbl(sm_z);

  if (SM_IS_A(sm_out, mat4)) {
    mat4_scale(*self, x, y, z, *sm_unwrap_mat4(sm_out, NULL));
  } else {
    mat4_t out;
    mat4_scale(*self, x, y, z, out);
    sm_out = sm_wrap_mat4(out, rb_obj_class(sm_self));
    rb_obj_call_init(sm_out, 0, 0);
  }

  return sm_out;
}



/*
 * Tests this Mat4 and another Mat4 for equivalency.
 *
 * call-seq:
 *    mat4 == other_mat4 -> bool
 */
static VALUE sm_mat4_equals(VALUE sm_self, VALUE sm_other)
{
  if (!RTEST(sm_other) || !SM_IS_A(sm_other, mat4)) {
    return Qfalse;
  }

  return mat4_equals(*sm_unwrap_mat4(sm_self, NULL), *sm_unwrap_mat4(sm_other, NULL)) ? Qtrue : Qfalse;
}



/*==============================================================================

  mat3_t functions

==============================================================================*/

static VALUE sm_wrap_mat3(const mat3_t value, VALUE klass)
{
  mat3_t *copy;
  VALUE sm_wrapped = Qnil;
  if (!RTEST(klass)) {
    klass = s_sm_mat3_klass;
  }
  sm_wrapped = Data_Make_Struct(klass, mat3_t, 0, free, copy);
  if (value) {
    mat3_copy(value, *copy);
  }
  return sm_wrapped;
}



static mat3_t *sm_unwrap_mat3(VALUE sm_value, mat3_t store)
{
  mat3_t *value;
  Data_Get_Struct(sm_value, mat3_t, value);
  if(store) mat3_copy(*value, store);
  return value;
}



/*
 * Gets the component of the Mat3 at the given index.
 *
 * call-seq: fetch(index) -> float
 */
static VALUE sm_mat3_fetch (VALUE sm_self, VALUE sm_index)
{
  static const int max_index = sizeof(mat3_t) / sizeof(s_float_t);
  const mat3_t *self = sm_unwrap_mat3(sm_self, NULL);
  int index = NUM2INT(sm_index);
  if (index < 0 || index >= max_index) {
    rb_raise(rb_eRangeError,
      "Index %d is out of bounds, must be from 0 through %d", index, max_index - 1);
  }
  return rb_float_new(self[0][NUM2INT(sm_index)]);
}



/*
 * Sets the Mat3's component at the index to the value.
 *
 * call-seq: store(index, value) -> value
 */
static VALUE sm_mat3_store (VALUE sm_self, VALUE sm_index, VALUE sm_value)
{
  static const int max_index = sizeof(mat3_t) / sizeof(s_float_t);
  mat3_t *self = sm_unwrap_mat3(sm_self, NULL);
  int index = NUM2INT(sm_index);
  if (index < 0 || index >= max_index) {
    rb_raise(rb_eRangeError,
      "Index %d is out of bounds, must be from 0 through %d", index, max_index - 1);
  }
  self[0][index] = (s_float_t)rb_num2dbl(sm_value);
  return sm_value;
}



/*
 * Returns the length in bytes of the Mat3. When compiled to use doubles as the
 * base type, this is always 72. Otherwise, when compiled to use floats, it's
 * always 36.
 *
 * call-seq: size -> fixnum
 */
static VALUE sm_mat3_size (VALUE self)
{
  return SIZET2NUM(sizeof(mat3_t));
}



/*
 * Returns the length of the Mat3 in components. Result is always 9.
 *
 * call-seq: length -> fixnum
 */
static VALUE sm_mat3_length (VALUE self)
{
  return SIZET2NUM(sizeof(mat3_t) / sizeof(s_float_t));
}



/*
 * Returns a copy of self.
 *
 * call-seq:
 *    copy(output = nil) -> output or new mat3
 */
 static VALUE sm_mat3_copy(int argc, VALUE *argv, VALUE sm_self)
 {
   VALUE sm_out;
   mat3_t *self;
   rb_scan_args(argc, argv, "01", &sm_out);
   self = sm_unwrap_mat3(sm_self, NULL);
   if (argc == 1) {
     if (!RTEST(sm_out)) {
       goto SM_LABEL(skip_output);
     }{
     SM_RAISE_IF_NOT_TYPE(sm_out, mat3);
     mat3_t *output = sm_unwrap_mat3(sm_out, NULL);
     mat3_copy (*self, *output);
   }} else if (argc == 0) {
 SM_LABEL(skip_output): {
     mat3_t output;
     mat3_copy (*self, output);
     sm_out = sm_wrap_mat3(output, rb_obj_class(sm_self));
     rb_obj_call_init(sm_out, 0, 0);
   }} else {
     rb_raise(rb_eArgError, "Invalid number of arguments to copy");
   }
   return sm_out;
 }



/*
 * Returns a Mat4 converted from the Mat3.
 *
 * call-seq:
 *    to_mat4(output = nil) -> output or new mat4
 */
 static VALUE sm_mat3_to_mat4(int argc, VALUE *argv, VALUE sm_self)
 {
   VALUE sm_out;
   mat3_t *self;
   rb_scan_args(argc, argv, "01", &sm_out);
   self = sm_unwrap_mat3(sm_self, NULL);
   if (argc == 1) {
     if (!RTEST(sm_out)) {
       goto SM_LABEL(skip_output);
     }{
     SM_RAISE_IF_NOT_TYPE(sm_out, mat4);
     mat4_t *output = sm_unwrap_mat4(sm_out, NULL);
     mat3_to_mat4 (*self, *output);
   }} else if (argc == 0) {
 SM_LABEL(skip_output): {
     mat4_t output;
     mat3_to_mat4 (*self, output);
     sm_out = sm_wrap_mat4(output, s_sm_mat3_klass);
     rb_obj_call_init(sm_out, 0, 0);
   }} else {
     rb_raise(rb_eArgError, "Invalid number of arguments to to_mat4");
   }
   return sm_out;
 }



/*
 * Transposes this matrix and returns the result.
 *
 * call-seq:
 *    transpose(output = nil) -> output or new mat3
 */
 static VALUE sm_mat3_transpose(int argc, VALUE *argv, VALUE sm_self)
 {
   VALUE sm_out;
   mat3_t *self;
   rb_scan_args(argc, argv, "01", &sm_out);
   self = sm_unwrap_mat3(sm_self, NULL);
   if (argc == 1) {
     if (!RTEST(sm_out)) {
       goto SM_LABEL(skip_output);
     }{
     SM_RAISE_IF_NOT_TYPE(sm_out, mat3);
     mat3_t *output = sm_unwrap_mat3(sm_out, NULL);
     mat3_transpose (*self, *output);
   }} else if (argc == 0) {
 SM_LABEL(skip_output): {
     mat3_t output;
     mat3_transpose (*self, output);
     sm_out = sm_wrap_mat3(output, rb_obj_class(sm_self));
     rb_obj_call_init(sm_out, 0, 0);
   }} else {
     rb_raise(rb_eArgError, "Invalid number of arguments to transpose");
   }
   return sm_out;
 }



/*
 * Returns an ajoint matrix.
 *
 * call-seq:
 *    adjoint(output = nil) -> output or new mat3
 */
 static VALUE sm_mat3_adjoint(int argc, VALUE *argv, VALUE sm_self)
 {
   VALUE sm_out;
   mat3_t *self;
   rb_scan_args(argc, argv, "01", &sm_out);
   self = sm_unwrap_mat3(sm_self, NULL);
   if (argc == 1) {
     if (!RTEST(sm_out)) {
       goto SM_LABEL(skip_output);
     }{
     SM_RAISE_IF_NOT_TYPE(sm_out, mat3);
     mat3_t *output = sm_unwrap_mat3(sm_out, NULL);
     mat3_adjoint (*self, *output);
   }} else if (argc == 0) {
 SM_LABEL(skip_output): {
     mat3_t output;
     mat3_adjoint (*self, output);
     sm_out = sm_wrap_mat3(output, rb_obj_class(sm_self));
     rb_obj_call_init(sm_out, 0, 0);
   }} else {
     rb_raise(rb_eArgError, "Invalid number of arguments to adjoint");
   }
   return sm_out;
 }



/*
 * Returns an orthogonal matrix.
 *
 * call-seq:
 *    orthogonal(output = nil) -> output or new mat3
 */
 static VALUE sm_mat3_orthogonal(int argc, VALUE *argv, VALUE sm_self)
 {
   VALUE sm_out;
   mat3_t *self;
   rb_scan_args(argc, argv, "01", &sm_out);
   self = sm_unwrap_mat3(sm_self, NULL);
   if (argc == 1) {
     if (!RTEST(sm_out)) {
       goto SM_LABEL(skip_output);
     }{
     SM_RAISE_IF_NOT_TYPE(sm_out, mat3);
     mat3_t *output = sm_unwrap_mat3(sm_out, NULL);
     mat3_orthogonal (*self, *output);
   }} else if (argc == 0) {
 SM_LABEL(skip_output): {
     mat3_t output;
     mat3_orthogonal (*self, output);
     sm_out = sm_wrap_mat3(output, rb_obj_class(sm_self));
     rb_obj_call_init(sm_out, 0, 0);
   }} else {
     rb_raise(rb_eArgError, "Invalid number of arguments to orthogonal");
   }
   return sm_out;
 }



/*
 * Returns a cofactor matrix.
 *
 * call-seq:
 *    cofactor(output = nil) -> output or new mat3
 */
 static VALUE sm_mat3_cofactor(int argc, VALUE *argv, VALUE sm_self)
 {
   VALUE sm_out;
   mat3_t *self;
   rb_scan_args(argc, argv, "01", &sm_out);
   self = sm_unwrap_mat3(sm_self, NULL);
   if (argc == 1) {
     if (!RTEST(sm_out)) {
       goto SM_LABEL(skip_output);
     }{
     SM_RAISE_IF_NOT_TYPE(sm_out, mat3);
     mat3_t *output = sm_unwrap_mat3(sm_out, NULL);
     mat3_cofactor (*self, *output);
   }} else if (argc == 0) {
 SM_LABEL(skip_output): {
     mat3_t output;
     mat3_cofactor (*self, output);
     sm_out = sm_wrap_mat3(output, rb_obj_class(sm_self));
     rb_obj_call_init(sm_out, 0, 0);
   }} else {
     rb_raise(rb_eArgError, "Invalid number of arguments to cofactor");
   }
   return sm_out;
 }



/*
 * Multiplies this Mat3 and another and returns the result.
 *
 * call-seq:
 *    multiply_mat3(mat3, output = nil) -> output or new mat3
 */
static VALUE sm_mat3_multiply(int argc, VALUE *argv, VALUE sm_self)
{
  VALUE sm_rhs;
  VALUE sm_out;
  mat3_t *self;
  mat3_t *rhs;
  rb_scan_args(argc, argv, "11", &sm_rhs, &sm_out);
  self = sm_unwrap_mat3(sm_self, NULL);
  SM_RAISE_IF_NOT_TYPE(sm_rhs, mat3);
  rhs = sm_unwrap_mat3(sm_rhs, NULL);
  if (argc == 2) {
    if (!RTEST(sm_out)) {
      goto SM_LABEL(skip_output);
    }{
    SM_RAISE_IF_NOT_TYPE(sm_out, mat3);
    mat3_t *output = sm_unwrap_mat3(sm_out, NULL);
    mat3_multiply(*self, *rhs, *output);
  }} else if (argc == 1) {
SM_LABEL(skip_output): {
    mat3_t output;
    mat3_multiply(*self, *rhs, output);
    sm_out = sm_wrap_mat3(output, rb_obj_class(sm_self));
    rb_obj_call_init(sm_out, 0, 0);
  }} else {
    rb_raise(rb_eArgError, "Invalid number of arguments to multiply_mat3");
  }
  return sm_out;
}



/*
 * Rotates a Vec3 using self and returns the result.
 *
 * call-seq:
 *    rotate_vec3(vec3, output = nil) -> output or new vec3
 */
static VALUE sm_mat3_rotate_vec3(int argc, VALUE *argv, VALUE sm_self)
{
  VALUE sm_rhs;
  VALUE sm_out;
  mat3_t *self;
  vec3_t *rhs;
  rb_scan_args(argc, argv, "11", &sm_rhs, &sm_out);
  self = sm_unwrap_mat3(sm_self, NULL);
  if (!SM_IS_A(sm_rhs, vec3) && !SM_IS_A(sm_rhs, vec4) && !SM_IS_A(sm_rhs, quat)) {
    rb_raise(rb_eTypeError,
      kSM_WANT_THREE_OR_FOUR_FORMAT_LIT,
      rb_obj_classname(sm_rhs));
    return Qnil;
  }
  rhs = sm_unwrap_vec3(sm_rhs, NULL);
  if (argc == 2) {
    if (!RTEST(sm_out)) {
      goto SM_LABEL(skip_output);
    }{
    if (!SM_IS_A(sm_out, vec3) && !SM_IS_A(sm_out, vec4) && !SM_IS_A(sm_out, quat)) {
      rb_raise(rb_eTypeError,
        kSM_WANT_THREE_OR_FOUR_FORMAT_LIT,
        rb_obj_classname(sm_out));
      return Qnil;
    }
    vec3_t *output = sm_unwrap_vec3(sm_out, NULL);
    mat3_rotate_vec3(*self, *rhs, *output);
  }} else if (argc == 1) {
SM_LABEL(skip_output): {
    vec3_t output;
    mat3_rotate_vec3(*self, *rhs, output);
    sm_out = sm_wrap_vec3(output, rb_obj_class(sm_rhs));
    rb_obj_call_init(sm_out, 0, 0);
  }} else {
    rb_raise(rb_eArgError, "Invalid number of arguments to rotate_vec3");
  }
  return sm_out;
}



/*
 * Convenience function to rotate a Vec3 by an inverse matrix and return the
 * result.
 *
 * call-seq:
 *    inv_rotate_vec3(vec3, output = nil) -> output or new vec3
 */
static VALUE sm_mat3_inv_rotate_vec3(int argc, VALUE *argv, VALUE sm_self)
{
  VALUE sm_rhs;
  VALUE sm_out;
  mat3_t *self;
  vec3_t *rhs;
  rb_scan_args(argc, argv, "11", &sm_rhs, &sm_out);
  self = sm_unwrap_mat3(sm_self, NULL);
  if (!SM_IS_A(sm_rhs, vec3) && !SM_IS_A(sm_rhs, vec4) && !SM_IS_A(sm_rhs, quat)) {
    rb_raise(rb_eTypeError,
      kSM_WANT_THREE_OR_FOUR_FORMAT_LIT,
      rb_obj_classname(sm_rhs));
    return Qnil;
  }
  rhs = sm_unwrap_vec3(sm_rhs, NULL);
  if (argc == 2) {
    if (!RTEST(sm_out)) {
      goto SM_LABEL(skip_output);
    }{
    if (!SM_IS_A(sm_out, vec3) && !SM_IS_A(sm_out, vec4) && !SM_IS_A(sm_out, quat)) {
      rb_raise(rb_eTypeError,
        kSM_WANT_THREE_OR_FOUR_FORMAT_LIT,
        rb_obj_classname(sm_out));
      return Qnil;
    }
    vec3_t *output = sm_unwrap_vec3(sm_out, NULL);
    mat3_inv_rotate_vec3(*self, *rhs, *output);
  }} else if (argc == 1) {
SM_LABEL(skip_output): {
    vec3_t output;
    mat3_inv_rotate_vec3(*self, *rhs, output);
    sm_out = sm_wrap_vec3(output, rb_obj_class(sm_rhs));
    rb_obj_call_init(sm_out, 0, 0);
  }} else {
    rb_raise(rb_eArgError, "Invalid number of arguments to inverse_rotate_vec3");
  }
  return sm_out;
}



/*
 * Returns the matrix determinant.
 *
 * call-seq:
 *    determinant -> float
 */
static VALUE sm_mat3_determinant(VALUE sm_self)
{
  return mat3_determinant(*sm_unwrap_mat3(sm_self, NULL));
}



/*
 * Returns the matrix inverse on success, nil on failure.
 *
 * call-seq:
 *    inverse(output = nil) -> output, new mat3, or nil
 */
static VALUE sm_mat3_inverse(int argc, VALUE *argv, VALUE sm_self)
{
  VALUE sm_out = Qnil;
  mat3_t *self;

  rb_scan_args(argc, argv, "01", &sm_out);
  self = sm_unwrap_mat3(sm_self, NULL);

  if (argc == 1) {
    mat3_t *output;

    if (!RTEST(sm_out)) {
      goto SM_LABEL(skip_output);
    }

    if (!SM_IS_A(sm_out, mat3)) {
      rb_raise(rb_eTypeError,
        "Invalid argument to output of inverse_general: expected %s, got %s",
        rb_class2name(s_sm_mat3_klass),
        rb_obj_classname(sm_out));
      return Qnil;
    }

    output = sm_unwrap_mat3(sm_out, NULL);
    if (!mat3_inverse(*self, *output)) {
      return Qnil;
    }

  } else if (argc == 0) {
    SM_LABEL(skip_output): {
      mat3_t output;
      if (!mat3_inverse(*self, output)) {
        return Qnil;
      }

      sm_out = sm_wrap_mat3(output, rb_obj_class(sm_self));
      rb_obj_call_init(sm_out, 0, 0);
    }
  } else {
    rb_raise(rb_eArgError, "Invalid number of arguments to inverse");
  }

  return sm_out;
}



/*
 * Allocates a new Mat3.
 *
 * call-seq:
 *    new()                      -> identity mat3
 *    new(m1, m2, ..., m8, m9)   -> new mat3 with components
 *    new([m1, m2, ..., m8, m9]) -> new mat3 with components
 *    new(mat3)                  -> copy of mat3
 *    new(mat4)                  -> new mat3 from mat4's inner 9x9 matrix
 *    new(quat)                  -> quat as mat3
 *    new(Vec3, Vec3, Vec3)      -> new mat3 with given row vectors
 */
static VALUE sm_mat3_new(int argc, VALUE *argv, VALUE self)
{
  VALUE sm_mat = sm_wrap_mat3(g_mat3_identity, self);
  rb_obj_call_init(sm_mat, argc, argv);
  return sm_mat;
}



/*
 * Sets the Mat3's components.
 *
 * call-seq:
 *    set(m1, m2, ..., m8, m9)   -> new mat3 with components
 *    set([m1, m2, ..., m8, m9]) -> new mat3 with components
 *    set(mat3)                  -> copy of mat3
 *    set(mat4)                  -> new mat3 from mat4's inner 9x9 matrix
 *    set(quat)                  -> quat as mat3
 *    set(Vec3, Vec3, Vec3)      -> new mat3 with given row vectors
 */
static VALUE sm_mat3_init(int argc, VALUE *argv, VALUE sm_self)
{
  mat3_t *self = sm_unwrap_mat3(sm_self, NULL);
  size_t arr_index = 0;

  switch (argc) {

  case 0: {
    // Identity (handled in _new)
    break;
  }

  // Copy Mat3 or provided [Numeric..]
  case 1: {
    // Copy Mat3
    if (SM_IS_A(argv[0], mat3)) {
      sm_unwrap_mat3(argv[0], *self);
      break;
    }

    // Copy Mat4
    if (SM_IS_A(argv[0], mat4)) {
      mat4_to_mat3(*sm_unwrap_mat4(argv[0], NULL), *self);
      break;
    }

    // Build from Quaternion
    if (SM_IS_A(argv[0], quat)) {
      mat3_from_quat(*sm_unwrap_quat(argv[0], NULL), *self);
      break;
    }

    // Optional offset into array provided
    if (0) {
      case 2:
      arr_index = NUM2SIZET(argv[1]);
    }

    // Array of values
    if (SM_RB_IS_A(argv[0], rb_cArray)) {
      VALUE arrdata = argv[0];
      const size_t arr_end = arr_index + 9;
      s_float_t *mat_elem = *self;
      for (; arr_index < arr_end; ++arr_index, ++mat_elem) {
        *mat_elem = rb_num2dbl(rb_ary_entry(arrdata, (long)arr_index));
      }
      break;
    }

    rb_raise(rb_eArgError, "Expected either an array of Numerics or a Mat3");
    break;
  }

  // Mat3(Vec3, Vec3, Vec3)
  case 3: {
    size_t arg_index;
    s_float_t *mat_elem = *self;
    for (arg_index = 0; arg_index < 3; ++arg_index, mat_elem += 3) {
      if (!SM_IS_A(argv[arg_index], vec3)) {
        rb_raise(
          rb_eArgError,
          "Argument %d must be a Vec3 when supplying three arguments to Mat3.initialize",
          (int)(arg_index + 1));
      }

      sm_unwrap_vec3(argv[arg_index], mat_elem);
    }
    break;
  }

  // Mat3(Numeric m00 .. m16)
  case 9: {
    s_float_t *mat_elem = *self;
    VALUE *argv_p = argv;
    for (; argc; --argc, ++argv_p, ++mat_elem) {
      *mat_elem = (s_float_t)rb_num2dbl(*argv_p);
    }
    break;
  }

  default: {
    rb_raise(rb_eArgError, "Invalid arguments to Mat3.initialize");
    break;
  }
  } // swtich (argc)

  return sm_self;
}



/*
 * Returns a string representation of self.
 *
 *    Mat3[].to_s     # => "{ 1.0, 0.0, 0.0,\n
 *                    #       0.0, 1.0, 0.0,\n"
 *                    #       0.0, 0.0, 1.0 }"
 *
 * call-seq:
 *    to_s -> string
 */
static VALUE sm_mat3_to_s(VALUE self)
{
  const s_float_t *v;
  v = (const s_float_t *)*sm_unwrap_mat3(self, NULL);
  return rb_sprintf(
    "{ "
    "%f, %f, %f" ",\n  "
    "%f, %f, %f" ",\n  "
    "%f, %f, %f"
    " }",
    v[0],   v[1],   v[2],
    v[3],   v[4],   v[5],
    v[6],   v[7],   v[8] );
}



/*
 * Returns a Mat3 describing a rotation around the given axis.
 *
 * call-seq:
 *    angle_axis(angle_degrees, axis_vec3, output = nil) -> output or new mat3
 */
static VALUE sm_mat3_angle_axis(int argc, VALUE *argv, VALUE self)
{
  VALUE sm_angle;
  VALUE sm_axis;
  VALUE sm_out;
  s_float_t angle;
  const vec3_t *axis;

  rb_scan_args(argc, argv, "21", &sm_angle, &sm_axis, &sm_out);
  if (!SM_IS_A(sm_axis, vec3) && !SM_IS_A(sm_axis, vec4) && !SM_IS_A(sm_axis, quat)) {
    rb_raise(rb_eTypeError,
      kSM_WANT_THREE_OR_FOUR_FORMAT_LIT,
      rb_obj_classname(sm_axis));
    return Qnil;
  }

  angle = (s_float_t)rb_num2dbl(sm_angle);
  axis = sm_unwrap_vec3(sm_axis, NULL);

  if (SM_IS_A(sm_out, mat3)) {
    mat3_t *out = sm_unwrap_mat3(sm_out, NULL);
    mat3_rotation(angle, (*axis)[0], (*axis)[1], (*axis)[2], *out);
  } else {
    mat3_t out;
    mat3_rotation(angle, (*axis)[0], (*axis)[1], (*axis)[2], out);
    sm_out = sm_wrap_mat3(out, self);
    rb_obj_call_init(sm_out, 0, 0);
  }

  return sm_out;
}



/*
 * Returns a Vec3 whose components are that of the row at the given index.
 *
 * call-seq:
 *    get_row3(index, output = nil) -> output or new vec3
 */
static VALUE sm_mat3_get_row3(int argc, VALUE *argv, VALUE sm_self)
{
  mat3_t *self;
  int index;
  VALUE sm_out;

  self = sm_unwrap_mat3(sm_self, NULL);
  index = NUM2INT(argv[0]);
  sm_out = Qnil;

  if (index < 0 || index > 2) {
    rb_raise(rb_eRangeError, "Index %d is out of range, must be (0 .. 2)", index);
    return Qnil;
  }

  switch (argc) {
  case 2: {
    vec3_t *out;

    sm_out = argv[1];

    if (RTEST(sm_out)) {
      if (!SM_IS_A(sm_out, vec3) && !SM_IS_A(sm_out, vec4) && !SM_IS_A(sm_out, quat)) {
        rb_raise(rb_eTypeError,
          kSM_WANT_THREE_OR_FOUR_FORMAT_LIT,
          rb_obj_classname(sm_out));
        return Qnil;
      }
    } else {
      goto SM_LABEL(no_output);
    }

    out = sm_unwrap_vec3(sm_out, NULL);
    mat3_get_row3(*self, index, *out);

    break;
  }

  case 1: SM_LABEL(no_output): {
    vec3_t out;
    mat3_get_row3(*self, index, out);
    sm_out = sm_wrap_vec3(out, Qnil);
    rb_obj_call_init(sm_out, 0, 0);
    break;
  }

  default: {
    rb_raise(rb_eArgError, "Invalid number of arguments to get_row3 - expected 1 or 2");
    break;
  }
  }

  return sm_out;
}



/*
 * Returns a Vec3 whose components are that of the column at the given index.
 *
 * call-seq:
 *    get_column3(index, output = nil) -> output or new vec3
 */
static VALUE sm_mat3_get_column3(int argc, VALUE *argv, VALUE sm_self)
{
  mat3_t *self;
  int index;
  VALUE sm_out;

  self = sm_unwrap_mat3(sm_self, NULL);
  index = NUM2INT(argv[0]);
  sm_out = Qnil;

  if (index < 0 || index > 2) {
    rb_raise(rb_eRangeError, "Index %d is out of range, must be (0 .. 2)", index);
    return Qnil;
  }

  switch (argc) {
  case 2: {
    vec3_t *out;

    sm_out = argv[1];

    if (RTEST(sm_out)) {
      if (!SM_IS_A(sm_out, vec3) && !SM_IS_A(sm_out, vec4) && !SM_IS_A(sm_out, quat)) {
        rb_raise(rb_eTypeError,
          kSM_WANT_THREE_OR_FOUR_FORMAT_LIT,
          rb_obj_classname(sm_out));
        return Qnil;
      }
    } else {
      goto SM_LABEL(no_output);
    }

    out = sm_unwrap_vec3(sm_out, NULL);
    mat3_get_column3(*self, index, *out);

    break;
  }

  case 1: SM_LABEL(no_output): {
    vec3_t out;
    mat3_get_column3(*self, index, out);
    sm_out = sm_wrap_vec3(out, Qnil);
    rb_obj_call_init(sm_out, 0, 0);
    break;
  }

  default: {
    rb_raise(rb_eArgError, "Invalid number of arguments to get_column3 - expected 1 or 2");
    break;
  }
  }

  return sm_out;
}



/*
 * Sets the matrix's row at the given index to the given vector.
 *
 * call-seq:
 *    set_row3(index, vec3) -> self
 */
static VALUE sm_mat3_set_row3(VALUE sm_self, VALUE sm_index, VALUE sm_value)
{
  const vec3_t *value;
  int index;
  mat3_t *self;

  if (!SM_IS_A(sm_value, vec3) && !SM_IS_A(sm_value, vec4) && !SM_IS_A(sm_value, quat)) {
    rb_raise(rb_eTypeError,
      kSM_WANT_THREE_OR_FOUR_FORMAT_LIT,
      rb_obj_classname(sm_value));
    return Qnil;
  }

  self = sm_unwrap_mat3(sm_self, NULL);
  value = sm_unwrap_vec3(sm_value, NULL);
  index = NUM2INT(sm_index);

  if (index < 0 || index > 2) {
    rb_raise(rb_eRangeError, "Index %d is out of range, must be (0 .. 2)", index);
    return Qnil;
  }

  mat3_set_row3(index, *value, *self);

  return sm_self;
}



/*
 * Sets the matrix's column at the given index to the given vector.
 *
 * call-seq:
 *    set_column3(index, value) -> self
 */
static VALUE sm_mat3_set_column3(VALUE sm_self, VALUE sm_index, VALUE sm_value)
{
  const vec3_t *value;
  int index;
  mat3_t *self;

  if (!SM_IS_A(sm_value, vec3) && !SM_IS_A(sm_value, vec4) && !SM_IS_A(sm_value, quat)) {
    rb_raise(rb_eTypeError,
      kSM_WANT_THREE_OR_FOUR_FORMAT_LIT,
      rb_obj_classname(sm_value));
    return Qnil;
  }

  self = sm_unwrap_mat3(sm_self, NULL);
  value = sm_unwrap_vec3(sm_value, NULL);
  index = NUM2INT(sm_index);

  if (index < 0 || index > 2) {
    rb_raise(rb_eRangeError, "Index %d is out of range, must be (0 .. 2)", index);
    return Qnil;
  }

  mat3_set_column3(index, *value, *self);

  return sm_self;
}



/*
 * Sets self to the identity matrix.
 *
 * call-seq:
 *    load_identity -> self
 */
static VALUE sm_mat3_identity(VALUE sm_self)
{
  mat3_t *self = sm_unwrap_mat3(sm_self, NULL);
  mat3_identity(*self);
  return sm_self;
}



/*
 * Scales Mat3's columns by X, Y, and Z and returns the result.
 *
 * call-seq:
 *    scale(x, y, z, output = nil) -> output or new mat3
 */
static VALUE sm_mat3_scale(int argc, VALUE *argv, VALUE sm_self)
{
  VALUE sm_out;
  VALUE sm_x, sm_y, sm_z;
  s_float_t x, y, z;
  mat3_t *self = sm_unwrap_mat3(sm_self, NULL);

  rb_scan_args(argc, argv, "31", &sm_x, &sm_y, &sm_z, &sm_out);
  x = rb_num2dbl(sm_x);
  y = rb_num2dbl(sm_y);
  z = rb_num2dbl(sm_z);

  if (SM_IS_A(sm_out, mat3)) {
    mat3_scale(*self, x, y, z, *sm_unwrap_mat3(sm_out, NULL));
  } else {
    mat3_t out;
    mat3_scale(*self, x, y, z, out);
    sm_out = sm_wrap_mat3(out, rb_obj_class(sm_self));
    rb_obj_call_init(sm_out, 0, 0);
  }

  return sm_out;
}



/*
 * Tests this Mat3 and another Mat3 for equivalency.
 *
 * call-seq:
 *    mat3 == other_mat3 -> bool
 */
static VALUE sm_mat3_equals(VALUE sm_self, VALUE sm_other)
{
  if (!RTEST(sm_other) || !SM_IS_A(sm_other, mat3)) {
    return Qfalse;
  }

  return mat3_equals(*sm_unwrap_mat3(sm_self, NULL), *sm_unwrap_mat3(sm_other, NULL)) ? Qtrue : Qfalse;
}



/*==============================================================================

  General-purpose functions

==============================================================================*/

/*
  Returns the memory address of the object.

  call-seq: address -> fixnum
 */
static VALUE sm_get_address(VALUE sm_self)
{
  void *data_ptr = NULL;
  Data_Get_Struct(sm_self, void, data_ptr);
  return ULL2NUM((unsigned long long)data_ptr);
}



void Init_bindings()
{
  ID kRB_CONST_SIZE, kRB_CONST_LENGTH, kRB_CONST_FLOAT_SIZE, kRB_CONST_TYPE;

  kRB_IVAR_MATHARRAY_LENGTH = rb_intern("__length");
  kRB_IVAR_MATHARRAY_CACHE  = rb_intern("__cache");
  kRB_IVAR_MATHARRAY_SOURCE = rb_intern("__source");
  kRB_CONST_SIZE            = rb_intern("SIZE");
  kRB_CONST_LENGTH          = rb_intern("LENGTH");
  kRB_CONST_FLOAT_SIZE      = rb_intern("SNOW_MATH_FLOAT_SIZE");
  kRB_CONST_TYPE            = rb_intern("TYPE");

  /*
   * The size in bytes of the base snow-math floating point type. Set to 4 when
   * using float or 8 when using double.
   */

  s_sm_snowmath_mod = rb_define_module("Snow");
  s_sm_vec3_klass   = rb_define_class_under(s_sm_snowmath_mod, "Vec3", rb_cObject);
  s_sm_vec4_klass   = rb_define_class_under(s_sm_snowmath_mod, "Vec4", rb_cObject);
  s_sm_quat_klass   = rb_define_class_under(s_sm_snowmath_mod, "Quat", rb_cObject);
  s_sm_mat3_klass   = rb_define_class_under(s_sm_snowmath_mod, "Mat3", rb_cObject);
  s_sm_mat4_klass   = rb_define_class_under(s_sm_snowmath_mod, "Mat4", rb_cObject);

  rb_const_set(s_sm_snowmath_mod, kRB_CONST_FLOAT_SIZE, INT2FIX(sizeof(s_float_t)));
  rb_const_set(s_sm_vec3_klass, kRB_CONST_SIZE, INT2FIX(sizeof(vec3_t)));
  rb_const_set(s_sm_vec4_klass, kRB_CONST_SIZE, INT2FIX(sizeof(vec4_t)));
  rb_const_set(s_sm_quat_klass, kRB_CONST_SIZE, INT2FIX(sizeof(quat_t)));
  rb_const_set(s_sm_mat3_klass, kRB_CONST_SIZE, INT2FIX(sizeof(mat3_t)));
  rb_const_set(s_sm_mat4_klass, kRB_CONST_SIZE, INT2FIX(sizeof(mat4_t)));
  rb_const_set(s_sm_vec3_klass, kRB_CONST_LENGTH, INT2FIX(sizeof(vec3_t) / sizeof(s_float_t)));
  rb_const_set(s_sm_vec4_klass, kRB_CONST_LENGTH, INT2FIX(sizeof(vec4_t) / sizeof(s_float_t)));
  rb_const_set(s_sm_quat_klass, kRB_CONST_LENGTH, INT2FIX(sizeof(quat_t) / sizeof(s_float_t)));
  rb_const_set(s_sm_mat3_klass, kRB_CONST_LENGTH, INT2FIX(sizeof(mat3_t) / sizeof(s_float_t)));
  rb_const_set(s_sm_mat4_klass, kRB_CONST_LENGTH, INT2FIX(sizeof(mat4_t) / sizeof(s_float_t)));

  rb_define_singleton_method(s_sm_vec3_klass, "new", sm_vec3_new, -1);
  rb_define_method(s_sm_vec3_klass, "initialize", sm_vec3_init, -1);
  rb_define_method(s_sm_vec3_klass, "set", sm_vec3_init, -1);
  rb_define_method(s_sm_vec3_klass, "fetch", sm_vec3_fetch, 1);
  rb_define_method(s_sm_vec3_klass, "store", sm_vec3_store, 2);
  rb_define_method(s_sm_vec3_klass, "size", sm_vec3_size, 0);
  rb_define_method(s_sm_vec3_klass, "length", sm_vec3_length, 0);
  rb_define_method(s_sm_vec3_klass, "to_s", sm_vec3_to_s, 0);
  rb_define_method(s_sm_vec3_klass, "address", sm_get_address, 0);
  rb_define_method(s_sm_vec3_klass, "copy", sm_vec3_copy, -1);
  rb_define_method(s_sm_vec3_klass, "normalize", sm_vec3_normalize, -1);
  rb_define_method(s_sm_vec3_klass, "inverse", sm_vec3_inverse, -1);
  rb_define_method(s_sm_vec3_klass, "negate", sm_vec3_negate, -1);
  rb_define_method(s_sm_vec3_klass, "cross_product", sm_vec3_cross_product, -1);
  rb_define_method(s_sm_vec3_klass, "multiply_vec3", sm_vec3_multiply, -1);
  rb_define_method(s_sm_vec3_klass, "add", sm_vec3_add, -1);
  rb_define_method(s_sm_vec3_klass, "subtract", sm_vec3_subtract, -1);
  rb_define_method(s_sm_vec3_klass, "reflect", sm_vec3_reflect, -1);
  rb_define_method(s_sm_vec3_klass, "project", sm_vec3_project, -1);
  rb_define_method(s_sm_vec3_klass, "dot_product", sm_vec3_dot_product, 1);
  rb_define_method(s_sm_vec3_klass, "magnitude_squared", sm_vec3_magnitude_squared, 0);
  rb_define_method(s_sm_vec3_klass, "magnitude", sm_vec3_magnitude, 0);
  rb_define_method(s_sm_vec3_klass, "scale", sm_vec3_scale, -1);
  rb_define_method(s_sm_vec3_klass, "divide", sm_vec3_divide, -1);
  rb_define_method(s_sm_vec3_klass, "==", sm_vec3_equals, 1);

  rb_define_singleton_method(s_sm_vec4_klass, "new", sm_vec4_new, -1);
  rb_define_method(s_sm_vec4_klass, "initialize", sm_vec4_init, -1);
  rb_define_method(s_sm_vec4_klass, "set", sm_vec4_init, -1);
  rb_define_method(s_sm_vec4_klass, "fetch", sm_vec4_fetch, 1);
  rb_define_method(s_sm_vec4_klass, "store", sm_vec4_store, 2);
  rb_define_method(s_sm_vec4_klass, "size", sm_vec4_size, 0);
  rb_define_method(s_sm_vec4_klass, "length", sm_vec4_length, 0);
  rb_define_method(s_sm_vec4_klass, "to_s", sm_vec4_to_s, 0);
  rb_define_method(s_sm_vec4_klass, "address", sm_get_address, 0);
  rb_define_method(s_sm_vec4_klass, "copy", sm_vec4_copy, -1);
  rb_define_method(s_sm_vec4_klass, "normalize", sm_vec4_normalize, -1);
  rb_define_method(s_sm_vec4_klass, "inverse", sm_vec4_inverse, -1);
  rb_define_method(s_sm_vec4_klass, "negate", sm_vec4_negate, -1);
  rb_define_method(s_sm_vec4_klass, "multiply_vec4", sm_vec4_multiply, -1);
  rb_define_method(s_sm_vec4_klass, "add", sm_vec4_add, -1);
  rb_define_method(s_sm_vec4_klass, "subtract", sm_vec4_subtract, -1);
  rb_define_method(s_sm_vec4_klass, "reflect", sm_vec4_reflect, -1);
  rb_define_method(s_sm_vec4_klass, "project", sm_vec4_project, -1);
  rb_define_method(s_sm_vec4_klass, "dot_product", sm_vec4_dot_product, 1);
  rb_define_method(s_sm_vec4_klass, "magnitude_squared", sm_vec4_magnitude_squared, 0);
  rb_define_method(s_sm_vec4_klass, "magnitude", sm_vec4_magnitude, 0);
  rb_define_method(s_sm_vec4_klass, "scale", sm_vec4_scale, -1);
  rb_define_method(s_sm_vec4_klass, "divide", sm_vec4_divide, -1);
  rb_define_method(s_sm_vec4_klass, "==", sm_vec4_equals, 1);

  rb_define_singleton_method(s_sm_quat_klass, "new", sm_quat_new, -1);
  rb_define_singleton_method(s_sm_quat_klass, "angle_axis", sm_quat_angle_axis, -1);
  rb_define_method(s_sm_quat_klass, "initialize", sm_quat_init, -1);
  rb_define_method(s_sm_quat_klass, "set", sm_quat_init, -1);
  rb_define_method(s_sm_quat_klass, "load_identity", sm_quat_identity, 0);
  rb_define_method(s_sm_quat_klass, "fetch", sm_quat_fetch, 1);
  rb_define_method(s_sm_quat_klass, "store", sm_quat_store, 2);
  rb_define_method(s_sm_quat_klass, "size", sm_quat_size, 0);
  rb_define_method(s_sm_quat_klass, "length", sm_quat_length, 0);
  rb_define_method(s_sm_quat_klass, "to_s", sm_quat_to_s, 0);
  rb_define_method(s_sm_quat_klass, "address", sm_get_address, 0);
  rb_define_method(s_sm_quat_klass, "inverse", sm_quat_inverse, -1);
  rb_define_method(s_sm_quat_klass, "multiply_quat", sm_quat_multiply, -1);
  rb_define_method(s_sm_quat_klass, "multiply_vec3", sm_quat_multiply_vec3, -1);
  rb_define_method(s_sm_quat_klass, "slerp", sm_quat_slerp, -1);
  // Borrow some functions from vec4
  rb_define_method(s_sm_quat_klass, "copy", sm_vec4_copy, -1);
  rb_define_method(s_sm_quat_klass, "negate", sm_vec4_negate, -1);
  rb_define_method(s_sm_quat_klass, "normalize", sm_vec4_normalize, -1);
  rb_define_method(s_sm_quat_klass, "scale", sm_vec4_scale, -1);
  rb_define_method(s_sm_quat_klass, "divide", sm_vec4_divide, -1);
  rb_define_method(s_sm_quat_klass, "add", sm_vec4_add, -1);
  rb_define_method(s_sm_quat_klass, "subtract", sm_vec4_subtract, -1);
  rb_define_method(s_sm_quat_klass, "dot_product", sm_vec4_dot_product, 1);
  rb_define_method(s_sm_quat_klass, "magnitude_squared", sm_vec4_magnitude_squared, 0);
  rb_define_method(s_sm_quat_klass, "magnitude", sm_vec4_magnitude, 0);
  rb_define_method(s_sm_quat_klass, "==", sm_vec4_equals, 1);

  rb_define_singleton_method(s_sm_mat4_klass, "new", sm_mat4_new, -1);
  rb_define_singleton_method(s_sm_mat4_klass, "translation", sm_mat4_translation, -1);
  rb_define_singleton_method(s_sm_mat4_klass, "angle_axis", sm_mat4_angle_axis, -1);
  rb_define_singleton_method(s_sm_mat4_klass, "frustum", sm_mat4_frustum, -1);
  rb_define_singleton_method(s_sm_mat4_klass, "perspective", sm_mat4_perspective, -1);
  rb_define_singleton_method(s_sm_mat4_klass, "orthographic", sm_mat4_orthographic, -1);
  rb_define_singleton_method(s_sm_mat4_klass, "look_at", sm_mat4_look_at, -1);
  rb_define_method(s_sm_mat4_klass, "initialize", sm_mat4_init, -1);
  rb_define_method(s_sm_mat4_klass, "set", sm_mat4_init, -1);
  rb_define_method(s_sm_mat4_klass, "to_mat3", sm_mat4_to_mat3, -1);
  rb_define_method(s_sm_mat4_klass, "load_identity", sm_mat4_identity, 0);
  rb_define_method(s_sm_mat4_klass, "fetch", sm_mat4_fetch, 1);
  rb_define_method(s_sm_mat4_klass, "store", sm_mat4_store, 2);
  rb_define_method(s_sm_mat4_klass, "size", sm_mat4_size, 0);
  rb_define_method(s_sm_mat4_klass, "length", sm_mat4_length, 0);
  rb_define_method(s_sm_mat4_klass, "to_s", sm_mat4_to_s, 0);
  rb_define_method(s_sm_mat4_klass, "address", sm_get_address, 0);
  rb_define_method(s_sm_mat4_klass, "copy", sm_mat4_copy, -1);
  rb_define_method(s_sm_mat4_klass, "transpose", sm_mat4_transpose, -1);
  rb_define_method(s_sm_mat4_klass, "inverse_orthogonal", sm_mat4_inverse_orthogonal, -1);
  rb_define_method(s_sm_mat4_klass, "adjoint", sm_mat4_adjoint, -1);
  rb_define_method(s_sm_mat4_klass, "scale", sm_mat4_scale, -1);
  rb_define_method(s_sm_mat4_klass, "multiply_mat4", sm_mat4_multiply, -1);
  rb_define_method(s_sm_mat4_klass, "multiply_vec4", sm_mat4_multiply_vec4, -1);
  rb_define_method(s_sm_mat4_klass, "transform_vec3", sm_mat4_transform_vec3, -1);
  rb_define_method(s_sm_mat4_klass, "rotate_vec3", sm_mat4_rotate_vec3, -1);
  rb_define_method(s_sm_mat4_klass, "inverse_rotate_vec3", sm_mat4_inv_rotate_vec3, -1);
  rb_define_method(s_sm_mat4_klass, "inverse_affine", sm_mat4_inverse_affine, -1);
  rb_define_method(s_sm_mat4_klass, "inverse_general", sm_mat4_inverse_general, -1);
  rb_define_method(s_sm_mat4_klass, "determinant", sm_mat4_determinant, 0);
  rb_define_method(s_sm_mat4_klass, "translate", sm_mat4_translate, -1);
  rb_define_method(s_sm_mat4_klass, "set_row3", sm_mat4_set_row3, 2);
  rb_define_method(s_sm_mat4_klass, "set_row4", sm_mat4_set_row4, 2);
  rb_define_method(s_sm_mat4_klass, "get_row3", sm_mat4_get_row3, -1);
  rb_define_method(s_sm_mat4_klass, "get_row4", sm_mat4_get_row4, -1);
  rb_define_method(s_sm_mat4_klass, "set_column3", sm_mat4_set_column3, 2);
  rb_define_method(s_sm_mat4_klass, "set_column4", sm_mat4_set_column4, 2);
  rb_define_method(s_sm_mat4_klass, "get_column3", sm_mat4_get_column3, -1);
  rb_define_method(s_sm_mat4_klass, "get_column4", sm_mat4_get_column4, -1);
  rb_define_method(s_sm_mat4_klass, "==", sm_mat4_equals, 1);

  rb_define_singleton_method(s_sm_mat3_klass, "new", sm_mat3_new, -1);
  rb_define_singleton_method(s_sm_mat3_klass, "angle_axis", sm_mat3_angle_axis, -1);
  rb_define_method(s_sm_mat3_klass, "initialize", sm_mat3_init, -1);
  rb_define_method(s_sm_mat3_klass, "set", sm_mat3_init, -1);
  rb_define_method(s_sm_mat3_klass, "to_mat4", sm_mat3_to_mat4, -1);
  rb_define_method(s_sm_mat3_klass, "load_identity", sm_mat3_identity, 0);
  rb_define_method(s_sm_mat3_klass, "fetch", sm_mat3_fetch, 1);
  rb_define_method(s_sm_mat3_klass, "store", sm_mat3_store, 2);
  rb_define_method(s_sm_mat3_klass, "size", sm_mat3_size, 0);
  rb_define_method(s_sm_mat3_klass, "length", sm_mat3_length, 0);
  rb_define_method(s_sm_mat3_klass, "to_s", sm_mat3_to_s, 0);
  rb_define_method(s_sm_mat3_klass, "address", sm_get_address, 0);
  rb_define_method(s_sm_mat3_klass, "copy", sm_mat3_copy, -1);
  rb_define_method(s_sm_mat3_klass, "transpose", sm_mat3_transpose, -1);
  rb_define_method(s_sm_mat3_klass, "adjoint", sm_mat3_adjoint, -1);
  rb_define_method(s_sm_mat3_klass, "cofactor", sm_mat3_cofactor, -1);
  rb_define_method(s_sm_mat3_klass, "orthogonal", sm_mat3_orthogonal, -1);
  rb_define_method(s_sm_mat3_klass, "scale", sm_mat3_scale, -1);
  rb_define_method(s_sm_mat3_klass, "multiply_mat3", sm_mat3_multiply, -1);
  rb_define_method(s_sm_mat3_klass, "rotate_vec3", sm_mat3_rotate_vec3, -1);
  rb_define_method(s_sm_mat3_klass, "inverse_rotate_vec3", sm_mat3_inv_rotate_vec3, -1);
  rb_define_method(s_sm_mat3_klass, "inverse", sm_mat3_inverse, -1);
  rb_define_method(s_sm_mat3_klass, "determinant", sm_mat3_determinant, 0);
  rb_define_method(s_sm_mat3_klass, "set_row3", sm_mat3_set_row3, 2);
  rb_define_method(s_sm_mat3_klass, "get_row3", sm_mat3_get_row3, -1);
  rb_define_method(s_sm_mat3_klass, "set_column3", sm_mat3_set_column3, 2);
  rb_define_method(s_sm_mat3_klass, "get_column3", sm_mat3_get_column3, -1);
  rb_define_method(s_sm_mat3_klass, "==", sm_mat3_equals, 1);

  #if BUILD_ARRAY_TYPE

  s_sm_vec3_array_klass = rb_define_class_under(s_sm_snowmath_mod, "Vec3Array", rb_cObject);
  rb_const_set(s_sm_vec3_array_klass, kRB_CONST_TYPE, s_sm_vec3_klass);
  rb_define_singleton_method(s_sm_vec3_array_klass, "new", sm_vec3_array_new, 1);
  rb_define_method(s_sm_vec3_array_klass, "fetch", sm_vec3_array_fetch, 1);
  rb_define_method(s_sm_vec3_array_klass, "store", sm_vec3_array_store, 2);
  rb_define_method(s_sm_vec3_array_klass, "resize!", sm_vec3_array_resize, 1);
  rb_define_method(s_sm_vec3_array_klass, "size", sm_vec3_array_size, 0);
  rb_define_method(s_sm_vec3_array_klass, "length", sm_mathtype_array_length, 0);
  rb_define_method(s_sm_vec3_array_klass, "address", sm_get_address, 0);

  s_sm_vec4_array_klass = rb_define_class_under(s_sm_snowmath_mod, "Vec4Array", rb_cObject);
  rb_const_set(s_sm_vec4_array_klass, kRB_CONST_TYPE, s_sm_vec4_klass);
  rb_define_singleton_method(s_sm_vec4_array_klass, "new", sm_vec4_array_new, 1);
  rb_define_method(s_sm_vec4_array_klass, "fetch", sm_vec4_array_fetch, 1);
  rb_define_method(s_sm_vec4_array_klass, "store", sm_vec4_array_store, 2);
  rb_define_method(s_sm_vec4_array_klass, "resize!", sm_vec4_array_resize, 1);
  rb_define_method(s_sm_vec4_array_klass, "size", sm_vec4_array_size, 0);
  rb_define_method(s_sm_vec4_array_klass, "length", sm_mathtype_array_length, 0);
  rb_define_method(s_sm_vec4_array_klass, "address", sm_get_address, 0);

  s_sm_quat_array_klass = rb_define_class_under(s_sm_snowmath_mod, "QuatArray", rb_cObject);
  rb_const_set(s_sm_quat_array_klass, kRB_CONST_TYPE, s_sm_quat_klass);
  rb_define_singleton_method(s_sm_quat_array_klass, "new", sm_quat_array_new, 1);
  rb_define_method(s_sm_quat_array_klass, "fetch", sm_quat_array_fetch, 1);
  rb_define_method(s_sm_quat_array_klass, "store", sm_quat_array_store, 2);
  rb_define_method(s_sm_quat_array_klass, "resize!", sm_quat_array_resize, 1);
  rb_define_method(s_sm_quat_array_klass, "size", sm_quat_array_size, 0);
  rb_define_method(s_sm_quat_array_klass, "length", sm_mathtype_array_length, 0);
  rb_define_method(s_sm_quat_array_klass, "address", sm_get_address, 0);

  s_sm_mat3_array_klass = rb_define_class_under(s_sm_snowmath_mod, "Mat3Array", rb_cObject);
  rb_const_set(s_sm_mat3_array_klass, kRB_CONST_TYPE, s_sm_mat3_klass);
  rb_define_singleton_method(s_sm_mat3_array_klass, "new", sm_mat3_array_new, 1);
  rb_define_method(s_sm_mat3_array_klass, "fetch", sm_mat3_array_fetch, 1);
  rb_define_method(s_sm_mat3_array_klass, "store", sm_mat3_array_store, 2);
  rb_define_method(s_sm_mat3_array_klass, "resize!", sm_mat3_array_resize, 1);
  rb_define_method(s_sm_mat3_array_klass, "size", sm_mat3_array_size, 0);
  rb_define_method(s_sm_mat3_array_klass, "length", sm_mathtype_array_length, 0);
  rb_define_method(s_sm_mat3_array_klass, "address", sm_get_address, 0);

  s_sm_mat4_array_klass = rb_define_class_under(s_sm_snowmath_mod, "Mat4Array", rb_cObject);
  rb_const_set(s_sm_mat4_array_klass, kRB_CONST_TYPE, s_sm_mat4_klass);
  rb_define_singleton_method(s_sm_mat4_array_klass, "new", sm_mat4_array_new, 1);
  rb_define_method(s_sm_mat4_array_klass, "fetch", sm_mat4_array_fetch, 1);
  rb_define_method(s_sm_mat4_array_klass, "store", sm_mat4_array_store, 2);
  rb_define_method(s_sm_mat4_array_klass, "resize!", sm_mat4_array_resize, 1);
  rb_define_method(s_sm_mat4_array_klass, "size", sm_mat4_array_size, 0);
  rb_define_method(s_sm_mat4_array_klass, "length", sm_mathtype_array_length, 0);
  rb_define_method(s_sm_mat4_array_klass, "address", sm_get_address, 0);

  #endif

}
