# This file is part of ruby-snowmath.
# Copyright (c) 2013 Noel Raymond Cower. All rights reserved.
# See COPYING for license details.

require 'snow-math'

module Snow ; end

module Snow

  #
  # Provides swizzle function support for Vec3, Vec4, and Quat. At runtime, if
  # a message matching `/^[xyzw]{3,4}$/`, sans `w` in Vec3's case, is sent to
  # any of those types, a new method will be generated to return a swizzled
  # object of that type or one that contains at least as many elements as
  # requested.
  #
  # For Vec3, valid swizzle components are `x`, `y`, and `z`. If a swizzle
  # function with three of those components is called, a new Vec3 is returned.
  # If a swizzle function with four components is called, a Vec4 is returned.
  # This is the same for Vec4, although it also provides a `w` component.
  #
  # For Quat types, behavior is similar to Vec4, though a 4-component swizzle
  # function returns a Quat instead of a Vec4.
  #
  #     # Good
  #     Vec3[1, 2, 3].xxx           # => Vec3[1, 1, 1]
  #     Vec3[1, 2, 3].xxzz          # => Vec4[1, 1, 3, 3]
  #     Vec4[1, 2, 3, 4].xzwy       # => Vec4[1, 3, 4, 2]
  #     Quat[1, 2, 3, 4].wzyx       # => Quat[4, 3, 2, 1]
  #
  #     # Bad
  #     Vec3[1, 2, 3].www           # => Invalid, no W component for Vec3
  #     Vec3[1, 2, 3].xx            # => Invalid, no 2-component vector type
  #     Vec3[1, 2, 3].xxxxx         # => Invalid, no 5-component vector type
  #
  module SwizzleSupport

    alias_method :__under_method_missing__, :method_missing

    #
    # Generates a swizzle function according to a class's @@SWIZZLE_CHARS and
    # @@SWIZZLE_MAPPING class variables.
    #
    # Overrides old method_missing implementation. The old implementation is
    # aliased as \_\_under_method_missing__ and called when no swizzle function
    # can be generated for a symbol.
    #
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
          define_method(sym) {
            klass.new(indices.map { |index| self.fetch(index) })
          }
        }
        return self.send(sym)
      end

      __under_method_missing__ sym, *args
    end

  end

end

class Snow::Vec2
  @@SWIZZLE_CHARS = /^[xy]{2,4}$/
  @@SWIZZLE_MAPPING = { 2 => self, 3 => ::Snow::Vec3, 4 => ::Snow::Vec4, 'x' => 0, 'y' => 1 }
  include ::Snow::SwizzleSupport
end

class Snow::Vec3
  @@SWIZZLE_CHARS = /^[xyz]{2,4}$/
  @@SWIZZLE_MAPPING = { 2 => ::Snow::Vec2, 3 => self, 4 => ::Snow::Vec4, 'x' => 0, 'y' => 1, 'z' => 2 }
  include ::Snow::SwizzleSupport
end

class Snow::Vec4
  @@SWIZZLE_CHARS = /^[xyzw]{2,4}$/
  @@SWIZZLE_MAPPING = { 2 => ::Snow::Vec2, 3 => ::Snow::Vec3, 4 => self, 'x' => 0, 'y' => 1, 'z' => 2, 'w' => 3 }
  include ::Snow::SwizzleSupport
end

class Snow::Quat
  @@SWIZZLE_CHARS = /^[xyzw]{2,4}$/
  @@SWIZZLE_MAPPING = { 2 => ::Snow::Vec2, 3 => ::Snow::Vec3, 4 => self, 'x' => 0, 'y' => 1, 'z' => 2, 'w' => 3 }
  include ::Snow::SwizzleSupport
end
