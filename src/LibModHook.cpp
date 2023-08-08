#include "LibModHook.h"

#define DllExport __declspec(dllexport)

DllExport std::string GetModName()
{
	return "MyMod";
}
