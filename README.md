# snow-math

    $ gem install snow-math [-- [--use-float | -F]]



## Intro

snow-math is a small, fairly simple library of 3D math routines implemented in
C with Ruby bindings. It's intended for use with OpenGL and such. Currently, it
provides four 3D math types:

    - Snow::Vec3
    - Snow::Vec4
    - Snow::Quat
    - Snow::Mat3
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


### Notes

#### Outputs

Keep in mind that for any function provides an output argument, that argument
is optional. If output is non-nil, the function must write the result of its
operation to the output object, assuming the object is a compatible output.

When nil, functions will always return a new object. Providing an output
argument can be helpful when you want to avoid some unnecessary allocations
and know that's a performance bottleneck (use a profiler before doing this).

Combined with the array types, you can either maintain a pool of 3D math
objects or use it to avoid cache misses.

In all cases where an output is provided, output may be the the object the
function is called on. So, for example, `vec.negate(vec)` is perfectly valid.
The SnowPalm code is designed to handle this and will not accidentally trash
itself because the input and output refer to the same location in memory.


#### Thread Safety

Act as though no object is thread-safe. That is, if an object is being modified
on one thread, it should not be read from or written to on another. One thread
at a time. If possible, don't share objects between threads. The best case
scenario is you are passing serializable messages to threads rather than giving
objects directly to other threads. That said, as long as only one thread is
interacting with a given object at a time, you should be fine. To reiterate
that with an example, if you have two threads and they both allocate their own
Vec3 object, you're fine. If you have two threads and they're both using the
same Vec3 object, you are playing with fire.

Typed arrays, like Vec3Array and so on, require a little more explanation. When
accessing elements of a typed array, the array returns an object that accesses
the array's memory. The object does not have its own buffer to play with. As
such, arrays and the elements of arrays are both not thread-safe and you should
not modify an array or its elements from multiple threads at the same time.
Also, never attempt to fetch an element from an array on multiple threads at a
time, as the underlying object cache of the array is being modified even if you
aren't altering any array elements.

If you need to pass an array's element to another thread but don't care about
it modifying the underlying array data, simply #copy, #clone, or #dup the
fetched object and pass ownership of the copy to the other thread -- or pass
the object via a deserializable message so that you're not even trying to pass
objects directly to threads.

When in doubt, don't use threads. When you have to use threads and are still in
doubt, use a messaging system to pass data between threads, like [ZeroMQ],
rather than passing objects directly to threads. Safety first.

(Further note: all snow-math types work with Marshal.load and Marshal.dump, so
make use of that where it's practical and smart to do so.)

[ZeroMQ]: http://www.zeromq.org


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
