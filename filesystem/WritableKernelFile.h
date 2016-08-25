#pragma once
#include "ReliableFilesizeKernelFile.h"

class WritableKernelFile : public ReliableFilesizeKernelFile
{
public:
	WritableKernelFile(FileHandle *file_handle);
	~WritableKernelFile();

	char write(BytesCnt, const char* buffer) override;
};
