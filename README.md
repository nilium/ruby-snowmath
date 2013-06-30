# snow-math

    $ gem install snow-math [-- [--use-float | -F]]



## Intro

snow-math is a small, fairly simple library of 3D math routines implemented in
C with Ruby bindings. It's intended for use with OpenGL and such. Currently, it
provides four 3D math types:

    - Snow::Vec3
    - Snow::Vec4
    - Snow::Quat
    - Snow::Mat4

_Most_ of their functionality is implemented in the C bindings, particularly
anything that should be moderately performant.

By default, snow-math uses 64-bit native floats as its underlying type. So, a
Vec3 is literally a `doube[3]` (or a `vec3_t` in C). If you want it to use
32-bit floats, simply pass the `--use-float` option to gem when installing it.
This will cause the bindings to be compiled using floats instead of doubles.
Like so:

    $ gem install snow-math -- --use-float

If you prefer shorter command-line options, `-F` is a synonym for `--use-float`.


## Usage

In your Ruby scripts, you just need to `require 'snow-math'` and all of the
library will be pulled in _with the exception of `Fiddle::Pointer` support_.
To include the Fiddle support, you'll also need to `require 'snow-math/ptr'`.
This is to avoid dragging the entirety of Fiddle into your code when you're not
using it.

Because it's difficult to produce useful documentation for these bindings, this
is a brief outline of the classes and their functions in Ruby, sans function
bodies. Where a description of the functions is missing, the functions do what
they say on the tin.

### Notes

#### Outputs

Also, keep in mind that for any function provides an output argument, that
argument is optional. If output is non-nil, the function will must write its
the result of its operation to the output object.

When nil, the function will always return a new object. Providing an output
argument can be helpful when you want to avoid some unnecessary allocations.

Combined with the array types, you can either maintain a pool of 3D math objects
or use it to avoid cache misses (though right now the array objects are not the
most-optimized as they allocate a wrapper object per `fetch` -- something to
keep in mind).

In all cases where an output is provided, output may be the the object the
function is called on. So, for example, `vec.negate(vec)` is perfectly valid.


#### Shared by All Types

All types share the following functions. These are not included in the class
bodies below except where their behaviour is notably different.

- `fetch(index)` and `store(index, value)` - To get and set values at the given
    component indices. For typed arrays, index is an array index. These do
    bounds-checking. They are always aliased as `[]` and `[]=` respectively.
    Typically, there are also simpler XYZW components for vectors and
    quaternions. These behave a little differently for typed arrays, which you
    can read about in their section below.
- `address` - Returns the memory address of the object's first component.
- `size` - Returns the size in bytes of the object in memory, not counting any
    overhead introduced by Ruby. This varies depending on whether the gem was
    built using floats or doubles. For typed arrays, this is the size of all
    elements combined. So, a 16-length Mat4Array using doubles is 2048 bytes in
    size, whereas a single Vec3 is 24 bytes.
- `length` - Returns the length in components of the object (3 for Vec3, 4 for
    Vec4 and Quat, and 16 for Mat4). The result of this function should be
    obvious for each type.
- `map(&block)` and `map` - In the first form, yields each component of an
    object to the block and returns an object of the same type with the results.
    In the second form, returns an Enumerator.
- `map!(&block)` and `map!` - Same as the above, but operates on self rather
    than creating a new object.
- `each(&object)` and `each` - Does what you think it does. Second form returns
    an Enumerator.
- `to_a` - Returns an array of all components in the given object.
- `to_ptr` - You have to `require 'snow-math/ptr'` for this. Returns a new
    `Fiddle::Pointer` pointing to the object's address.
- `to_s` - Converts the object to a string that looks more or less like
    `"{ fetch(0), fetch(1), ..., fetch(length - 1) }"`.


#### Swizzling

Vectors and quaternions generate swizzle methods on first use. So, by calling
`some_vector.zyx`, the some_vector's class, will generate a swizzle function
that returns a new Vec3 with components Z, Y, and X of the original vector, in
that order. The components you can use for swizzling on each type are fairly
obvious but are as follows:

- __Vec3__  
    Components: X, Y, and Z.  
    Swizzling three components returns a Vec3.  
    Swizzling four components returns a Vec4.

- __Vec4__  
    Components: X, Y, Z, and W.  
    Swizzling three components returns a Vec3.  
    Swizzling four components returns a Vec4.

