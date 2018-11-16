/*
	neon-diff - Application to colorify, highlight and beautify unified diffs.

	Copyright (C) 2018 - Mladen Milinkovic <maxrd2@smoothware.net>

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <https://www.gnu.org/licenses/>.

*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <getopt.h>
#include <uchar.h>
#include <wchar.h>
#include <string.h>
#include <cassert>

#include "neonapp.h"
#include "diffparser.h"

NeonApp *app = nullptr;

int
utf8CharLen(const char *ch) {
	if((*ch & 0x80) == 0)
		return 1;
	if((*ch & 0xE0) == 0xC0)
		return 2;
	if((*ch & 0xF0) == 0xE0)
		return 3;
	if((*ch & 0xF8) == 0xF0)
		return 4;
	return 0;
}

int
main(int argc, char *argv[])
{
	int inputFileCount = 0;
	const char *inputFile[argc];
	const char *outputFile = nullptr;

	opterr = 0;
	for(;;) {
		static const struct option longOpts[] = {
			{"input", required_argument, nullptr, 'i'},
			{"output", required_argument, nullptr, 'o'},
			{"ignore-spaces", no_argument, nullptr, 's'},
			{"convert-indent", required_argument, nullptr, 'I'},
			{"tab-width", required_argument, nullptr, 't'},
			{"show-tabs", optional_argument, nullptr, 'T'},
			{"help", no_argument, 0, 'h'},
			{0, 0, 0, 0}
		};

		const int ch = getopt_long(argc, argv, "i:o:sI:T::t:h", longOpts, nullptr);

		if(ch == -1)
			break;
		switch(ch) {
		case 'i': // input
			inputFile[inputFileCount++] = optarg;
			break;

		case 'o': // output
			outputFile = optarg;
			break;

		case 's': // ignore-spaces
			NeonApp::ignoreSpaces_ = true;
			break;

		case 'I': // convert-indent
			NeonApp::indentWidth_ = atoi(optarg);
			if(NeonApp::indentWidth_ < 0)
				NeonApp::indentWidth_ = 0;
			break;

		case 't': // tab-width
			NeonApp::tabWidth_ = atoi(optarg);
			if(NeonApp::tabWidth_ < 1)
				NeonApp::tabWidth_ = 1;
			break;

		case 'T': // show-tabs
			if(optarg)
				optarg[utf8CharLen(optarg)] = 0;
			NeonApp::tabCharacter_ = optarg && *optarg ? optarg : "\uffeb";
			break;

		case 'h': // help
			fprintf(stderr,
					"Usage: neon-diff [-h] [-i <input file>] [-o <output file>] [input file]...\n"
					"Take unified diff input, detect changed content and add colored highlighting.\n"
					"\n"
					"  -i, --input=<filename>     read unified diff from input file, use - for STDIN (default)\n"
					"  -o, --output=<filename>    write colored diff to output file, use - for STDOUT (default)\n"
					"\n"
					"  -s, --ignore-spaces        ignore white-space when matching/highlighting differences\n"
					"  -I, --convert-indent=<len> change indentation spaces from <len> to current tab length\n"
					"  -t, --tab-width=<width>    set tab stop every [width] characters (default: 4)\n"
					"  -T, --show-tabs=[char]     display tabs with character (default: \uffeb)\n"
					"\n"
					"  -h, --help                 show this help message\n"
					"\n"
					"\n"
					"EXAMPLES:\n"
					"  $ git diff | neon-diff | less -RFX\n"
					" Colorize git's diff output and pass it through less pager.\n"
					"\n"
					"  $ neon-diff changes.patch\n"
					" Read changes.patch file, colorize and output to stdout.\n"
					"\n"
					"  $ neon-diff -I4 -t8 changes.patch | less -R\n"
					" Read changes.patch file, change indent from 4 to 8 chars and output to stdout.\n"
					"\n"
					"  $ diff --unified file1.txt file2.txt | neon-diff | less -R\n"
					" Colorize differences between file1.txt and file2.txt and output through less pager.\n"
					"\n"
					"\n"
					"GIT INTEGRATION:\n"
					"  $ git config --global interactive.diffFilter 'neon-diff'\n"
					"  $ git config --global core.pager 'neon-diff | less -RFX'\n"
					"  $ git config --global pager.diff 'neon-diff | less -RFX'\n"
					"  $ git config --global pager.show 'neon-diff | less -RFX'\n"
					"  $ git config --global pager.log 'neon-diff | less -RFX'\n"
					" This will setup git to colorize output through neon-diff.\n"
					" See git-config --help for more details.\n"
					"\n"
					"\n"
					"neon-diff homepage: <https://github.com/maxrd2/neon-diff>\n"
					);
			return 1;

		case '?':
			if(optopt == 'i' || optopt == 'o')
				fprintf(stderr, "ERROR: Option -%c requires a filename argument.\n", optopt);
			else
				fprintf(stderr, "ERROR: Unknown option `-%c'.\n", optopt);
			return 1;

		default:
			fprintf(stderr, "ERROR: Unknown option `-%c'.\n", optopt);
			return 1;
		}
	}

	// process remaining arguments as input file names
	for(int i = optind; i < argc; i++)
		inputFile[inputFileCount++] = argv[i];

	if(inputFileCount == 0)
		inputFile[inputFileCount++] = "-";

	// prepare output
	FILE *out = !outputFile || (outputFile[0] == '-' && outputFile[1] == 0) ? stdout : fopen(outputFile, "w");
	if(!out) {
		fprintf(stderr, "ERROR: Unable to open file \"%s\" for writing.\n", outputFile);
		return 1;
	}

	// process each input file
	for(int i = 0; i < inputFileCount; i++) {
		FILE *in = inputFile[i][0] == '-' && inputFile[i][1] == 0 ? stdin : fopen(inputFile[i], "r");
		if(!in) {
			fprintf(stderr, "ERROR: Unable to open file \"%s\" for reading.\n", inputFile[i]);
			return 1;
		}

		app = new NeonApp(in, out);
		app->parser_->processInput();

		if(in != stdin)
			fclose(in);

		delete app;
	}

	if(out != stdout)
		fclose(out);

	return 0;
}
