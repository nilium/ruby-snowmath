# This file is part of ruby-snowmath.
# Copyright (c) 2013 Noel Raymond Cower. All rights reserved.
# See COPYING for license details.

require 'snow-math/bindings'

module Snow ; end

if Snow.const_defined?(:Vec3Array)
  #
  # A contiguous array of Vec3s. Allocated as a single block of memory so that
  # it can easily be passed back to C libraries (like OpenGL) and to aid with
  # cache locality.
  #
  # Useful for storing vertex data such as positions, normals, and so on.
  #
  class Snow::Vec3Array
    class << self ; alias_method :[], :new ; end

    alias_method :[], :fetch
    alias_method :[]=, :store
  end
end

#
# A 3-component vector class.
#
class Snow::Vec3

  # Shortcut through to new
  class << self ; alias_method :[], :new ; end

  alias_method :[], :fetch
  alias_method :[]=, :store
  alias_method :dup, :copy
  alias_method :clone, :copy


  # Returns the X component of the vector.
  #
  # call-seq: x -> float
  def x
    self[0]
  end

  # Sets the X component of the vector.
  #
  # call-seq: x = value -> value
  def x=(value)
    self[0] = value
  end

  # Returns the Y component of the vector.
  #
  # call-seq: y -> float
  def y
    self[1]
  end

  # Sets the Y component of the vector.
  #
  # call-seq: y = value -> value
  def y=(value)
    self[1] = value
  end

  # Returns the Z component of the vector.
  #
  # call-seq: z -> float
  def z
    self[2]
  end

  # Sets the Z component of the vector.
  #
  # call-seq: z = value -> value
  def z=(value)
    self[2] = value
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

  # Calls #cross_product(rhs, self)
  #
  # call-seq: cross_product!(rhs) -> self
  def cross_product!(rhs)
    cross_product rhs, self
  end

  # Calls #multiply_vec3(rhs, self)
  #
  # call-seq: multiply_vec3!(rhs) -> self
  def multiply_vec3!(rhs)
    multiply_vec3 rhs, self
  end

  # Calls #multiply_vec3 and #scale, respectively.
  #
  # call-seq:
  #     multiply(vec3, output) -> output or new vec3
  #     multiply(scalar, output) -> output or new vec3
  def multiply(rhs, output = nil)
    case rhs
    when ::Snow::Vec3, ::Snow::Vec4, ::Snow::Quat then multiply_vec3(rhs, output)
    when Numeric then scale(rhs, output)
    else raise TypeError, "Invalid type for RHS"
    end
  end

  # Calls #multiply(rhs, self)
  #
  # call-seq: multiply!(rhs) -> self
  def multiply!(rhs)
    multiply rhs, self
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


  alias_method :-, :subtract
  alias_method :+, :add
  alias_method :^, :cross_product
  alias_method :**, :dot_product
  alias_method :*, :multiply
  alias_method :/, :divide
  alias_method :-@, :negate
  alias_method :~, :inverse

end
