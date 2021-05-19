#pragma once
#include <string>

enum WatchType
{
	kInt,
	kFloat,
	kString,
	kVector3,
	kBitfield
};


class WatchEntry
{
public:
	WatchEntry(int addr, WatchType type, std::string info = std::string("")) : m_globalAddress(addr), m_type(type), m_info(info), m_drawInGame(true) {}

	int m_globalAddress;
	WatchType m_type;
	std::string m_info;
	std::string m_value;
	bool m_drawInGame;

	void UpdateValue();
	bool operator==(WatchEntry *other)
	{
		return this->m_globalAddress == other->m_globalAddress;
	}

private:
};

std::string GetDisplayForType(uint64_t * globalAddr, WatchType type);
std::string GetDisplayForType(int addrId, WatchType type);