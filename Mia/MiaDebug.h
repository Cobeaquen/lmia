#pragma once
#include <vector>
#include "mia.h"

class MiaDebug
{
public:
	MiaDebug(Mia _mia);
	Mia mia;
	std::vector<uint16_t> breakPoints;
};

