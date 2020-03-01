#ifndef MARKIV_FILESCANNER_H_
#define MARKIV_FILESCANNER_H_

#include "Ogre.h"	// needed for the win32 def

using namespace Ogre;

// class for reading data directory contents
class FileScanner {
public:
	FileScanner();
	bool scan(const char* subdir, const char* extension, bool recursive = false);	// very basic scanner
	int getFileCount();
	char* getFileName(int i);
	bool doesFileExist(const char* filename); // checks scanned files for this one

private:
	bool readDirFile(const char* prefix, const char* postfix);

	int mFileCount;
	char mFileName[256][256];
};

#endif
