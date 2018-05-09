import csv
import itertools as it
import json
import os
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


def create_data_file(out_filename, dtype, shape):
    """
    Args:
        out_filename: The name of the binary file. It must be in the same
            directory.
        dtype: The type of the numpy array.
    """
    out_file = open(out_filename, 'wb+')
    dat_file = np.memmap(out_file, dtype=dtype, shape=shape)
    return dat_file


def save_data_file(file, filename):
    file.flush()
    size = float(file.nbytes) / (1024 ** 2)
    print('written %s : %.3f MB' % (filename, size))


def create_data_file_from_list(lst, out_filename, dtype):
    """Write a list in a binary file as a numpy array.
    Args:
        lst: The list that will be written in the file.
        out_filename: The name of the binary file. It must be in the same
            directory.
        dtype: The type of the numpy array.
    """
    with open(out_filename, 'wb+') as out_file:
        dat_file = np.memmap(out_file, dtype=dtype, shape=(len(lst),))
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
          suffix, m, s), end='\r')
    # Print New Line on Complete
    if iteration == total:
        print()


class Metadata:

    def __init__(self, path, name):
        self.path = path
        self.name = name
        self.size = -1
        self.fields = None
        self.features = None
        self.modalities = None
        self.targets = None
        self.exposure = None
        self.csv_filename = None

        folder = os.path.join(path, name)
        if not os.path.exists(folder):
            os.makedirs(folder)

    def set_size(self, size):
        self.size = size

    def set_fields(self, fields):
        self.fields = fields

    def set_features(self, features):
        self.features = features

    def set_targets(self, targets):
        self.targets = targets

    def set_exposure(self, exposure):
        self.exposure = exposure

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
            set(self.targets) - set([self.exposure])
        return list(unused_fields)

    def get_metadata_filename(self):
        return os.path.join(self.path, self.name, 'metadata.json')

    def save(self):
        metadata_filename = self.get_metadata_filename()
        with open(metadata_filename, 'w') as metadata_file:
            json.dump(self.__dict__, metadata_file, indent=4)
        print("Saved metadata to ", metadata_filename)

    def load(self):
        metadata_filename = self.get_metadata_filename()
        with open(metadata_filename, 'r') as metadata_file:
            self.__dict__ = json.load(metadata_file)
        # print("Loaded metadata from ", metadata_filename)

    def save_simple_config(self):
        config_filename = os.path.join(self.path, self.name, 'metadata.cfg')
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
        return os.path.join(self.path, self.name, "features.dat")

    def get_exposure_filename(self):
        return os.path.join(self.path, self.name, "exposure.dat")

    def get_target_filename(self, target):
        return os.path.join(self.path, self.name, "target_" + target + ".dat")

    def process(self, csv_filename, data_transform=None):
        self.csv_filename = csv_filename
        features = self.features
        if not features:
            raise Exception("No features found.")
        targets = self.targets
        if not targets:
            raise Exception("No targets found.")
        exposure = self.exposure
        if not features:
            raise Exception("No exposure found.")
        name = self.name
        if not name:
            raise Exception("Missing dataset name.")

        print('Starting data importation.')
        sep = detect_csv_separator(csv_filename)
        nb_lines = count_line(csv_filename) - 1
        self.size = nb_lines

        nb_features = len(features)

        data_filename = self.get_feature_filename()
        observations = create_data_file(data_filename, np.dtype('u1'),
                                        (nb_lines, nb_features))

        exposure_filename = self.get_exposure_filename()
        exposure_data = create_data_file(exposure_filename,
                                         np.dtype('float32'), (nb_lines))

        target_filenames = [self.get_target_filename(t) for t in targets]
        target_data = [create_data_file(f, np.dtype('float32'), (nb_lines))
                       for f in target_filenames]

        features_mapping = [{} for i in range(nb_features)]

        print("Importing", '{:,}'.format(nb_lines).replace(',', ' '), "lines.")
        with open(csv_filename) as csv_file:
            line = csv_file.readline()[:-1]
            self.fields = [s.strip() for s in line.split(sep)]
            nb_fields = len(self.fields)

            features_index = [self.fields.index(f) for f in self.features]
            if len(features_index) != nb_features:
                raise Exception("Invalid features")

            exposure_index = [i for i in range(nb_fields) if
                              self.fields[i] == self.exposure]
            if len(exposure_index) != 1:
                raise Exception("Invalid Exposure field.")
            exposure_index = exposure_index[0]

            targets_index = [self.fields.index(t) for t in self.targets]
            if len(targets_index) != len(self.targets):
                raise Exception("Invalid targets")

            for i, line in enumerate(csv_file):
                if i >= nb_lines:
                    break
                line = line[:-1]
                values = [s.strip() for s in line.split(sep)]
                if len(values) != nb_fields:
                    raise Exception("Inconsistent number of fields",
                                    len(values), "in line", i + 1,
                                    "expecting", nb_fields)
                for j, index in enumerate(features_index):
                    v = values[index]
                    if data_transform:
                        v = data_transform(self.features[j], v)
                    a = features_mapping[j].setdefault(v, \
                        len(features_mapping[j]))
                    if a > 200:
                        raise Exception("Feature", self.features[j],
                                        "has too many modalities " +
                                        "( more than 200).")
                    observations[i, j] = a
                exposure_data[i] = values[exposure_index]
                for idx, t in enumerate(target_data):
                    t[i] = values[targets_index[idx]]

                if i % 1000 == 0 or i == nb_lines - 1:
                    progressbar.printProgressBar(i, nb_lines - 1,
                                                 prefix='Progress:',
                                                 suffix='Complete',
                                                 length=50)
        save_data_file(observations, data_filename)
        save_data_file(exposure_data, exposure_filename)
        for i in range(len(target_data)):
            save_data_file(target_data[i], target_filenames[i])

        modalities = {f: features_mapping[i] for i, f in
                      enumerate(self.features)}
        # invert index and modality and return list of modalities
        for k, m in modalities.items():
            m = {v: k for k, v in m.items()}
            modalities[k] = [m[k] for k in sorted(m)]
            if len(modalities[k]) == 1:
                raise Exception("Feature", k, " has only one modality and it"
                                "therefore colinear to the intercept. Please "
                                "remove it from the dataset as it will cause "
                                "problems if included.")

        self.set_modalities(modalities)
        self.save()
