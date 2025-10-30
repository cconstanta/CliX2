# CliX Project

Qt application for management system.

## Automated Builds

This project uses GitHub Actions to automatically build:
- **Windows**: EXE file with all dependencies
- **Linux**: Portable binary with all libraries

## How to get builds

1. Go to **Actions** tab
2. Select the latest workflow run
3. Download artifacts from the **Artifacts** section

## Windows Version
- File: `CliX.exe`
- Includes all required DLLs
- Ready to run on any Windows machine

## Linux Version  
- Folder: `CliX-linux-portable`
- Run: `./run-clix.sh`
- Contains all required libraries
