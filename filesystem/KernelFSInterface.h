#pragma once

#include <string>

using namespace std;

class KernelFSInterface
{
public:
	virtual void close(string) = 0;
};

