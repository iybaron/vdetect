#include <sstream>
#include <string>
#include <vector>
#include <map>

using namespace std;

#ifndef VDETECT_H
#define VDETECT_H

string strFileName = "vdetect.str";
vector<string> testFileVector;
vector<string> knownStringNames;
map<string, string> stringMap;
map<string, vector<int> > decStringMap;
bool qflag = false;
bool sflag = false;
bool containsMalformedString = false;
bool foundMatch = false;
stringstream sstream;

void parseCommandLineInput(int, char*[]);
void readKnownStrings(void);
void knownStringsToDec(void);
void scanFiles(void);
int scanNextFile(string, vector<int>, string);
bool isMalformed(string);
bool isComment(string);
bool isHexDigit(char);
int hexCharsToInt(char, char);

#endif
