# This file is part of ruby-snowmath.
# Copyright (c) 2013 Noel Raymond Cower. All rights reserved.
# See COPYING for license details.

require 'snow-math/bindings'

module Snow ; end

if Snow.const_defined?(:Mat3Array)
  #
  # A contiguous array of Mat3s. Allocated as a single block of memory so that
  # it can easily be passed back to C libraries (like OpenGL) and to aid with
  # cache locality.
  #
  # May be useful when subclassed as a stack to recreate now-drepcated OpenGL
  # functionality, though perhaps less-so than a Mat4 stack.
  #
  class Snow::Mat3Array
    class << self ; alias_method :[], :new ; end

    alias_method :[], :fetch
    alias_method :[]=, :store
  end
end

#
# A 3x3 matrix class. Often useful for representation rotations.
#
class Snow::Mat3

  IDENTITY = self.new.freeze
  ONE      = self.new(Array.new(9, 1)).freeze
  ZERO     = self.new(Array.new(9, 0)).freeze

  class << self ; alias_method :[], :new ; end

  alias_method :[], :fetch
  alias_method :[]=, :store
  alias_method :dup, :copy
  alias_method :clone, :copy


  def to_quat
    Quat.new(self)
  end

  #
  # Calls #transpose(self)
  #
  # call-seq: transpose! -> self
  #
  def transpose!
    transpose self
  end

  #
  # Calls #inverse(self)
  #
  # call-seq: inverse! -> self
  #
  def inverse!
    inverse self
  end

  #
  # Calls #adjoint(self)
  #
  # call-seq: adjoint! -> self
  #
  def adjoint!
    adjoint self
  end

  #
  # Calls #cofactor(self)
  #
  # call-seq: cofactor! -> self
  #
  def cofactor!
    cofactor self
  end

  #
  # Calls #multiply_mat3(rhs, self)
  #
  # call-seq: multiply_mat3!(rhs) -> self
  #
  def multiply_mat3!(rhs)
    multiply_mat3 rhs, self
  end

  #
  # Multiplies self and RHS and returns the result. This is a wrapper around
  # other multiply methods. See multiply_mat3, rotate_vec3, and #scale for more
  # reference.
  #
  # In the third form, the scalar value provided is passed for all three columns
  # when calling scale.
  #
  # call-seq:
  #   multiply(mat3, output = nil) -> output or new mat3
  #   multiply(vec3, output = nil) -> output or new vec3
  #   multiply(scalar, output = nil) -> output or new mat3
  #
  def multiply(rhs, out = nil)
    case rhs
    when ::Snow::Mat3 then multiply_mat3(rhs, out)
    when ::Snow::Vec3 then rotate_vec3(rhs, out)
    when Numeric      then scale(rhs, rhs, rhs, out)
    else raise TypeError, "Invalid type for RHS"
    end
  end

  #
  # Calls #multiply(rhs, self).
  #
  # call-seq:
  #     multiply!(mat3) -> self
  #     multiply!(vec3) -> vec3
  #     multiply!(scalar) -> self
  #
  def multiply!(rhs)
    multiply rhs, case rhs
      when Mat3, Numeric then self
      when Vec3 then rhs
      else raise TypeError, "Invalid type for RHS"
      end
  end

  #
  # Calls scale(x, y, z, self)
  #
  # call-seq: scale!(x, y, z) -> self
  #
  def scale!(x, y, z)
    scale x, y, z, self
  end


  alias_method :*, :multiply
  alias_method :**, :scale
  alias_method :~, :transpose

end
