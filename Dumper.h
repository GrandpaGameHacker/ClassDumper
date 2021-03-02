#pragma once
#include "stdheaders.h"
#include <fstream>
#include "Memory.h"
#include "RTTI.h"
#include "Path.h"
#include "StringConversions.h"

const unsigned int TYPEDESCRIPTOR_SIGNITURE = 0x56413F2E;
const BYTE RET_INSTR = 0xC3;
const BYTE RET_INT_INSTR = 0xC2;

using namespace std;
extern ofstream VTableLog;
extern ofstream InheritanceLog;
extern Console g_console;

bool IsValid(void* VTable_start, SectionInfo* sectionInfo);
vector<uintptr_t> GetListOfFunctions(void* VTable_start, SectionInfo* sectionInfo);
vector<uintptr_t> FindAllVTables(SectionInfo* sectionInfo);

string DemangleMicrosoft(char* symbol);
void StringFilter(string& string, const std::string& substring);
void FilterSymbol(string& symbol);

bool SymbolComparator(uintptr_t v1, uintptr_t v2);
void SortSymbols(vector<uintptr_t>& vtableList);


void InitializeLogs(const string& folderName);
void LogModuleStart(char* moduleName);
void LogModuleEnd(char* moduleName);
void CloseLogs();
void DumpVTableInfo(uintptr_t VTable, SectionInfo* sectionInfo);
void DumpInheritanceInfo(uintptr_t VTable);
