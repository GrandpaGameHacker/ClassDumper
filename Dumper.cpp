#include "Dumper.h"
#include "Path.h"
bool VTHelper::IsValid(void* VTable_start, SectionInfo* sectionInfo)
{
	static uintptr_t type_info_vtable_cache = 0;
	uintptr_t* vftable_ptr = reinterpret_cast<uintptr_t*>(VTable_start);
	uintptr_t* meta_ptr = vftable_ptr - 1;
	if (sectionInfo->RDATA.base <= *meta_ptr && *meta_ptr <= sectionInfo->RDATA.end) {
		if (sectionInfo->TEXT.base <= *vftable_ptr && *vftable_ptr <= sectionInfo->TEXT.end) {
			CompleteObjectLocator* COL = reinterpret_cast<CompleteObjectLocator*>(*meta_ptr);
			if (COL->signature == 1 || COL->signature == 0) {
#ifdef _WIN64
				auto TypeDesc = COL->GetTypeDescriptor(sectionInfo->ModuleBase);
#else
				auto TypeDesc = COL->pTypeDescriptor;
#endif
				if (!TypeDesc) {
					return false;
				}
				if (!TypeDesc->pVFTable) {
					return false;
				}
				if (type_info_vtable_cache && TypeDesc->pVFTable == type_info_vtable_cache) {
					return true;
				}
				if (sectionInfo->RDATA.base <= TypeDesc->pVFTable && TypeDesc->pVFTable <= sectionInfo->RDATA.end) {
					auto test_function = reinterpret_cast<uintptr_t*>(TypeDesc->pVFTable);
					if (sectionInfo->TEXT.base <= *test_function && *test_function <= sectionInfo->TEXT.end){
						type_info_vtable_cache = TypeDesc->pVFTable;
						return true;
					}
				}

			}
		}
	}
	return false;
}

size_t VTHelper::GetNumberOfFunctions(void* VTable_start, SectionInfo* sectionInfo)
{
	size_t numberOfFunctions = 0;
	uintptr_t* vtable = reinterpret_cast<uintptr_t*>(VTable_start);
	uintptr_t functionptr = *vtable;
	while (sectionInfo->TEXT.base <= functionptr && functionptr <= sectionInfo->TEXT.end) {
		numberOfFunctions++;
		vtable++;
		functionptr = *vtable;
	}
	return numberOfFunctions;
}

std::vector<uintptr_t> VTHelper::GetListOfFunctions(void* VTable_start, SectionInfo* sectionInfo)
{
	std::vector<uintptr_t> functionList;
	size_t nFunctions = GetNumberOfFunctions(VTable_start, sectionInfo);
	uintptr_t* vftable_ptr = reinterpret_cast<uintptr_t*>(VTable_start);
	for (size_t i = 0; i < nFunctions; i++) {
		functionList.push_back(*vftable_ptr);
		vftable_ptr++;
	}
	return functionList;
}

std::vector<uintptr_t> VTHelper::FindAll(SectionInfo* sectionInfo)
{
	std::vector<uintptr_t> VTableList;
	uintptr_t ptr = sectionInfo->RDATA.base;
	while (ptr < sectionInfo->RDATA.end) {
		if (IsValid(reinterpret_cast<void*>(ptr), sectionInfo)) {
			VTableList.push_back(ptr);
		}
		ptr += sizeof(uintptr_t);
	}
	return VTableList;
}

std::string DemangleMSVC(char* symbol)
{
	std::string VFTableSymbolStart = "??_7";
	std::string VFTableSymbolEnd = "6B@";
	char buff[0x1000] = { 0 };
	memset(buff, 0, 0x1000);
	char* pSymbol = symbol;
	if (*(char*)(symbol + 4) == '?') pSymbol = symbol + 1;
	else if (*(char*)symbol == '.') pSymbol = symbol + 4;
	else if (*(char*)symbol == '?') pSymbol = symbol + 2;

	else
	{
		g_console.WriteBold("invalid msvc mangled name");
	}
	std::string ModifiedSymbol = std::string(pSymbol);
	ModifiedSymbol.insert(0, VFTableSymbolStart);
	ModifiedSymbol.insert(ModifiedSymbol.size(), VFTableSymbolEnd);
	if (!((UnDecorateSymbolName(ModifiedSymbol.c_str(), buff, 0x1000, 0)) != 0))
	{
		g_console.FWriteBold("Error Code: %d", GetLastError());
		return std::string(symbol); //Failsafe
	}
	return std::string(buff);
}

