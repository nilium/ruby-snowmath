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

class Snow::Vec3 ; include Snow::MathPointers ; end
class Snow::Vec4 ; include Snow::MathPointers ; end
class Snow::Quat ; include Snow::MathPointers ; end
class Snow::Mat4 ; include Snow::MathPointers ; end

if Snow.const_defined?(:Vec3Array)
  class Snow::Vec3Array ; include Snow::MathPointers ; end
end

if Snow.const_defined?(:Vec4Array)
  class Snow::Vec4Array ; include Snow::MathPointers ; end
end

if Snow.const_defined?(:QuatArray)
  class Snow::QuatArray ; include Snow::MathPointers ; end
end

if Snow.const_defined?(:Mat4Array)
  class Snow::Mat4Array ; include Snow::MathPointers ; end
end

