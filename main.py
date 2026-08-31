from core.Microcontroller import Microcontroller
from utils.check_dependencies import check_dependencies
from utils.utils import check_path_exists, get_elf_file
from utils.argument_parser import parse_arguments
import os
from tqdm import tqdm
from pathlib import Path

if __name__ == "__main__":

    project_path, output_path, mcu_name, save_counts =  parse_arguments()

    # check if the output folder doesn't exist
    if output_path == "build":
        output_path = "/".join(str(Path(__file__).resolve()).split("/")[:-1]) +"/"+output_path # absolute path of the build project
        if not check_path_exists(output_path):
            os.system("mkdir build")
    else:
        output_path = str(Path(output_path).resolve())

    if not check_path_exists(output_path):
        print("\033[31mThe output folder does not exist!\033[0m")
        exit()

    if not check_path_exists(project_path):
        print("\033[31mThe project folder does not exist!\033[0m")
        exit()

    mcu = Microcontroller(mcu_name, project_path, output_path,save_counts)

    # here check if all the dependencies exist
    code = check_dependencies(mcu.get_dependencies())
    if code == 1:
        exit()

    pbar = tqdm(total=4,leave=True,position=0)
    log_bar = tqdm(total=0,position=1,leave=True,bar_format="{desc}")
    mcu.log_bar = log_bar

    # 1 - COMPILATION:
    # compilation is optional if the binary file already exists
    if not get_elf_file(project_path):
        if code ==2:
            print(f"\033[31mToolchain is not installed! You cannot compile projects! You can use binaries only!\033[0m")
            exit(1)
        else:
            pbar.set_description("Compiling ...")
            mcu.compile_project()
    pbar.update(1)

    # 2- INSTRUCTION TRACING:
    pbar.set_description("Instruction tracing ...")
    mcu.trace_instructions()
    pbar.update(1)

    # 3- ENERGY ESTIMATION:
    pbar.set_description("Estimating energy ...")
    mcu.count_instructions()
    mcu.estimate_energy_consumption()
    pbar.update(1)

    # 4- SCORE CALCULATION:
    pbar.set_description("Calculating score ...")
    mcu.calculate_energy_score()
    pbar.set_description("finished!")
    pbar.update(1)
    pbar.close()
    log_bar.close()


    print("\033[32mEnergy consumption:\033[0m",mcu.energy_consumption,"mJ")
    print("\033[33mEnergy score:\033[0m",mcu.energy_score,"/1")
