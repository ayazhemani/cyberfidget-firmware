# SPDX-License-Identifier: MIT
# Copyright (c) 2023-2026 Dismo Industries LLC

"""
PlatformIO pre-build script: Update Version

Embeds the current git commit hash and version string into the firmware's
version.h file, which is included in the build. This allows the firmware to
report its exact version and commit when queried, aiding in debugging and support.

Strategy:
1. Read the current git commit hash using "git rev-parse HEAD" and 
    the version string from a VERSION file.
2. Open the version.h file and replace the VERSION_STRING and GIT_COMMIT macros
    with the retrieved values.
3. Save the modified version.h file for inclusion in the build.
"""
import os
from subprocess import check_output, CalledProcessError

VERSION_HEADER_PATH = os.path.join("lib", "Globals", "version.h")
VERSION_SOURCE_FILE = "VERSION"


def get_git_commit() -> str:
    "Returns the current git commit hash."
    try:
        commit_hash = check_output(["git", "rev-parse", "HEAD"]).decode().strip() or os.getenv("GIT_COMMIT")
        return commit_hash
    except CalledProcessError as e:
        print(f"Error retrieving git commit: {e}")
        return "unknown"

def get_version() -> (int, int, int):
    "Returns the current version tuple from the VERSION file."
    if os.path.exists(VERSION_SOURCE_FILE):
        with open(VERSION_SOURCE_FILE, "r") as f:
            version = f.read().strip()
            major, minor, patch = version.split(".")
            return int(major), int(minor), int(patch)
    else:
        print(f"Version source file not found at {VERSION_SOURCE_FILE}")
        return 0, 0, 0
    

def update_version_header(major: int, minor: int, patch: int, commit_hash: str):
    version_header_template = VERSION_HEADER_PATH + ".template"
    if not os.path.exists(version_header_template):
        print(f"Version header template not found at {version_header_template}")
        return

    with open(version_header_template, "r") as f:
        content = f.read()

    # Replace the FW_VERSION_MAJOR, FW_VERSION_MINOR, FW_VERSION_PATCH, and FW_VERSION_TAG macros
    # in the template with the actual version and commit hash

    content = content.replace("#define FW_VERSION_MAJOR 0", f"#define FW_VERSION_MAJOR {major}")
    content = content.replace("#define FW_VERSION_MINOR 0", f"#define FW_VERSION_MINOR {minor}")
    content = content.replace("#define FW_VERSION_PATCH 0", f"#define FW_VERSION_PATCH {patch}")
    content = content.replace("#define FW_VERSION_TAG \"\"", f"#define FW_VERSION_TAG \"{commit_hash}\"")

    with open(VERSION_HEADER_PATH, "w") as f:
        f.write(content)
    print(f"Updated version header with version: {major}.{minor}.{patch}+{commit_hash}")

if __name__ in ("__main__", "SCons.Script"):
    major, minor, patch = get_version()
    commit_hash = get_git_commit()
    update_version_header(major, minor, patch, commit_hash)