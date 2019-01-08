from background_task import background

from . import models

import pypricing.dataset as ds


@background
def process_dataset(dataset_id):
    dataset = models.Dataset.objects.get(pk=dataset_id)
    print('Start Processing Dataset : ' + str(dataset))
    config = dataset.get_config()
    try:
        ds.Dataset().process(config)
    except Exception as e:
        print('Exception raised during processing...')
        print(e)
        return
    print('Done Processing Dataset')
