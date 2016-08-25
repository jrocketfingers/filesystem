#include "utils.h"

#include <cstring>

void parse_file_path(char* filepath, ParsedFilepath* parsed) {
	parsed->partition = filepath[0];

	char* dot = strchr(filepath, '.');

	memcpy(parsed->filename, &filepath[3], dot - filepath - 3);
	memcpy(parsed->ext, dot + 1, 3);
}
