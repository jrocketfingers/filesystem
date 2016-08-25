#include "FS.h"
#include "KernelFS.h"
#include "FSDirectory.h"

KernelFS kernelFSInstance;
KernelFS *FS::myImpl = &kernelFSInstance;

FS::FS() {

}

FS::~FS() {

}

char FS::mount(Partition *partition) {
	return myImpl->mount(partition);
}

char FS::unmount(char partition) {
	return myImpl->unmount(partition);
}

char FS::format(char part) {
	return myImpl->format(part);
}

char FS::readRootDir(char part, EntryNum n, Directory &d) {
	return myImpl->readRootDir(part, n, d);
}

char FS::doesExist(char *fname) {
	return myImpl->doesExist(fname, nullptr);
}

File* FS::open(char *fname, char mode) {
	return myImpl->open(fname, mode);
}

char FS::deleteFile(char *fname) {
	try {
		myImpl->deleteFile(fname);
	}
	catch (NoFileFoundException) {
		return 0;
	}

	return 1;
}
