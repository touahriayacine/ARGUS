from utils.load_configuration import load_configuration
from utils.utils import get_elf_file,generate_qemu_flash
from pathlib import Path
import subprocess
import shlex
import signal
import os
import pandas as pd 
import numpy as np

class Microcontroller:
    def __init__(self,mcu_name,project_path,output_path,save_counts,log_bar=None):
        self.mcu = mcu_name
        self.current_path = "/".join(str(Path(__file__).resolve()).split("/")[:-1])
        self.output_path = output_path
        self.project_path = str(Path(project_path).resolve())
        self.configuration = load_configuration(mcu_name,self.current_path + "/mcu_configs.json")
        self.unique_instructions = []
        self.instruction_counts = []
        self.save_into_csv = save_counts
        self.energy_consumption = None
        self.energy_score = None
        self.log_bar = log_bar

    def get_dependencies(self):
        if self.configuration is not None:
            return self.configuration["dependencies"]
        else:
            print("\033[31mThis microcontroller doesn't exist!\033[0m")
            exit()

    def compile_project(self):
        # here compile the project given as input

        if self.configuration is not None:
            command  = self.configuration["compiler_command"]
            command = command.replace("_argus_path_placeholder_",self.current_path)
            command = command.replace("_project_path_placeholder_",self.project_path)
            end = False
            while(not end):
                proc = subprocess.Popen(
                    shlex.split(command),
                    start_new_session=True,
                    stdout=subprocess.PIPE,
                    stderr=subprocess.STDOUT,
                    text=True,
                    bufsize=1,
                    cwd=self.project_path
                )
                end = True
                for line in proc.stdout:
                    self.log_bar.set_description_str(f"\033[2;37m{line.strip()}\033[0m")
                    if "fullclean" in line:
                        end = False
                        os.system(f"rm -rf {self.project_path}/build/")
                        subprocess.run("idf.py fullclean",stdout=subprocess.PIPE, stderr=subprocess.PIPE,shell=True,cwd=self.project_path)
                self.log_bar.set_description_str("")

        else:
            print("\033[31mThis microcontroller doesn't exist!\033[0m")
            exit()
        

    
    def trace_instructions(self):
        # here qemu instruction tracing

        if self.configuration is not None:
            command  = self.configuration["qemu_command"]
            command = command.replace("_argus_path_placeholder_",self.current_path)
            command = command.replace("_output_path_placeholder_",self.output_path)
            command = command.replace("_firmware_path_placeholder_",get_elf_file(self.project_path))

            if self.mcu == "esp32c3":
                # generate the qemu_flash.bin file
                qemu_flash_file = generate_qemu_flash(self.project_path)
                command = command.replace("_qemu_flash_bin_placeholder_",qemu_flash_file)

            while(True):
                proc = subprocess.Popen(
                    shlex.split(command),
                    start_new_session=True,
                    stdout=subprocess.PIPE,
                    stderr=subprocess.STDOUT,
                    text=True,
                    bufsize=1,
                    cwd=self.project_path
                )
                finished = False
                for line in proc.stdout:
                    self.log_bar.set_description_str(f"\033[2;37m{line.strip()}\033[0m")
                    if self.mcu == "esp32c3":
                        if "finished" in line:
                            os.killpg(proc.pid, signal.SIGTERM)
                            finished = True
                    else:
                        finished = True
                self.log_bar.set_description_str("")
                if not finished:
                    continue

                break

        else:
            print("\033[31mThis microcontroller doesn't exist!\033[0m")
            exit()

    def count_instructions(self):
        # here count the total occurence of each instruction in the execution trace

        if self.configuration is not None:
            start_main = False
            with open(self.output_path+"/trace.txt", "r")as f:
                for line in f:
                    if not start_main and self.configuration["start_count_condition"] in line:
                        start_main = True
                    if start_main and self.configuration["end_count_condition"] not in line:
                        tmp = line.split()
                        if len(tmp) > 2:
                            instruction = line.split()[1]
                            if instruction not in self.unique_instructions:
                                self.unique_instructions.append(instruction)
                                self.instruction_counts.append(1)
                            else:
                                i = self.unique_instructions.index(instruction)
                                self.instruction_counts[i] +=1

            if self.save_into_csv:
                df = pd.DataFrame([np.array(self.instruction_counts)], columns=self.unique_instructions)
                project_name = self.project_path.split("/")[-1]
                df.insert(0,"Algorithm",[project_name])
                DATA_FILE = self.output_path+"/instruction_counts.csv"
                df.to_csv(DATA_FILE,index=False)
        else:
            print("\033[31mThis microcontroller doesn't exist!\033[0m")
            exit()


    def estimate_energy_consumption(self):
        # load the estimation model and perform the estimation

        # sort the instructions according to the model
        self.unique_instructions = np.array(self.unique_instructions)
        self.instruction_counts = np.array(self.instruction_counts)

        # Map instruction -> measured count
        count_map = dict(zip(
            self.unique_instructions,
            self.instruction_counts
        ))

        # Configuration defines the desired order
        config_instructions = np.array(
            self.configuration["instructions"]
        )

        # Reorder counts and fill missing instructions with 0
        self.instruction_counts = np.array([
            count_map.get(instruction, 0)
            for instruction in config_instructions
        ])

        # Instructions are now exactly in configuration order
        self.unique_instructions = config_instructions

        # Coefficients are already in configuration order
        coefs = np.array(self.configuration["coefs"])

        # count_i * coefficient_i, then sum
        self.energy_consumption = np.dot(
            self.instruction_counts,
            coefs
        )

    def calculate_energy_score(self):
        # normalize the energy consumption to a score bounded between 0 and 1
        Emax = self.configuration["Emax"]
        self.energy_score = 1- min(self.energy_consumption / Emax,1)
