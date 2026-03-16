# Getting Started

This document outlines the process for developers to prepare and use the software properly. There are different processes based on the system at use. 

For developers on Windows systems, read [Preparing the Development Environment](#preparing-the-development-environment) to get started.

> This document was last updated on 3/16/2026.

# Preparing the Development Environment

## 1. Windows Compiler

### 1a. Downloading MSYS2

Download MSYS2 from [msys2.org](https://www.msys2.org/). Under the installation section of the home page is a version for both x86_64- and arm64- based systems.

### 1b. Downloading mingw-w64

Now, run the application **MSYS2 MSYS**. It will open a terminal, in which you will enter the command:

```bash
pacman -S mingw-w64-ucrt-x86_64-toolchain
```

It will ask you if you would like to install all components. The default is to install all components, so press enter. It will then ask you to confirm installation, which you can do by entering `y` and pressing enter. Wait for the components to install.

### 1c. Adding location to PATH

Right click the Windows button on the task bar (or press `WINDOWS KEY + X`) to open a context menu and click System. This will open settings. Click `Advanced System Settings`, then click `Environment Variables` at the bottom.

Under system variables, select `Path` then click `Edit`. Click new, then paste the path to the installed binary files (default is `C:\msys64\ucrt64\bin`). Press `OK` to save your changes.