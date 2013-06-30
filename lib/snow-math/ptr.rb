# This file is part of ruby-snowmath.
# Copyright (c) 2013 Noel Raymond Cower. All rights reserved.
# See COPYING for license details.

require 'snow-math'
require 'fiddle'

module Snow ; end

module Snow::MathPointers

  def to_ptr
    Fiddle::Pointer.new(self.address, self.size)
  end

end

Snow::Vec3.include(Snow::MathPointers)
Snow::Vec4.include(Snow::MathPointers)
Snow::Quat.include(Snow::MathPointers)
Snow::Mat4.include(Snow::MathPointers)
Snow::Vec3Array.include(Snow::MathPointers) if Snow.const_defined?(:Vec3Array)
Snow::Vec4Array.include(Snow::MathPointers) if Snow.const_defined?(:Vec4Array)
Snow::QuatArray.include(Snow::MathPointers) if Snow.const_defined?(:QuatArray)
Snow::Mat4Array.include(Snow::MathPointers) if Snow.const_defined?(:Mat4Array)
