#ifndef NEONAPP_H
#define NEONAPP_H

#include <stdio.h>
#include <list>

class Block;
typedef std::list<Block *> BlockList;

class NeonApp
{
public:
	NeonApp(FILE *inputStream, FILE *outputStream);
	virtual ~NeonApp();

protected:
	void processInput();

	bool readLine();
	void stripAnsi();

	void outputBlock(const char *block, const char *blockEnd);
	Block * longestMatch(const char *rem, const char *remEnd, const char *add, const char *addEnd);
	BlockList compareBlocks(const char *a, const char *aEnd, const char *b, const char *bEnd);
	void outputDiff();

	void setColor(const char *color);
	void setHighlight(const char *highlight);

	void printChar(const char ch);

private:
	friend int main(int argc, char *argv[]);

	static NeonApp *instance_;

	FILE *input_;
	FILE *output_;

	char *buf_;
	int bufLen_;
	int bufSize_;

	char *line_;
	int lineLen_;

	const char *blockRem_;
	const char *blockAdd_;

	bool outLineStart_;
	bool outLineIndent_;

	const char *selectedColor_;
	const char *selectedHighlight_;

	const char *printedColor_;
	const char *printedHighlight_;
};

#endif // NEONAPP_H
