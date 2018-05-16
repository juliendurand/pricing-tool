import os
import subprocess
import sys

import documentation
import results


def get_result_path(name):
    config_file = name + '.cfg'
    result_name = ""
    with open(config_file) as config:
        config.readline()
        result_name = config.readline()[:-1]
    return os.path.join("results", result_name)


def compile():
    print("Compiling regression algorithms...")
    command = "time g++ -Wall -std=c++11 -O3 ./src/*.cpp -o ./bin/glm"
    result = subprocess.run([command], shell=True)
    if result.returncode != 0:
        raise Exception("Fatal Error during compilation.")
    print("Compiling Finished.\n")


def fit(name):
    print("Fitting model...")
    config_file = name + '.cfg'
    command = "time ./bin/glm " + config_file
    result = subprocess.run([command], shell=True)
    if result.returncode != 0:
        raise Exception("Fatal Error during model fitting.")
    print("Fitting Finished\n")


def generate_documentation(name):
    print("Generating Documentation...")
    config_file = name + '.cfg'
    metadata_path = ""
    with open(config_file) as config:
        metadata_path = config.readline()[:-1]
    result_path = get_result_path(name)
    r = results.Results(result_path, metadata_path)
    basename = os.path.basename(result_path)
    doc = documentation.Documentation('doc/' + basename + '/', basename, r)
    doc.save_html()
    command = 'open doc/' + basename + '/' + basename + ".html"
    subprocess.run([command], shell=True)
    print("Document Finished\n")


if __name__ == '__main__':
    if len(sys.argv) != 2:
        raise Exception("Invalid number of options, expecting only one : " \
                        "[config filename].")
    name = sys.argv[1]

    result_name = get_result_path(name)
    if not os.path.exists(result_name):
        os.makedirs(result_name)

    compile()
    fit(name)
    generate_documentation(name)
