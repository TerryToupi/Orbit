import os
import subprocess

def premake_build(premake_paths, premake_targets, parent_dir, system):
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

    # Define the absolute path for Build.lua
    build_file_path = os.path.join(parent_dir, "Build.lua")

    # Check if Build.lua exists
    if not os.path.exists(build_file_path):
        print(f"Build.lua not found at {build_file_path}")
        exit(1)

    # Define the Premake target based on the current system
    premake_target = premake_targets[system]

    # Define the command to run Premake
    premake_command = [premake_executable, "--file=" + build_file_path, premake_target]

    # Run the Premake command
    try:
        subprocess.run(premake_command, check=True)
        print(f"Premake executed successfully for {system} with target {premake_target}.")
    except PermissionError:
        print("Permission denied: Check if the Premake executable has the correct permissions.")
    except subprocess.CalledProcessError as e:
        print(f"Error running Premake: {e}")

    
