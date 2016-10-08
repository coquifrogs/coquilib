#include "support/test_base.h"

#define CLI_DECLARATION
#include "cli.h"

#define CLI_IMPLEMENTATION
#include "cli.h"

TEST_CASE("Short option string parameter", "") {
	const char* fileName = nullptr;

	cli::Parser parser = {
		cli::OptionString('S', "string", "some string", false, &fileName)
	};

	const char* argv[] {
		"testExe", "-S", "fileName"
	};

	REQUIRE(parser.parse(3, argv));
	REQUIRE(strcmp(fileName, "fileName") == 0);
}

TEST_CASE("Option types", "") {
	int intOption = 0;
	float floatOption = 0;
	const char* stringOption = NULL;
	const char* pathOption = NULL;
	bool flagOption = false;
	int flagCountOption = 0;

	cli::Parser parser = {
		cli::OptionInt('I', "int", "some int", false, &intOption),
		cli::OptionFloat('F', "float", "some float", false, &floatOption),
		cli::OptionString('S', "string", "some string", false, &stringOption),
		cli::OptionPath('f', "output-file", "some path", false, &pathOption),
		cli::OptionFlag('D', "debug", "some flag", &flagOption),
		cli::OptionFlagCount('V', "verbose", "verbosity", &flagCountOption)
	};
	parser.printOptionsUsage();

	const char* argv[] {
		"testExe", "-S", "fileName", "--output-file", "output.txt", "-F", "0.5", "-VVV", "-D", "-I", "23", "File1", "File2"
	};

	REQUIRE(parser.parse(13, argv));
	REQUIRE(intOption == 23);
	REQUIRE(floatOption - 0.5f < 0.0001f);
	REQUIRE(strcmp(stringOption, "fileName") == 0);
	REQUIRE(strcmp(pathOption, "output.txt") == 0);
	REQUIRE(flagOption);
	REQUIRE(flagCountOption == 3);
	auto remaining = parser.getRemainingArgs();
	REQUIRE(remaining.size() == 2);
	REQUIRE(strcmp(remaining[0], "File1") == 0);
	REQUIRE(strcmp(remaining[1], "File2") == 0);
}


TEST_CASE("Long option string parameter", "") {	
	const char* fileName = NULL;

	cli::Parser parser = {
		cli::OptionString('S', "string", "some string", false, &fileName)
	};

	const char* argv[] {
		"testExe", "--string", "fileName"
	};

	REQUIRE(parser.parse(3, argv));
	REQUIRE(strcmp(fileName, "fileName") == 0);
}

TEST_CASE("Long option required string parameter", "") {
	const char* fileName = NULL;

	cli::Parser parser = {
		cli::OptionString('S', "string", "some string", true, &fileName)
	};

	const char* argv[] {
		"testExe",
	};

	REQUIRE(!parser.parse(1, argv));
}

TEST_CASE("Short option string flag count", "") {
	int verbosity = 0;
	bool debug = false;

	cli::Parser parser = {
		cli::OptionFlagCount('v', "verbose", "Verbosity of the thing", &verbosity),
		cli::OptionFlag('D', "debug", "Enable debug mode", &debug)
	};

	const char* argv[] {
		"testExe", "-vvvD"
	};

	parser.parse(2, argv);

	REQUIRE(verbosity == 3);
	REQUIRE(debug);
}

TEST_CASE("Short option string parameter in middle", "") {
	int verbosity = 0;
	const char* fileName = NULL;

	cli::Parser parser = {
		cli::OptionFlagCount('v', "verbose", "Verbosity of the thing", &verbosity),
		cli::OptionString('S', "string", "some string", false, &fileName)
	};

	const char* argv[] {
		"testExe", "-vvSv", "fileName"
	};

	REQUIRE(!parser.parse(3, argv));
}

TEST_CASE("Short option string parameter at end", "") {
	int verbosity = 0;
	const char* fileName = NULL;

	cli::Parser parser = {
		cli::OptionFlagCount('v', "verbose", "Verbosity of the thing", &verbosity),
		cli::OptionString('S', "string", "some string", false, &fileName)
	};

	const char* argv[] {
		"testExe", "-vvvS", "fileName"
	};

	REQUIRE(parser.parse(3, argv));
	REQUIRE(strcmp(fileName, "fileName") == 0);
}

TEST_CASE("Default option values", "") {
	int verbosity = 0;
	const char* fileName = "fileName";

	cli::Parser parser = {
		cli::OptionFlagCount('v', "verbose", "Verbosity of the thing", &verbosity),
		cli::OptionString('S', "string", "some string", false, &fileName)
	};

	const char* argv[] {
		"testExe", "-vvv"
	};

	REQUIRE(parser.parse(2, argv));
	REQUIRE(strcmp(fileName, "fileName") == 0);
}

TEST_CASE("Path validation", "") {
	const char* fileName = NULL;

	cli::Parser parser = {
		cli::OptionPathExisting('S', "string", "some string", false, &fileName)
	};

	const char* argv[] {
		"testExe", "-S", "../CmakeLists.txt"
	};

	REQUIRE(parser.parse(3, argv));
	REQUIRE(parser.validatePathOptions());
	REQUIRE(strcmp(fileName, "../CmakeLists.txt") == 0);
}

TEST_CASE("Remaining args", "") {
	int verbosity = 0;
	const char* fileName = "fileName";

	cli::Parser parser = {
		cli::OptionFlagCount('v', "verbose", "Verbosity of the thing", &verbosity),
		cli::OptionString('S', "string", "some string", false, &fileName)
	};

	const char* argv[] = {
		"testExe", "-vvv", "-S", "something", "somefile"
	};

	REQUIRE(parser.parse(5, argv));
	auto files = parser.getRemainingArgs();
	REQUIRE(files.size() == 1);
	REQUIRE(strcmp(files[0], "somefile") == 0);
}