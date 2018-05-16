import csv
import itertools as it
import json
import math
import os
import sys
import time

import numpy as np


def count_line(filename):
    f = open(filename, 'rb')
    bufgen = it.takewhile(lambda x: x, (f.raw.read(1024 * 1024)
                                        for _ in it.repeat(None)))
    return sum(buf.count(b'\n') for buf in bufgen)


def detect_csv_separator(filename):
    """Utility function to automatically detect the separator character
    in a csv file."""
    with open(filename) as csvfile:
        first_line = csvfile.readline()
        return csv.Sniffer().sniff(first_line).delimiter


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
    return np.memmap(file_path, dtype=dtype, shape=shape)


start_time = 0


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
    print('\r%s |%s| %s%% %s in %sm%ss' % (prefix, bar, percent,
          suffix, m, s), end='')
    # Print New Line on Complete
    if iteration == total:
        print()


class Metadata:

    def __init__(self, path):
        self.path = path
        self.size = -1
        self.fields = None
        self.features = None
        self.modalities = None
        self.targets = None
        self.csv_filename = None

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

    def get_unused_fields(self):
        unused_fields = set(self.fields) - set(self.features) - \
            set(self.targets)
        return list(unused_fields)

    def get_metadata_filename(self):
        return os.path.join(self.path, 'metadata.json')

    def save(self):
        metadata_filename = self.get_metadata_filename()
        with open(metadata_filename, 'w') as metadata_file:
            json.dump(self.__dict__, metadata_file, indent=4)
        print("Saved metadata to ", metadata_filename)

    def load(self):
        metadata_filename = self.get_metadata_filename()
        with open(metadata_filename, 'r') as metadata_file:
            self.__dict__ = json.load(metadata_file)

    def save_simple_config(self):
        config_filename = os.path.join(self.path, 'metadata.cfg')
        with open(config_filename, 'w') as config:
            config.write(str(self.size) + '\n')
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

    def process(self, config):
        csv_filename = config['filename']
        data_transform = config['data_transform']
        data_filter = config['data_filter']
        features = config['features']
        targets = config['targets']
        self.csv_filename = csv_filename
        self.features = features
        if not features:
            raise Exception("No features found.")

        self.targets = targets
        if not targets:
            raise Exception("No targets found.")

        print('Starting data importation from', csv_filename)
        nb_lines = count_line(csv_filename) - 1

        nb_features = len(features)

        data_filename = self.get_feature_filename()
        observations = np.empty((nb_lines, nb_features), np.dtype('u1'))

        target_filenames = [self.get_target_filename(t) for t in targets]
        target_data = [np.empty((nb_lines), np.dtype('float32'))
                       for f in target_filenames]

        features_mapping = [{} for i in range(nb_features)]

        print("Importing", '{:,}'.format(nb_lines).replace(',', ' '), "lines.")

        nb_fields = 0
        features_index = []
        targets_index = []
        sep = detect_csv_separator(csv_filename)
        with open(csv_filename) as csv_file:
            line = csv_file.readline()[:-1]
            self.fields = [s.strip() for s in line.split(sep)]
            nb_fields = len(self.fields)

            features_index = [self.fields.index(f) for f in self.features]
            if len(features_index) != nb_features:
                raise Exception("Invalid features")

            targets_index = [self.fields.index(t) for t in self.targets]
            if len(targets_index) != len(self.targets):
                raise Exception("Invalid targets")

        nb_observations = 0
        with open(csv_filename) as csv_file:
            reader = csv.DictReader(csv_file)
            for i, row in enumerate(reader):
                if data_filter and not data_filter(row):
                    continue
                if data_transform:
                    data_transform(row)
                values = list(row.values())
                if len(values) != nb_fields:
                    raise Exception("Inconsistent number of fields",
                                    len(values), "in line", i + 1,
                                    "expecting", nb_fields)
                for j, index in enumerate(features_index):
                    v = values[index]
                    a = features_mapping[j].setdefault(v, \
                        len(features_mapping[j]))
                    if a > 200:
                        raise Exception("Feature", self.features[j],
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
                                   data_filename,
                                   np.dtype('u1'),
                                   (nb_observations, nb_features))
        for i, target in enumerate(target_data):
            create_data_file_from_list(target[:nb_observations],
                                       target_filenames[i],
                                       np.dtype('float32'),
                                       (nb_observations))

        modalities = {f: features_mapping[i] for i, f in
                      enumerate(self.features)}
        # invert index and modality and return list of modalities
        for k, m in modalities.items():
            m = {v: k for k, v in m.items()}
            modalities[k] = [m[k] for k in sorted(m)]
            if len(modalities[k]) == 1:
                raise Exception("Feature", k, " has only one modality and is "
                                "therefore colinear to the intercept. Please "
                                "remove it from the dataset as it will cause "
                                "problems if included.")

        self.size = nb_observations
        self.set_modalities(modalities)
        self.save()
        self.save_simple_config()


if __name__ == '__main__':
    if len(sys.argv) != 2:
        raise Exception("Invalid number of options, expecting only one : " \
                        "[config filename].")
    filename = sys.argv[1]
    print("Processing congif file :", filename)
    with open(filename) as config_file:
        config = json.load(config_file)
        context = {"math": math}
        exec(config["filter"], context)
        exec(config["transform"], context)
        config['data_transform'] = context['data_transform']
        config['data_filter'] = context['data_filter']
        dataset = Metadata(config['path'])
        dataset.process(config)
