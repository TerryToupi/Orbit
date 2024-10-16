import os
import platform
import subprocess

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

# Check if the system is supported
if system not in premake_paths:
    print(f"Unsupported system: {system}")
    exit(1)

# Path to the Premake executable based on the current system
premake_executable = premake_paths[system]

# Print the path for debugging
print(f"Attempting to find Premake executable at: {premake_executable}")

# Check if the Premake executable exists
if not os.path.exists(premake_executable):
    print(f"Premake executable not found for {system} at {premake_executable}")
    exit(1)

# Define the Premake target based on the current system
premake_target = premake_targets[system]

# Define the command to run Premake
premake_command = [premake_executable, "--file=Build.lua", premake_target]

# Run the Premake command
try:
    subprocess.run(premake_command, check=True)
    print(f"Premake executed successfully for {system} with target {premake_target}.")
except subprocess.CalledProcessError as e:
    print(f"Error running Premake: {e}")

# Optional: Wait for user input before closing (Windows-specific)
if system == "Windows":
    input("Press Enter to exit...")
