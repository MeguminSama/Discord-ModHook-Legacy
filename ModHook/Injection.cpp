#include "Injection.h"

// a blank exported function - this will be ordinal #1
__declspec(dllexport) void __cdecl ExportedFunction(void)
{
}

static decltype(&CreateFileW) g_origCreateFileW;
static decltype(&GetFileAttributesW) g_origGetFileAttributesW;
static decltype(&CreateProcessW) g_origCreateProcessW;

bool asarHasBeenCopied = false;

std::wstring handle_path(std::wstring path)
{
	// TODO: Injection without copying app.asar to _app.asar
	// Currently I have to copy app.asar to _app.asar
	// This is because by default app.asar in %localappdata% is redirected to .\resources\vencord.asar
	// I need to find a way to differentiate between the ASAR calls to know when to
	//  pick whether to show the vencord files or the original app.asar files
	//
	// Please help I've spent so long on this 😭
	bool isCheckingType = path.starts_with(L"\\\\?\\");

	if (!asarHasBeenCopied && path.ends_with(L"resources\\_app.asar"))
	{
		auto pos = path.find(L"resources\\_app.asar");
		auto path_copy = std::wstring(path);
		path_copy.replace(pos, wcslen(L"resources\\_app.asar"), L"resources\\app.asar");
		std::filesystem::copy_file(path_copy, path, std::filesystem::copy_options::overwrite_existing);
		asarHasBeenCopied = true;
	}

	if (path.find(L"resources\\_app.asar") != std::wstring::npos)
	{
		//auto file = fopen(".\\handle_path.log", "a");
		auto pos = path.find(L"resources\\_app.asar");
		// fwprintf(file, std::format(L"{}: {} -> ", isCheckingType ? L"Type:" : L"Create:", path).data());
		path.replace(pos, wcslen(L"resources\\_app.asar"), L"resources\\app.asar");
		// fwprintf(file, std::format(L"{}\n", path).data());
		// fclose(file);
		return path;
	}

	auto pos = path.find(L"resources\\app.asar");
	if (pos != std::wstring::npos)
	{
		auto new_path = (std::wstring(L".\\") + std::wstring(path.substr(pos)));
		new_path.replace(2, wcslen(L"resources\\app.asar"), L"resources\\vencord.asar");
		// MessageBoxW(NULL, path.data(), L"CreateFileW", MB_OK);
		return new_path;
	}

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
	const std::wstring_view path{ lpFileName };
	auto new_path = handle_path(std::wstring(path));
	return g_origCreateFileW(new_path.data(), dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
}

DWORD WINAPI GetFileAttributesW_wrap(_In_ LPCWSTR lpFileName)
{
	std::wstring_view path{ lpFileName };
	auto new_path = handle_path(std::wstring(path));
	return g_origGetFileAttributesW(new_path.data());
};

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved)
{
	if (fdwReason == DLL_PROCESS_ATTACH)
	{
		if (MH_Initialize() != MH_OK)
		{
			return 1;
		}

		if (MH_CreateHook(&CreateFileW, CreateFileW_wrap, (void**)&g_origCreateFileW) != MH_OK)
		{
			return 1;
		}

		if (MH_EnableHook(&CreateFileW) != MH_OK)
		{
			return 1;
		}

		if (MH_CreateHook(&GetFileAttributesW, GetFileAttributesW_wrap, (void**)&g_origGetFileAttributesW) != MH_OK)
		{
			return 1;
		}

		if (MH_EnableHook(&GetFileAttributesW) != MH_OK)
		{
			return 1;
		}
	}

	return 1;
}