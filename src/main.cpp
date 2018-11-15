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
#include <unistd.h>

#include "neonapp.h"
#include "diffparser.h"

NeonApp *app = nullptr;

int
main(int argc, char *argv[])
{
	FILE *in = stdin;
	FILE *out = stdout;

	int ch;
	while((ch = getopt(argc, argv, "i:o:h")) != -1) {
		switch(ch) {
		case 'h':
			fprintf(stderr,
					"Usage: neon-diff [-h] [-i <input file>] [-o <output file>]\n"
					"Take unified diff input, detect changed content and add colored highlighting.\n"
					"\n"
					"  -i <input file>    read unified diff from input file, use - for STDIN (default)\n"
					"  -o <output file>   write colored diff to outout file, use - for STDOUT (default)\n"
					"  -h                 show this help message\n"
					"\n"
					"\n"
					"EXAMPLES:\n"
					"  $ git diff | neon-diff | less -R\n"
					" This will colorify git's diff output and pass it through less pager.\n"
					"\n"
					"  $ neon-diff -i cool-changes.patch | less -R\n"
					" This will read cool-changes.patch file, colorify and output it through less pager.\n"
					"\n"
					"  $ neon-diff -i cool-changes.patch | less -R\n"
					" This will read cool-changes.patch file, colorify and output it through less pager.\n"
					"\n"
					"  $ diff --unified file1.txt file2.txt | colordiff | less -R"
					" This will colorify diff output and pass it through less pager.\n"
					"\n"
					"\n"
					"GIT INTEGRATION:\n"
					"  $ git config --global interactive.diffFilter 'neon-diff'\n"
					"  $ git config --global pager.diff 'neon-diff | less'\n"
					"  $ git config --global pager.show 'neon-diff | less'\n"
					"  $ git config --global pager.log 'neon-diff | less'\n"
					" This will setup git to colorify output through neon-diff. See git-config --help for more details.\n"
					"\n"
					"\n"
					"neon-diff homepage: <https://github.com/maxrd2/neon-diff>\n"
					);
			return 1;

		case 'i':
			in = optarg[0] == '-' && optarg[1] == 0 ? stdin : fopen(optarg, "r");
			if(!in) {
				fprintf(stderr, "ERROR: Unable to open file \"%s\" for reading.\n", optarg);
				return 1;
			}
			break;

		case 'o':
			out = optarg[0] == '-' && optarg[1] == 0 ? stdout : fopen(optarg, "w");
			if(!out) {
				fprintf(stderr, "ERROR: Unable to open file \"%s\" for writing.\n", optarg);
				return 1;
			}
			break;

		case '?':
			if(optopt == 'i' || optopt == 'o')
				fprintf(stderr, "ERROR: Option -%c requires an argument.\n", optopt);
			else
				fprintf(stderr, "ERROR: Unknown option `-%c'.\n", optopt);
			return 1;

		default:
			return 1;
		}
	}

	app = new NeonApp(in, out);

	app->parser_->processInput();

	if(in != stdin)
		fclose(in);
	if(out != stdout)
		fclose(out);

	return 0;
}
