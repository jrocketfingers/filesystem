#pragma once

#include <map>
#include <string>
#include "FSBitvector.h"
#include "FileHandle.h"
#include "KernelFile.h"
#include "KernelFSInterface.h"
#include "part.h"

using namespace std;

const ClusterNo kCacheClusters = 100;

typedef struct {
	unsigned char dirty[kCacheClusters] = { 0 };
	unsigned char valid[kCacheClusters] = { 0 };
	ClusterNo tag[kCacheClusters];
	char data[kCacheClusters][2048];

	ClusterNo next_slot = 0;
} Cache;

// Implements filesystem related operations over a wrapped partition
class FSPartition {
public:
	FSPartition(Partition *p, char letter);

	void Mount(KernelFSInterface* fs);
	char Format();
	ClusterNo Allocate();
	ClusterNo Allocate(ClusterNo n);
	void Deallocate(ClusterNo n);
	ClusterNo GetRootDirectoryIndexCluster();
	FileHandle* GetRootDirectoryFileHandle();
	void CommitCache(ClusterNo n);

	ClusterNo GetNumOfFreeclusters() const;

	/* Wrappers */
	ClusterNo GetNumOfClusters() const;
	int ReadCluster(ClusterNo n, char *buffer);
	int WriteCluster(ClusterNo n, const char *buffer);

	~FSPartition();
	/* End wrappers */
private:
	Partition *p;
	ClusterNo n_bitvector_clusters;
	ClusterNo n_allocated_clusters = 0;

	FSBitvector *bitvector = nullptr;
	bool _is_mounted = false;

	map<string, FileHandle> open_files;

	FileHandle* root_dir;

	char letter;

	Cache cache;
	RWLock cache_lock;

	RWLock allocation_lock;
};
