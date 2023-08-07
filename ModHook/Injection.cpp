#include "Injection.h"

// a blank exported function - this will be ordinal #1
__declspec(dllexport) void __cdecl ExportedFunction(void)
{
}

static decltype(&CreateFileW) g_origCreateFileW;
static decltype(&GetFileAttributesW) g_origGetFileAttributesW;
static decltype(&CreateProcessW) g_origCreateProcessW;

bool asarHasBeenCopied = false;
bool isFirstCall = true;
int callCount = 0;

bool vencordIsLoaded = false;

std::wstring handle_path(std::wstring path)
{

	if (path.find(L"Vencord") != std::wstring::npos)
	{
		vencordIsLoaded = true;
	}

	if (path.ends_with(L"resources\\_app.asar"))
	{
		auto pos = path.find(L"resources\\_app.asar");
		path.replace(pos, wcslen(L"resources\\_app.asar"), L"resources\\app.asar");
		auto accessLog = fopen("./access.log", "a");
		fwprintf(accessLog, std::format(L"{}\n", path.data()).data());
		fclose(accessLog);
		return path;
	}

	if (vencordIsLoaded)
	{
		auto accessLog = fopen("./access.log", "a");
		fwprintf(accessLog, std::format(L"{}\n", path.data()).data());
		fclose(accessLog);
		return path;
	}

	if (path.ends_with(L"resources\\app.asar"))
	{

		auto new_path = std::wstring(L".\\resources\\vencord.asar");

		auto accessLog = fopen("./access.log", "a");
		fwprintf(accessLog, std::format(L"{}\n", new_path.data()).data());
		fclose(accessLog);
		// // MessageBoxW(NULL, path.data(), L"CreateFileW", MB_OK);

		return new_path;
	}

	auto accessLog = fopen("./access.log", "a");
	fwprintf(accessLog, std::format(L"{}\n", path.data()).data());
	fclose(accessLog);

	return path;
}

