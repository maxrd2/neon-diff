#include "neonapp.h"
#include "diffparser.h"

#include <stdio.h>

// https://github.com/catchorg/Catch2 - A modern, C++-native, header-only, test framework for unit-tests, TDD and BDD
#define CATCH_CONFIG_MAIN
#include "catch.hpp"

class TestParser : public DiffParser {
public:
	TestParser() : DiffParser(nullptr) {}

	using DiffParser::handlerForLine; // redeclare public
};

NeonApp *app = nullptr;


TEST_CASE("input lines are detected properly", "[DiffParser]") {
	TestParser parser;

	SECTION("lines with no ANSI escapes") {
		const char line[] = "---";
		REQUIRE(true == parser.handlerForLine(line, "---", sizeof(line)));
		REQUIRE(true == parser.handlerForLine(line, "---", sizeof(line) - 1));
		REQUIRE(false == parser.handlerForLine(line, "---", sizeof(line) - 2));
	}

	SECTION("lines with leading ANSI escapes") {
		const char lineAnsiLead[] = "\e33;12;11;11m\e33m\em---";
		REQUIRE(true == parser.handlerForLine(lineAnsiLead, "---", sizeof(lineAnsiLead)));
		REQUIRE(true == parser.handlerForLine(lineAnsiLead, "---", sizeof(lineAnsiLead) - 1));
		REQUIRE(false == parser.handlerForLine(lineAnsiLead, "---", sizeof(lineAnsiLead) - 2));
	}

	SECTION("lines with mixed ANSI escapes") {
		const char lineAnsiMix[] = "\e33;12;11;11m-\e33m-\em-";
		REQUIRE(true == parser.handlerForLine(lineAnsiMix, "---", sizeof(lineAnsiMix)));
		REQUIRE(true == parser.handlerForLine(lineAnsiMix, "---", sizeof(lineAnsiMix) - 1));
		REQUIRE(false == parser.handlerForLine(lineAnsiMix, "---", sizeof(lineAnsiMix) - 2));
	}
}
