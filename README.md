# Discord ModHook

## In-Memory Mod Injection

### Inject mods into Discord without modifying any of Discord's files!

## Building

This uses MSBuild. No commandline instructions available yet.

You'll need Visual Studio (only tested on vs2022).

Make sure you're targeting a Release build on x64.

`ModHookInjection.dll` is 32bit, even though it appears in the bin/x64 folder.
This is because Discord is a 32bit program, so the DLL needs to be 32bit.
However the injector `LibModHook.dll` can (and should) be x64.

## Caveats

Windows only... unless someone wants to PR linux support.

## Running

A pre-made GUI for this is available at https://github.com/MeguminSama/Discord-ModLoader

To use the library, you need `LibModHook.dll` and `ModHookInjection.dll`.

You can see an example using ModHook in [my JavaScript library for ModHook](https://github.com/MeguminSama/Discord-Modhook-JS)

`ModHook.exe` is a proof-of-concept loader. You will need to change the source to use it.

## Why not 64-bit?

Discord uses 32bit for their Windows build, so a x64 build wouldn't be able to inject into the Discord executable.
