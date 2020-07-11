/*!
NandradCodeGenerator - creates NANDRAD_KeywordList.cpp and class-specific implementation files

Example syntax:

> NandradCodeGenerator NANDRAD /path/to/NANDRAD/src
*/

#include <iostream>

#include "CodeGenerator.h"

const char * const SYNTAX =
		"SYNTAX:  NandradCodeGenerator <namespace> <path/to/src> <generateQtSrc> <prefix> <ncg-dir>\n"
		"         <namespace> is usually NANDRAD (used also to compose file names).\n"
		"         <path/to/<lib>/src> is + separated list of input directories to read the header files from.\n"
		"         Keywordlist-source files are written into the first (or only) source directory.\n"
		"         <prefix> is the file prefix <prefix>_KeywordList.cpp.\n"
		"         <generateQtSrc> is 1 when Qt source should be generated, 0 otherwise.\n"
		"         <ncg-dir> is the path to the directory where ncg_xxx.cpp files are written to.\n";


// ******* MAIN ********

int main(int argc, char *argv[]) {
	std::cout << "-----------------------------------------------------------------------" << std::endl;
	std::cout << "NandradCodeGenerator, based on IBK KeywordListCreator" << std::endl;
	std::cout << "Extracting keywords from header files..." << std::endl;

	if (argc != 6) {
		std::cerr << "Invalid syntax." << std::endl;
		std::cerr << argc << " Arguments received" << std::endl;
		for (int i=0; i<argc; ++i)
			std::cerr << "  " << argv[i] << std::endl;
		std::cerr << std::endl;
		std::cerr << SYNTAX << std::endl;

		return EXIT_FAILURE;
	}

	CodeGenerator cg;
	cg.handleArguments(argv);

	// the next function parses the input files and does all the error handling
	if (!cg.parseDirectories())
		return EXIT_FAILURE;  // error messages where already written to file

	cg.generateKeywordList();
	cg.generateReadWriteCode();

	return EXIT_SUCCESS;
}

