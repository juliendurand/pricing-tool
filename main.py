import subprocess

import documentation
import results


def compile():
    print("Compiling regression algorithms...")
    command = "time g++ -Wall -std=c++11 -O3 ./src/*.cpp -o ./bin/sgd"
    result = subprocess.run([command], shell=True)
    if result.returncode != 0:
        raise Exception("Fatal Error during compilation.")
    print("Compiling Finished.\n")


def fit():
    print("Fitting model...")
    command = "time ./bin/sgd data/mrh chargeDDEA"
    command = "time ./bin/sgd data/mrh nbsinDDE"
    result = subprocess.run([command], shell=True)
    if result.returncode != 0:
        raise Exception("Fatal Error during model fitting.")
    print("Fitting Finished\n")


def generate_documentation():
    print("Generating Documentation...")
    r = results.Results("data", "mrh")
    doc = documentation.Documentation("doc/mrh/chargeDDEa/", "chargeDDEa", r)
    doc.save_html()
    command = "open doc/mrh/chargeDDEa/chargeDDEa.html"
    subprocess.run([command], shell=True)
    print("Document Finished\n")


if __name__ == '__main__':
    compile()
    fit()
    generate_documentation()