- __Quat__  
    Components: X, Y, Z, and W.  
    Swizzling three components returns a Vec3.  
    Swizzling four components returns a Quat.


### Types

#### Snow::Vec3

    class Snow::Vec3

        def self.new() -> Vec3[0, 0, 0]
        def self.new(x, y, z) -> Vec3[x, y, z]
        def self.new(other: Vec3 | Vec4 | Quat) -> Vec3[other.x, other.y, other.z]
        Aliased as :[], so you can use Vec3[0, 1, 2] to call Vec3.new(0, 1, 2).

        self == rhs
        Compares whether two Vec3s are equivalent. rhs may also be a Quat or Vec4,
        in which case the first three components of either will be compared with
        self.

        def set() -> self [no-op]
        def set(x, y, z) -> self
        def set(other) -> self
        Set takes the same arguments as self.new(...) and will call the same initialize
        function as new(). If no arguments are provided to set, it does nothing.

        def copy(output = nil) -> new vec3 or output
        If output is nil, this is equivalent to calling o.dup or o.class.new(o).
        With an output, the method copies self's components to other. For the most
        part, you probably only want to use this with an output.

        def inverse(output = nil) -> new vec3 or output
        def inverse! -> self
        Returns a vector whose components are the multiplicative inverse of the
        vector's components.
        Aliased as :~
        
        def negate(output = nil) -> new vec3 or output
        def negate! -> self
        Returns a vector whose components are the negated form of the vector's
        components. Beware of -0.0 here.
        Aliased as :-@

        def normalize(output = nil) -> new vec3 or output
        def normalize!() -> self

        def cross_product(rhs, output = nil) -> new vec3 or output
        def cross_product!(rhs) -> self

        def multiply_vec3(rhs, output = nil) -> new vec3 or output
        Multiplies both vector's components together and returns the result.

        def multiply(vec3, output = nil) -> same as multiply_vec3
        def multiply(scalar, output = nil) -> same as scale
        def multiply!(rhs) -> same as multiply(rhs, self)
        Non-mutable form aliased as :*

        def add(rhs, output = nil) -> new vec3 or output
        def add!(rhs)
        Aliased as :+

        def subtract(rhs, output = nil) -> new vec3 or output
        def subtract!(rhs) -> self
        Aliased as :-

        def dot_product(rhs) -> Float
        Aliased as :**

        def magnitude_squared -> Float
        def magnitude -> Float
        Returns the squared magnitude and the magnitude, respectively. To give you
        an idea of the difference, magnitude == Math.sqrt(magnitude_squared). So
        if you don't need the exact magnitude and only want to compare the magnitude
        of two things, you can skip a call to sqrt and use the squared magnitude.

        def scale(scalar, output = nil) -> new vec3 or output
        def divide(scalar, output = nil) -> new vec3 or output
        Multiplies or divides all components of the vector by a scalar. In the case
        of divide, a scalar of 0 is undefined behaviour.
        divide is aliased as :/

        def size -> size in bytes
        def length -> length in floats
        size is either 24 or 12, depending on whether the gem was built to use
        floats or doubles. length is always 3.

        def x
        def x=(new_x)
        Same as fetch(0) and store(0, new_x) respectively

        def y
        def y=(new_y)
        Same as fetch(1) and store(1, new_y) respectively

        def z
        def z=(new_z)
        Same as fetch(2) and store(2, new_z) respectively
        
    end



#### Snow::Vec4

