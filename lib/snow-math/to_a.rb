# This file is part of ruby-snowmath.
# Copyright (c) 2013 Noel Raymond Cower. All rights reserved.
# See COPYING for license details.

require 'snow-math'

module Snow

  [:Vec3, :Vec4, :Quat, :Mat3, :Mat4,
   :Vec3Array, :Vec4Array, :QuatArray, :Mat3Array, :Mat4Array].each {
    |klass_sym|
    if const_defined?(klass_sym)
      const_get(klass_sym).class_exec {

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

      }
    end
  }

end
