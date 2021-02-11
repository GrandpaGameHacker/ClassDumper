#pragma once
#include <fstream>
#include "Memory.h"
#include "stdheaders.h"
#include "RTTI.h"
const unsigned int TYPEDESCRIPTOR_SIGNITURE = 0x56413F2E;

using namespace std;
extern ofstream VTableLog;
extern ofstream InheritanceLog;
extern Console g_console;

namespace VTHelper
{
	bool IsValid(void* VTable_start, SectionInfo* sectionInfo);
	vector<uintptr_t> GetListOfFunctions(void* VTable_start, SectionInfo* sectionInfo);
	vector<uintptr_t> FindAll(SectionInfo* sectionInfo);
}

string DemangleMSVC(char* symbol);
void StringFilter(string& string, const std::string& substring);
void FilterSymbol(string& Symbol);


void InitializeLogs();
void LogModuleStart(char* moduleName);
void LogModuleEnd(char* moduleName);
void CloseLogs();
void DumpVTableInfo(uintptr_t VTable, SectionInfo* sectionInfo);
void DumpInheritanceInfo(uintptr_t VTable, SectionInfo* sectionInfo);
