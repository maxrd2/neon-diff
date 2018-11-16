#ifndef NEONAPP_H
#define NEONAPP_H

#include <stdio.h>

class DiffParser;

class NeonApp
{
public:
	NeonApp(FILE *inputStream, FILE *outputStream);
	virtual ~NeonApp();

	void setColor(const char *color);
	void setHighlight(const char *highlight);

	void printNewLine();
	void printAnsiCodes();
	void printChar(const char ch, bool writeAnsi = true);

	inline bool ignoreSpaces() { return ignoreSpaces_; }
	inline int indentWidth() { return indentWidth_; }
	inline const char * tabCharacter() { return tabCharacter_; }
	inline int tabWidth() { return tabWidth_; }

private:
	friend int main(int argc, char *argv[]);

	static bool ignoreSpaces_;
	static int indentWidth_;
	static const char *tabCharacter_;
	static int tabWidth_;

	DiffParser *parser_;

	FILE *output_;

	bool outputOnStart_;
	bool outputOnIndent_;
	int outputIndex_;
	int outputSpaces_;

	const char *selectedColor_;
	const char *selectedHighlight_;

	const char *printedColor_;
	const char *printedHighlight_;
};

extern NeonApp *app;

#endif // NEONAPP_H
