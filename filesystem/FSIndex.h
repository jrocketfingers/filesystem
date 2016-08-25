#pragma once

#include "FSPartition.h"

typedef struct {
	ClusterNo index[512] = { 0 };
	ClusterNo cluster;
} NestedIndex;

typedef struct {
	bool is_nested = false;
	ClusterNo index = 0;
	ClusterNo nested_index = 0;
} IndexCursor;

class FSPartition;

class FSIndex
{
public:
	FSIndex(FSPartition *partition);
	FSIndex(FSPartition *partition, ClusterNo index_cluster);
	~FSIndex();

	ClusterNo GetPhysCluster(ClusterNo cluster_number);
	void DeallocateAll();
	ClusterNo Allocate();

	ClusterNo GetIndexCluster();
private:
	ClusterNo get_logical_cluster(IndexCursor* cursor);

	FSPartition *partition;
	ClusterNo index_cluster;
	ClusterNo index[256] = { 0 };
	NestedIndex* nested_index[256] = { nullptr };
	IndexCursor next;

	static const ClusterNo SECOND_LEVEL_INDEX = 256;
	static const ClusterNo NUM_INDEX_ENTRIES = 512;

	RWLock lock;
};

