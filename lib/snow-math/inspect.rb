# This file is part of ruby-snowmath.
# Copyright (c) 2013 Noel Raymond Cower. All rights reserved.
# See COPYING for license details.

require 'snow-math'

module Snow

  [:Vec3, :Vec4, :Quat, :Mat3, :Mat4].each {
    |klass_sym|
    if const_defined?(klass_sym)
      const_get(klass_sym).class_exec {

        def inspect
          "<#{self.class.name}:0x#{self.address.to_s(16)} #{(0 ... self.length).map { |i| self.fetch(i) }.join(' ')}>"
        end

      }
    end
  }

  [:Vec3Array, :Vec4Array, :QuatArray, :Mat3Array, :Mat4Array].each {
    |klass_sym|
    if const_defined?(klass_sym)
      const_get(klass_sym).class_exec {

        def inspect
          "<#{self.class.name}:0x#{self.address.to_s(16)} #{(0 ... self.length).map { |i| self.fetch(i).inspect }.join(' ')}>"
        end

      }
    end
  }

end
