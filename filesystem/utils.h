#pragma once

typedef struct {
	char filename[8] = { 0 };
	char ext[3] = { 0 };
	char partition;
} ParsedFilepath;

void parse_file_path(char* filepath, ParsedFilepath* parsed);
