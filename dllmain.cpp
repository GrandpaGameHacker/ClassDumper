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
    g_console.Write("[+] DLL Injected");
    auto moduleList = GetModuleList(hModule); // Skip our module, we dont need that one
    if (moduleList.empty()){
        g_console.WriteBold("GetModuleList Failed!\n");
        Sleep(5000);
        FreeLibraryAndExitThread(hModule, -1);
    }
    g_console.FWrite("[i] %s has %zu loaded modules\n", moduleList[0]->szModule, moduleList.size());
    InitializeLogs();
    for (auto t_module : moduleList) {
        g_console.FWrite("[i] scanning %s\n", t_module->szModule);
        LogModuleStart(t_module->szModule);

        SectionInfo* si = GetSectionInformation(t_module);
        auto vtable_list = VTHelper::FindAll(si);
        g_console.FWriteBold("[i] Found %d tables!\n", vtable_list.size());

        for (uintptr_t vtable : vtable_list) {
            DumpVTableInfo(vtable, si);
            DumpInheritanceInfo(vtable, si);
        }
        LogModuleEnd(t_module->szModule);
    }

    CloseLogs();
    FreeLibraryAndExitThread(hModule, -1);
}