Vec4 is fundamentally the same as Vec3, except it lacks a `cross_product`
function, has four components, and provides a `w` accessor for the fourth
component. In addition, it is at times interchangeable with Quat.

    class Snow::Vec4

        def self.new() -> Vec4[0, 0, 0, 1]
        def self.new(x, y, z, w = 1) -> Vec4[x, y, z, w]
        def self.new(other: Vec3 | Vec4 | Quat) -> Vec4[other.x, other.y, other.z, other.w | 1]
        Aliased as :[], so you can use Vec4[0, 1, 2, 3] to call Vec4.new(0, 1, 2, 3).
        When calling Vec4.new and supplying a Vec3 to copy from, the fourth component,
        w, will be set to 1.

        self == rhs
        Compares whether two Vec4s are equivalent. rhs may also be a Quat.

        def set() -> self [no-op]
        def set(x, y, z, w = 1) -> self
        def set(other) -> self
        Set takes the same arguments as self.new(...) and will call the same initialize
        function as new(). If no arguments are provided to set, it does nothing.

        def copy(output = nil) -> new vec4 or output
        If output is nil, this is equivalent to calling o.dup or o.class.new(o).
        With an output, the method copies self's components to other. For the most
        part, you probably only want to use this with an output.

        def inverse(output = nil) -> new vec4 or output
        def inverse! -> self
        Returns a vector whose components are the multiplicative inverse of the
        vector's components.
        
        def negate(output = nil) -> new vec4 or output
        def negate! -> self
        Returns a vector whose components are the negated form of the vector's
        components. Beware of -0.0 here.

        def normalize(output = nil) -> new vec4 or output
        def normalize!() -> self

        def multiply_vec4(rhs, output = nil) -> new vec4 or output
        def multiply_vec4!(rhs) -> self
        Multiplies both vector's components together and returns the result.

        def multiply(vec4, output = nil) -> same as multiply_vec4
        def multiply(numeric, output = nil) -> same as scale
        def multiply!(rhs) -> same as multiply(rhs, self)
        Aliased as :*

        def add(rhs, output = nil) -> new vec4 or output
        def add!(rhs)

        def subtract(rhs, output = nil) -> new vec4 or output
        def subtract!(rhs) -> self

        def dot_product(rhs) -> Float

        def magnitude_squared -> Float
        def magnitude -> Float
        Returns the squared magnitude and the magnitude, respectively. To give you
        an idea of the difference, magnitude == Math.sqrt(magnitude_squared). So
        if you don't need the exact magnitude and only want to compare the magnitude
        of two things, you can skip a call to sqrt and use the squared magnitude.

        def scale(scalar, output = nil) -> new vec4 or output
        def divide(scalar, output = nil) -> new vec4 or output
        Multiplies or divides all components of the vector by a scalar. In the case
        of divide, a scalar of 0 is undefined behaviour.

        def size -> size in bytes
        def length -> length in floats
        size is either 32 or 16. depending on whether the gem was built to use
        floats or doubles. length is always 4.

        def x
        def x=(new_x)
        Same as fetch(0) and store(0, new_x) respectively

        def y
        def y=(new_y)
        Same as fetch(1) and store(1, new_y) respectively

        def z
        def z=(new_z)
        Same as fetch(2) and store(2, new_z) respectively

        def w
        def w=(new_w)
        Same as fetch(3) and store(3, new_w) respectively
        
    end



#### Snow::Quat

