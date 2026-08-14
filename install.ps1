$ErrorActionPreference = "Stop"

$repo = "CodeItAftab/dashtype"
$installDir = "$env:LOCALAPPDATA\dashtype"

Write-Host "Fetching latest dashtype release..."

$release = Invoke-RestMethod -Uri "https://api.github.com/repos/$repo/releases/latest"

$asset = $release.assets | Where-Object {
    $_.name -eq "dashtype-windows.zip"
}

if (-not $asset) {
    Write-Error "Could not find dashtype-windows.zip in the latest release."
    exit 1
}

# Create installation directory
New-Item -ItemType Directory -Force -Path $installDir | Out-Null

$zipPath = "$installDir\dashtype.zip"

Write-Host "Downloading $($asset.browser_download_url)..."

Invoke-WebRequest `
    -Uri $asset.browser_download_url `
    -OutFile $zipPath

Write-Host "Extracting..."

Expand-Archive `
    -Path $zipPath `
    -DestinationPath $installDir `
    -Force

Remove-Item $zipPath -Force

# Find the actual executable
$exe = Get-ChildItem `
    -Path $installDir `
    -Recurse `
    -Filter "dashtype.exe" `
    -File |
    Select-Object -First 1

if (-not $exe) {
    Write-Error "Could not find dashtype.exe after extraction."
    exit 1
}

$exeDir = $exe.DirectoryName

Write-Host "Found executable:"
Write-Host $exe.FullName

# Add the directory containing dashtype.exe to PATH
$userPath = [Environment]::GetEnvironmentVariable("Path", "User")

$pathEntries = $userPath -split ';' | Where-Object { $_ -ne "" }

if ($pathEntries -notcontains $exeDir) {
    $newPath = ($pathEntries + $exeDir) -join ';'

    [Environment]::SetEnvironmentVariable(
        "Path",
        $newPath,
        "User"
    )

    Write-Host "Added $exeDir to your PATH."
}
else {
    Write-Host "$exeDir is already on your PATH."
}

Write-Host ""
Write-Host "Dashtype installed successfully!"
Write-Host "Restart your terminal and run: dashtype"