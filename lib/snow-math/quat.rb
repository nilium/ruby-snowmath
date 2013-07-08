# This file is part of ruby-snowmath.
# Copyright (c) 2013 Noel Raymond Cower. All rights reserved.
# See COPYING for license details.

require 'snow-math/bindings'

module Snow ; end

if Snow.const_defined?(:QuatArray)
  class Snow::QuatArray
    class << self ; alias_method :[], :new ; end

    alias_method :[], :fetch
    alias_method :[]=, :store
  end
end

class Snow::Quat

  class << self ; alias_method :[], :new ; end

  alias_method :[], :fetch
  alias_method :[]=, :store
  alias_method :dup, :copy
  alias_method :clone, :copy

  def x
    self[0]
  end

  def x=(value)
    self[0] = value
  end

  def y
    self[1]
  end

  def y=(value)
    self[1] = value
  end

  def z
    self[2]
  end

  def z=(value)
    self[2] = value
  end

  def w
    self[3]
  end

  def w=(value)
    self[3] = value
  end

  def normalize!
    normalize self
  end

  def inverse!
    inverse self
  end

  def negate!
    negate self
  end

  def multiply_quat!(rhs)
    multiply_quat rhs, self
  end

  def multiply(rhs, output = nil)
    case rhs
    when ::Snow::Quat then multiply_quat(rhs, output)
    when ::Snow::Vec3 then multiply_vec3(rhs, output)
    when Numeric then scale(rhs, output)
    else raise TypeError, "Invalid type for RHS"
    end
  end

  def multiply!(rhs)
    multiply_quat! rhs
  end

  def add!(rhs)
    add rhs, self
  end

  def subtract!(rhs)
    subtract rhs, self
  end

  def scale!(rhs)
    scale rhs, self
  end

  def divide!(rhs)
    divide rhs, self
  end

  alias_method :-, :subtract
  alias_method :+, :add
  alias_method :**, :dot_product
  alias_method :*, :multiply
  alias_method :/, :divide
  alias_method :-@, :negate
  alias_method :~, :inverse

end
