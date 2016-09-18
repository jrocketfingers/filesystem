#include <cassert>
#include "FSPartition.h"

#define CACHE_ENABLED

FSPartition::FSPartition(Partition * p, char letter) {
	this->p = p;
	this->letter = letter;
}

FSPartition::~FSPartition() {
	delete bitvector;

	cache_lock.acquireWrite();
	for (ClusterNo i = 0; i < kCacheClusters; i++) {
		if (cache.dirty[i] && cache.valid[i])
			this->p->writeCluster(cache.tag[i], cache.data[i]);
	}
	cache_lock.releaseWrite();
}

ClusterNo FSPartition::GetRootDirectoryIndexCluster()
{
	return bitvector->GetClusterSize(); // bitvector's cluster size == index of the root directory (the first cluster after the bitvector)
}

FileHandle * FSPartition::GetRootDirectoryFileHandle()
{
	return root_dir;
}

void FSPartition::Mount(KernelFSInterface* fs) {
	n_bitvector_clusters = (p->getNumOfClusters() / 8) / 2048 + 1;
	char buffer[ClusterSize];

	p->readCluster(0, buffer);

	bitvector = new FSBitvector(64, buffer);

	FileHandle* file_handle = new FileHandle(fs);
	FSIndex* index = new FSIndex(this, GetRootDirectoryIndexCluster(), false);
	file_handle->SetIndex(index);
	file_handle->SetPartition(this);

	root_dir = file_handle;

	_is_mounted = true;
}

char FSPartition::Format() {
	if(bitvector != nullptr)
		delete bitvector;

	bitvector = new FSBitvector(p->getNumOfClusters());

	ClusterNo n_bitvector_clusters = p->getNumOfClusters() / 2048 / 8 + 1; /* number of bitvector clusters*/
	for (ClusterNo i = 0; i < n_bitvector_clusters; i++) {
		bitvector->Allocate(i, nullptr); // allocate the bitvector cluster
	}

	char buffer[2048] = { 0 };
	bitvector->Allocate(GetRootDirectoryIndexCluster(), nullptr);
	WriteCluster(GetRootDirectoryIndexCluster(), buffer);

	/* update the directory handle index */
	FSIndex* index;
	if ((index = root_dir->GetIndex()) != nullptr) {
		delete index;
		index = new FSIndex(this, GetRootDirectoryIndexCluster());
		root_dir->SetIndex(index);
	}

	char* bitvector_data = bitvector->GetData();
	for(ClusterNo i = 0; i < n_bitvector_clusters; i++)
		WriteCluster(i, bitvector_data + i * 2048);

	return 1;
}

ClusterNo FSPartition::Allocate()
{
	BitvectorUpdate update;

	allocation_lock.acquireWrite();
	ClusterNo cluster = bitvector->Allocate(&update);
	this->WriteCluster(update.changed_cluster, update.changed_cluster_data);
	n_allocated_clusters++;
	allocation_lock.releaseWrite();

	return cluster;
}

ClusterNo FSPartition::Allocate(ClusterNo cluster)
{
	BitvectorUpdate update;

	allocation_lock.acquireWrite();
	ClusterNo allocated_cluster = bitvector->Allocate(cluster, &update);
	this->WriteCluster(update.changed_cluster, update.changed_cluster_data);
	n_allocated_clusters++;
	allocation_lock.releaseWrite();

	return allocated_cluster;
}

void FSPartition::Deallocate(ClusterNo n) {
	BitvectorUpdate update;

	allocation_lock.acquireWrite();
	bitvector->Deallocate(n, &update);
	this->WriteCluster(update.changed_cluster, update.changed_cluster_data);
	n_allocated_clusters--;
	allocation_lock.releaseWrite();
}

ClusterNo FSPartition::GetNumOfClusters() const {
	return this->p->getNumOfClusters();
}

void FSPartition::CommitCache(ClusterNo n) {
	ClusterNo cache_entry = n % kCacheClusters;

	if (n != cache.tag[cache_entry]) {
		if (cache.valid[cache_entry] && cache.dirty[cache_entry])
			this->p->writeCluster(cache.tag[cache_entry], cache.data[cache_entry]);

		cache.valid[cache_entry] = 1;
		cache.dirty[cache_entry] = 0;
		this->p->readCluster(n, cache.data[cache_entry]);
		cache.tag[cache_entry] = n;
	}
}

ClusterNo FSPartition::GetNumOfFreeclusters() const
{
	return GetNumOfClusters() - n_allocated_clusters;
}

int FSPartition::ReadCluster(ClusterNo n, char * buffer) {
#ifdef CACHE_ENABLED
	ClusterNo cache_entry = n % kCacheClusters;

	while (true) {
		cache_lock.acquireRead();
		if (cache_lock.tryAcquireWrite(true)) {
			CommitCache(n);
			cache_lock.releaseWrite();

			memcpy(buffer, cache.data[cache_entry], ClusterSize);
			cache_lock.releaseRead();

			return 1;
		}
		cache_lock.releaseRead();
		Sleep(0);
	}
#else
	this->p->readCluster(n, buffer);
#endif

	return 1;
}

int FSPartition::WriteCluster(ClusterNo n, const char * buffer)
{
#ifdef CACHE_ENABLED
	ClusterNo cache_entry = n % kCacheClusters;

	cache_lock.acquireWrite();
	CommitCache(n);

	memcpy(cache.data[cache_entry], buffer, ClusterSize);
	cache.dirty[cache_entry] = 1;
	cache_lock.releaseWrite();
#else
	this->p->writeCluster(n, buffer);
#endif

	return 1;
}
