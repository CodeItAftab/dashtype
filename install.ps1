$ErrorActionPreference = "Stop"

$repo = "CodeItAftab/dashtype"
$installDir = "$env:LOCALAPPDATA\dashtype"

Write-Host "Fetching latest dashtype release..."
$release = Invoke-RestMethod -Uri "https://api.github.com/repos/$repo/releases/latest"
$asset = $release.assets | Where-Object { $_.name -eq "dashtype-windows.zip" }

if (-not $asset) {
    Write-Error "Could not find dashtype-windows.zip in the latest release."
    exit 1
}

New-Item -ItemType Directory -Force -Path $installDir | Out-Null
$zipPath = "$installDir\dashtype.zip"

Write-Host "Downloading $($asset.browser_download_url)..."
Invoke-WebRequest -Uri $asset.browser_download_url -OutFile $zipPath

Write-Host "Extracting..."
Expand-Archive -Path $zipPath -DestinationPath $installDir -Force
Remove-Item $zipPath

$userPath = [Environment]::GetEnvironmentVariable("Path", "User")
if ($userPath -notlike "*$installDir*") {
    [Environment]::SetEnvironmentVariable("Path", "$userPath;$installDir", "User")
    Write-Host "Added $installDir to your PATH. Restart your terminal to use 'dashtype' directly."
} else {
    Write-Host "$installDir is already on your PATH."
}

Write-Host "Installed! Run 'dashtype' in a new terminal to get started."
