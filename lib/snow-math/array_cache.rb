require 'snow-math/bindings'

module Snow

  [:Vec3Array, :Vec4Array, :QuatArray, :Mat4Array].each {
    |klass_sym|
    if const_defined?(klass_sym)
      const_get(klass_sym).class_exec {
        alias_method :__fetch__, :fetch
        alias_method :__resize__!, :resize!
        alias_method :__array_cache_initialize__, :initialize

        def initialize(*args)
          @__cache__ = []
          __array_cache_initialize__(*args)
        end

        def fetch(index)
          @__cache__[index] || (@__cache__[index] = __fetch__(index))
        end

        alias_method :[], :fetch

        def resize!(new_length)
          @__cache__ = []
          __resize__!(new_length)
        end

        def resize(new_length)
          arr = self.class.new(new_length)
          self_length = self.length
          (0 ... (self_length < new_length ? self_length : new_length)).each {
            |index|
            arr.store(index, self.fetch(index))
          }
          return arr
        end
      }
    end
  }

end
