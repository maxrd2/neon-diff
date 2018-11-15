#include "colors.h"

// ansi colors
const char *colorRed = "\33[91m";
const char *colorGreen = "\33[92m";
const char *colorYellow = "\33[93m";
const char *colorBlue = "\33[94m";
const char *colorMagenta = "\33[95m";
const char *colorCyan = "\33[96m";
const char *colorWhite = "\33[97m";
const char *colorReset = "\33[m";

// colors used for diff parts
const char *colorFileInfo = colorWhite;
const char *colorBlockRange = colorCyan;
const char *colorBlockHeading = colorBlue;
const char *colorLineDel = colorRed;
const char *colorLineAdd = colorGreen;
const char *colorLineContext = colorReset;

// inverts background and foreground colors
const char *highlightOn = "\33[7m";
const char *highlightOff = "\33[27m";
const char *highlightReset = highlightOff;
