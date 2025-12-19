Import("env")
import shutil
import os

def post_build_callback(source, target, env):
    """Copy and rename firmware after successful build"""
    firmware_source = str(target[0])
    firmware_name = "OpenSprinkler-2.2.1.4-flowapi.bin"
    
    # Copy to build directory with custom name
    build_dir = os.path.dirname(firmware_source)
    firmware_dest_build = os.path.join(build_dir, firmware_name)
    shutil.copy(firmware_source, firmware_dest_build)
    print(f"Created: {firmware_dest_build}")
    
    # Copy to project root for easy access
    project_root = env.get("PROJECT_DIR")
    firmware_dest_root = os.path.join(project_root, firmware_name)
    shutil.copy(firmware_source, firmware_dest_root)
    print(f"Created: {firmware_dest_root}")

env.AddPostAction("$BUILD_DIR/${PROGNAME}.bin", post_build_callback)

