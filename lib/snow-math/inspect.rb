# This file is part of ruby-snowmath.
# Copyright (c) 2013 Noel Raymond Cower. All rights reserved.
# See COPYING for license details.

require 'snow-math/bindings'
require 'snow-math/to_a'

module Snow

  #
  # Provides support for inspect calls on Snow math types.
  #
  module InspectSupport

    #
    # Returns a serializable string for a given type.
    #
    # Output is defined as `<Type:Address Elements>`. For example:
    #
    #     Vec4[1, 2, 3, 4]   # => <Snow::Vec4:0x7f831981f5e0 1.0 2.0 3.0 4.0>
    #
    #     # The following example inner Vec3's elements have been made 0.0 even
    #     # though they would ordinarily be garbage values.
    #     Vec3Array[1]       # => <Snow::Vec3Array:0x7fe193b297a0 <Snow::Vec3:0x7fe193b297a0 0.0 0.0 0.0>>
    #
    # Bear in mind that the address may not be useful for serialization beyond
    # determining when a given object has already been deserialized. You may
    # want to treat it as semi-unique ID if serializing a math object, though
    # there is absolutely no guarantee it will be unique between allocations and
    # deallocations, as memory may obviously be reused.
    #
    def inspect
      "<#{self.class.name}:0x#{self.address.to_s(16)} #{(0 ... self.length).map { |i| self.fetch(i).inspect }.join(' ')}>"
    end

  end

  class Vec2 ; include ::Snow::InspectSupport ; end
  class Vec3 ; include ::Snow::InspectSupport ; end
  class Vec4 ; include ::Snow::InspectSupport ; end
  class Quat ; include ::Snow::InspectSupport ; end
  class Mat3 ; include ::Snow::InspectSupport ; end
  class Mat4 ; include ::Snow::InspectSupport ; end

  if const_defined?(:Vec2Array)
    class Vec2Array ; include ::Snow::InspectSupport ; end
  end

  if const_defined?(:Vec3Array)
    class Vec3Array ; include ::Snow::InspectSupport ; end
  end

  if const_defined?(:Vec4Array)
    class Vec4Array ; include ::Snow::InspectSupport ; end
  end

  if const_defined?(:QuatArray)
    class QuatArray ; include ::Snow::InspectSupport ; end
  end

  if const_defined?(:Mat3Array)
    class Mat3Array ; include ::Snow::InspectSupport ; end
  end

  if const_defined?(:Mat4Array)
    class Mat4Array ; include ::Snow::InspectSupport ; end
  end

end
