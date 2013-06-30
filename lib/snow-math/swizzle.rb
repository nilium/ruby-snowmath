# This file is part of ruby-snowmath.
# Copyright (c) 2013 Noel Raymond Cower. All rights reserved.
# See COPYING for license details.

require 'snow-math'

module Snow ; end

module Snow::Swizzle

  def method_missing(sym, *args)
    chars = sym.to_s
    if chars =~ self.class.class_variable_get(:@@SWIZZLE_CHARS)
      mapping = self.class.class_variable_get(:@@SWIZZLE_MAPPING)
      arg_indices = chars.each_char.map {
        |char|
        index = mapping[char]
        if index.nil?
          raise ArgumentError, "No index mapping for swizzle character #{char} found"
        end
        index
      }
      swizzle_klass = mapping[chars.length]

      if swizzle_klass.nil?
        raise ArgumentError, "No swizzle class defined for #{chars.length} components"
      end

      self.class.class_exec(arg_indices, swizzle_klass) {
        |indices, klass|
        define_method(sym) { klass.new(indices.map { |index| self.fetch(index) }) }
      }
      return self.send(sym)
    end

    super sym, *args
  end

end

class Snow::Vec3
  @@SWIZZLE_CHARS = /^[xyz]{3,4}$/
  @@SWIZZLE_MAPPING = { 3 => self, 4 => ::Snow::Vec4, 'x' => 0, 'y' => 1, 'z' => 2 }
  include ::Snow::Swizzle
end

class Snow::Vec4
  @@SWIZZLE_CHARS = /^[xyzw]{3,4}$/
  @@SWIZZLE_MAPPING = { 3 => ::Snow::Vec3, 4 => self, 'x' => 0, 'y' => 1, 'z' => 2, 'w' => 3 }
  include ::Snow::Swizzle
end

class Snow::Quat
  @@SWIZZLE_CHARS = /^[xyzw]{3,4}$/
  @@SWIZZLE_MAPPING = { 3 => ::Snow::Vec3, 4 => self, 'x' => 0, 'y' => 1, 'z' => 2, 'w' => 3 }
  include ::Snow::Swizzle
end
