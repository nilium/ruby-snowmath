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

  class << self ; alias_method :[], :new ; end

  alias_method :[], :fetch
  alias_method :[]=, :store
  alias_method :dup, :copy
  alias_method :clone, :copy

  def transpose!
    transpose self
  end

  def inverse_orthogonal!
    inverse_orthogonal self
  end

  def adjoint!
    adjoint self
  end

  def multiply_mat4!(rhs)
    multiply_mat4 rhs, self
  end

  def multiply(rhs, out = nil)
    raise "Invalid type for output, must be the same as RHS" if !out.nil? && !out.kind_of?(rhs.class)
    case rhs
    when ::Snow::Mat4 then multiply_mat4(rhs, out)
    when ::Snow::Vec4 then multiply_vec4(rhs, out)
    when ::Snow::Vec3 then transform_vec3(rhs, out)
    when Numeric      then scale(rhs, rhs, rhs, out)
    else raise TypeError, "Invalid type for RHS"
    end
  end

  def multiply!(rhs)
    multiply rhs, case rhs
      when Mat4, Numeric then self
      when Vec4, Vec3 then rhs
      else raise TypeError, "Invalid type for RHS"
      end
  end

  def scale!(x, y, z)
    scale x, y, z, self
  end

  def translate!(*args)
    translate *args, self
  end

  def inverse_affine!
    inverse_affine self
  end

  def inverse_general!
    inverse_general self
  end

  alias_method :*, :multiply
  alias_method :**, :scale
  alias_method :~, :transpose

end
