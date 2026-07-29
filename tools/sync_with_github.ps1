# Developed by Amine Saoud ibn al-Bashir.
param(
    [switch]$SkipPull
)

$ErrorActionPreference = "Stop"
$repoRoot = Resolve-Path (Join-Path $PSScriptRoot "..")
Set-Location $repoRoot

if (-not $SkipPull) {
    git pull --ff-only
}

python tools/update_repository_index.py
git status --short --branch
