# Getting Started

> **Author:** Nathaniel Alger
> 
> **Last Updated:** 3/16/2026

This document outlines the process for developers to prepare and use the software properly. There are different processes based on the system at use. 

Start at [Downloading IDE and Cloning](#downloading-ide-and-cloning).

# Downloading IDE and Cloning

Visual Studio Code was primary used in development, so it is advised to use it in order to maintain consistency. There are some default project settings provided in the repository for VS Code. 

[Download Visual Studio Code here](https://code.visualstudio.com/).

Once you have installed the Code IDE, setup git on your device and clone the repository with 

```bash
git clone git@github.com:nathanalger/rv64i-simulator.git
```

Next, you can move on to [Preparing the Development Environment](#preparing-the-development-environment).

# Preparing the Development Environment

Different systems naturally require different methods of compilation. Below are instructions to prepare your system to compile based on the operating system used.

Operating Systems:

[Windows 10/11](#windows-systems)

## Windows Systems

Windows systems generally require a couple steps before C++ code can be compiled. Below are the general steps to take to get your system ready to compile if necessary.

### a. Downloading MSYS2

Download MSYS2 from [msys2.org](https://www.msys2.org/). Under the installation section of the home page is a version for both x86_64- and arm64- based systems.

### b. Downloading mingw-w64

Now, run the application **MSYS2 MSYS**. It will open a terminal, in which you will enter the command:

```bash
pacman -S mingw-w64-ucrt-x86_64-toolchain
```

It will ask you if you would like to install all components. The default is to install all components, so press enter. It will then ask you to confirm installation, which you can do by entering `y` and pressing enter. Wait for the components to install.

### c. Adding location to PATH

Right click the Windows button on the task bar (or press `WINDOWS KEY + X`) to open a context menu and click System. This will open settings. Click `Advanced System Settings`, then click `Environment Variables` at the bottom.

Under system variables, select `Path` then click `Edit`. Click new, then paste the path to the installed binary files (default is `C:\msys64\ucrt64\bin`). Press `OK` to save your changes.