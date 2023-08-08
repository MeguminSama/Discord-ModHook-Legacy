#include "ModHookInjection.h"

// a blank exported function - this will be ordinal #1
__declspec(dllexport) void CALLBACK DetourFinishHelperProcess()
{
	// Do Nothing
}

static decltype(&CreateFileW) o_CreateFileW = CreateFileW;
static decltype(&GetFileAttributesW) o_GetFileAttributesW = GetFileAttributesW;
static decltype(&CreateProcessW) o_CreateProcessW = CreateProcessW;

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
		return path;
	}

	if (vencordIsLoaded)
		return path;

	if (path.ends_with(L"resources\\app.asar"))
	{
		return std::wstring(L".\\resources\\vencord.asar");
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
	MessageBoxW(NULL, lpCommandLine, L"ModHookInjection", 0);
	if (std::wstring(lpCommandLine).find(L"--type=renderer") == std::wstring::npos)
	{
		return o_CreateProcessW(lpApplicationName, lpCommandLine, lpProcessAttributes, lpThreadAttributes, bInheritHandles, dwCreationFlags, lpEnvironment, lpCurrentDirectory, lpStartupInfo, lpProcessInformation);
	}

	std::filesystem::path pInjectDllPath = std::wstring(L"ModHookInjection.dll");
	auto szInjectDllFullPath = std::filesystem::absolute(pInjectDllPath).string();

	BOOL success = DetourCreateProcessWithDllExW(lpApplicationName, lpCommandLine, lpProcessAttributes, lpThreadAttributes, bInheritHandles, dwCreationFlags, lpEnvironment, lpCurrentDirectory, lpStartupInfo, lpProcessInformation, szInjectDllFullPath.data(), o_CreateProcessW);

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
	{
		return 1;
	}

	if (fdwReason == DLL_PROCESS_ATTACH)
	{
		MessageBoxW(NULL, L"ModHookInjection.dll loading", std::format(L"{}", DetourRestoreAfterWith()).data(), 0);

		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());

		DetourAttach(&(PVOID &)o_CreateFileW, CreateFileW_wrap);
		DetourAttach(&(PVOID &)o_GetFileAttributesW, GetFileAttributesW_wrap);
		DetourAttach(&(PVOID &)o_CreateProcessW, CreateProcessW_wrap);

		MessageBoxW(NULL, L"ModHookInjection.dll loaded", std::format(L"{}", GetCurrentProcessId()).data(), 0);

		DetourTransactionCommit();
	}

	return 1;
}
