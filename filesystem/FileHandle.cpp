#include <cassert>
#include "FileHandle.h"
#include "FSDirectory.h"

FileHandle::FileHandle(KernelFSInterface *fs) {
	this->fs = fs;
	fpath.clear();
}

FileHandle::FileHandle(KernelFSInterface *fs, const char *fpath) : fpath(fpath)
{
	this->fs = fs;
}

FileHandle::~FileHandle()
{
}

void FileHandle::SetPartition(FSPartition * partition)
{
	this->partition = partition;
}

FSPartition * FileHandle::GetPartition()
{
	return partition;
}

void FileHandle::SetIndex(FSIndex* index)
{
	this->index = index;
}

FSIndex* FileHandle::GetIndex()
{
	return index;
}

void FileHandle::SetDirectoryCluster(ClusterNo cluster)
{
	directory_cluster = cluster;
}

ClusterNo FileHandle::GetDirectoryCluster()
{
	return directory_cluster;
}

void FileHandle::SetDirectoryEntry(EntryNum entry)
{
	directory_entry = entry;
}

EntryNum FileHandle::GetDirectoryEntry()
{
	return directory_entry;
}

void FileHandle::SetFilename(char * filename)
{
	strncpy(this->filename, filename, 8);
}

char * FileHandle::GetFilename()
{
	return filename;
}

void FileHandle::SetExtension(char * extension)
{
	strncpy(this->ext, extension, 3);
}

char * FileHandle::GetExtension()
{
	return ext;
}

void FileHandle::SetFilesize(unsigned long filesize)
{
	this->filesize = filesize;
}

void FileHandle::UpdateFilesize(unsigned long filesize)
{
	this->filesize = filesize;

	FSDirectory directory(partition->GetRootDirectoryFileHandle());

	Entry entry;
	directory.GetEntry(directory_entry, &entry);
	entry.size = filesize;
	directory.UpdateEntry(this->directory_entry, &entry);
}

unsigned long FileHandle::GetFilesize()
{
	return filesize;
}

void FileHandle::incReaders()
{
	lock.acquireRead();

	access_count_lock.acquireWrite();
	n_readers++;
	access_count_lock.releaseWrite();
}

void FileHandle::decReaders()
{
	access_count_lock.acquireWrite();
	assert(n_readers != 0);
	n_readers--;
	access_count_lock.releaseWrite();

	lock.releaseRead();
}

void FileHandle::incWriters()
{
	lock.acquireWrite();

	access_count_lock.acquireWrite();
	assert(n_readers == 0);
	assert(n_writers == 0);
	n_writers++;
	access_count_lock.releaseWrite();
}

void FileHandle::decWriters()
{
	access_count_lock.acquireWrite();
	assert(n_writers == 1);
	assert(n_readers == 0);
	n_writers--;
	access_count_lock.releaseWrite();

	lock.releaseWrite();
}

RWLock * FileHandle::GetLock()
{
	return &lock;
}
