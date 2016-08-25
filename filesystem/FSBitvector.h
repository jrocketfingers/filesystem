#pragma once

#include <Windows.h>
#include "RWLock.h"
#include "part.h"

typedef struct {
	size_t byte;
	char offset;
} BitvectorLocation;

/* describes changes to be made to a bitvector cluster */
typedef struct {
	char* changed_cluster_data;
	ClusterNo changed_cluster;
} BitvectorUpdate;

class FSBitvector {
public:
	FSBitvector(ClusterNo n_clusters);
	FSBitvector(ClusterNo n_clusters, char *buffer);
	~FSBitvector();

	ClusterNo Allocate(BitvectorUpdate* update);

	/* returns the changed bitvector cluster */
	ClusterNo Allocate(ClusterNo cluster, BitvectorUpdate* update);

	/* returns the changed bitvector cluster */
	ClusterNo Deallocate(ClusterNo cluster, BitvectorUpdate* update);

	ClusterNo GetFirstFree();
	char* GetData();
	size_t GetByteSize();
	ClusterNo GetClusterSize();
private:
	bool get(size_t idx);
	void set(size_t idx, bool value);

	BitvectorLocation _get_location(size_t idx);
	void _initialize_update_data(BitvectorUpdate* update, ClusterNo cluster);

	char* _bitvector;
	size_t size;

	RWLock lock;
};
