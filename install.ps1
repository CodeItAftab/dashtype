$ErrorActionPreference = "Stop"

$repo = "CodeItAftab/dashtype"
$installDir = Join-Path $env:LOCALAPPDATA "dashtype"
$zipPath = Join-Path $env:TEMP "dashtype-windows.zip"

Write-Host ""
Write-Host "Installing Dashtype..."
Write-Host ""

# --------------------------------------------------
# Get latest release
# --------------------------------------------------

Write-Host "Fetching latest dashtype release..."

try {
    $release = Invoke-RestMethod `
        -Uri "https://api.github.com/repos/$repo/releases/latest" `
        -UseBasicParsing
}
catch {
    Write-Error "Failed to fetch the latest Dashtype release."
    exit 1
}

$asset = $release.assets | Where-Object {
    $_.name -eq "dashtype-windows.zip"
} | Select-Object -First 1

if (-not $asset) {
    Write-Error "Could not find dashtype-windows.zip in the latest release."
    exit 1
}

# --------------------------------------------------
# Download
# --------------------------------------------------

Write-Host "Downloading $($asset.browser_download_url)..."

try {
    Invoke-WebRequest `
        -Uri $asset.browser_download_url `
        -OutFile $zipPath `
        -UseBasicParsing
}
catch {
    Write-Error "Failed to download Dashtype."
    exit 1
}

# --------------------------------------------------
# Prepare installation directory
# --------------------------------------------------

Write-Host "Preparing installation directory..."

if (Test-Path $installDir) {
    Remove-Item `
        -Path $installDir `
        -Recurse `
        -Force
}

New-Item `
    -ItemType Directory `
    -Path $installDir `
    -Force | Out-Null

# --------------------------------------------------
# Extract
# --------------------------------------------------

Write-Host "Extracting..."

try {
    Expand-Archive `
        -Path $zipPath `
        -DestinationPath $installDir `
        -Force
}
catch {
    Remove-Item $zipPath -Force -ErrorAction SilentlyContinue
    Write-Error "Failed to extract Dashtype."
    exit 1
}

Remove-Item $zipPath -Force -ErrorAction SilentlyContinue

# --------------------------------------------------
# Find executable
# --------------------------------------------------

Write-Host "Locating Dashtype executable..."

$exe = Get-ChildItem `
    -Path $installDir `
    -Recurse `
    -Filter "dashtype.exe" `
    -File |
    Select-Object -First 1

if (-not $exe) {
    Write-Error "Installation failed: dashtype.exe was not found."
    exit 1
}

$exePath = $exe.FullName
$exeDir = $exe.DirectoryName

Write-Host "Found:"
Write-Host "  $exePath"

# --------------------------------------------------
# Add executable directory to User PATH
# --------------------------------------------------

Write-Host "Configuring PATH..."

$userPath = [Environment]::GetEnvironmentVariable(
    "Path",
    "User"
)

if ($null -eq $userPath) {
    $userPath = ""
}

# Split PATH into individual entries
$pathEntries = @(
    $userPath -split ';' |
    ForEach-Object { $_.Trim() } |
    Where-Object { $_ -ne "" }
)

# Check for the executable directory
$pathExists = $false

foreach ($entry in $pathEntries) {
    try {
        if (
            [System.IO.Path]::GetFullPath($entry).TrimEnd('\') -ieq
            [System.IO.Path]::GetFullPath($exeDir).TrimEnd('\')
        ) {
            $pathExists = $true
            break
        }
    }
    catch {
        # Ignore malformed PATH entries
    }
}

if (-not $pathExists) {

    $pathEntries += $exeDir

    $newUserPath = $pathEntries -join ';'

    [Environment]::SetEnvironmentVariable(
        "Path",
        $newUserPath,
        "User"
    )

    Write-Host "Added Dashtype to your User PATH."
}
else {
    Write-Host "Dashtype is already in your User PATH."
}

# --------------------------------------------------
# Update current PowerShell process PATH
# --------------------------------------------------

$currentPathEntries = @(
    $env:Path -split ';' |
    ForEach-Object { $_.Trim() } |
    Where-Object { $_ -ne "" }
)

$currentPathExists = $false

foreach ($entry in $currentPathEntries) {
    try {
        if (
            [System.IO.Path]::GetFullPath($entry).TrimEnd('\') -ieq
            [System.IO.Path]::GetFullPath($exeDir).TrimEnd('\')
        ) {
            $currentPathExists = $true
            break
        }
    }
    catch {
        # Ignore malformed PATH entries
    }
}

if (-not $currentPathExists) {
    $env:Path = ($currentPathEntries + $exeDir) -join ';'
}

# --------------------------------------------------
# Verify installation
# --------------------------------------------------

Write-Host ""
Write-Host "Verifying installation..."

if (-not (Test-Path $exePath)) {
    Write-Error "Installation failed: executable does not exist."
    exit 1
}

$command = Get-Command "dashtype" -ErrorAction SilentlyContinue

if ($command) {

    Write-Host ""
    Write-Host "Dashtype installed successfully!"
    Write-Host ""
    Write-Host "Executable:"
    Write-Host "  $exePath"
    Write-Host ""
    Write-Host "You can now run:"
    Write-Host "  dashtype"
}
else {

    Write-Host ""
    Write-Host "Dashtype was installed successfully."
    Write-Host ""
    Write-Host "Executable:"
    Write-Host "  $exePath"
    Write-Host ""
    Write-Host "Close this terminal and open a new PowerShell window."
    Write-Host "Then run:"
    Write-Host "  dashtype"
}