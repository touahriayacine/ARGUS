from pathlib import Path
import subprocess
import sys

def check_path_exists(path):
    path = Path(path)
    if path.exists():
        return 1
    else:
        return 0


def get_elf_file(path):
    path = path + "/build/"
    path = Path(path).resolve()
    elf_files = list(path.glob("*.elf"))
    if elf_files:
        return str(elf_files[0])
    else:
        return None


def generate_qemu_flash(path):
    qemu_flash_file = "/".join(get_elf_file(path).split("/")[:-1])+"/qemu_flash.bin"
    if not check_path_exists(qemu_flash_file):
        build = Path(path) / "build"

        subprocess.run(
            [
                sys.executable,
                "-m",
                "esptool",
                "--chip=esp32c3",
                "merge-bin",
                "--output=qemu_flash.bin",
                "--pad-to-size=4MB",
                "@flash_args",
            ],
            cwd=build,
            check=True,
            stderr=subprocess.PIPE,
            stdout=subprocess.PIPE
        )

    return qemu_flash_file