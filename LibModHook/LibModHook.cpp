// LibModHook.cpp : Defines the exported functions for the DLL.
//

#include "framework.h"
#include "LibModHook.h"

int hookDiscord(HookDiscordOpts *options)
{
    auto szInjectDllFullPath = std::filesystem::absolute(options->modhookDllName).string();

    auto args = std::format(R"("{}" --modhook-custom-asar="{}" --modhook-asar-hook-toggle-query="{}" --modhook-original-asar-name="{}")",
                            options->pathToDiscordExecutable,
                            options->pathToCustomAsar,
                            options->asarHookToggleQuery,
                            options->originalAsarName);

    printf("Starting Discord with args: %s\n", args.data());

    PROCESS_INFORMATION ProcessInfo;
    memset((void *)&ProcessInfo, 0, sizeof(ProcessInfo));

    STARTUPINFOA StartupInfo;
    memset((void *)&StartupInfo, 0, sizeof(StartupInfo));
    StartupInfo.cb = sizeof(StartupInfo);

    if (DetourCreateProcessWithDllExA(NULL, args.data(), NULL, NULL, 0, CREATE_SUSPENDED, NULL, NULL, &StartupInfo, &ProcessInfo, szInjectDllFullPath.data(), NULL) == 0)
    {
        printf("Failed to create process\n");
        return 1;
    }

    ResumeThread(ProcessInfo.hThread);

    CloseHandle(ProcessInfo.hProcess);
    CloseHandle(ProcessInfo.hThread);

    return 0;
}
