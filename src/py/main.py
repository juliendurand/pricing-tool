import os
import subprocess
import sys

import config


def compile():
    print("Compiling regression algorithms...")
    command = "time g++ -Wall -std=c++11 -O3 ./src/cpp/*.cpp -o ./bin/glm"
    result = subprocess.run(command, shell=True)
    if result.returncode != 0:
        raise Exception("Fatal Error during compilation.")
    print("Compilation Finished.\n")


def fit(config):
    result_path = config.get_result_path()
    if not os.path.exists(result_path):
        os.makedirs(result_path)
    print('Fitting model...')
    command = 'time ./bin/glm ' + config.filename
    result = subprocess.run(command, shell=True)
    if result.returncode != 0:
        raise Exception('Fatal Error during model fitting.')
    print('Model Finished\n')


def generate_documentation(config):
    command = 'time python src/py/documentation.py ' + config.filename
    result = subprocess.run(command, shell=True)
    if result.returncode != 0:
        raise Exception('Fatal Error during documentation.')
    print('Documentation Finished\n')


if __name__ == '__main__':
    if len(sys.argv) != 2:
        raise Exception("Invalid number of options, expecting only one : "
                        "[config filename].")
    config_filename = sys.argv[1]
    configuration = config.Config(config_filename)

    compile()
    fit(configuration)
    generate_documentation(configuration)
