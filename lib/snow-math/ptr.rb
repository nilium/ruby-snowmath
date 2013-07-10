# This file is part of ruby-snowmath.
# Copyright (c) 2013 Noel Raymond Cower. All rights reserved.
# See COPYING for license details.

require 'snow-math/bindings'
require 'fiddle'

module Snow

  #
  # A module to provide to_ptr methods for all Snow math types. This is optional
  # and only used if you require `snow-math/ptr` because it depends on Fiddle,
  # so if you don't want Fiddle pulled into your environment, you don't want to
  # require the file for this.
  #
  module FiddlePointerSupport
    #
    # Returns a Fiddle::Pointer to the memory address of the object that extends
    # through the size of the object.
    #
    # This function is only provided if you've required `snow-math/ptr`.
    #
    # call-seq: to_ptr -> Fiddle::Pointer
    #
    def to_ptr
      Fiddle::Pointer.new(self.address, self.size)
    end
  end

  class Vec2
    include ::Snow::FiddlePointerSupport
  end

  class Vec3
    include ::Snow::FiddlePointerSupport
  end

  class Vec4
    include ::Snow::FiddlePointerSupport
  end

  class Quat
    include ::Snow::FiddlePointerSupport
  end

  class Mat3
    include ::Snow::FiddlePointerSupport
  end

  class Mat4
    include ::Snow::FiddlePointerSupport
  end

  if const_defined?(:Vec2Array)
    class Vec2Array
      include ::Snow::FiddlePointerSupport
    end
  end

  if const_defined?(:Vec3Array)
    class Vec3Array
      include ::Snow::FiddlePointerSupport
    end
  end

  if const_defined?(:Vec4Array)
    class Vec4Array
      include ::Snow::FiddlePointerSupport
    end
  end

  if const_defined?(:QuatArray)
    class QuatArray
      include ::Snow::FiddlePointerSupport
    end
  end

  if const_defined?(:Mat3Array)
    class Mat3Array
      include ::Snow::FiddlePointerSupport
    end
  end

  if const_defined?(:Mat4Array)
    class Mat4Array
      include ::Snow::FiddlePointerSupport
    end
  end

end
