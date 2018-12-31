import csv
import itertools as it
import json
import math
import os
import sys
import time

import numpy as np


start_time = 0


def detect_csv_separator(filename):
    """
    Utility function to automatically detect the separator character in
    a csv file.
    """
    with open(filename) as csvfile:
        first_line = csvfile.readline()
        return csv.Sniffer().sniff(first_line).delimiter


def count_line(filename):
    '''
    Fast count the number of lines in a file.
    '''
    f = open(filename, 'rb')
    bufgen = it.takewhile(lambda x: x, (f.raw.read(1024 * 1024)
                                        for _ in it.repeat(None)))
    return sum(buf.count(b'\n') for buf in bufgen)


def create_data_file_from_list(lst, out_filename, dtype, shape):
    """Write a list in a binary file as a numpy array.
    Args:
        lst: The list that will be written in the file.
        out_filename: The name of the binary file. It must be in the same
            directory.
        dtype: The type of the numpy array.
        shape: The shape of the numpy array.
    """
    with open(out_filename, 'wb+') as out_file:
        out_file = open(out_filename, 'wb+')
        dat_file = np.memmap(out_file, dtype=dtype, shape=shape)
        dat_file[:] = lst[:]
        dat_file.flush()
        size = float(dat_file.nbytes) / (1024 ** 2)
        print('written %s : %.3f MB' % (out_filename, size))


def load_data(file_path, dtype='int32', shape=None):
    '''
    Loads a numpy array in memory from the filesystem.
    '''
    return np.memmap(file_path, dtype=dtype, shape=shape)


