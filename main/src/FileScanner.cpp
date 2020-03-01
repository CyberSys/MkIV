#include "FileScanner.h"
#include <fstream>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>


using namespace std;

FileScanner::FileScanner() {
	mFileCount = 0;
	for (int i = 0; i < 256; i++) {
		strcpy(mFileName[i], "Unread");
	}
}

// scan a subdirectory for files of a particular file extension
bool FileScanner::scan(const char* subdir, const char* extension, bool recursive) {
	char tmp[256];
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
	// Windows implementation
	int length = strlen(subdir);
	char wintendoSubdir[256];
	strcpy(wintendoSubdir, subdir);
	for (int i = 0; i < length; i++) { // replace slashes with the Wintendo equivalents
		if (wintendoSubdir[i] == '/') {
			wintendoSubdir[i] = 92; // the '/' character, which we can't type here as it is also the C++ control char
		}
	}
	sprintf(tmp, "dir /b %s*%s > tmp.txt", wintendoSubdir, extension);
	printf("string now reads %s\n", tmp);
#else
	printf("scanning Unix directory %s\n", subdir);
	// Unix Implementation
	if (recursive) {
		sprintf(tmp, "ls -1 -R %s*%s > tmp.txt", subdir, extension);
	} else {
		sprintf(tmp, "ls -1 %s*%s > tmp.txt", subdir, extension);
	}
#endif
	// list the files
	printf("directory scan command: %s\n", tmp);
	system(tmp);

	// list the files in memory
	if (!readDirFile(subdir, extension)) {
		printf("ERROR reading file into memory in scanner\n");
		return false;
	}

	// remove the temporary file
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
	// Windows implementation
	system("del tmp.txt");	// TODO check
#else
	// Unix Implementation
	system("rm -f tmp.txt");
#endif

	return true;
}

int FileScanner::getFileCount() {
	return mFileCount;
}

char* FileScanner::getFileName(int i) {
	return mFileName[i];
}

// checks scanned files for this one
bool FileScanner::doesFileExist(const char* filename) {
	printf("DEBUG: looking for %s\n", filename);
	for (int i = 0; i < mFileCount; i++) {
		printf("DEBUG: check %s\n", mFileName[i]);
		if (strcmp(mFileName[i], filename) == 0) {
			return true;
		}
	}

	return false;
}

// pull out directory contents from tmp.txt and load into memory sans path and extension
bool FileScanner::readDirFile(const char* prefix, const char* postfix) {
	// load file
	ifstream file;

	// append path to file
	file.open("tmp.txt");
	if (!file.is_open()) {
		printf("Error: Could not open file tmp.txt\n");
		return false;
	}

	#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
		int prefixLength = 0;
	#else
		int prefixLength = strlen(prefix);	// windows dir doesn't list path name as well
	#endif
	int postfixLength = strlen(postfix);

	mFileCount = 0;
	char fileName[256];
	while (file.getline(fileName, 256)) {
		// cut off pre and post fixes
		int postfixStart = strlen(fileName) - postfixLength;
		char tempName[256];
		int j = 0;
		for (int i = prefixLength; i < postfixStart; i++) {
			tempName[j] = fileName[i];
			j++;
		}
		tempName[j] = '\0';

		// add to entries
		strcpy(mFileName[mFileCount], tempName);
		mFileCount++;
	}

	file.close();

	return true;
}

