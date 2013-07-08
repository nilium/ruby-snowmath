# This file is part of ruby-snowmath.
# Copyright (c) 2013 Noel Raymond Cower. All rights reserved.
# See COPYING for license details.

require 'snow-math'

module Snow

  #
  # Provides basic support for converting Snow math objects to Ruby arrays. In
  # addition, it also provides rudimentary support for each, map, and map! for
  # all math types.
  #
  # For example:
  #
  #     # Arrays of cells by column
  #     Mat4[*(1 .. 16)].group_by {
  #       |cell|
  #       (cell.floor - 1) % 4
  #     }
  #
  #     # Arrays of cells by row
  #     Mat4[*(1 .. 16)].group_by {
  #       |cell|
  #       ((cell - 1) / 4).floor
  #     } # => { 0 => [1, 2, 3, 4],
  #       #      1 => [5, 6, 7, 8],
  #       #      2 => [9, 10, 11, 12],
  #       #      3 => [13, 14, 15, 16] }
  #
  # Note that these examples are only showing that you can use these types like
  # most others that include the Enumerable module. The above examples are not
  # sane ways to get columns or rows out of a Mat4.
  #
  module ArraySupport

    include ::Enumerable

    #
    # Returns an array composed of the elements of self.
    #
    # call-seq: to_a -> new_ary
    #
    def to_a
      (0 ... self.length).each.map { |index| fetch(index) }
    end

    #
    # Iterates over all elements of the object and yields them to a block.
    # In the second form, returns an Enumerator.
    #
    # call-seq:
    #   each { |elem| block } -> self
    #   each -> Enumerator
    #
    def each(&block)
      return to_enum(:each) unless block_given?
      (0 ... self.length).each {
        |index|
        yield(fetch(index))
      }
      self
    end

    #
    # In the first form, iterates over all elements of the object, yields them
    # to the block given, and overwrites the element's value with the value
    # returned by the block.
    #
    # In the second form, returns an Enumerator.
    #
    # The return value of the block must be the same kind of object as was
    # yielded to the block. So, if yielded a Vec3, the block must return a Vec3.
    # If yielded a Numeric, it must return a Numeric.
    #
    # call-seq:
    #   map! { |elem| block } -> self
    #   map! -> Enumerator
    #
    def map!(&block)
      return to_enum(:map!) unless block_given?
      (0 ... self.length).each {
        |index|
        store(index, yield(fetch(index)))
      }
      self
    end

    #
    # In the first form, duplicates self and then calls map! on the duplicated
    # object, passing the block to map!.
    #
    # In the second form, returns an Enumerator.
    #
    # The return value of the block must be the same kind of object as was
    # yielded to the block. So, if yielded a Vec3, the block must return a Vec3.
    # If yielded a Numeric, it must return a Numeric.
    #
    # call-seq:
    #   map { |elem| block } -> new object
    #   map -> Enumerator
    #
    def map(&block)
      return to_enum(:map) unless block_given?
      self.dup.map!(&block)
    end

  end

  class Vec3 ; include ::Snow::ArraySupport ; end
  class Vec4 ; include ::Snow::ArraySupport ; end
  class Quat ; include ::Snow::ArraySupport ; end
  class Mat3 ; include ::Snow::ArraySupport ; end
  class Mat4 ; include ::Snow::ArraySupport ; end

  if const_defined?(:Vec3Array)
    class Vec3Array
      include ::Snow::ArraySupport

      #
      # Duplicates the Vec3Array and returns it.
      #
      # call-seq: dup -> new vec3_array
      #
      def dup
        self.class.new(self)
      end
    end
  end

  if const_defined?(:Vec4Array)
    class Vec4Array
      include ::Snow::ArraySupport

      #
      # Duplicates the Vec4Array and returns it.
      #
      # call-seq: dup -> new vec4_array
      #
      def dup
        self.class.new(self)
      end
    end
  end

  if const_defined?(:QuatArray)
    class QuatArray
      include ::Snow::ArraySupport

      #
      # Duplicates the QuatArray and returns it.
      #
      # call-seq: dup -> new quat_array
      #
      def dup
        self.class.new(self)
      end
    end
  end

  if const_defined?(:Mat3Array)
    class Mat3Array
      include ::Snow::ArraySupport

      #
      # Duplicates the Mat3Array and returns it.
      #
      # call-seq: dup -> new mat3_array
      #
      def dup
        self.class.new(self)
      end
    end
  end

  if const_defined?(:Mat4Array)
    class Mat4Array
      include ::Snow::ArraySupport

      #
      # Duplicates the Mat4Array and returns it.
      #
      # call-seq: dup -> new mat4_array
      #
      def dup
        self.class.new(self)
      end
    end
  end

end
