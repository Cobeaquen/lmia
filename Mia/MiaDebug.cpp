#include "MiaDebug.h"

MiaDebug::MiaDebug(Mia _mia)
{
	mia = _mia;
	breakPoints = std::vector<uint16_t>();
}