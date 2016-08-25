#pragma once
#include "KernelFile.h"
class ReliableFilesizeKernelFile : public KernelFile
{
public:
	ReliableFilesizeKernelFile(FileHandle* file_handle);
	virtual ~ReliableFilesizeKernelFile();

	BytesCnt read(BytesCnt n, char* buffer) override;
};

