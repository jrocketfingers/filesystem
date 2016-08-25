#include "KernelFS.h"
#include "FSDirectory.h"
#include "WritableKernelFile.h"
#include "ReadableKernelFile.h"
#include "utils.h"

KernelFS::KernelFS() {
	partitions = new FSPartition*[26]();
}

KernelFS::~KernelFS() {

}

char KernelFS::mount(Partition *partition) {
	fsLock.acquireWrite();

	char i;

	for (i = 0; i < 26; i++) {
		if (partitions[i] == nullptr)
			break;
	}

	if (i < 26) {
		partitions[i] = new FSPartition(partition);
		partitions[i]->Mount(this);

		i = i + 'A';
	}
	else {
		i = '0';
	}

	fsLock.releaseWrite();

	return i;
}

bool KernelFS::unmount(char part) {
	bool result = false;
	FSPartition *unmounted;

	fsLock.acquireWrite();

	if (partitions[part - 'A'] == nullptr) {
		result = false;
	}
	else {
		unmounted = partitions[part - 'A'];
		partitions[part - 'A'] = nullptr;
		delete unmounted;
		result = true;
	}

	fsLock.releaseWrite();

	return result;
}

char KernelFS::format(char part) {
	FSPartition *p = partitions[part - 'A'];

	return p->Format();
	return 0;
}

char KernelFS::readRootDir(char part, EntryNum n, Directory & d)
{
	FSPartition *partition = partitions[part - 'A'];
	FSDirectory directory(partition->GetRootDirectoryFileHandle());

	EntryNum entry_num = n;
	char read_so_far;
	for(read_so_far = 0; read_so_far < ENTRYCNT; read_so_far++, entry_num++) { // maximum returned etnries is 64, since Directory: typedef Entry[64] Directory;
		if (!directory.GetEntry(entry_num, (d + read_so_far)))
			break;
	}

	return read_so_far;
}

char KernelFS::doesExist(char* fpath, FileHandle** file_handle)
{
	/* Search the open file table */
	map<string, FileHandle*>::iterator it = file_table.find(string(fpath));

	if(it != file_table.end()) {
		if (file_handle != nullptr) {
			if (*file_handle != nullptr) // if I already allocated
				delete *file_handle;

			/* an open handle already exists */
			*file_handle = it->second;
		}

		return 1;
	}

	Entry entry;

	bool found = true;

	FSPartition *partition = partitions[fpath[0] - 'A'];

	FSDirectory directory(partition->GetRootDirectoryFileHandle());
	
	try {
		EntryNum entry_num = directory.FindEntry(fpath, &entry);

		/* a new handle has to be filled out */
		if (file_handle != nullptr) {
			(*file_handle)->SetFilename(entry.name);
			(*file_handle)->SetExtension(entry.ext);
			FSIndex* index = new FSIndex(partition, entry.indexCluster);
			(*file_handle)->SetIndex(index);
			(*file_handle)->SetFilesize(entry.size);
			(*file_handle)->SetPartition(partition);
			(*file_handle)->SetDirectoryEntry(entry_num);
		}

		return 1;
	}
	catch (NoFileFoundException) {
		return 0;
	}
}

File* KernelFS::open(char* fpath, char mode) {
	fsLock.acquireRead();

	FileHandle* file_handle = new FileHandle(this, fpath);

	if (!doesExist(fpath, &file_handle) && mode == 'w') {
		if(file_handle != nullptr)
			delete file_handle;

		file_handle = _createPhysicalFile(fpath);
	}

	File* file = _createFileInstance(file_handle, mode);
	fsLock.releaseRead();

	return file;
}

void KernelFS::close(string fpath)
{
	file_table.erase(fpath);
}

char KernelFS::deleteFile(char * fname)
{
	ParsedFilepath parsed;
	parse_file_path(fname, &parsed);

	string string_fname(fname);

	file_table.erase(string_fname);

	FSPartition *partition = partitions[parsed.partition - 'A'];

	FSDirectory directory(partition->GetRootDirectoryFileHandle());

	FileHandle* file_handle = new FileHandle(this, fname);
	doesExist(fname, &file_handle);

	file_handle->GetIndex()->DeallocateAll();

	directory.DeleteEntry(file_handle->GetDirectoryEntry());

	delete file_handle;

	return 1;
}

FileHandle* KernelFS::_createPhysicalFile(char* fpath) {
	FileHandle* file_handle = new FileHandle(this, fpath);

	ParsedFilepath parsed;
	parse_file_path(fpath, &parsed);

	FSPartition* partition = partitions[parsed.partition - 'A'];

	file_handle->SetFilename(parsed.filename);
	file_handle->SetExtension(parsed.ext);
	file_handle->SetPartition(partition);

	ClusterNo index_cluster = partition->Allocate();
	FSIndex *index = new FSIndex(partition, index_cluster);

	file_handle->SetIndex(index);
	file_handle->SetDirectoryCluster(partition->GetRootDirectoryIndexCluster());

	/* clean the index cluster */
	char buffer[ClusterSize] = { 0 };

	partition->WriteCluster(index_cluster, buffer);

	/* Update the directory */
	FSDirectory directory(partition->GetRootDirectoryFileHandle());

	file_handle->SetDirectoryEntry(directory.AddEntry(file_handle));

	/* Add the file to the list of open files */
	file_table.insert(pair<string, FileHandle*>(fpath, file_handle));

	return file_handle;
}

File * KernelFS::_createFileInstance(FileHandle * file_handle, char mode)
{
	KernelFile* kernel_file = nullptr;
	if (mode == 'w') {
		kernel_file = new WritableKernelFile(file_handle);
	}
	else {
		kernel_file = new ReadableKernelFile(file_handle);
	}

	File* file = new File();

	file->myImpl = kernel_file;

	return file;
}