ofstream VTableLog;
ofstream InheritanceLog;

void InitializeLogs()
{
	wstring vtable_path = GetDesktopPath();
	vtable_path.append(L"\\vtable.txt");
	VTableLog.open(vtable_path);

	wstring inheritance_path = GetDesktopPath();
	inheritance_path.append(L"\\inheritance.txt");
	InheritanceLog.open(inheritance_path);
}

void LogModuleStart(char* moduleName)
{
	VTableLog << "<" << moduleName << '>' << endl;
	InheritanceLog << "<" << moduleName << '>' << endl;
}

void LogModuleEnd(char* moduleName)
{
	VTableLog << "< end " << moduleName << '>' << endl << endl;
	InheritanceLog << "< end " << moduleName << '>' << endl << endl;
}

void CloseLogs()
{
	VTableLog.close();
	InheritanceLog.close();
}

void DumpVTableInfo(uintptr_t VTable, SectionInfo* sectionInfo)
{
	uintptr_t* vftable_ptr = reinterpret_cast<uintptr_t*>(VTable);
	uintptr_t* meta_ptr = vftable_ptr - 1;
	CompleteObjectLocator* COL = reinterpret_cast<CompleteObjectLocator*>(*meta_ptr);
#ifdef _WIN64
	TypeDescriptor* pTypeDescriptor = COL->GetTypeDescriptor(sectionInfo->ModuleBase);
	ClassHierarchyDescriptor* pClassDescriptor = COL->GetClassDescriptor(sectionInfo->ModuleBase);
#else
	TypeDescriptor* pTypeDescriptor = COL->pTypeDescriptor;
	ClassHierarchyDescriptor* pClassDescriptor = COL->pClassDescriptor;
#endif
	string className = DemangleMSVC(&pTypeDescriptor->name);
	bool MultipleInheritance = pClassDescriptor->attributes & 0b01;
	bool VirtualInheritance = pClassDescriptor->attributes & 0b10;
	char MH = (MultipleInheritance) ? 'M' : ' ';
	char VH = (VirtualInheritance) ? 'V' : ' ';
	VTableLog << MH << VH << hex << VTable << "\t" << className << "\t" << endl;
}

void DumpInheritanceInfo(uintptr_t VTable, SectionInfo* sectionInfo)
{
//	uintptr_t* vftable_ptr = reinterpret_cast<uintptr_t*>(VTable);
//	uintptr_t* meta_ptr = vftable_ptr - 1;
//	CompleteObjectLocator* COL = reinterpret_cast<CompleteObjectLocator*>(*meta_ptr);
//#ifdef _WIN64
//	TypeDescriptor* pTypeDescriptor = COL->GetTypeDescriptor(sectionInfo->ModuleBase);
//	ClassHierarchyDescriptor* pClassDescriptor = COL->GetClassDescriptor(sectionInfo->ModuleBase);
//	BaseClassArray* pClassArray = pClassDescriptor->GetBaseClassArray(sectionInfo->ModuleBase);
//#else
//	TypeDescriptor* pTypeDescriptor = COL->pTypeDescriptor;
//	ClassHierarchyDescriptor* pClassDescriptor = COL->pClassDescriptor;
//	BaseClassArray* pClassArray = pClassDescriptor->pBaseClassArray;
//#endif
//	string className = DemangleMSVC(&pTypeDescriptor->name);
//	unsigned long numBaseClasses = pClassDescriptor->numBaseClasses;
//	for (unsigned long i = 0; i < numBaseClasses; i++) {
//		// Dump each base class for the current class
//	}
	// Complete dump for class
}