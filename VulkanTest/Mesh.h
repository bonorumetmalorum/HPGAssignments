#pragma once
#include <vector>
class Mesh
{
public:
	Mesh();
	~Mesh();

private:
	std::vector<float> vertices;
	uint32_t offset = 0;
};

