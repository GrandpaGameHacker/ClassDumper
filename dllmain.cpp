#include "dllmain.h"


Console g_console = Console(CONSOLE_TITLE);

BOOL APIENTRY DllMain( HMODULE hModule,
					   DWORD  ul_reason_for_call,
					   LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		CreateThread(nullptr, NULL, (LPTHREAD_START_ROUTINE)&DllThread, hModule, NULL, nullptr);
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

void DllThread(HMODULE hModule)
{
	g_console.Write(HeadingArt);
	auto start = chrono::high_resolution_clock::now();
	auto moduleList = GetModuleList(hModule); // Skip our module, we dont need that one
	if (moduleList.empty()){
		g_console.WriteBold("GetModuleList Failed!\n");
		Sleep(5000);
		FreeLibraryAndExitThread(hModule, -1);
	}
	g_console.FWrite("[i] %s has %zu non-system modules\n", moduleList[0]->szModule, moduleList.size());

	string moduleName = moduleList[0]->szModule;
	moduleName = moduleName.substr(0, moduleName.length() - 4); // remove the .exe from the name
	InitializeLogs(moduleName); // Create the log directories and file streams
	for (auto target_module : moduleList) {
		g_console.FWrite("[i] scanning %s\n", target_module->szModule);

		// Read PE file and get section headers
		SectionInfo* sectInfo = GetSectionInformation(target_module);
		if (!sectInfo) {
			g_console.WriteBold("Error: NULL Section Information!\n");
			continue;
		}

		LogModuleStart(target_module->szModule);

		auto vtable_list = FindAllVTables(sectInfo); // Scan for vtables
		g_console.FWrite("[i] Found %d virtual function tables!\n\n", vtable_list.size());

		// Sort vtables alphabetically by class name then address
		SortSymbols(vtable_list);

		//Dump all the metadata for this module
		for (uintptr_t vtable : vtable_list) {
			DumpVTableInfo(vtable, sectInfo);
			DumpInheritanceInfo(vtable);
		}

		LogModuleEnd(target_module->szModule);
	}
	auto end = chrono::high_resolution_clock::now();
	g_console.FWrite("[+] Took %d milliseconds\n", chrono::duration_cast<chrono::milliseconds>(end - start).count());
	g_console.Write("[i] Output will be in Desktop\\Class_Dumper\\...");
	g_console.WaitInput();
	CloseLogs();
	FreeLibraryAndExitThread(hModule, -1);
}
