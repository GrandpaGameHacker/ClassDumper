#include "Dumper.h"

bool VTHelper::IsValid(void* VTable_start, SectionInfo* sectionInfo)
{
	uintptr_t* vftable_ptr = reinterpret_cast<uintptr_t*>(VTable_start);
	uintptr_t* meta_ptr = vftable_ptr - 1;
	if (sectionInfo->RDATA.base <= *meta_ptr && *meta_ptr <= sectionInfo->RDATA.end) {
		if (sectionInfo->TEXT.base <= *vftable_ptr && *vftable_ptr <= sectionInfo->TEXT.end) {
			CompleteObjectLocator* COL = reinterpret_cast<CompleteObjectLocator*>(*meta_ptr);
			if (COL->signature == 1 || COL->signature == 0) {
				auto TypeDesc = COL->GetTypeDescriptor(sectionInfo->ModuleBase);
				g_console.Write((char*)&TypeDesc->name);
				return true;
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

std::vector<uintptr_t> VTHelper::FindVTables(SectionInfo* sectionInfo)
{
	std::vector<uintptr_t> VTableList;
	uintptr_t ptr = sectionInfo->RDATA.base;
	while (ptr < sectionInfo->RDATA.end) {
		if (IsValid(reinterpret_cast<void*>(ptr), sectionInfo)) {
			VTableList.push_back(ptr);
			g_console.FWrite("%p\n", ptr);
		}
		ptr += sizeof(uintptr_t);
	}
	return VTableList;
}
