param(
    [Parameter(Mandatory = $true)]
    [ValidateScript({ Test-Path $_ -PathType Leaf })]
    [string]$InputFile
)

$imageExts = @('.png')
$videoExts = @('.mp4')

$inputBaseName = [System.IO.Path]::GetFileNameWithoutExtension($InputFile)
$inputExtension = [System.IO.Path]::GetExtension($InputFile)

# Ensure required tool is in path
if ($imageExts -contains $inputExtension) {
    $tool = 'magick'
    $type = 'image'
}
elseif ($videoExts -contains $inputExtension) {
    $tool = 'ffprobe'
    $type = 'video'
}
else {
    Write-Error "Unsupported file type: $inputExtension. Supported image types: $($imageExts -join ', '). Supported video types: $($videoExts -join ', ')."
    exit 1
}

if (-not (Get-Command $tool -ErrorAction SilentlyContinue)) {
    Write-Error "Required tool '$tool' not found. Please install it and ensure it is in your PATH."
    exit 1
}

# Get output folder. If COMFYUI_PATH env var exists, then output to user's workflows folder.  Otherwise, the current folder.
$outputFolder = if ($env:COMFYUI_PATH) {
    $testPath = Join-Path $env:COMFYUI_PATH "user\default\workflows"
    if (Test-Path $testPath) { $testPath } else { Get-Location }
}
else {
    Get-Location
}

# Get fq output filename.
do {
    $timestamp = Get-Date -Format "yyyyMMdd-HHmmss"
    $outputFile = Join-Path $outputFolder "$inputBaseName-$($inputExtension.TrimStart('.'))-$timestamp.json"
} while (Test-Path $outputFile)

# extract the workflow
if ($type -eq 'image') {
    $workflow = & $tool $InputFile -format "%[workflow]" info:
    if ($LASTEXITCODE -ne 0 -or -not $workflow) {
        Write-Error "No workflow found in the image metadata."
        exit 1
    }
}
else {
    # it's a video
    $ffprobeJson = & $tool -v quiet -print_format json -show_format $InputFile | ConvertFrom-Json
    if (-not $ffprobeJson -or -not $ffprobeJson.format -or -not $ffprobeJson.format.tags) {
        Write-Error "Failed to read metadata from video with ffprobe."
        exit 1
    }
    $workflow = $ffprobeJson.format.tags.workflow
    if (-not $workflow) {
        Write-Error "No workflow found in the video metadata."
        exit 1
    }
}

try {
    $workflow | Set-Content -Path $outputFile -Encoding UTF8 -NoNewline -ErrorAction Stop
    Write-Host "Workflow saved to: $outputFile" 
}
catch {
    Write-Error "Failed to write workflow file: $($_.Exception.Message)"
    exit 1
}