Quat is functionally similar to Vec4, except it provides some functions specific
to quaternions. Why these aren't just part of Vec4 is a mystery for the ages
and an attempt to make code more readable.

    class Snow::Quat

        def self.new() -> Quat[0, 0, 0, 1]
        def self.new(x, y, z, w = 1) -> Quat[x, y, z, w]
        def self.new(Mat4) -> Quat from Mat4
        def self.new(other: Vec3 | Quat | Quat) -> Quat[other.x, other.y, other.z, other.w | 1]
        Aliased as :[], so you can use Quat[0, 1, 2, 3] to call Quat.new(0, 1, 2, 3).
        When calling Quat.new and supplying a Vec3 to copy from, the fourth component,
        w, will be set to 1. If provided a Mat4, it will be converted to a Quat. Without
        arguments, Quat.new will return an identity quaternion.

        self == rhs
        Compares whether two Quats are equivalent. rhs may be either a Quat or
        Vec4.

        def set() -> self [no-op]
        def set(x, y, z, w = 1) -> self
        def set(Mat4) -> self
        def set(other) -> self
        This takes the same arguments as self.new and in fact just calls the Quat's
        initializer with the new arguments. The only difference is that calling set()
        without arguments will not set this quaternion to the identity quaternion,
        because the allocator is the one responsible for that. (To do that, use
        load_identity)

        load_identity() -> self
        Resets self to the identity quaternion.

        def copy(output = nil) -> new quat or output
        If output is nil, this is equivalent to calling o.dup or o.class.new(o).
        With an output, the method copies self's components to other. For the most
        part, you probably only want to use this with an output.

        def inverse(output = nil) -> new quat or output
        def inverse! -> self
        Returns the inverse of this quaternion (or writes the inverse to an output
        or itself). This is not the same as a vector's inverse -- keep that in mind.
        
        def negate(output = nil) -> new quat or output
        def negate! -> self
        Returns a quaternion whose components are the negated form of the vector's
        components. Beware of -0.0 here. This is the same as negating a vector.

        def normalize(output = nil) -> new quat or output
        def normalize!() -> self

        def multiply_quat(rhs: Quat, output = nil) -> new quat or output
        def multiply!(rhs: Quat) -> self
        The first form, multiply_quat, multiplies two quaternions and returns the
        resulting quaternion. The second form, multiply!, is the same as calling
        multiply_quat(rhs, self).

        def mutliply_vec3(rhs: Vec3, output = nil) -> new vec3 or output
        Multiplies a vec3 by a quat and returns the resulting vec3.

        def multiply(vec3, output = nil) -> new vec3 or output
        def multiply(quat, output = nil) -> new quat or output
        def multiply(numeric, output = nil) -> new quat or output
        In its first form, it's the same as #multiply_vec3. In its second, it's the
        same as #multiply_quat. In its third, it's the same as #scale.

        def add(rhs, output = nil) -> new quat or output
        def add!(rhs)
        Same as Vec4#add and Vec4#add!.

        def subtract(rhs, output = nil) -> new quat or output
        def subtract!(rhs) -> self
        Same as Vec4#subtract and Vec4#subtract!.

        def dot_product(rhs) -> Float
        Same as Vec4#dot_product.

        def magnitude_squared -> Float
        def magnitude -> Float
        Returns the squared magnitude and the magnitude, respectively. To give you
        an idea of the difference, magnitude == Math.sqrt(magnitude_squared). So
        if you don't need the exact magnitude and only want to compare the magnitude
        of two things, you can skip a call to sqrt and use the squared magnitude.

        def scale(scalar, output = nil) -> new quat or output
        def divide(scalar, output = nil) -> new quat or output
        Multiplies or divides all components of the quaternion by a scalar. In the
        case of divide, a scalar of 0 is undefined behaviour.

        def size -> size in bytes
        def length -> length in floats
        size is either 32 or 16. depending on whether the gem was built to use
        floats or doubles. length is always 4.

        def x
        def x=(new_x)
        Same as fetch(0) and store(0, new_x) respectively

        def y
        def y=(new_y)
        Same as fetch(1) and store(1, new_y) respectively

        def z
        def z=(new_z)
        Same as fetch(2) and store(2, new_z) respectively

        def w
        def w=(new_w)
        Same as fetch(3) and store(3, new_w) respectively
        
    end



