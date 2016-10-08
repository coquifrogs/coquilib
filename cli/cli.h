/*
A simple, header-only CLI parser for C++. Depends only on C standard library and C++ STL.

Author: John Cannon

# Why?
	I was frustrated with the over-complicated and bloated solutions I found
	online, so I decided to make one I like.  This Parser class allows you to
	use plain variables to capture the parsed arguments in a clean way. There
	is minimal code required to map arguments to those variables and no 
	template magic.

	It does use void pointers internally.  It doesn't allocate anything
	aside from a couple of internal std::vectors and does not modify the
	argument list that is passed to it.

# License
	This software is dual-licensed to the public domain and under the following
	license: you are granted a perpetual, irrevocable license to copy, modify,
	publish, and distribute this file as you see fit.

# Compiling
	Include <cli.h> in your code, typically in the same .cpp as your main()
	function.

	If you need to use declarations in multiple files, use the macros 
	CLI_DECLARATION and CLI_IMPLEMENTATION to include just the declarations
	or implementations respectively.  By default, both are included.

	Link with -std=c++1

# Example
	#include <cli.h>

	int main(int argc, const char* argv[]) {
		const char* inputFilenam = NULL;
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

# Usage
	The Parser class accepts an initializer list of Option structs.  Convenience
	functions are provided for creating Option structs.  Each Option function
	accepts a pointer to a variable of the type that it provides.  This allows
	you to simply use normal variables for option outputs and intialize them 
	with any default value you want before calling parse().

	The parse(int argc, const char* argv[]) method returns true if all options
	were successfully parsed, false if some error occured (such as a missing,
	required argument).

	The getRemainingArguments() method returns a std::vector of arguments that 
	are not related to any options (e.g., a list of input files).

# Option types
	Convenience functions are provided for creating Option structs that you can 
	pass to the initializer for the Parser class:

	OptionFlag - A simple boolean flag option
	OptionFlagCount - A flag option that can occur multiple times is counted (i.e., verbosity)
	OptionInt - An integer option
	OptionFloat - A floating point option
	OptionString - A string option

	A couple of experimental option types:

	OptionPath - A file path option (can optionally be validated)
	OptionPathExisting - A file path option that must point to an existing file (can optionally be validated)

*/

#if !defined(CLI_DECLARATION) && !defined(CLI_IMPLEMENTATION)
#define CLI_DECLARATION 1
#define CLI_IMPLEMENTATION 1
#endif

#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <initializer_list>

#ifndef CLI_LOG_ERROR
#define CLI_LOG_ERROR(fmt, ...) fprintf(stderr, fmt, ##__VA_ARGS__)
#endif

#ifndef CLI_LOG_USAGE
#define CLI_LOG_USAGE(fmt, ...) fprintf(stderr, fmt, ##__VA_ARGS__)
#endif

#if defined(CLI_DECLARATION) && !defined(_CLI_DECLARATION_INCLUSION_GUARD)
#define _CLI_DECLARATION_INCLUSION_GUARD

namespace cli {

struct Option {
	enum class Type {
		Flag,
		FlagCount,
		Int,
		Float,
		String,
		Path,
		PathExisting
	};
	Type type;
	char shortName;
	const char* longName;
	const char* description;
	bool isRequired;
	
	bool isSet;
	void* valuePointer;

	bool requiresParameter() const {
		return type != Option::Type::Flag && type != Option::Type::FlagCount;
	}

	template<typename T> 
	T& as() const {
		return *static_cast<T*>(valuePointer);
	}
};

Option OptionFlag(char shortName, const char* longName, const char* description, bool* valuePointer);
Option OptionFlagCount(char shortName, const char* longName, const char* description, int* valuePointer);
Option OptionInt(char shortName, const char* longName, const char* description, bool required, int* valuePointer);
Option OptionFloat(char shortName, const char* longName, const char* description, bool required, float* valuePointer);
Option OptionString(char shortName, const char* longName, const char* description, bool required, const char** valuePointer);
Option OptionPath(char shortName, const char* longName, const char* description, bool required, const char** valuePointer);
Option OptionPathExisting(char shortName, const char* longName, const char* description, bool required, const char** valuePointer);

class Parser {
public:
	Parser(std::initializer_list<Option> options) : options(options), executableName(nullptr) {}
	bool parse(int argc, const char* argv[]);
	bool validatePathOptions();
	void printOptionsUsage();

