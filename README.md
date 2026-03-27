# uncomfy

Extract a workflow from an existing ComfyUI image or video and save it as a JSON workflow file that can be reopened in ComfyUI. Only likely to work with png and mp4 files saved via ComfyUI's native *Save Image* and *Save Video* nodes.

This standalone executable has *no* dependencies. Make it available system-wide by adding it to your PATH.

The original Powershell version is still available in /ps for anyone who prefers it (see its README).

## Usage

```bat
uncomfy.exe <file>
```

## Output

Saves a JSON file containing the extracted workflow

* If the COMFYUI_PATH environment variable is set, the file is saved to your ComfyUI workflows folder.
* If not set, the file is saved to the current directory.

## Building

### Windows (MSYS2 / MinGW)

```bat
gcc src/*.c -o uncomfy.exe -O2 -s -ffunction-sections -fdata-sections -fno-asynchronous-unwind-tables -fno-unwind-tables -fomit-frame-pointer -Wl,--gc-sections
```

### Linux

```bash
gcc src/*.c -o uncomfy -O2 -s -ffunction-sections -fdata-sections -fomit-frame-pointer -fno-asynchronous-unwind-tables -fno-unwind-tables -Wl,--gc-sections
```

This is optimised to produce a small, fast executable.

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.