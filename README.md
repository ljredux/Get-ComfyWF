# Get-ComfyWF

Extract a workflow from an existing ComfyUI image or video and save it as a JSON workflow file that can be reopened in ComfyUI. Only likely to work with media saved via ComfyUI's native save image and save video nodes.

Very much a work in progress, written for my own benefit while tweaking prompts and juggling output files. Powershell for now. Python later.

## Usage

```powershell
.\Get-ComfyWF.ps1 "file.png"
```

## Output

Saves a JSON file containing the extracted workflow

When the COMFYUI_PATH environment variable is defined, the file is saved to your ComfyUI workflows folder. If it is not defined, the file is saved to the current folder.

## Requirements

* [ImageMagick](https://imagemagick.org) for processing images (`magick.exe` must be in your PATH).
* [FFmpeg](https://www.ffmpeg.org) for processing videos (`ffprobe.exe` must be in your PATH).

You can install them via winget (recommended, as it sets up PATH automatically):

```powershell
winget install ImageMagick.ImageMagick
winget install Gyan.FFmpeg
```

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.