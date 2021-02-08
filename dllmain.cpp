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
    g_console.FWrite("[i] %s has %zu non-system modules\n", moduleList[0]->szModule, moduleList.size());
    InitializeLogs();
    for (auto target_module : moduleList) {
        g_console.FWrite("[i] scanning %s\n", target_module->szModule);
        LogModuleStart(target_module->szModule);

        SectionInfo* sectInfo = GetSectionInformation(target_module);
        if (!sectInfo) {
            g_console.WriteBold("Error with SectionInfo!\n");
            continue;
        }
        auto vtable_list = VTHelper::FindAll(sectInfo);
        g_console.FWrite("[i] Found %d tables!\n\n", vtable_list.size());

        for (uintptr_t vtable : vtable_list) {
            DumpVTableInfo(vtable, sectInfo);
            DumpInheritanceInfo(vtable, sectInfo);
        }
        LogModuleEnd(target_module->szModule);
    }
    g_console.WaitInput();
    CloseLogs();
    FreeLibraryAndExitThread(hModule, -1);
}
