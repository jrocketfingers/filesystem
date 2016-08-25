#include "WritableKernelFile.h"



WritableKernelFile::WritableKernelFile(FileHandle *file_handle) : ReliableFilesizeKernelFile(file_handle)
{
	this->file_handle->incWriters();
}


WritableKernelFile::~WritableKernelFile()
{
	this->file_handle->decWriters();
}

char WritableKernelFile::write(BytesCnt len, const char * buffer)
{
	char ret = KernelFile::write(len, buffer);

	if (filepos > file_handle->GetFilesize()) {
		file_handle->UpdateFilesize(filepos);
	}

	return ret;
}
