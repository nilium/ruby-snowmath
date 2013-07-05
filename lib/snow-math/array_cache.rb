require 'snow-math/bindings'

module Snow

  [:Vec3Array, :Vec4Array, :QuatArray, :Mat4Array].each {
    |klass_sym|
    if const_defined?(klass_sym)
      const_get(klass_sym).class_exec {
        alias_method :__fetch__, :fetch
        alias_method :__array_cache_initialize__, :initialize

        def initialize(*args)
          @__cache__ = []
          __array_cache_initialize__(*args)
        end

        def fetch(index)
          @__cache__[index] || (@__cache__[index] = __fetch__(index))
        end

        alias_method :[], :fetch
      }
    end
  }

end
