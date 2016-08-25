#include "FSPartition.h"

FSPartition::FSPartition(Partition * p) {
	this->p = p;
}

FSPartition::~FSPartition() {
	delete bitvector;
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
	FSIndex* index = new FSIndex(this, GetRootDirectoryIndexCluster());
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
	p->writeCluster(update.changed_cluster, update.changed_cluster_data);
	allocation_lock.releaseWrite();

	return cluster;
}

ClusterNo FSPartition::Allocate(ClusterNo cluster)
{
	BitvectorUpdate update;

	allocation_lock.acquireWrite();
	ClusterNo allocated_cluster = bitvector->Allocate(cluster, &update);
	p->writeCluster(update.changed_cluster, update.changed_cluster_data);
	allocation_lock.releaseWrite();

	return allocated_cluster;
}

void FSPartition::Deallocate(ClusterNo n) {
	BitvectorUpdate update;

	allocation_lock.acquireWrite();
	bitvector->Deallocate(n, &update);
	p->writeCluster(update.changed_cluster, update.changed_cluster_data);
	allocation_lock.releaseWrite();
}

ClusterNo FSPartition::GetNumOfClusters() const {
	return this->p->getNumOfClusters();
}

int FSPartition::ReadCluster(ClusterNo n, char * buffer) {
	return this->p->readCluster(n, buffer);
}

int FSPartition::WriteCluster(ClusterNo n, const char * buffer)
{
	return this->p->writeCluster(n, buffer);
}

int FSPartition::WriteCluster(ClusterNo n, const char * buffer, size_t size, size_t offset) {
	char prep_buffer[ClusterSize] = { 0 };

	memcpy(prep_buffer + offset, buffer, size);

	return FSPartition::WriteCluster(n, prep_buffer);
}
