# Actuarial P&C Technical Pricing

## Objective

The objective of this package is to provide a robust set of tools for pricers in the P&C insurance industry. It can help you to :
   - preprocess the modeling data (claims and contract images) ;
   - fit poisson glm regression to model claim frequency ;
   - fit gamma glm regression to model claim costs ;
   - select the best features automaticaly ;
   - generate the full documentation for you models.

It is especially designed to handle large dataset and can easily work with several millions of observations and hundreds of features on a commodity computer.

## Contact / Help

If you want to contact me or need some help, it will be my pleasure to exchange with you.

Please send me an email at julien.durand.1981@gmail.com

## Installation

The package should be able to install and run on any machine. I am using a mac but you should be able to work on a standard PC (Windows or Linux) or a server.

### Prerequisites

In order to be able to use this package you need to have the following requirements met :
  - Python 3 (idealy 3.6)
  - c++11 compiler
  - git

Specific instructions to install these dependencies are available on the web for almost any system you may have.

### Download the package from github

```
git clone https://github.com/juliendurand/pricing-tool.git
```

### Create a virtual environment

If you do not know what a virtual environment is, then it's probably best to skip this step and move to the next paragraph.

If you know how to a use virtual environment, let's create one :

```
python3 -m venv venv
```

Activate the new environment :

```
source venv/bin/activate
```

### Install all python dependency packages

It's now time to install all the requirement python packages :

```
pip install -r requirements.txt
```

This command may take a few minutes as it downloads and installs the following required python packages :
  - numpy
  - pandas
  - matplotlib
  - jinja2
  - markdown

Congratulations, you should now be ready to run the package !

## Getting Started

For the following steps I recommend to use a notebook interface (like jupyter), but you can also simply type in the command in your favorite command line / shell.

### Modeling database

You need to create a csv file containing the modeling data : one line per contract image. You also need to add 3 additional columns for each peril :
  - exposure
  - number of claims
  - total claim cost.

Use the comma character ',' as separator as this is the most standard choice.

### Create a data processing configuration file

You now need to create a json file to describe you modeling database. Please find below an example that you will need to modify to your situation.

Content of the file **config_dataset.json** :

```
{
    "filename": "./data/mydataset.csv",

    "path": "./dataset/myproject",

    "features": [
        "FEATURE_1",
        "FEATURE_2",
        "FEATURE_3",
        "FEATURE_4",
        "FEATURE_5"
    ],

    "targets": [
        "EXPOSURE_PERIL_1",
        "NB_CLAIMS_PERIL_1",
        "COST_PERIL_1",
        "EXPOSURE_PERIL_2",
        "NB_CLAIMS_PERIL_2",
        "COST_PERIL_2"
    ],

    "train_size": 0.8,

    "filter": "def data_filter(row):\n    return True\n",

    "transform": "def data_transform(row):\n    row['FEATURE_1'] = int(math.log(float(row['FEATURE_1'])))\n",

    "train": "def data_train(row):\n    return row['Random'] != '9' and row['Random'] != '10'\n"
}
```

### Run the data preprocessing

You are now ready to launch the data preprocessing :

```
python src/py/dataset.py config_dataset.json
```

This step generate several binary files (.dat extension) in the folder ./dataset/myproject. These files are optimized to run the glm regressions.

### Define a model configuration file

For each model that you want to fit, you need to create a model configuration file similar to the exemple below.

Content of the file **mymodel.cfg** :

```
Model title
./dataset/myproject
mymodel
poisson
EXPOSURE_PERIL_1
NB_CLAIMS_PERIL_1
7
EXCLUDED_FEATURE_1
EXCLUDED_FEATURE_2
...
```

The format of this file is quite simple : it's a text file with one parameter per line. The parameter expected on each line are :
  - 1st line : Model title (will be displayed as the documentation title).
  - 2nd line : path of the dataset. It must match what has been provided in the configuration dataset file.
  - 3rd line : name of the model.
  - 4th line : loss function (poisson or gamma).
  - 5th line : weight (exposure for frequency / number of claims for severity).
  - 6th line : target (number of claims for frequency / total cost for severity).
  - 7 th line : maximum number of features to include in the model.
  - following lines : names of excluded features (one per line).

### Fit and document your model

Finally, you can fit and document your model :

```
python src/py/main.py mymodel.cfg
```

The console will provide useful information on the feature selection process and you may want to adjust it to better fit your specific data.

### Read the documentation

The documentation of your model is available in html format in the folder ./doc/mymodel

