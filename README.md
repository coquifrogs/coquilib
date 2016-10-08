# coquilib

A small collection of utility libraries for my personal projects.

Library | Description
--------| -----------
[cli](cli)| A simple, header-only CLI parser for C++.  Depends only on C standard library and C++ STL.

# License

See each library folder for its corresponding license.

# Tests

The provided `CMakeLists.txt` file will build all the tests for the libraries.
You can run the unit tests as follows:

```sh
mkdir build && cd build
cmake ..
make
ctest  # or run each individual test executable in the build/tests folder
```