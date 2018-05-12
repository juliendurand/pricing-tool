import subprocess
import sys

import documentation
import results


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
    r = results.Results("data", "mrh")
    doc = documentation.Documentation('doc/mrh/' + name + '/', name, r)
    doc.save_html()
    command = 'open doc/mrh/' + name + '/' + name + ".html"
    subprocess.run([command], shell=True)
    print("Document Finished\n")


if __name__ == '__main__':
    if len(sys.argv) != 2:
        raise Exception("Invalid number of options, expecting only one : " \
                        "[config filename].")
    name = sys.argv[1]

    compile()
    fit(name)
    generate_documentation(name)