HANDLE WINAPI CreateFileW_wrap(
		_In_ LPCWSTR lpFileName,
		_In_ DWORD dwDesiredAccess,
		_In_ DWORD dwShareMode,
		_In_opt_ LPSECURITY_ATTRIBUTES lpSecurityAttributes,
		_In_ DWORD dwCreationDisposition,
		_In_ DWORD dwFlagsAndAttributes,
		_In_opt_ HANDLE hTemplateFile)
{
	const std::wstring_view path{lpFileName};
	auto new_path = handle_path(std::wstring(path));
	return g_origCreateFileW(new_path.data(), dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
}

DWORD WINAPI GetFileAttributesW_wrap(_In_ LPCWSTR lpFileName)
{
	std::wstring_view path{lpFileName};
	auto new_path = handle_path(std::wstring(path));
	return g_origGetFileAttributesW(new_path.data());
};

BOOL WINAPI CreateProcessW_wrap(
		_In_opt_ LPCWSTR lpApplicationName,
		_Inout_opt_ LPWSTR lpCommandLine,
		_In_opt_ LPSECURITY_ATTRIBUTES lpProcessAttributes,
		_In_opt_ LPSECURITY_ATTRIBUTES lpThreadAttributes,
		_In_ BOOL bInheritHandles,
		_In_ DWORD dwCreationFlags,
		_In_opt_ LPVOID lpEnvironment,
		_In_opt_ LPCWSTR lpCurrentDirectory,
		_In_ LPSTARTUPINFOW lpStartupInfo,
		_Out_ LPPROCESS_INFORMATION lpProcessInformation)
{
	if (std::wstring(lpCommandLine).find(L"--type=renderer") == std::wstring::npos)
	{
		return g_origCreateProcessW(lpApplicationName, lpCommandLine, lpProcessAttributes, lpThreadAttributes, bInheritHandles, dwCreationFlags, lpEnvironment, lpCurrentDirectory, lpStartupInfo, lpProcessInformation);
	}
	// MessageBoxW(NULL, lpCommandLine, L"CreateProcessW", MB_OK);
	// return g_origCreateProcessW(lpApplicationName, lpCommandLine, lpProcessAttributes, lpThreadAttributes, bInheritHandles, dwCreationFlags, lpEnvironment, lpCurrentDirectory, lpStartupInfo, lpProcessInformation);
	char pInjectDllPath[512] = ".\\ModHookInjection.dll";

	char szInjectDllFullPath[512];

	// get full path from dll filename
	memset(szInjectDllFullPath, 0, sizeof(szInjectDllFullPath));
	if (GetFullPathNameA(pInjectDllPath, sizeof(szInjectDllFullPath) - 1, szInjectDllFullPath, NULL) == 0)
	{
		MessageBoxW(NULL, L"Invalid DLL Path", L"CreateProcessW", MB_OK);
		printf("Invalid DLL path\n");

		return 0;
	}

	// get NtQueryInformationProcess function ptr
	NtQueryInformationProcess = (unsigned long(__stdcall *)(void *, unsigned long, void *, unsigned long, unsigned long *))GetProcAddress(GetModuleHandleA("ntdll.dll"), "NtQueryInformationProcess");
	if (NtQueryInformationProcess == NULL)
	{
		MessageBoxW(NULL, L"Failed to find NtQueryInformationProcess function", L"CreateProcessW", MB_OK);
		printf("Failed to find NtQueryInformationProcess function\n");

		return 0;
	}

	// auto isMojom = std::wstring(lpApplicationName).find(L"mojom.NetworkService") != std::wstring::npos;
	// DWORD flags = isMojom ? dwCreationFlags & CREATE_SUSPENDED : dwCreationFlags;

	auto success = g_origCreateProcessW(lpApplicationName, lpCommandLine, lpProcessAttributes, lpThreadAttributes, bInheritHandles, dwCreationFlags & CREATE_SUSPENDED, lpEnvironment, lpCurrentDirectory, lpStartupInfo, lpProcessInformation);
	if (!success)
	{
		MessageBoxW(NULL, std::format(L"Failed to create process\n{}\n{}", lpApplicationName, lpCommandLine).data(), L"CreateProcessW", MB_OK);
		return success;
	}
	// else if (isMojom)
	// {
	// 	return success;
	// }

	// inject DLL into target process
	if (InjectDll(lpProcessInformation->hProcess, lpProcessInformation->hThread, szInjectDllFullPath) != 0)
	{
		MessageBoxW(NULL, L"Failed to inject DLL", L"CreateProcessW", MB_OK);
		printf("Failed to inject DLL\n");

		// error
		TerminateProcess(lpProcessInformation->hProcess, 0);
		CloseHandle(lpProcessInformation->hThread);
		CloseHandle(lpProcessInformation->hProcess);

		return 0;
	}

	// MessageBoxW(NULL, L"Finished", L"CreateProcessW", MB_OK);
	printf("Finished\n");

	// close handles
	// CloseHandle(lpProcessInformation->hThread);
	// CloseHandle(lpProcessInformation->hProcess);

	return success;
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved)
{
	if (fdwReason == DLL_PROCESS_ATTACH)
	{
		if (MH_Initialize() != MH_OK)
		{
			return 1;
		}

		if (MH_CreateHook(&CreateFileW, CreateFileW_wrap, (void **)&g_origCreateFileW) != MH_OK)
		{
			return 1;
		}

		if (MH_EnableHook(&CreateFileW) != MH_OK)
		{
			return 1;
		}

		if (MH_CreateHook(&GetFileAttributesW, GetFileAttributesW_wrap, (void **)&g_origGetFileAttributesW) != MH_OK)
		{
			return 1;
		}

		if (MH_EnableHook(&GetFileAttributesW) != MH_OK)
		{
			return 1;
		}

		if (MH_CreateHook(&CreateProcessW, CreateProcessW_wrap, (void **)&g_origCreateProcessW) != MH_OK)
		{
			return 1;
		}

		if (MH_EnableHook(&CreateProcessW) != MH_OK)
		{
			return 1;
		}
	}

	return 1;
}