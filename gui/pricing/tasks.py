import subprocess

from background_task import background

from . import models

import pypricing.dataset as ds


@background
def process_dataset(dataset_id):
    try:
        dataset = models.Dataset.objects.get(pk=dataset_id)
        print('Start Processing Dataset : ' + str(dataset))
        config = dataset.get_config()
        ds.Dataset().process(config, ds.printProgressBar)
    except Exception as e:
        print('Exception raised during processing...')
        print(e)
        return
    print('Done Processing Dataset')


@background
def run_model(run_id):
    try:
        run = models.Run.objects.get(pk=run_id)
        model = run.model
        print('Start Running Model : ' + str(model) + ' ' + str(run))
        config = model.get_config()
        # print(config)
        complete_process = subprocess.run(['./bin/glm'],
                                          input=config,
                                          stdout=subprocess.PIPE,
                                          stderr=subprocess.STDOUT,
                                          universal_newlines=True,
                                          check=True)
        print(complete_process.stdout)
    except Exception as e:
        print('Exception raised during processing...')
        print(e)
        return
    print('Done Running Model')
