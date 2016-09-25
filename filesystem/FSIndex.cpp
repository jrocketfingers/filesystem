#include "FSIndex.h"

FSIndex::FSIndex(FSPartition *partition)
{
	this->partition = partition;
}

FSIndex::FSIndex(FSPartition *partition, ClusterNo index_cluster) : FSIndex(partition)
{
	this->index_cluster = index_cluster;
}

FSIndex::FSIndex(FSPartition *partition, ClusterNo index_cluster, bool fresh) : FSIndex(partition, index_cluster)
{
	if (!fresh) {
		partition->ReadCluster(index_cluster, (char*)index);

		for (next.index = 0; next.index < SECOND_LEVEL_INDEX; next.index++) {
			if (index[next.index] == 0)
				break;
		}

		if (next.index == SECOND_LEVEL_INDEX) {
			next.is_nested = true;

			/* check for the 2nd level indexes */
			for (next.index = SECOND_LEVEL_INDEX, next.nested_index = 0; next.index < NUM_INDEX_ENTRIES; next.index++, next.nested_index++) {
				nested_index[next.nested_index] = new NestedIndex();

				if (index[next.index] == 0)
					break;

				partition->ReadCluster(index[next.index], (char*)(nested_index[next.nested_index]->index));
			}
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
	for (int i = 0; i < NUM_INDEX_ENTRIES; i++) {
		if (index[i] == 0)
			break;

		if (i >= SECOND_LEVEL_INDEX) {
			for (int j = 0; j < NUM_INDEX_ENTRIES; j++) {
				ClusterNo phys = nested_index[i - SECOND_LEVEL_INDEX]->index[j];
				if (phys == 0)
					break;

				partition->Deallocate(phys);
			}

			delete nested_index[i - SECOND_LEVEL_INDEX];
			nested_index[i - SECOND_LEVEL_INDEX] = nullptr;
		}

		partition->Deallocate(index[i]);
	}

	memset(index, 0, sizeof(index[0]) * 512);

	next.index = 0;
	next.is_nested = false;
	next.nested_index = 0;
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

			nested_index[next.index - 256] = new NestedIndex();
			ClusterNo new_index_cluster = partition->Allocate();
			nested_index[next.index - 256]->cluster = new_index_cluster;
			index[next.index] = new_index_cluster;
			partition->WriteCluster(index_cluster, (char*)index);
		}
	 }

	if (next.is_nested) {
		/* if the index is topped out allocate another */
		if (next.nested_index == NUM_INDEX_ENTRIES) {
			next.nested_index = 0;

			next.index++;

			nested_index[next.index - 256] = new NestedIndex();
			ClusterNo new_index_cluster = partition->Allocate();
			nested_index[next.index - 256]->cluster = new_index_cluster;
			index[next.index] = new_index_cluster;
			partition->WriteCluster(index_cluster, (char*)index);
		}

		/* otherwise just fill the current one */
		nested_index[next.index - 256]->index[next.nested_index++] = phys_cluster;
		partition->WriteCluster(nested_index[next.index - 256]->cluster, (char*)(nested_index[next.index - 256]->index));
	}

	return get_logical_cluster(&next) - 1; // return the previous logical cluster
}

ClusterNo FSIndex::GetIndexCluster()
{
	return index_cluster;
}

ClusterNo FSIndex::get_logical_cluster(IndexCursor* cursor)
{
	ClusterNo cluster;

	if (cursor->is_nested)
		cluster = 256 + (cursor->index - 256) * 512 + cursor->nested_index;
	else
		cluster = cursor->index;

	return cluster;
}
