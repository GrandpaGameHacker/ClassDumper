#pragma once
#include "stdheaders.h"
#include "RTTI.h"
extern Console g_console; // for debugging
namespace VTHelper
{
	bool IsValid(void* VTable_start, SectionInfo* sectionInfo);
	size_t GetNumberOfFunctions(void* VTable_start, SectionInfo* sectionInfo);
	std::vector<uintptr_t> GetListOfFunctions(void* VTable_start, SectionInfo* sectionInfo);
	std::vector<uintptr_t> FindVTables(SectionInfo* sectionInfo);
}

namespace ClassHelper {
}
