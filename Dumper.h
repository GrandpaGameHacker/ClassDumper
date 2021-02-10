#pragma once
#include <fstream>
#include "Memory.h"
#include "stdheaders.h"
#include "RTTI.h"



using namespace std;
extern ofstream VTableLog;
extern ofstream InheritanceLog;
extern Console g_console; // for debugging
namespace VTHelper
{
	bool IsValid(void* VTable_start, SectionInfo* sectionInfo);
	vector<uintptr_t> GetListOfFunctions(void* VTable_start, SectionInfo* sectionInfo);
	vector<uintptr_t> FindAll(SectionInfo* sectionInfo);
}

string DemangleMSVC(char* symbol);
void StrFilter(std::string& string, const std::string& substring);
void ApplySymbolFilters(std::string& Symbol);


void InitializeLogs();
void LogModuleStart(char* moduleName);
void LogModuleEnd(char* moduleName);
void CloseLogs();
void DumpVTableInfo(uintptr_t VTable, SectionInfo* sectionInfo);
void DumpInheritanceInfo(uintptr_t VTable, SectionInfo* sectionInfo);
