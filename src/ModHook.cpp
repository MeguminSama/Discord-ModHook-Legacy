#include "ModHook.h"

int hookExecutable(std::string executable)
{
	auto pInjectDllPath = std::string("ModHookInjection.dll");
	auto szInjectDllFullPath = std::filesystem::absolute(pInjectDllPath).string();

	LPCSTR DllsToInject[1] = {szInjectDllFullPath.data()};

	PROCESS_INFORMATION ProcessInfo;
	memset((void *)&ProcessInfo, 0, sizeof(ProcessInfo));

	STARTUPINFOA StartupInfo;
	memset((void *)&StartupInfo, 0, sizeof(StartupInfo));
	StartupInfo.cb = sizeof(StartupInfo);

	if (DetourCreateProcessWithDllsA(NULL, executable.data(), NULL, NULL, 0, CREATE_SUSPENDED, NULL, NULL, &StartupInfo, &ProcessInfo, 1, DllsToInject, NULL) == 0)
	{
		printf("Failed to create process\n");
		return 1;
	}

	ResumeThread(ProcessInfo.hThread);

	CloseHandle(ProcessInfo.hProcess);
	CloseHandle(ProcessInfo.hThread);

	return 0;
}

int main()
{
	return hookExecutable("c:\\Users\\megu\\AppData\\Local\\DiscordCanary\\app-1.0.73\\DiscordCanary.exe");
}