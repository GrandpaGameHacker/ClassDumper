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
        g_console.WriteBold("GetModuleList Failed!");
        Sleep(5000);
        FreeLibraryAndExitThread(hModule, -1);
    }
    auto mainProgram = moduleList[0];
    SectionInfo* si = GetSectionInformation(mainProgram);

    g_console.FWrite("[i] %s has %zu loaded modules\n", mainProgram->szModule, moduleList.size());
    g_console.FWrite("[i] %s TEXT@ 0x%p, RDATA@ 0x%p\n", mainProgram->szModule, si->TEXT.base, si->RDATA.base);
    g_console.FWrite("[i] %s TEXT Size: %d, RDATA Size: %d\n", mainProgram->szModule, si->TEXT.size, si->RDATA.size);


    auto vlist = VTHelper::FindVTables(si);
    g_console.FWriteBold("[i] Found %d tables!", vlist.size());

    Sleep(15000);
    FreeLibraryAndExitThread(hModule, -1);
}
