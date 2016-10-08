#include <cli.h>

int main(int argc, const char* argv[]) {
	const char* inputFilename = NULL;
	int verbosity = 0;
	
	cli::Parser parser = {
		cli::OptionString('i', "input-file", "input file", true, &inputFilename),
		cli::OptionFlagCount('v', "verbose", "verbose logging", &verbosity)
	};

	if(!parser.parse(argc, argv)) {
		fprintf(stderr, "\nUsage: cli_example [-v] -i INPUT\n\n");

		// Prints auto-generated options usage list
		parser.printOptionsUsage();
		return -1;
	}

	printf("Input file = %s\n", inputFilename);
	printf("Verbosity = %d\n", verbosity);

	return 0;
}