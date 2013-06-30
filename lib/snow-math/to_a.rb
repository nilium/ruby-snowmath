# This file is part of ruby-snowmath.
# Copyright (c) 2013 Noel Raymond Cower. All rights reserved.
# See COPYING for license details.

require 'snow-math'

module Snow ; end

module Snow::EnumeratorSupport
  include ::Enumerable

  def to_a
    (0 ... self.length).each.map { |index| fetch(index) }
  end

  def each(&block)
    return to_enum(:each) unless block_given?
    (0 ... self.length).each {
      |index|
      yield(fetch(index))
    }
    self
  end

  def map!(&block)
    return to_enum(:map!) unless block_given?
    (0 ... self.length).each {
      |index|
      store(index, yield(fetch(index)))
    }
    self
  end

  def map(&block)
    return to_enum(:map) unless block_given?
    self.dup.map!(&block)
  end

end

if Snow.const_defined?(:Vec3Array)
  class Snow::Vec3Array ; include Snow::EnumeratorSupport ; end
  class Snow::Vec4Array ; include Snow::EnumeratorSupport ; end
  class Snow::QuatArray ; include Snow::EnumeratorSupport ; end
  class Snow::Mat4Array ; include Snow::EnumeratorSupport ; end
end

class Snow::Vec3 ; include Snow::EnumeratorSupport ; end
class Snow::Vec4 ; include Snow::EnumeratorSupport ; end
class Snow::Quat ; include Snow::EnumeratorSupport ; end
class Snow::Mat4 ; include Snow::EnumeratorSupport ; end
