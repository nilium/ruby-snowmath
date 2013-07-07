# This file is part of ruby-snowmath.
# Copyright (c) 2013 Noel Raymond Cower. All rights reserved.
# See COPYING for license details.

require 'snow-math/bindings'

module Snow ; end

if Snow.const_defined?(:Mat4Array)
  class Snow::Mat3Array
    class << self ; alias_method :[], :new ; end

    alias_method :[], :fetch
    alias_method :[]=, :store
  end
end

class Snow::Mat3

  class << self ; alias_method :[], :new ; end

  alias_method :[], :fetch
  alias_method :[]=, :store

  def transpose!
    transpose self
  end

  def inverse!
    inverse self
  end

  def adjoint!
    adjoint self
  end

  def cofactor!
    cofactor self
  end

  def multiply_mat3!(rhs)
    multiply_mat3 rhs, self
  end

  def multiply(rhs, out = nil)
    raise "Invalid type for output, must be the same as RHS" if !out.nil? && !out.kind_of?(rhs.class)
    case rhs
    when ::Snow::Mat3 then multiply_mat3(rhs, out)
    when ::Snow::Vec3 then rotate_vec3(rhs, out)
    when Numeric      then scale(rhs, rhs, rhs, out)
    else raise TypeError, "Invalid type for RHS"
    end
  end

  def multiply!(rhs)
    multiply rhs, case rhs
      when Mat3, Numeric then self
      when Vec3 then rhs
      else raise TypeError, "Invalid type for RHS"
      end
  end

  def scale!(x, y, z)
    scale x, y, z, self
  end

  alias_method :*, :multiply
  alias_method :**, :scale
  alias_method :~, :transpose

end
