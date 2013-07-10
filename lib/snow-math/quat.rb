# This file is part of ruby-snowmath.
# Copyright (c) 2013 Noel Raymond Cower. All rights reserved.
# See COPYING for license details.

require 'snow-math/bindings'

module Snow ; end

if Snow.const_defined?(:QuatArray)
  #
  # A contiguous array of Quats. Allocated as a single block of memory so that
  # it can easily be passed back to C libraries (like OpenGL) and to aid with
  # cache locality.
  #
  class Snow::QuatArray
    class << self ; alias_method :[], :new ; end

    alias_method :[], :fetch
    alias_method :[]=, :store
  end
end

#
# A simple quaternion class for representation rotations.
#
class Snow::Quat

  class << self ; alias_method :[], :new ; end

  alias_method :[], :fetch
  alias_method :[]=, :store
  alias_method :dup, :copy
  alias_method :clone, :copy


  def to_vec2
    Vec2.new(self)
  end

  def to_vec3
    Vec3.new(self)
  end

  def to_vec4
    Vec4.new(self)
  end

  def to_quat
    Quat.new(self)
  end

  def to_mat3
    Mat3.new(self)
  end

  def to_mat4
    Mat4.new(self)
  end

  # Returns the X component of the quaternion.
  #
  # call-seq: x -> float
  def x
    self[0]
  end

  # Sets the X component of the quaternion.
  #
  # call-seq: x = value -> value
  def x=(value)
    self[0] = value
  end

  # Returns the Y component of the quaternion.
  #
  # call-seq: y -> float
  def y
    self[1]
  end

  # Sets the Y component of the quaternion.
  #
  # call-seq: y = value -> value
  def y=(value)
    self[1] = value
  end

  # Returns the Z component of the quaternion.
  #
  # call-seq: z -> float
  def z
    self[2]
  end

  # Sets the Z component of the quaternion.
  #
  # call-seq: z = value -> value
  def z=(value)
    self[2] = value
  end

  # Returns the W component of the quaternion.
  #
  # call-seq: w -> float
  def w
    self[3]
  end

  # Sets the W component of the quaternion.
  #
  # call-seq: w = value -> value
  def w=(value)
    self[3] = value
  end

  # Calls #normalize(self)
  #
  # call-seq: normalize! -> self
  def normalize!
    normalize self
  end

  # Calls #inverse(self)
  #
  # call-seq: inverse! -> self
  def inverse!
    inverse self
  end

  # Calls #negate(self)
  #
  # call-seq: negate! -> self
  def negate!
    negate self
  end

  # Calls #multiply_quat(rhs, self)
  #
  # call-seq: multiply_quat!(rhs) -> self
  def multiply_quat!(rhs)
    multiply_quat rhs, self
  end

  # Calls #multiply_vec3(rhs, rhs)
  #
  # call-seq: multiply_vec3!(rhs) -> rhs
  def multiply_vec3!(rhs)
    multiply_vec3 rhs, rhs
  end

  # Wrapper around #multiply_quat, #multiply_vec3, and #scale respectively.
  #
  # call-seq:
  #     multiply(quat, output = nil) -> output or new quat
  #     multiply(scalar, output = nil) -> output or new quat
  #     multiply(vec3, output = nil) -> output or new vec3
  def multiply(rhs, output = nil)
    case rhs
    when ::Snow::Quat then multiply_quat(rhs, output)
    when ::Snow::Vec3 then multiply_vec3(rhs, output)
    when Numeric then scale(rhs, output)
    else raise TypeError, "Invalid type for RHS"
    end
  end

  # Calls #multiply(rhs, self) for scaling and Quat multiplication, otherwise
  # calls #multiply(rhs, rhs) for Vec3 multiplication.
  #
  # call-seq:
  #     multiply!(quat) -> self
  #     multiply!(scalar) -> self
  #     multiply!(vec3) -> vec3
  def multiply!(rhs)
    case rhs
    when ::Snow::Vec3 then multiply(rhs, rhs)
    else multiply(rhs, self)
    end
  end

  # Calls #add(rhs, self)
  #
  # call-seq: add!(rhs) -> self
  def add!(rhs)
    add rhs, self
  end

  # Calls #subtract(rhs, self)
  #
  # call-seq: subtract!(rhs) -> self
  def subtract!(rhs)
    subtract rhs, self
  end

  # Calls #scale(rhs, self)
  #
  # call-seq: scale!(rhs) -> self
  def scale!(rhs)
    scale rhs, self
  end

  # Calls #divide(rhs, self)
  #
  # call-seq: divide!(rhs) -> self
  def divide!(rhs)
    divide rhs, self
  end

  # Calls #slerp(destination, alpha, self)
  #
  # call-seq: slerp!(destination, alpha) -> self
  def slerp!(destination, alpha)
    slerp(destination, alpha, self)
  end


  alias_method :-, :subtract
  alias_method :+, :add
  alias_method :**, :dot_product
  alias_method :*, :multiply
  alias_method :/, :divide
  alias_method :-@, :negate
  alias_method :~, :inverse

end
