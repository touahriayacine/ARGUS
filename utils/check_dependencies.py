import shutil
import time

def check_dependencies(dependencies):
    for program in dependencies:
        if not shutil.which(program):
            
            if program in ["idf.py","avr-gcc"]:
                return 2
            else:
                print(f"\033[31m{program} is not installed!\033[0m")
            
            return 1
    
    return 0