#### Snow::Mat4

    class Snow::Mat4

        def self.new() -> identity mat4
        def self.new(m1, m2, m3, ..., m16) -> mat4 with the given components
        def self.new([Vec4, Vec4, Vec4, Vec4]) -> mat4 with rows defined by the given Vec4s
        def self.new(Quat) -> Mat4 from Quat
        def self.new(Mat4) -> copy of the given Mat4
        Aliased as `[]`, so you can use Mat4[...] to create a new Mat4.

        def self.translation(x, y, z, output = nil) -> new mat4 or output
        def self.translation(Vec3, output = nil) -> new mat4 or output
        Returns a translation matrix.

        def self.angle_axis(angle_degrees, axis: Vec3, output = nil) -> new mat4 or output
        Returns a new rotation matrix for the given angle and axis. The angle is in
        degrees. This might offend some people, but I assure you, your sanity will
        be preserved by doing this.

        def self.frustum(left, right, bottom, top, z_near, z_far, output = nil) -> new mat4 or output
        def self.orthographic(left, right, bottom, top, z_near, z_far, output = nil) -> new mat4 or output
        def self.perspective(fov_y, aspect, z_near, z_far, output = nil) -> new mat4 or output
        Returns the given kinds of projection matrices. In the case of perspective,
        fov_y is specified in degrees. Again, your sanity will thank you.

        def self.look_at(eye: Vec3, center: Vec3, up: Vec3, output = nil) -> new mat4 or output
        Returns a look-at matrix.

        def set(...)
        Same variations and arguments as self.new(...)

        def load_identity() -> self
        Resets self to the identity matrix.

        def copy(output = nil) -> new mat4 or output
        Copies self to the given output matrix.

        def transpose(output = nil) -> new mat4 or output
        def transpose!() -> self

        def inverse_orthogonal(output = nil) -> new mat4 or output
        def inverse_affine(output = nil) -> new mat4 or output on success, nil on failure
        def inverse_general(output = nil) -> new mat4 or output on success, nil on failure
        def inverse_orthogonal!() -> self
        def inverse_affine!() -> self on success, nil on failure
        def inverse_general!() -> self on success, nil on failure

        def translate(x, y, z, output = nil) -> new mat4 or output
        def translate(vec3, output = nil) -> new mat4 or output
        def translate!(x, y, z) -> self
        def translate!(vec3) -> self
        Essentially multiplies this matrix by a translation matrix with the given
        translation and returns it.

        def multiply_mat4(rhs, output = nil) -> new mat4 or output
        def multiply_mat4!(rhs) -> self
        Multiplies this and the rhs matrix and returns the result.

        def multiply_vec4(rhs, output = nil) -> new vec4 or output
        Transforms a vec4 by the matrix and returns the result.

        def multiply_vec3(rhs, output = nil) -> new vec3 or output
        def rotate_vec3(rhs, output = nil) -> new vec3 or output
        In the first form, transforms a vec3 by the matrix and returns the result.
        In the second form, rotates a vec3 by the matrix, ignoring any translation,
        and returns the result.

        def inverse_rotate_vec3(vec3, output = nil) -> new vec3 or output
        Essentially just a convenience function.

        def multiply(mat4, output = nil) -> same as mutliply_mat4
        def multiply(vec3, output = nil) -> same as mutliply_vec3
        def multiply(numeric, output = nil) -> same as scale(N, N, N, output)
        def multiply!(rhs) -> same as multiply(rhs, self)
        The fourth form, multiply!, will fail if attempting to multiply a vec3.
        Aliased as `*`

        def adjoint(output = nil) -> new mat4 or output
        def adjoint!() -> new mat4 or output

        def determinant -> Float

        def scale(x, y, z, output = nil) -> new mat4 or output
        def scale!(x, y, z) -> new mat4 or output
        Returns a matrix with its inner 3x3 matrix's rows scaled by the given
        components.

        def set_row3, set_column3, set_row4, set_column4(index, V) -> self
        V is a vector with the number of components in the function name. Sets the
        Mat4's row or column to the components of the given vec3 or vec4.

        def get_row3, get_column3, get_row4, get_column4(index, output = nil) -> row/column
        Returns a vector with the number of components as specific by the name for
        the given row or column in the Mat4.

    end



#### Snow::Vec3Array, Snow::Vec4Array, Snow::QuatArray, Snow::Mat4Array

All typed arrays have the same interface and differ only in the kind of object
they contain. As such, this applies to all typed arrays.

    class Snow::Vec3Array, Snow::Vec4Array, Snow::QuatArray, Snow::Mat4Array
        
        def self.new(length) -> new array
        def self.new(array) -> copy of array
        Aliased as `[]`, so you can create an array, for example, by writing
        `Vec3Array[16]`. The length provided must be greater than zero, otherwise
        the returned array is nil.

        Array elements are always uninitialized on allocation.

        def fetch(index) -> object of array element type
        Returns an object that references the array's internal data. Manipulating
        this object manipulates the data in the array -- it is not simply a copy of
        the array's data.

        At present, this always allocates a new wrapper object, so use it sparingly.
        This will likely change in the future, but just be aware of this.

        def store(index, value) -> value
        Copies a value object's data into the array's data. If the value object is
        already part of the array -- that is, it was created using the array's
        fetch function, this is a no-op.

    end



## License

snow-math, like most of my gems, is licensed under a simplified BSD license.
And like most of my gems, I will say as usual that if this is a problem for
you, please contact me. The license is reproduced here:

    Copyright (c) 2013, Noel Raymond Cower <ncower@gmail.com>.
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    1. Redistributions of source code must retain the above copyright notice, this
       list of conditions and the following disclaimer. 
    2. Redistributions in binary form must reproduce the above copyright notice,
       this list of conditions and the following disclaimer in the documentation
       and/or other materials provided with the distribution. 

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
    ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

    The views and conclusions contained in the software and documentation are those
    of the authors and should not be interpreted as representing official policies,
    either expressed or implied, of the FreeBSD Project.
