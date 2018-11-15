#ifndef NEONAPP_H
#define NEONAPP_H

#include <stdio.h>

class DiffParser;

class NeonApp
{
public:
	NeonApp(FILE *inputStream, FILE *outputStream);
	virtual ~NeonApp();

	void outputBlock(const char *block, const char *blockEnd);

	void setColor(const char *color);
	void setHighlight(const char *highlight);

	void printNewLine();
	void printChar(const char ch);
	void printCharNoAnsi(const char ch);

private:
	friend int main(int argc, char *argv[]);

	DiffParser *parser_;

	FILE *output_;

	bool outLineStart_;
	bool outLineIndent_;

	const char *selectedColor_;
	const char *selectedHighlight_;

	const char *printedColor_;
	const char *printedHighlight_;
};

extern NeonApp *app;

#endif // NEONAPP_H
