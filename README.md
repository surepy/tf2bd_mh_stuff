# stuff
Sometimes I find myself rewriting different versions of the same logic in lots of unrelated projects I work on. Here's some of that stuff.

All code is tested clang-7/8/9/10 and gcc-8/9 in C++2a mode.

"Dependencies" below refers only to other files *in this repository*.

| File | Description | Dependencies |
| - | - | - | - |
| `algorithm/algorithm.hpp` | Helpers for `<algorithm>`. | ✅ None
| `data/bit_float.hpp` | Conversion to/from floating point formats with <= bits in the native double type. For example, 16 bit "half" floats, or OpenGL 11/10 bit floats. | ✅ None
| `data/bits.hpp` | Functions to copy arbitrary numbers of bits from source to destination, with arbitrary source and destination *bit* offsets. | ✅ None
| `data/variable_pusher.hpp` | RAII type that assigns a variable on construction, and restores the initial value on destruction. | ✅ None
| `io/getopt.hpp` | Wrapper around `getopt` that saves the user from having to interact with global variables. | ✅ None
| `math/interpolation.hpp` | Interpolation functions, such as `lerp` and `remap`. | ❔ Optional dependency on `math/uint128.hpp`
| `math/uint128.hpp` | Simple 128-bit unsigned integer type. | ✅ None
| `memory/buffer.hpp` | Blob of memory that takes advantage of `realloc` for situational performance improvements. | ✅ None
| `memory/unique_object.hpp` | Similar to `std::unique_ptr`, but does not require a pointer type. Helpful for things like freeing integer-based handles. | ✅ None
| `text/case_insensitive_string.hpp` | Case insensitive `std::string` and `std::string_view` traits. | ✅ None
| `text/charconv_helper.hpp` | Convenience functions for `std::from_chars` and `std::to_chars`. | ✅ None
| `text/memstream.hpp` | `std::basic_iostream` that is backed by arbitrary memory. | ✅ None
| `text/string_insertion.hpp` | `operator<<` for `std::string`. Does not support stream manipulators, but is very useful for constructing simple strings without requiring `std::stringstream` (faster too!). | ✅ None
| `text/stringops.hpp` | Helper functions for `std::string` manipulation. | ✅ None
| `utility.hpp` | Miscellaneous helper functions. | ✅ None
