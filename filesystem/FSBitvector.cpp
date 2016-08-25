#include <cassert>
#include "FSBitvector.h"

FSBitvector::FSBitvector(ClusterNo n_clusters) {
	size = n_clusters / 8 + 1;
	_bitvector = new char[GetClusterSize() * 2048]();
}

FSBitvector::FSBitvector(ClusterNo n_clusters, char* buffer) : FSBitvector(n_clusters) {
	memcpy(_bitvector, buffer, size);
}

FSBitvector::~FSBitvector() {
	delete _bitvector;
}

/* Finds a free cluster to allocate */
ClusterNo FSBitvector::Allocate(BitvectorUpdate* update)
{
	ClusterNo allocated_cluster = GetFirstFree();

	lock.acquireWrite();
	set(allocated_cluster, true);

	if (update != nullptr) {
		update->changed_cluster = allocated_cluster / 2048;
		update->changed_cluster_data = _bitvector + (allocated_cluster / 2048);
	}
	lock.releaseWrite();

	return allocated_cluster;
}

/* Allocates the specified cluster */
ClusterNo FSBitvector::Allocate(ClusterNo cluster, BitvectorUpdate* update)
{
	ClusterNo ret;

	lock.acquireRead();
	if (get(cluster)) {
		ret = 0;
	}
	else {
		assert(lock.tryAcquireWrite(true));
		set(cluster, true);

		_initialize_update_data(update, cluster);

		ret = cluster;
		lock.releaseWrite();
	}
	lock.releaseRead();

	return ret;
}

bool FSBitvector::get(size_t idx) {
	BitvectorLocation loc = _get_location(idx);

	return _bitvector[loc.byte] & (1 << loc.offset);
}

void FSBitvector::set(size_t idx, bool value) {
	BitvectorLocation loc = _get_location(idx);

	if (value)
		_bitvector[loc.byte] |= (1 << loc.offset);
	else
		_bitvector[loc.byte] &= ~(1 << loc.offset);
}

ClusterNo FSBitvector::Deallocate(ClusterNo cluster, BitvectorUpdate * update)
{
	lock.acquireWrite();
	set(cluster, false);
	lock.releaseWrite();

	_initialize_update_data(update, cluster);

	return cluster;
}

ClusterNo FSBitvector::GetFirstFree() {
	unsigned long index;

	/* scan the bit vector
	* work in groups of 4 bytes, search for a set bit, bit scan forward is the fastest method
	*/
	ClusterNo i;

	lock.acquireRead();
	for (i = 0; i < size; i += 4) {
		unsigned long data = ~(((unsigned long)(unsigned char)_bitvector[0 + i]) | 
							  (((unsigned long)(unsigned char)_bitvector[1 + i]) << 8) |
						      (((unsigned long)(unsigned char)_bitvector[2 + i]) << 16) |
							  (((unsigned long)(unsigned char)_bitvector[3 + i]) << 24));

		if (_BitScanForward(&index, data))
			break;
	}
	lock.releaseRead();

	return index + i * 8;
}

char * FSBitvector::GetData() {
	return _bitvector;
}

size_t FSBitvector::GetByteSize() {
	return size;
}

ClusterNo FSBitvector::GetClusterSize()
{
	return size / 2048 + 1;
}

BitvectorLocation FSBitvector::_get_location(size_t idx) {
	BitvectorLocation loc;
	loc.byte = idx / 8;
	loc.offset = idx % 8;

	return loc;
}

void FSBitvector::_initialize_update_data(BitvectorUpdate* update, ClusterNo cluster) {
	if (update != nullptr) {
		update->changed_cluster_data = _bitvector + (cluster / 2048);
		update->changed_cluster = cluster / 2048;
	}
}