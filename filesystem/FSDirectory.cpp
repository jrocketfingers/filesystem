#include "FSDirectory.h"
#include "FileHandle.h"

#include "utils.h"

FSDirectory::FSDirectory(FileHandle* file_handle) : KernelFile(file_handle)
{
	this->file_handle->incWriters();
}


FSDirectory::~FSDirectory()
{
	this->file_handle->decWriters();
}

EntryNum FSDirectory::AddEntry(FileHandle * file_handle)
{
	EntryNum entry_num;

	if (!seek_empty(&entry_num)) {
		ClusterNo cluster = index->Allocate();
		char buffer[2048] = { 0 };

		partition->WriteCluster(index->GetPhysCluster(cluster), buffer);
		entry_num = cluster * ClusterSize / 20;
	}

	Entry entry;
	memcpy(entry.name, file_handle->GetFilename(), 8);
	memcpy(entry.ext, file_handle->GetExtension(), 3);
	entry.indexCluster = file_handle->GetIndex()->GetIndexCluster();
	entry.size = 0;

	seek(20 * entry_num);
	write(20, (char*)&entry);

	return filepos / 20 - 1;
}

char FSDirectory::GetEntry(EntryNum entry_num, Entry * entry)
{
	seek(entry_num * 20);

	BytesCnt bytes_read = read(20, (char*)entry);

	return bytes_read == 20;
}

char FSDirectory::UpdateEntry(EntryNum entry_num, const Entry * entry)
{

	seek(entry_num * 20);

	write(20, (char*)entry);

	return 1;
}

bool FSDirectory::DeleteEntry(EntryNum entry_num)
{
	Entry entry = {
		"\0\0\0\0\0\0\0",
		"\0\0",
		'\0',
		0,
		0
	};
	UpdateEntry(entry_num, &entry);

	return true;
}

EntryNum FSDirectory::FindEntry(char * fpath, Entry* entry)
{
	ParsedFilepath parsed;
	parse_file_path(fpath, &parsed);

	seek(0);

	Entry* temp_entry;

	if (entry != nullptr)
		temp_entry = entry;
	else
		temp_entry = new Entry();

	EntryNum current = 0;

	char status;

	do {
		if (!GetEntry(current++, temp_entry))
			throw NoFileFoundException();
	} while ((strncmp(temp_entry->name, parsed.filename, 8) != 0) || (strncmp(temp_entry->ext, parsed.ext, 3) != 0));

	if (entry == nullptr)
		delete temp_entry;

	return filepos / 20 - 1;
}

char FSDirectory::seek(BytesCnt loc)
{
	filepos = loc;

	return 1;
}

/* NOT THREAD SAFE */
char FSDirectory::seek_empty(EntryNum *entry_num) {
	seek(0);

	char empty_entry[20] = { 0 };
	char buffer[20] = { 0xFF };

	EntryNum current_entry = -1; // I am aware that this is unsigned, I just need it to be 0 after the first run

	do {
		if (read(20, buffer) < 20)
			return 0;

		current_entry++;
	} while (memcmp(buffer, empty_entry, 20) != 0);

	if (entry_num != nullptr)
		*entry_num = current_entry;

	return 1;
}
