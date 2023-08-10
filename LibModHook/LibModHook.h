#include <stdio.h>
#include <string>
#include <filesystem>
#include "ModHook.h"

#define DllExport __declspec(dllexport)

extern "C"
{
	typedef struct HookDiscordOpts
	{
		char *pathToDiscordExecutable;
		char *originalAsarName;
		char *pathToCustomAsar;
		char *asarHookToggleQuery;
		char *modhookDllName;
	} HookDiscordOpts;

	DllExport int hookDiscord(HookDiscordOpts *options);
}
