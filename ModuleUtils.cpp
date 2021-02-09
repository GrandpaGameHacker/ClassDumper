#include "ModuleUtils.h"

bool IsSystemModule(MODULEENTRY32* Module)
{
	std::string modstr = Module->szExePath;
	if (modstr.find("\\Windows\\") != std::string::npos) {
		return true;
	}
	return false;
}

void GetModuleInfo(char* ModuleName, MODULEINFO* ModuleInfo) {
	HMODULE Module{ 0 };
	Module = GetModuleHandle(ModuleName);
	if(Module != NULL)
		GetModuleInformation(GetCurrentProcess(), Module, ModuleInfo, sizeof(MODULEINFO));
}

std::vector<MODULEENTRY32*> GetModuleList(HMODULE skipModule)
{
	auto moduleList = std::vector<MODULEENTRY32*>();
	HANDLE hModuleSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, GetCurrentProcessId());
	if (hModuleSnap == INVALID_HANDLE_VALUE)
	{
		return moduleList;
	}
	MODULEENTRY32* ModuleEntry = new MODULEENTRY32;
	ModuleEntry->dwSize = sizeof(MODULEENTRY32);
	if (!Module32First(hModuleSnap, ModuleEntry)) {
		delete ModuleEntry;
		return moduleList;
	}
	for (;;) {
		if (ModuleEntry->hModule != skipModule && !IsSystemModule(ModuleEntry)) {
			moduleList.push_back(ModuleEntry);
		}
		else {
			delete ModuleEntry;
		}
		
		ModuleEntry = new MODULEENTRY32;
		ModuleEntry->dwSize = sizeof(MODULEENTRY32);
		if (!Module32Next(hModuleSnap, ModuleEntry)) {
			delete ModuleEntry;
			return moduleList;
		}
	}
}

SectionInfo* GetSectionInformation(MODULEENTRY32* Module)
{
	SectionInfo* sectionInfo = new SectionInfo;
	memset((void*)sectionInfo, 0, sizeof(SectionInfo));
	BYTE* MBase = Module->modBaseAddr;
	sectionInfo->ModuleBase = reinterpret_cast<uintptr_t>(MBase);
	IMAGE_DOS_HEADER* DosHeader = reinterpret_cast<IMAGE_DOS_HEADER*>(MBase);
	if (!DosHeader || DosHeader->e_magic != IMAGE_DOS_SIGNATURE) {
		return nullptr;
	}
	IMAGE_NT_HEADERS* NTHeader = reinterpret_cast<IMAGE_NT_HEADERS*>(MBase + DosHeader->e_lfanew);
	if (!NTHeader || NTHeader->Signature != IMAGE_NT_SIGNATURE){
		return nullptr;
	}
	WORD NumberOfSections = NTHeader->FileHeader.NumberOfSections;
	IMAGE_SECTION_HEADER* Section = IMAGE_FIRST_SECTION(NTHeader);
	bool TEXT_found = false;
	bool RDATA_found = false;
	for (WORD i = 0; i < NumberOfSections; i++) {
		if (strcmp((const char*)Section[i].Name, ".text") == 0)
		{
			sectionInfo->TEXT.base = Section[i].VirtualAddress + (uintptr_t)MBase;
			sectionInfo->TEXT.size = Section[i].SizeOfRawData;
			sectionInfo->TEXT.end = sectionInfo->TEXT.base + sectionInfo->TEXT.size - 1;
			TEXT_found = true;
			continue;
		}
		if (strcmp((const char*)Section[i].Name, ".rdata") == 0)
		{
			sectionInfo->RDATA.base = Section[i].VirtualAddress + (uintptr_t)MBase;
			sectionInfo->RDATA.size = Section[i].SizeOfRawData;
			sectionInfo->RDATA.end = sectionInfo->RDATA.base + sectionInfo->RDATA.size - 1;
			RDATA_found = true;
		}
		if (TEXT_found && RDATA_found)
			break;
	}
	if (sectionInfo->TEXT.base && sectionInfo->RDATA.base) {
		return sectionInfo;
	}
	delete sectionInfo;
	return nullptr;
}



