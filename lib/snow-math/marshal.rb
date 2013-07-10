require 'snow-math/bindings'

module Snow ; end

module Snow::BaseMarshalSupport # :nodoc: all

  #
  def _dump(level)
    # level ignored because it's not applicable
    Marshal.dump(self.to_a)
  end

  module MarshalLoadSupport
    def _load(args)
      new(*Marshal.load(args))
    end
  end

  def self.included(base)
    base.extend(MarshalLoadSupport)
  end

end

module Snow::ArrayMarshalSupport # :nodoc: all

  def _dump(level)
    to_dump = [self.length, *self.to_a.map { |elem| elem.copy }]
    Marshal.dump(to_dump)
  end

  module MarshalLoadSupport
    def _load(args)
      info = Marshal.load(args)
      arr = new(info[0])
      # if not equal, then either something is corrupt or depth was 0
      (1 ... info.length).each { |index| arr.store(index - 1, info[index]) }
      arr
    end
  end

  def self.included(base)
    base.extend(MarshalLoadSupport)
  end

end

module Snow
  class Vec2 ; include ::Snow::BaseMarshalSupport ; end
  class Vec3 ; include ::Snow::BaseMarshalSupport ; end
  class Vec4 ; include ::Snow::BaseMarshalSupport ; end
  class Quat ; include ::Snow::BaseMarshalSupport ; end
  class Mat3 ; include ::Snow::BaseMarshalSupport ; end
  class Mat4 ; include ::Snow::BaseMarshalSupport ; end

  if const_defined?(:Vec2Array)
    class Vec2Array ; include ::Snow::ArrayMarshalSupport ; end
  end

  if const_defined?(:Vec3Array)
    class Vec3Array ; include ::Snow::ArrayMarshalSupport ; end
  end

  if const_defined?(:Vec4Array)
    class Vec4Array ; include ::Snow::ArrayMarshalSupport ; end
  end

  if const_defined?(:QuatArray)
    class QuatArray ; include ::Snow::ArrayMarshalSupport ; end
  end

  if const_defined?(:Mat3Array)
    class Mat3Array ; include ::Snow::ArrayMarshalSupport ; end
  end

  if const_defined?(:Mat4Array)
    class Mat4Array ; include ::Snow::ArrayMarshalSupport ; end
  end

end
