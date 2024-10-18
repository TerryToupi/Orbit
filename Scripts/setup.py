import os
import platform

from setupVulkan import is_vulkan_installed, install_vulkan_sdk
from setupPremake import premake_build  

# Get the absolute path to the directory where the script is located, then move one directory up
script_dir = os.path.dirname(os.path.abspath(__file__))
parent_dir = os.path.abspath(os.path.join(script_dir, ".."))

# Detect the current operating system
system = platform.system()

# Define paths for Premake executables based on the OS using the parent directory
premake_paths = {
    "Windows": os.path.join(parent_dir, "External", "Premake", "Windows", "premake5.exe"),
    "Linux": os.path.join(parent_dir, "External", "Premake", "Linux", "premake5"),
    "Darwin": os.path.join(parent_dir, "External", "Premake", "macOS", "premake5")
}

# Define the Premake targets for each platform
premake_targets = {
    "Windows": "vs2022",    # Visual Studio 2022 for Windows
    "Linux": "gmake2",      # GNU Make for Linux
    "Darwin": "xcode4"      # Xcode for macOS
} 

if __name__ == "__main__": 
    vulkan_download_path = os.path.join(parent_dir, "External", "Vulkan")
    vulkan_installation_path = os.path.join(vulkan_download_path, "Installation")  

    # Check if the directory exists
    if not os.path.exists(vulkan_download_path):
        # Directory does not exist, create it
        os.makedirs(vulkan_download_path)
        print(f"Created directory: {vulkan_download_path}")
    else:
        print(f"Directory already exists: {vulkan_download_path}")

    # Check if the directory exists
    if not os.path.exists(vulkan_installation_path):
        # Directory does not exist, create it
        os.makedirs(vulkan_installation_path)
        print(f"Created directory: {vulkan_installation_path}")
    else:
        print(f"Directory already exists: {vulkan_installation_path}")

    # Check if Vulkan is installed, and if not, install it 
    if not is_vulkan_installed(system=system):
        install_vulkan_sdk(system=system, download_path=vulkan_download_path)
    else:
        print("Vulkan is already installed. Skipping installation.") 

    premake_build(
        premake_paths=premake_paths,
        premake_targets=premake_targets, 
        parent_dir=parent_dir,
        system=system
    )

    # Optional: Wait for user input before closing (Windows-specific)
    if system == "Windows":
        input("Press Enter to exit...")