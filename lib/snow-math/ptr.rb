# This file is part of ruby-snowmath.
# Copyright (c) 2013 Noel Raymond Cower. All rights reserved.
# See COPYING for license details.

require 'snow-math'
require 'fiddle'

module Snow

  [:Vec3, :Vec4, :Quat, :Mat3, :Mat4,
   :Vec3Array, :Vec4Array, :QuatArray, :Mat3Array, :Mat4Array].each {
    |klass_sym|
    if const_defined?(klass_sym)
      const_get(klass_sym).class_exec {

        def to_ptr
          Fiddle::Pointer.new(self.address, self.size)
        end

      }
    end
  }

end
