# Discord ModHook

## In-Memory Mod Injection

### Inject mods into Discord without modifying any of Discord's files!

## Configuration

- Edit `ModHook/resources/vencord.asar/index.js` to point to your vencord `dist/patcher.js` file.
- Edit `EXE_PATH` in `ModHook.h` to point to your `Discord.exe` (note: NOT Update.exe).

## Caveats

Windows only... for now (until I get the windows version in a state I'm happy with)

Currently the script copies `app.asar` to `_app.asar` in order to function, even though they're the same file.
This is because I haven't found an easy way to differentiate between win32 calls to the "modified" asar file (whenever it needs to call `resources/vencord.asasr`)
and the "original" asar file (whenever it needs to call `app.asar` in %localappdata%)

Some help for this would be much appreciated!!!

## Building

```shell
cmake --preset x86-release
cmake --build out/build/x86-release
```

The files should now be in `out/build/x86-release/`.

## Running

Just run `ModHook.exe` from inside `out/build/x86-release` and it should launch Discord with Vencord loaded.

## Why not 64-bit?

Discord uses 32bit for their Windows build, so a x64 build wouldn't be able to inject into the Discord executable.
