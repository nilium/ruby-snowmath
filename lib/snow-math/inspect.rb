# This file is part of ruby-snowmath.
# Copyright (c) 2013 Noel Raymond Cower. All rights reserved.
# See COPYING for license details.

require 'snow-math'

module Snow ; end

module Snow::MathInspect

  def inspect
    "<#{self.class.name}:0x#{self.address.to_s(16)} #{(0 ... self.length).map { |i| self[i] }.join(' ')}>"
  end

end

class Snow::Vec3 ; include Snow::MathInspect ; end
class Snow::Vec4 ; include Snow::MathInspect ; end
class Snow::Quat ; include Snow::MathInspect ; end
class Snow::Mat4 ; include Snow::MathInspect ; end
