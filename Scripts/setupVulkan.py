import os
import platform
import subprocess
import sys
import requests 
from tqdm import tqdm  # We will use 'tqdm' for the progress bar

vulkan_url = "https://sdk.lunarg.com/sdk/download/latest/windows/vulkan-sdk.exe"
installer = "vulkan-sdk.exe" 

def download_file(url, dest_folder, filename):
    # Ensure the destination folder exists
    if not os.path.exists(dest_folder):
        os.makedirs(dest_folder)

    # Create the full file path
    filepath = os.path.join(dest_folder, filename)

    # Start downloading the file using requests
    print(f"Downloading {url} to {filepath}...")

    try:
        # Streaming the request to avoid large memory consumption
        response = requests.get(url, stream=True)
        response.raise_for_status()  # Check for HTTP errors

        # Total size in bytes.
        total_size = int(response.headers.get('content-length', 0))

        # Open the file and write it in chunks
        with open(filepath, 'wb') as f, tqdm(
            desc=filename,
            total=total_size,
            unit='B',
            unit_scale=True,
            unit_divisor=1024,
        ) as bar:
            for chunk in response.iter_content(chunk_size=8192):
                if chunk:
                    f.write(chunk)
                    bar.update(len(chunk))  # Update the progress bar

        print(f"Downloaded {filename} to {filepath}")
    except requests.exceptions.HTTPError as err:
        print(f"HTTP error occurred: {err}")
        sys.exit(1)
    except Exception as err:
        print(f"An error occurred: {err}")
        sys.exit(1)
    
    return filepath

def is_vulkan_installed(system):
    if system == "Windows":
        vulkan_sdk = os.environ.get("VULKAN_SDK")
        if vulkan_sdk:
            print(f"Vulkan SDK detected at {vulkan_sdk}")
            return True
        else:
            print("Vulkan SDK not found on Windows.")
            return False

    elif system == "Linux":
        try:
            subprocess.run(["vulkaninfo"], stdout=subprocess.PIPE, stderr=subprocess.PIPE, check=True)
            print("Vulkan is already installed on Linux.")
            return True
        except subprocess.CalledProcessError:
            print("Vulkan is not installed on Linux.")
            return False

    elif system == "Darwin":  # macOS
        try:
            result = subprocess.run(["brew", "list", "molten-vk"], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
            if result.returncode == 0:
                print("MoltenVK (Vulkan SDK) is already installed on macOS.")
                return True
            else:
                print("MoltenVK is not installed on macOS.")
                return False
        except FileNotFoundError:
            print("Homebrew is not installed on macOS.")
            return False

    else:
        print(f"Unsupported OS: {system}")
        return False

def install_vulkan_sdk(system, download_path):
    permissionGranted = False
    while not permissionGranted:
        reply = str(input("Would you like to install VulkanSDK {0:s}? [Y/N]: ".format(installer))).lower().strip()[:1]
        if reply == 'n':
            return
        permissionGranted = (reply == 'y') 

    if system == "Windows":
        # Define the custom download path inside the project directory
        filepath = download_file(vulkan_url, download_path, installer)

        print(f"Running the Vulkan SDK installer from {filepath}...")
        os.startfile(os.path.abspath(filepath))         
        print("Re-run this script after the installer!")
        quit()

    elif system == "Linux":
        print("Installing Vulkan SDK for Linux...")
        subprocess.run(["sudo", "apt", "install", "vulkan-sdk"], check=True)
        print("Vulkan SDK installed on Linux")

    elif system == "Darwin":  # macOS
        print("Installing Vulkan SDK for macOS...")
        subprocess.run(["brew", "install", "molten-vk"], check=True)
        print("Vulkan SDK (MoltenVK) installed on macOS")

    else:
        print(f"Unsupported OS: {system}")
        sys.exit(1)
