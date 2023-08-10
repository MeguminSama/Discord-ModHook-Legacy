#include "ModHookInjection.h"

static decltype(&CreateFileW) o_CreateFileW = CreateFileW;
static decltype(&GetFileAttributesW) o_GetFileAttributesW = GetFileAttributesW;
static decltype(&CreateProcessW) o_CreateProcessW = CreateProcessW;

static LPWSTR COMMAND_LINE_ARGS = GetCommandLineW();

std::wstring modHookCustomAsar = L"";
std::wstring modHookAsarHookToggleQuery = L"";
std::wstring modHookOriginalAsarName = L"";

bool modIsLoaded = false;

void parseCommandLineArgs(LPWSTR args)
{
	int argCount;
	LPWSTR *argList = CommandLineToArgvW(COMMAND_LINE_ARGS, &argCount);

	for (int i = 0; i < argCount; i++)
	{
		std::wstring arg = std::wstring(argList[i]);
		if (arg.find(L"--modhook-custom-asar=") != std::wstring::npos)
		{
			auto pos = arg.find(L"--modhook-custom-asar=");
			arg.replace(pos, wcslen(L"--modhook-custom-asar="), L"");
			modHookCustomAsar = arg;
		}
		else if (arg.find(L"--modhook-asar-hook-toggle-query=") != std::wstring::npos)
		{
			auto pos = arg.find(L"--modhook-asar-hook-toggle-query=");
			arg.replace(pos, wcslen(L"--modhook-asar-hook-toggle-query="), L"");
			modHookAsarHookToggleQuery = arg;
		}
		else if (arg.find(L"--modhook-original-asar-name=") != std::wstring::npos)
		{
			auto pos = arg.find(L"--modhook-original-asar-name=");
			arg.replace(pos, wcslen(L"--modhook-original-asar-name="), L"");
			modHookOriginalAsarName = arg;
		}
	}

	if (modHookCustomAsar.empty())
	{
		MessageBoxW(NULL, L"modHookCustomAsar is empty", L"modHookCustomAsar", MB_OK);
	}
	if (modHookAsarHookToggleQuery.empty())
	{
		MessageBoxW(NULL, L"modHookAsarHookToggleQuery is empty", L"modHookAsarHookToggleQuery", MB_OK);
	}
	if (modHookOriginalAsarName.empty())
	{
		MessageBoxW(NULL, L"modHookOriginalAsarName is empty", L"modHookOriginalAsarName", MB_OK);
	}
}

std::wstring handle_path(std::wstring path)
{
	if (path.find(modHookAsarHookToggleQuery) != std::wstring::npos)
	{
		modIsLoaded = true;
	}

	if (path.ends_with(modHookOriginalAsarName))
	{
		auto pos = path.length() - modHookOriginalAsarName.length();
		path.replace(pos, modHookOriginalAsarName.length(), L"app.asar");
		return path;
	}

	if (modIsLoaded)
		return path;

	if (path.ends_with(L"app.asar"))
	{
		return std::wstring(modHookCustomAsar.data());
	}

	return path;
}

static HANDLE WINAPI CreateFileW_wrap(
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
	return o_CreateFileW(new_path.data(), dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
}

static DWORD WINAPI GetFileAttributesW_wrap(_In_ LPCWSTR lpFileName)
{
	std::wstring_view path{lpFileName};
	auto new_path = handle_path(std::wstring(path));
	return o_GetFileAttributesW(new_path.data());
}

static BOOL WINAPI CreateProcessW_wrap(
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
		return o_CreateProcessW(lpApplicationName, lpCommandLine, lpProcessAttributes, lpThreadAttributes, bInheritHandles, dwCreationFlags, lpEnvironment, lpCurrentDirectory, lpStartupInfo, lpProcessInformation);
	}

	std::filesystem::path pInjectDllPath = std::wstring(L"ModHookInjection.dll");
	auto szInjectDllFullPath = std::filesystem::absolute(pInjectDllPath).string();

	auto newCommandLine = std::format(L"{} --modhook-custom-asar={} --modhook-asar-hook-toggle-query={} --modhook-original-asar-name={}",
																		lpCommandLine,
																		modHookCustomAsar,
																		modHookAsarHookToggleQuery,
																		modHookOriginalAsarName);

	BOOL success = DetourCreateProcessWithDllExW(lpApplicationName, newCommandLine.data(), lpProcessAttributes, lpThreadAttributes, bInheritHandles, dwCreationFlags, lpEnvironment, lpCurrentDirectory, lpStartupInfo, lpProcessInformation, szInjectDllFullPath.data(), o_CreateProcessW);

	if (!success)
	{
		printf("Failed to create process\n");
		return success;
	}

	ResumeThread(lpProcessInformation->hThread);

	return success;
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved)
{
	if (DetourIsHelperProcess())
		return 1;

	if (fdwReason == DLL_PROCESS_ATTACH)
	{
		DetourRestoreAfterWith();

		parseCommandLineArgs(COMMAND_LINE_ARGS);

		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());

		DetourAttach(&(PVOID &)o_CreateFileW, CreateFileW_wrap);
		DetourAttach(&(PVOID &)o_GetFileAttributesW, GetFileAttributesW_wrap);
		DetourAttach(&(PVOID &)o_CreateProcessW, CreateProcessW_wrap);

		DetourTransactionCommit();
	}

	return 1;
}
