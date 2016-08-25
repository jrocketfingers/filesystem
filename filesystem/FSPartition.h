#pragma once

#include <map>
#include <string>
#include "FSBitvector.h"
#include "FileHandle.h"
#include "KernelFile.h"
#include "KernelFSInterface.h"
#include "part.h"

using namespace std;

// Implements filesystem related operations over a wrapped partition
class FSPartition {
public:
	FSPartition(Partition *p);

	void Mount(KernelFSInterface* fs);
	char Format();
	ClusterNo Allocate();
	ClusterNo Allocate(ClusterNo n);
	void Deallocate(ClusterNo n);
	ClusterNo GetRootDirectoryIndexCluster();
	FileHandle* GetRootDirectoryFileHandle();

	/* Wrappers */
	ClusterNo GetNumOfClusters() const;
	int ReadCluster(ClusterNo n, char *buffer);
	int WriteCluster(ClusterNo n, const char *buffer);
	int WriteCluster(ClusterNo n, const char *buffer, size_t size, size_t offset);

	~FSPartition();
	/* End wrappers */
private:
	Partition *p;
	ClusterNo n_bitvector_clusters;
	FSBitvector *bitvector = nullptr;
	bool _is_mounted = false;

	map<string, FileHandle> open_files;

	FileHandle* root_dir;

	RWLock allocation_lock;
};
