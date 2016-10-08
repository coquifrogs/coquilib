# cli::Parser

A simple, header-only CLI parser for C++.  Depends only on C standard library
and C++ STL.

# Why?

I was frustrated with the over-complicated and bloated solutions I found
online, so I decided to make one I like.  This `Parser` class allows you to
use plain variables to capture the parsed arguments in a clean way. There
is minimal code required to map arguments to those variables and no 
template magic.

It does use void pointers internally.  It doesn't allocate anything
aside from a couple of internal `std::vectors` and does not modify the
argument list that is passed to it.

# License

This software is dual-licensed to the public domain and under the following
license: you are granted a perpetual, irrevocable license to copy, modify,
publish, and distribute this file as you see fit.

# Compiling
Include `<cli.h>` in your code, typically in the same .cpp as your `main()`
function.

If you need to use declarations in multiple files, use the macros 
`CLI_DECLARATION` and `CLI_IMPLEMENTATION` to include just the declarations
or implementations respectively.  By default, both are included.

Link with `-std=c++11`

# Example
```c++
#include <cli.h>

int main(int argc, const char* argv[]) {
	const char* inputFilename = NULL;
	int verbosity = 0;
	
	cli::Parser parser = {
		cli::OptionString('i', "input-file", "input file", true, &inputFilename),
		cli::OptionFlagCount('v', "verbose", "verbose", &verbosity)
	};

	if(!parser.parse(argc, argv)) {
		// Print custom usage message here

		// Prints auto-generated options usage list
		parser.printOptionsUsage();
		return -1;
	}

	...

	return 0;
}
```

# Usage

The `Parser` class accepts an initializer list of `Option` structs.  Convenience
functions are provided for creating `Option` structs.  Each `Option` function
accepts a pointer to a variable of the type that it provides.  This allows
you to simply use normal variables for option outputs and intialize them 
with any default value you want before calling `parse()`.

The `parse(int argc, const char* argv[])` method returns `true` if all options
were successfully parsed, `false` if some error occured (such as a missing,
required argument).

The `getRemainingArguments()` method returns a `std::vector` of arguments that 
are not related to any options (e.g., a list of input files).

# Option types

Convenience functions are provided for creating `Option` structs that you can 
pass to the initializer for the `Parser` class:

Function | Description
--- | ---
`OptionFlag` | A simple boolean flag option
`OptionFlagCount` | A flag option that can occur multiple times is counted (i.e., verbosity)
`OptionInt` | An integer option
`OptionFloat` | A floating point option
`OptionString` | A string option
`OptionPath` | A file path option (optionally validated) *experimental*
`OptionPathExisting` | A file path option that must point to an existing file (optionally validated) *experimental*
