# N64 Tech Demo

A Nintendo 64 homebrew project built with [Libdragon](https://libdragon.dev/).

## Libdragon Version

This project uses the **preview branch** of libdragon, which includes cutting-edge features:

- **OpenGL 1.1** - Hardware-accelerated 3D graphics with RSP T&L pipeline
- **MPEG1 Video** - Video playback up to 2Mbps at 320x240
- **TrueType Fonts** - Advanced text rendering with kerning and word-wrapping
- **Multithreading** - Threads, mutexes, condition variables, and atomics
- **Audio Enhancements** - Improved compression and concurrent playback
- **Controller Pak** - Virtual filesystem support

**Note:** Preview branch APIs may change. For stable, backward-compatible development, consider using the [trunk/stable branch](https://github.com/DragonMinded/libdragon/wiki/Stable-branch--Changelog).

Learn more: [Preview Branch Documentation](https://github.com/DragonMinded/libdragon/wiki/Preview-branch)

## Supported Platforms

This project uses Dev Containers to provide a consistent development environment across all platforms:

| Platform | Architecture | Status | Notes |
|----------|-------------|--------|-------|
| macOS (Intel) | x86_64 | ✅ Fully Supported | Native platform, no emulation |
| macOS (Apple Silicon) | ARM64 | ✅ Fully Supported | Uses Rosetta 2 emulation |
| Linux | x86_64/AMD64 | ✅ Fully Supported | Native platform, no emulation |
| Linux | ARM64 | ✅ Supported | Uses QEMU emulation (slower) |
| Windows 10/11 | x86_64 | ✅ Supported | Requires WSL2 + Docker Desktop |

All platforms build **MIPS binaries** for the Nintendo 64 using the same toolchain.

## Development Setup

This project uses VS Code Dev Containers for a consistent development environment across all platforms.

### Prerequisites

**All Platforms:**
- [Visual Studio Code](https://code.visualstudio.com/)
- [Dev Containers extension](https://marketplace.visualstudio.com/items?itemName=ms-vscode-remote.remote-containers) for VS Code

**Platform-Specific:**

<details>
<summary><b>macOS (Intel or Apple Silicon)</b></summary>

1. Install [Docker Desktop for Mac](https://www.docker.com/products/docker-desktop/)
2. Start Docker Desktop and ensure it's running
3. **Apple Silicon only:** Docker will automatically use Rosetta 2 for x86_64 emulation

</details>

<details>
<summary><b>Windows 10/11</b></summary>

1. Install [WSL2](https://learn.microsoft.com/en-us/windows/wsl/install):
   ```powershell
   wsl --install
   ```
2. Install [Docker Desktop for Windows](https://www.docker.com/products/docker-desktop/)
3. In Docker Desktop settings, ensure "Use the WSL 2 based engine" is enabled
4. Configure Git for Linux line endings:
   ```bash
   git config --global core.autocrlf input
   ```
5. **Important:** Clone this repository inside WSL2 (not in Windows filesystem):
   ```bash
   # Open WSL2 terminal
   cd ~
   git clone <repository-url>
   ```

</details>

<details>
<summary><b>Linux (Ubuntu/Debian/Fedora/etc.)</b></summary>

1. Install Docker:
   ```bash
   # Ubuntu/Debian
   sudo apt-get update
   sudo apt-get install docker.io docker-compose

   # Fedora
   sudo dnf install docker docker-compose
   ```
2. Add your user to the docker group:
   ```bash
   sudo usermod -aG docker $USER
   ```
3. Log out and back in for group changes to take effect
4. Start Docker:
   ```bash
   sudo systemctl start docker
   sudo systemctl enable docker
   ```

</details>

### Getting Started

1. Clone this repository
2. Open the folder in VS Code
3. When prompted, click "Reopen in Container" (or run `Dev Containers: Reopen in Container` from the command palette)
4. Wait for the container to build (first time will take 5-10 minutes as it downloads the toolchain)
5. Once you see "✓ Libdragon Dev Container Ready!", you're all set!

## Building the ROM

The build process is **identical on all platforms** once you're inside the Dev Container.

### Build Steps

1. Open the integrated terminal in VS Code (inside the Dev Container)
2. Run the build command:
   ```bash
   make
   ```
3. The build process will compile your code and create `n64-tech-demo.z64`

### Build Output

```
mips64-elf-gcc -c src/main.c -o build/src/main.o
mips64-elf-ld ... -o n64-tech-demo.elf
n64tool -l 1M -h ... -o n64-tech-demo.z64 n64-tech-demo.elf
```

**Success!** You should see `n64-tech-demo.z64` in your project directory.

### Clean Build

To remove build artifacts and start fresh:
```bash
make clean
```

### Debug Build

To build with debug symbols (useful for debugging with emulators that support it):
```bash
make debug
```

This builds with `-g -O0` flags (debug symbols, no optimization).

## VS Code Build Automation

This project includes VS Code tasks for streamlined N64 development.

### Quick Start

**Build ROM:** `Cmd+Shift+B` → `Enter`
**Run ROM:** Double-click `n64-tech-demo.z64` in Finder

### Available Build Tasks

Press `Cmd+Shift+B` (macOS) or `Ctrl+Shift+B` (Windows/Linux) to access:

| Task | Description |
|------|-------------|
| **Build ROM** (default) | Quick incremental build (`-O2`) |
| Build ROM (Debug) | Build with debug symbols (`-g -O0`) |
| Rebuild All | Clean + full rebuild |
| Rebuild All (Debug) | Clean + debug rebuild |
| Clean Build Artifacts | Remove all build files |
| Show ROM Info | Display ROM size and info |
| Open ROM Location | Show where to find ROM on your Mac |

**Tip:** Press `Cmd+Shift+P` → "Run Task" to see all tasks with descriptions.

### Development Workflow

**Standard Iteration:**
1. Edit code in `src/main.c`
2. Press `Cmd+Shift+B` → `Enter` (quick build)
3. Double-click `n64-tech-demo.z64` in Finder
4. ROM opens in Ares emulator
5. Test and repeat

**Debug Build:**
1. Press `Cmd+Shift+B`
2. Select "Build ROM (Debug)"
3. Open ROM in Ares
4. Use Ares debugger tools with symbols

### Why No F5 Launch?

VS Code's launch configurations (F5) can't launch emulators from inside the Dev Container. This is a known limitation - emulators run on your host Mac, but tasks run inside the Linux container.

**Solution:** Use the simple two-step workflow above (build in VS Code, open ROM in Finder).

For detailed task documentation, see [`.vscode/README.md`](.vscode/README.md).

## Running the ROM

You can run the ROM in an emulator like:
- [Ares](https://ares-emu.net/) (recommended for accuracy)
- [simple64](https://simple64.github.io/)
- [Project64](https://www.pj64-emu.com/)

Or on real hardware using a flashcart like:
- EverDrive 64
- 64drive
- SummerCart64

## Project Structure

```
n64-tech-demo/
├── .devcontainer/           # Dev Container configuration
│   ├── devcontainer.json    # VS Code Dev Container settings
│   ├── docker-compose.yml   # Container orchestration (platform support)
│   └── Dockerfile           # Custom image extending libdragon base
├── src/                     # Source code
│   └── main.c              # Main entry point
├── build/                   # Build artifacts (generated, gitignored)
├── Makefile                # Build configuration
├── .gitignore              # Git ignore rules
└── README.md               # This file
```

### Architecture Overview

- **Dev Container**: Provides consistent development environment across all platforms
- **Platform Handling**: `docker-compose.yml` specifies `linux/amd64` for cross-platform compatibility
- **Toolchain**: MIPS cross-compiler runs inside x86_64 Linux container
- **Output**: Compiled N64 ROM (`.z64` file) for MIPS architecture

## Troubleshooting

### Common Issues

<details>
<summary><b>macOS: "No matching manifest for linux/arm64"</b></summary>

This means Docker is trying to pull the ARM64 version of the image, which doesn't exist.

**Solution:**
The `docker-compose.yml` file specifies `platform: linux/amd64`. Ensure you're using the latest version of Docker Desktop with Rosetta 2 support enabled.

If the issue persists, manually pull the image:
```bash
docker pull --platform=linux/amd64 ghcr.io/dragonminded/libdragon:latest
```

</details>

<details>
<summary><b>Windows: Permission errors or "cannot create directory"</b></summary>

This usually happens when the repository is cloned in the Windows filesystem (e.g., `/mnt/c/Users/...`) instead of the WSL2 filesystem.

**Solution:**
1. Open WSL2 terminal (not PowerShell)
2. Clone the repository in your WSL2 home directory:
   ```bash
   cd ~
   git clone <repository-url>
   code n64-tech-demo
   ```

</details>

<details>
<summary><b>Linux: "permission denied while trying to connect to Docker daemon"</b></summary>

Your user doesn't have permission to access Docker.

**Solution:**
```bash
sudo usermod -aG docker $USER
```
Then log out and back in, or run:
```bash
newgrp docker
```

</details>

<details>
<summary><b>All Platforms: "Container failed to start"</b></summary>

**Check Docker is running:**
- macOS/Windows: Open Docker Desktop
- Linux: `sudo systemctl status docker`

**Check logs:**
In VS Code, open the Output panel and select "Dev Containers" from the dropdown to see detailed error messages.

**Rebuild container:**
1. Run command: `Dev Containers: Rebuild Container`
2. This will rebuild from scratch

</details>

<details>
<summary><b>Slow build performance on Apple Silicon or Linux ARM64</b></summary>

This is expected when running x86_64 containers on ARM64 systems. The libdragon Docker image only exists for x86_64, so it must be emulated.

**Performance tips:**
- First build takes longest (downloading + compiling)
- Subsequent builds are faster (caching)
- Consider closing other resource-intensive applications

The libdragon team may release ARM64 images in the future, which would eliminate emulation overhead.

</details>

## Resources

### Libdragon Documentation
- [Libdragon API Reference](https://libdragon.dev/) - Complete API documentation
- [Libdragon GitHub](https://github.com/DragonMinded/libdragon) - Source code and examples
- [Libdragon Wiki](https://github.com/DragonMinded/libdragon/wiki) - Installation and guides
- [Preview Branch Features](https://github.com/DragonMinded/libdragon/wiki/Preview-branch) - What's in preview
- [Installing Libdragon](https://github.com/DragonMinded/libdragon/wiki/Installing-libdragon) - Official installation guide

### Tutorials & Community
- [N64 Squid Tutorials](https://n64squid.com/homebrew/libdragon/) - Beginner-friendly guides
- [n64brew Discord](https://discord.gg/WqFgNWf) - Active community for questions and help
- [N64brew Wiki](https://n64brew.dev/wiki/Libdragon) - Community knowledge base

### Development Tools
- [Dev Containers Documentation](https://code.visualstudio.com/docs/devcontainers/containers) - VS Code setup
- [Ares Emulator](https://ares-emu.net/) - Recommended N64 emulator for development

## License

This project is open source and available under the MIT License.
