#include "FSIndex.h"

FSIndex::FSIndex(FSPartition *partition)
{
	this->partition = partition;
}

FSIndex::FSIndex(FSPartition *partition, ClusterNo index_cluster) : FSIndex(partition)
{
	this->index_cluster = index_cluster;
	partition->ReadCluster(index_cluster, (char*)index);

	int i, j;
	for (i = 0; i < SECOND_LEVEL_INDEX; i++) {
		if (index[i] == 0)
			break;
	}

	if (i == SECOND_LEVEL_INDEX) {
		next.is_nested = true;

		/* check for the 2nd level indexes */
		for (i = SECOND_LEVEL_INDEX, j = 0; i < NUM_INDEX_ENTRIES; i++, j++) {
			if (index[i] == 0)
				break;

			nested_index[j] = new NestedIndex();
			partition->ReadCluster(index[i], (char*)(nested_index[j]->index));
		}
	}
}

FSIndex::~FSIndex()
{
}

ClusterNo FSIndex::GetPhysCluster(ClusterNo cluster_number)
{
	ClusterNo ret;

	if (cluster_number >= SECOND_LEVEL_INDEX) {
		int i = (cluster_number - SECOND_LEVEL_INDEX) / NUM_INDEX_ENTRIES;
		int j = (cluster_number - SECOND_LEVEL_INDEX) % NUM_INDEX_ENTRIES;

		if (nested_index[i] == nullptr || nested_index[i]->index == nullptr)
			ret = 0;
		else
			ret = nested_index[i]->index[j];

		return ret;
	}
	else {
		if (index[cluster_number] == 0)
			ret = 0;

		ret = index[cluster_number];
	}

	return ret;
}

void FSIndex::DeallocateAll()
{
	ClusterNo phys_cluster;
	int file_cluster = 0;
	while ((phys_cluster = GetPhysCluster(file_cluster)) != 0) {
		partition->Deallocate(phys_cluster);
		file_cluster++;
	}

	partition->Deallocate(this->index_cluster);

	for (int i = 0; i < SECOND_LEVEL_INDEX; i++) {
		if (nested_index[i] != nullptr)
			delete nested_index[i];
	}

	memset(index, 0, sizeof(index[0]) * 256);
}

ClusterNo FSIndex::Allocate()
{
	ClusterNo phys_cluster = partition->Allocate();
	if (!next.is_nested) {
		/* fill a regular first level entry */
		if(next.index < SECOND_LEVEL_INDEX) {
			index[next.index++] = phys_cluster;
			partition->WriteCluster(index_cluster, (char*)index);
		} else {
			/* mark it as nested, to be handled with the nested check */
			next.is_nested = true;
			next.nested_index = 0;
		}
	 }

	if (next.is_nested) {
		/* if the index is topped out allocate another */
		if (next.nested_index == NUM_INDEX_ENTRIES) {
			next.nested_index = 0;

			nested_index[next.index] = new NestedIndex();
			nested_index[next.index]->cluster = partition->Allocate();

			next.index++;
		}

		/* otherwise just fill the current one */
		nested_index[next.index]->index[next.nested_index++] = phys_cluster;
		partition->WriteCluster(nested_index[next.index]->cluster, (char*)(nested_index[next.index]->index));
	}

	return get_logical_cluster(&next) - 1; // return the previous logical cluster
}

ClusterNo FSIndex::GetIndexCluster()
{
	return index_cluster;
}

ClusterNo FSIndex::get_logical_cluster(IndexCursor* cursor)
{
	ClusterNo cluster = cursor->index;
	if (cursor->is_nested)
		cluster += cursor->nested_index;

	return cluster;
}
