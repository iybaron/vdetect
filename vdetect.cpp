#include <iostream>
#include <fstream>
#include <string.h>
#include "vdetect.h"

int main(int argc, char *argv[]) {

	int ret;
	parseCommandLineInput(argc, argv);
	readKnownStrings();
	knownStringsToDec();
	scanFiles();
	
	if(!containsMalformedString && !foundMatch) {
		ret = 0;
	} else if(!containsMalformedString) {
		ret = 1;
	} else if(!foundMatch) {
		ret = 2;
	} else {
		ret = 3;
	}
	
	return ret;
}

/*
 * Read arguments from command line and store them where appropriate
 */
void parseCommandLineInput(int argCount, char *args[]) {
	int offset = 1;
	bool fileSpecified = false;

	while(offset < argCount) {
		
		//option '-q': don't print output, just exit status code
		if(strcmp(args[offset], "-q") == 0) {
			qflag = true;
			
		//option '-s': stop scanning when 1st match is found
		} else if(strcmp(args[offset], "-s") == 0) {
			sflag = true;
			
		//use file named in command line instead of vdetect.str	
		} else if(strcmp(args[offset], "-d") == 0) {
			offset++;
			strFileName = args[offset];
			
		//load filenames into vector
		} else {
			fileSpecified = true;
			while(offset < argCount) {
				testFileVector.push_back(args[offset++]);
			}
		}

		offset++;
		
	}

	//if file is not specified take input from command line
	if(!fileSpecified) {
		ofstream ofile("filex");

		if(ofile.is_open()) {
			cout << "Waiting for input" << endl;
			string input;
			getline(cin, input);
			ofile << input << endl;
			ofile.close();
			testFileVector.push_back("filex");
		}
	}
}

/*
 * Reads strings from file containing known strings into memory
 */ 
void readKnownStrings() {
	
	ifstream strFile(strFileName.c_str());
	
	if(strFile.is_open()) {
		
		string line;
		
		int lineNumber = 0;
		while(getline(strFile, line)) {
			
			lineNumber++;
			
			if(!isComment(line)) {
				if(isMalformed(line)) {
					containsMalformedString = true;
					if(!qflag) {
						cout << "line " << lineNumber << ": Malformed line in vdetect.str" << endl;
					}
				} else {
					
					string name;
					string value;
					
					sstream.clear();
					sstream.str(string());
					sstream << line;
					getline(sstream, name, ':');
					getline(sstream, value);
				
					knownStringNames.push_back(name);
					stringMap[name] = value;
				}
			}
		}
		
		strFile.close();
	}
}

/*
 * Converts known strings into decimal representation
 */
void knownStringsToDec() {

	for(int i = 0; i < knownStringNames.size(); i++) {
		string signature = stringMap[knownStringNames[i]];
		int length = signature.size();
		vector<int> stringAsInt;

		for(int j = 0; j < length; j++) {
			if((signature[j] == '\\')
					&& (j+1 < length && signature[j+1] == 'x')
					&& (j+2 < length && isHexDigit(signature[j+2]))
					&& (j+3 < length && isHexDigit(signature[j+3]))) {

				stringAsInt.push_back(hexCharsToInt(signature[j+2], signature[j+3]));
				j += 3;
			} else {
				stringAsInt.push_back(signature[j]);
			}
		}

		decStringMap[knownStringNames[i]] = stringAsInt;
	}

}

/*
 * Open each specified file and determine whether it contains any known strings.
 */
void scanFiles() {
	int i = 0;
	int matchesFound = 0;
	while(i < testFileVector.size()) {
		
		int j = 0;
		while(j < knownStringNames.size() && !(sflag && matchesFound > 0)) {
			
			vector<int> currentKnownString = decStringMap.at(knownStringNames[j]);
			matchesFound += scanNextFile(testFileVector[i], currentKnownString, knownStringNames[j]);
			j++;
		}
		
		i++;
	}
	
	if(matchesFound > 0) {
		foundMatch = true;
	}
	
}

/*
 * Determine if file specified by 'fileName' contains any known strings.
 */

int scanNextFile(string fileName, vector<int> knownString, string knownStringName) {

	int matches = 0;
	ifstream ifile(fileName.c_str(), ios::binary);
	
	if(ifile.is_open()) {
		char c;
		ifile >> noskipws;
		int byteno = 0;
		int j = 0;
		
		while(ifile >> c && !(sflag && matches > 0)) {
			
			if(c == knownString[j]) {
				j++;
			} else if(c == knownString[0]) {
				j = 1;
			} else {
				j = 0;
			}
			
			if(knownString.size() > 0 && j == knownString.size()) {
				
				matches++;
				
				if(!qflag) {
					cout << fileName << "(" << (byteno - knownString.size() + 1) << "): matched signature ";
					for(int i = 0; i < knownStringName.size(); i++) {
						cout << knownStringName[i];
					}
					
					cout << endl;	
				}
				
				j = 0;
			}
			
			byteno++;
		}
		
		ifile.close();
	}
	
	return matches;	
}


/*
 * Returns true if line is malformed
 */
bool isMalformed(string knownLine) {
	bool lineIsMalformed = false;
	
	size_t found = knownLine.find(':');
	
	if(found == string::npos) {
		lineIsMalformed = true;
	}
	
	return lineIsMalformed;
}

/*
 * Returns true if line is a comment (begins with '#')
 */
bool isComment(string knownLine) {
	bool lineIsComment = false;
	
	if(knownLine[0] == '#') {
		lineIsComment = true;
	}
	
	return lineIsComment;
}

/*
 * Returns true if c is a valid hexadecimal value (0-9, a-f, A-F)
 */
bool isHexDigit(char c) {
	bool valid = false;

	if( (c > 47 && c < 58) || (c > 64 && c < 71) || (c > 96 && c < 103)) {
		valid = true;
	}

	return valid;
}

/*
 * Converts two characters representing valid hex digits to a single int
 * Returns -1 if parameters are invalid
 */
int hexCharsToInt(char c1, char c2) {
	int val = -1;

	if(isHexDigit(c1) && isHexDigit(c2)) {
		if(c1 < 58) {
			val = (c1 - 48) * 16;	
		} else if(c1 < 71) {
			val = (c1 - 55) * 16;
		} else {
			val = (c1 - 87) * 16;
		}

		if(c2 < 58) {
			val += (c2 - 48);	
		} else if(c2 < 71) {
			val += (c2 - 55);
		} else {
			val += (c2 - 87);
		}
	}

	return val;
}








