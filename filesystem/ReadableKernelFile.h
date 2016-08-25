#pragma once
#include "ReliableFilesizeKernelFile.h"

class ReadableKernelFile : public ReliableFilesizeKernelFile
{
public:
	ReadableKernelFile(FileHandle *file_handle);
	~ReadableKernelFile();

	char write(BytesCnt n, const char* buffer) override;
};

