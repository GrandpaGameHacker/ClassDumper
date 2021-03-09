#pragma once
#include "stdheaders.h"
#include <fstream>

#include "VTables.h"
#include "RTTI.h"
#include "Symbols.h"
#include "Path.h"
#include "StringConversions.h"


using namespace std;

const BYTE RET_INSTR = 0xC3;
const BYTE RET_INT_INSTR = 0xC2;

extern ofstream VTableLog;
extern ofstream InheritanceLog;
extern Console g_console;

void InitializeLogs(const string& folderName);
void LogModuleStart(char* moduleName);
void LogModuleEnd(char* moduleName);
void CloseLogs();
void DumpVTableInfo(uintptr_t VTable, SectionInfo* sectionInfo);
void DumpInheritanceInfo(uintptr_t VTable);