	const std::vector<const char*>& getRemainingArgs() const {
		return remaining;
	}

private:
	std::vector<Option> options;
	std::vector<const char*> remaining;
	const char* executableName;

	int applyOption(Option& opt, int argc, const char** argv);
	bool isNumeric(const char* str, bool floatingPoint);
	int handleOption(int argc, const char** argv);
	const char* optionTypeDisplayName(Option::Type type);
	bool checkExistsReadable(const char* path);

};

}; // end namespace

#endif // CLI_DECLARATION

#if defined(CLI_IMPLEMENTATION) && !defined(_CLI_IMPLEMENTATION_INCLUSION_GUARD)
#define _CLI_IMPLEMENTATION_INCLUSION_GUARD

namespace cli {

bool Parser::parse(int argc, const char* argv[]) {
	executableName = argv[0];

	for(int i = 1; i < argc; ++i) {
		int result = handleOption(argc-i,argv+i);
		// error
		if(result < 0) return false;
		// skip any consumed parameters
		i+= result;
	}
	for(Option& opt : options) {
		if(opt.isRequired && !opt.isSet) {
			CLI_LOG_ERROR("error: option -%c/--%s is required\n", opt.shortName, opt.longName);
			return false;
		}
	}
	return true;
}

bool Parser::validatePathOptions() {
	bool valid = true;
	for(Option& opt: options) {
		if(opt.type == Option::Type::PathExisting && !checkExistsReadable(opt.as<const char*>())) {
			valid = false;
			CLI_LOG_ERROR("error: option -%c/--%s requires a readable file\n", opt.shortName, opt.longName);
		}
	}
	return valid;
}

void Parser::printOptionsUsage() {
	CLI_LOG_USAGE("Options:\n");
	for(Option& opt : options) {
		if(opt.requiresParameter()) {
			CLI_LOG_USAGE("  -%c, --%s <%s>\t%s", opt.shortName, opt.longName, optionTypeDisplayName(opt.type), opt.description);
		} else {
			CLI_LOG_USAGE("  -%c, --%s\t%s", opt.shortName, opt.longName, opt.description);
		}
		if(opt.isRequired) {
			CLI_LOG_USAGE(" (required)");
		}
		CLI_LOG_USAGE("\n");
	}		
}

int Parser::applyOption(Option& opt, int argc, const char** argv) {
	if(opt.type != Option::Type::FlagCount && opt.isSet) {
		CLI_LOG_ERROR("error: option -%c/--%s shouldn't be specified more than once\n", opt.shortName, opt.longName);
		return -1;
	}
	opt.isSet = true;
	// Other types expect an argument
	if(opt.requiresParameter() && argc < 2) {
		CLI_LOG_ERROR("error: option -%c/--%s requires a parameter\n", opt.shortName, opt.longName);
		return -1;
	}

	const char* argParam = argv[1];

	switch(opt.type) {
		case Option::Type::Flag:
			opt.as<bool>() = true;
			return 0;
		case Option::Type::FlagCount:
			opt.as<int>()++;
			return 0;
		case Option::Type::Int:
			if(!isNumeric(argParam, false)) {
				CLI_LOG_ERROR("error: invalid integer value \"%s\" specified for option -%c/--%s\n", argParam, opt.shortName, opt.longName);
				return -1;
			}
			opt.as<int>() = atoi(argParam);
			return 1;
		case Option::Type::Float:
			if(!isNumeric(argParam, true)) {
				CLI_LOG_ERROR("error: invalid float value \"%s\" specified for option -%c/--%s\n", argParam, opt.shortName, opt.longName);
				return -1;
			}
			opt.as<float>() = atof(argParam);
			return 1;
		case Option::Type::String:
		case Option::Type::Path:
		case Option::Type::PathExisting:
			opt.as<const char*>() = argParam;
			return 1;
	}
	return 0;
}

bool Parser::isNumeric(const char* str, bool floatingPoint) {
	bool beginExponent = false;
	bool foundDecimal = false;
	for(int i = 0; i < strlen(str); ++i) {
		switch(str[i]) {
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
				continue;
			case '-':
			case '+':
				if(i == 0 || (floatingPoint && beginExponent)) {
					continue;
				}
				return false;
			case '.':
				if(floatingPoint && !foundDecimal) {
					foundDecimal = true;
					continue;
				}
				return false;
			case 'e':
			case 'E':
				if(floatingPoint) {
					beginExponent = true;
					floatingPoint = false;
				}
				return false;
			default:
				return false;
		}
	}
	return true;
}

int Parser::handleOption(int argc, const char** argv) {
	const char* arg = argv[0];
	if(strlen(arg) > 1 && arg[0] == '-') {
		// Handle concatenated short options
		if(arg[1] != '-') {
			for(int i = 1; i < strlen(arg); ++i) {
				bool handled = false;
				for(Option& opt : options) {
					if(opt.shortName == arg[i]) {
						// arguments requiring parameters can't be in the middle of the list
						if(opt.requiresParameter() && i < strlen(arg) - 1) {
							CLI_LOG_ERROR("error: short option -%c cannot be used in the middle of a flag list, it requires a value\n", opt.shortName);
							return -1;
						}
						handled = true;
						int result = applyOption(opt, argc, argv);
						if(result != 0) return result;
					}
				}
				if(!handled) {
					CLI_LOG_ERROR("error: unknown short option -%c\n", arg[i]);
					return -1;
				}
			}
		} else {
			for(Option& opt : options) {
				if(strcmp(arg+2, opt.longName) == 0) {
					return applyOption(opt, argc, argv);
				}
			}
			CLI_LOG_ERROR("error: unknown option %s\n", argv[0]);
			return -1;
		}
	} else {
		remaining.push_back(arg);
	}
	return 0;
}

const char* Parser::optionTypeDisplayName(Option::Type type) {
	switch(type) {
		case Option::Type::Flag:
		case Option::Type::FlagCount:
			return "flag";
		case Option::Type::Int:
			return "integer";
		case Option::Type::Float:
			return "float";
		case Option::Type::String:
			return "string";
		case Option::Type::Path:
		case Option::Type::PathExisting:
			return "path";
		default:
			return "unknown";
	}
}

bool Parser::checkExistsReadable(const char* path) {
	FILE* f = fopen(path, "rb");
	if(f != NULL) {
		fclose(f);
		return true;
	} else {
		return false;
	}
}

Option OptionFlag(char shortName, const char* longName, const char* description, bool* valuePointer){
	return Option {Option::Type::Flag, shortName, longName, description, false, false, valuePointer};
}

Option OptionFlagCount(char shortName, const char* longName, const char* description, int* valuePointer){
	return Option {Option::Type::FlagCount, shortName, longName, description, false, false, valuePointer};
}

Option OptionInt(char shortName, const char* longName, const char* description, bool required, int* valuePointer){
	return Option {Option::Type::Int, shortName, longName, description, required, false, valuePointer};
}

Option OptionFloat(char shortName, const char* longName, const char* description, bool required, float* valuePointer){
	return Option {Option::Type::Float, shortName, longName, description, required, false, valuePointer};
}

Option OptionString(char shortName, const char* longName, const char* description, bool required, const char** valuePointer){
	return Option {Option::Type::String, shortName, longName, description, required, false, valuePointer};
}

Option OptionPath(char shortName, const char* longName, const char* description, bool required, const char** valuePointer){
	return Option {Option::Type::Path, shortName, longName, description, required, false, valuePointer};
}

Option OptionPathExisting(char shortName, const char* longName, const char* description, bool required, const char** valuePointer){
	return Option {Option::Type::PathExisting, shortName, longName, description, required, false, valuePointer};
}

}; // end namespace

#endif // CLI_IMPLEMENTATION
