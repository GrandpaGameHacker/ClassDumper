#pragma once
#include "stdheaders.h"
#include "RTTI.h"
#include "Memory.h"
using namespace std;

const unsigned int TYPEDESCRIPTOR_SIGNITURE = 0x56413F2E;
bool IsValid(void* VTable_start, SectionInfo* sectionInfo);
vector<uintptr_t> GetListOfFunctions(void* VTable_start, SectionInfo* sectionInfo);
vector<uintptr_t> FindAllVTables(SectionInfo* sectionInfo);
