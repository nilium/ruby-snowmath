# This file is part of ruby-snowmath.
# Copyright (c) 2013 Noel Raymond Cower. All rights reserved.
# See COPYING for license details.

require 'snow-math/bindings'

module Snow ; end

if Snow.const_defined?(:Mat4Array)
  #
  # A contiguous array of Mat4s. Allocated as a single block of memory so that
  # it can easily be passed back to C libraries (like OpenGL) and to aid with
  # cache locality.
  #
  # May also be useful to subclass as a stack of Mat4s akin to now-deprecated
  # functionality in OpenGL.
  #
  class Snow::Mat4Array
    class << self ; alias_method :[], :new ; end

    alias_method :[], :fetch
    alias_method :[]=, :store
  end
end

#
# A 4x4 matrix. Useful for anything from rotation to projection to almost any
# other 3D transformation you might need.
#
class Snow::Mat4

  IDENTITY = self.new.freeze
  ONE      = self.new(Array.new(16, 1)).freeze
  ZERO     = self.new(Array.new(16, 0)).freeze

  class << self ; alias_method :[], :new ; end

  alias_method :[], :fetch
  alias_method :[]=, :store
  alias_method :dup, :copy
  alias_method :clone, :copy


  def to_quat
    Quat.new(self)
  end

  # Calls #transpose(self)
  #
  # call-seq: transpose! -> self
  def transpose!
    transpose self
  end

  # Calls #inverse_orthogonal(self)
  #
  # call-seq: inverse_orthogonal! -> self
  def inverse_orthogonal!
    inverse_orthogonal self
  end

  # Calls #adjoint(self)
  #
  # call-seq: adjoint! -> self
  def adjoint!
    adjoint self
  end

  # Calls #multiply_mat4(rhs, self)
  #
  # call-seq: multiply_mat4!(rhs) -> self
  def multiply_mat4!(rhs)
    multiply_mat4 rhs, self
  end

  # Calls #multiply_vec4(rhs, rhs)
  #
  # call-seq: multiply_vec4!(rhs) -> rhs
  def multiply_vec4!(rhs)
    multiply_vec4 rhs, rhs
  end

  # Calls #transform_vec3(rhs, rhs)
  #
  # call-seq: transform_vec3!(rhs) -> rhs
  def transform_vec3!(rhs)
    transform_vec3 rhs, rhs
  end

  # Calls #inverse_transform_vec3(rhs, rhs)
  #
  # call-seq: inverse_transform_vec3!(rhs) -> rhs
  def rotate_vec3!(rhs)
    inverse_transform_vec3 rhs, rhs
  end

  # Calls #inverse_rotate_vec3(rhs, rhs)
  #
  # call-seq: inverse_rotate_vec3!(rhs) -> rhs
  def inverse_rotate_vec3!(rhs)
    inverse_rotate_vec3 rhs, rhs
  end

  # Calls #multiply_mat4, #multiply_vec4, #transform_vec3, and #scale,
  # respectively.
  #
  # When calling multiply with scalar as rhs, scalar is passed as the value to
  # scale all columns by.
  #
  # call-seq:
  #     multiply(mat4, output = nil) -> output or new mat4
  #     multiply(vec4, output = nil) -> output or new vec4
  #     multiply(vec3, output = nil) -> output or new vec3
  #     multiply(scalar, output = nil) -> output or new mat4
  def multiply(rhs, out = nil)
    case rhs
    when ::Snow::Mat4 then multiply_mat4(rhs, out)
    when ::Snow::Vec4 then multiply_vec4(rhs, out)
    when ::Snow::Vec3 then transform_vec3(rhs, out)
    when Numeric      then scale(rhs, rhs, rhs, out)
    else raise TypeError, "Invalid type for RHS"
    end
  end

  # Calls #multiply(rhs, self) when rhs is a scalar or Mat4, otherwise calls
  # #multiply(rhs, rhs).
  def multiply!(rhs)
    multiply rhs, case rhs
      when Mat4, Numeric then self
      when Vec4, Vec3 then rhs
      else raise TypeError, "Invalid type for RHS"
      end
  end

  # Calls #scale(x, y, z, self)
  #
  # call-seq: scale!(x, y, z) -> self
  def scale!(x, y, z)
    scale x, y, z, self
  end

  # Calls #translate(*args, self)
  #
  # call-seq:
  #     translate!(vec3) -> self
  #     translate!(x, y, z) -> self
  def translate!(*args)
    translate(*args, self)
  end

  # Calls #inverse_affine(self)
  #
  # call-seq: inverse_affine! -> self
  def inverse_affine!
    inverse_affine self
  end

  # Calls #inverse_general(self)
  #
  # call-seq: inverse_general! -> self
  def inverse_general!
    inverse_general self
  end


  alias_method :*, :multiply
  alias_method :**, :scale
  alias_method :~, :transpose

end