def printProgressBar(iteration, total, prefix='', suffix='', decimals=1,
                     length=100, fill='█'):
    """
    Call in a loop to create terminal progress bar
    @params:
        iteration   - Required  : current iteration (Int)
        total       - Required  : total iterations (Int)
        prefix      - Optional  : prefix string (Str)
        suffix      - Optional  : suffix string (Str)
        decimals    - Optional  : number of decimals in % complete (Int)
        length      - Optional  : character length of bar (Int)
        fill        - Optional  : bar fill character (Str)
    """
    global start_time
    if iteration == 0:
        start_time = time.time()
    value = 100 * (iteration / float(total))
    percent = ("{0:." + str(decimals) + "f}").format(value)
    filledLength = int(length * iteration // total)
    bar = fill * filledLength + '-' * (length - filledLength)
    elapsed_time = int(time.time() - start_time)
    m = str(elapsed_time // 60).zfill(2)
    s = str(elapsed_time % 60).zfill(2)
    print('\r%s |%s| %s%% %s' % (prefix, bar, percent, suffix), end = '\r')
    # Print New Line on Complete
    if iteration == total:
        print()


class Dataset:
    '''
    Encapsulate all the data en metadata for a glm regression.
    '''

    def __init__(self, path):
        self.path = path
        self.size = -1
        self.fields = None
        self.features = None
        self.modalities = None
        self.targets = None
        self.csv_filename = None
        self.train_size = None
        self.test_size = None

        if not os.path.exists(path):
            os.makedirs(path)

    def set_size(self, size):
        self.size = size

    def set_fields(self, fields):
        self.fields = fields

    def set_features(self, features):
        self.features = features

    def set_targets(self, targets):
        self.targets = targets

    def set_modalities(self, modalities):
        self.modalities = modalities

    def count_features(self):
        return len(self.features)

    def count_modalities(self):
        return sum(self.count_modalities_per_feature())

    def count_modalities_per_feature(self):
        return [len(v) for k, v in self.modalities.items()]

    def get_offsets(self):
        return np.cumsum([0] + self.count_modalities_per_feature())

    def get_feature_index(self, feature):
        if feature in self.features:
            return self.features.index(feature)
        return -1

    def get_feature_range(self, feature):
        feature_idx = self.get_feature_index(feature)
        offsets = self.get_offsets()
        return list(range(offsets[feature_idx], offsets[feature_idx + 1]))

    def get_modalities(self, feature):
        return self.modalities[feature]

    def get_feature_modality_by_index(self, idx):
        offsets = self.get_offsets()
        for feature_idx in range(self.count_features()):
            if offsets[feature_idx + 1] > idx:
                break
        modality_idx = idx - offsets[feature_idx]
        feature = self.features[feature_idx]
        modality = self.modalities[feature][modality_idx]
        return feature, modality

    def get_unused_fields(self):
        unused_fields = set(self.fields) - set(self.features) - \
            set(self.targets)
        return list(unused_fields)

    def get_dataset_filename(self):
        return os.path.join(self.path, 'dataset.json')

    def save(self):
        dataset_filename = self.get_dataset_filename()
        with open(dataset_filename, 'w') as dataset_file:
            json.dump(self.__dict__, dataset_file, indent=4)
        print("Saved dataset to ", dataset_filename)

    def load(self):
        dataset_filename = self.get_dataset_filename()
        with open(dataset_filename, 'r') as dataset_file:
            self.__dict__ = json.load(dataset_file)

    def save_simple_config(self):
        config_filename = os.path.join(self.path, 'dataset.cfg')
        with open(config_filename, 'w') as config:
            config.write(str(self.size) + '\n')
            config.write(str(self.train_size) + '\n')
            config.write(str(self.test_size) + '\n')
            config.write(str(self.count_features()) + '\n')
            config.write(str(self.count_modalities()) + '\n')
            for i in range(self.count_features()):
                config.write(self.features[i] + '\n')
            for i in range(self.count_features()):
                modalities = self.modalities[self.features[i]]
                for m in modalities:
                    config.write(str(m) + '\n')
            offsets = self.get_offsets()
            for o in offsets:
                config.write(str(o) + '\n')

    def get_feature_filename(self):
        return os.path.join(self.path, "features.dat")

    def get_target_filename(self, target):
        return os.path.join(self.path, "column_" + target + ".dat")

    def get_train_filename(self):
        return os.path.join(self.path, "train.dat")

    def get_test_filename(self):
        return os.path.join(self.path, "test.dat")

    def process(self, config):
        csv_filename = config['filename']
        data_transform = config['data_transform']
        data_filter = config['data_filter']
        data_train = config['data_train']
        features = config['features']
        targets = config['targets']

        if not features:
            raise Exception("No features found.")
        if not targets:
            raise Exception("No targets found.")

        print('Starting data importation from', csv_filename)
        nb_lines = count_line(csv_filename) - 1
        print("Importing", '{:,}'.format(nb_lines).replace(',', ' '), "lines.")

        delimiter = detect_csv_separator(csv_filename)

        nb_features = len(features)

        observations = np.empty((nb_lines, nb_features), np.dtype('u1'))
        target_data = [np.empty((nb_lines), np.dtype('float32'))
                       for t in targets]

        features_mapping = [{} for i in range(nb_features)]
        nb_fields = 0
        fields = []
        features_index = []
        targets_index = []
        nb_observations = 0
        train_set = []
        test_set = []
        random = (np.random.rand(nb_lines) < config['train_size'])

        with open(csv_filename) as csv_file:
            reader = csv.DictReader(csv_file, delimiter=delimiter)
            fields = [field.strip() for field in reader.fieldnames]
            nb_fields = len(fields)

            features_index = [fields.index(f) for f in features]
            if len(features_index) != nb_features:
                raise Exception("Invalid features")

            targets_index = [fields.index(t) for t in targets]
            if len(targets_index) != len(targets):
                raise Exception("Invalid targets")

            for i, row in enumerate(reader):
                if data_filter and not data_filter(row):
                    continue
                if data_transform:
                    data_transform(row)
                if (data_train(row) if data_train else random[i]):
                    train_set.append(nb_observations)
                else:
                    test_set.append(nb_observations)
                values = list(row.values())
                if len(values) != nb_fields:
                    raise Exception("Inconsistent number of fields",
                                    len(values), "in line", i + 1,
                                    "expecting", nb_fields)
                for j, index in enumerate(features_index):
                    v = values[index]
                    a = features_mapping[j].setdefault(v,
                                                       len(features_mapping[j])
                                                       )
                    if a > 200:
                        raise Exception("Feature", features[j],
                                        "has too many modalities " +
                                        "( more than 200).")
                    observations[nb_observations, j] = a
                for idx, t in enumerate(target_data):
                    t[nb_observations] = float(values[targets_index[idx]])
                if i % 1000 == 0 or i == nb_lines - 1:
                    printProgressBar(i, nb_lines - 1,
                                     prefix='Progress:',
                                     suffix='Complete',
                                     length=50)
                nb_observations += 1

        create_data_file_from_list(observations[:nb_observations, :],
                                   self.get_feature_filename(),
                                   np.dtype('u1'),
                                   (nb_observations, nb_features))
        for i, t in enumerate(targets):
            target = target_data[i]
            create_data_file_from_list(target[:nb_observations],
                                       self.get_target_filename(t),
                                       np.dtype('float32'),
                                       (nb_observations))
        create_data_file_from_list(train_set,
                                   self.get_train_filename(),
                                   np.dtype('int32'),
                                   (len(train_set)))
        create_data_file_from_list(test_set,
                                   self.get_test_filename(),
                                   np.dtype('int32'),
                                   (len(test_set)))

        modalities = {f: features_mapping[i] for i, f in enumerate(features)}

        # invert index and modality and return list of modalities
        for k, m in modalities.items():
            m = {v: k for k, v in m.items()}
            modalities[k] = [m[k] for k in sorted(m)]
            if len(modalities[k]) == 1:
                raise Exception("Feature", k, " has only one modality and is "
                                "therefore colinear to the intercept. Please "
                                "remove it from the dataset as it will cause "
                                "problems if included.")

        self.csv_filename = csv_filename
        self.size = nb_observations
        self.train_size = len(train_set)
        self.test_size = len(test_set)
        self.fields = fields
        self.features = features
        self.targets = targets
        self.set_modalities(modalities)


if __name__ == '__main__':
    if len(sys.argv) != 2:
        raise Exception("Invalid number of options, expecting only one : "
                        "[config filename].")
    filename = sys.argv[1]
    print("Processing config file :", filename)
    with open(filename) as config_file:
        config = json.load(config_file)
        context = {"math": math}
        exec(config["filter"], context)
        exec(config["transform"], context)
        exec(config["train"], context)
        config['data_transform'] = context['data_transform']
        config['data_filter'] = context['data_filter']
        config['data_train'] = context['data_train']
        dataset = Dataset(config['path'])
        dataset.process(config)
        dataset.save()
        dataset.save_simple_config()
