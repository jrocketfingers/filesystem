#pragma once

#include <exception>
#include "KernelFile.h"
#include "FileHandle.h"

using namespace std;

class NoFileFoundException : runtime_error {
public:
	NoFileFoundException() : runtime_error("ReadCluster has failed") {}
	~NoFileFoundException() {}
};

class FSDirectory : KernelFile {
public:
	FSDirectory(FileHandle* file_handle);
	~FSDirectory();

	EntryNum AddEntry(FileHandle* file_handle);
	char GetEntry(EntryNum entry_num, Entry* entry);
	char UpdateEntry(EntryNum entry_num, const Entry* entry);
	bool DeleteEntry(EntryNum entry);
	EntryNum FindEntry(char* fpath, Entry* entry);

	char seek(BytesCnt loc) override;
private:
	char seek_empty(EntryNum *entry);
};

