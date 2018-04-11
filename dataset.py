import contextlib
import json
import mmap
import os
from struct import Struct

import pandas as pd
import numpy as np


class Feature:
    
    def __init__(self, name, filename, mode, size, ftype, min_value, max_value):
        self.name = name
        self.filename = filename
        self.size = size
        self.type = ftype
        self.values = {}
        self.min_value = float(min_value)
        self.max_value = float(max_value)
        self.count = 0
        
        if mode not in ('r', 'w',):
            raise Exception("Invalid mode for feature, should be 'r' for reading or 'w' for writing.")
        
        self.array = np.memmap(filename, mode=mode + '+', dtype=self.get_dtype(), shape=(self.size,))
    
    def __repr__(self):
        return str((self.get_name(), self.get_type(), self.size, self.values, self.min_value, self.max_value, self.count))
        
    def get_name(self):
        return self.name
                            
    def get_filename(self):
        return self.filename
        
    def get_type(self):
        return self.type
                            
    def get_dtype(self):
        t = self.get_type()
        if 'float' in t:
            return 'float32'
        elif 'int' in t:
            if abs(self.min_value) < 100 and abs(self.max_value) < 100:
                return 'int8'
            elif abs(self.min_value) < 10000 and abs(self.max_value) < 10000:
                return 'int16'
            return 'int32'
        elif 'object' in t:
            if self.count < 200:
                return 'int8'
            elif self.count < 10000:
                return 'int16'
            else:
                return 'int32'
        else:
            raise Exception("incompatible type for feature: " + self.name)
            
    def should_skip(self):
        return self.count > 300
    
    def cast_float(self, value):
        result = self.values.get(value)
        if result is None:
            result = float(value)
            self.values[value] = result
        return result
    
    def cast_int(self, value):
        result = self.values.get(value)
        if result is None:
            result = int(float(value))
            self.values[value] = result
        return result
    
    def cast_object(self, value):
        result = self.values.get(value)
        if result is None:
            result = len(self.values)
            self.values[value] = result
        return result
            
    def get_cast(self):
        if self.type == 'float':
            return self.cast_float
        elif self.type == 'int':
            return self.cast_int
        elif self.type == 'object':
            return self.cast_object
        else:
            raise Exception("Unknown type for feature: ", name)
            
    def normalize(self):
        return self.array / np.max(self.array)
    
    def open(self):
        pass
                            
    def flush(self):
        if self.get_type() in ('float', 'int',):
            self.values = {}
            self.min_value = float(np.min(self.array))
            self.max_value = float(np.max(self.array))
   
    def close(self):
        if hasattr(self, 'array'):
            self.flush()
            self.array._mmap.close()
            del self.array            
    
class Dataset:
                            
    filename = 'dataset.json'
    
    def __init__(self, folder):
        self.folder = folder
        self.csv_filename = ''
        self.features = []
        self.features_index = {}
        self.size = 0
        data_folder_path = os.path.join(folder, 'data')
        if not os.path.exists(data_folder_path):
            os.makedirs(data_folder_path)
        if os.path.exists(os.path.join(folder, self.filename)):
            self.open()
            
    def __repr__(self):
        return '\n'.join(map(str, self.features))
    
    def __getitem__(self, key):
        idx = self.features_index.get(key, None)
        return self.features[idx] if idx is not None else None

    def __setitem__(self, key, value):
        raise Exception('Features are not mutable : this operation is not allowed')           
        
    def add_feature(self, feature):
        i = len(self.features)
        self.features.append(feature)
        self.features_index[feature.get_name()] = i
    
    def open(self):
        with open(os.path.join(self.folder, self.filename), 'r') as in_file:
            content = json.load(in_file)
            self.csv_filename = content['csv_filename']
            self.csv_size = content['size']
            for f in content['features']:
                self.add_feature(Feature(f['name'], f['filename'], 'r', f['size'], f['type'], f['min_value'], f['max_value']))
    
    def save(self):
        self.flush()
        content = {
            'csv_filename': self.csv_filename,
            'features':  [{
                    'name': f.get_name(),
                    'filename': f.get_filename(),
                    'size': f.size,
                    'type': f.get_type(),
                    'values': f.values,
                    'min_value': f.min_value,
                    'max_value': f.max_value,
                    'count': f.count,
             } for f in self.features],
            'size': self.size,
        }
        with open(os.path.join(self.folder, self.filename), 'w+') as out_file:
            json.dump(content, out_file, sort_keys=True, indent=4)
        print('saved in:', os.path.join(self.folder, self.filename))
        self.close()
    
    def get_ftype(self, dtype_name):
        if 'float' in dtype_name:
            return 'float'
        elif 'int' in dtype_name:
            return 'int'
        elif 'object' in dtype_name:
            return 'object'
        else:
            raise Exception('Unknown type for feature: ', sel.name, '.')
    
    def load_csv(self, csv_filename):
        path = os.path.join('.', self.folder, "data")
        filelist = [f for f in os.listdir(path) if f.endswith(".npy")]
        for f in filelist:
            os.remove(os.path.join(path, f))
        self.csv_filename = csv_filename
        with open(self.csv_filename, 'r') as csv:
            i = 0
            for line in csv:
                i += 1
            self.size = i - 1
        df = pd.read_csv(csv_filename, nrows=10000)  
        min_value = None
        max_value = None
        for name in list(df):
            if 'object' not in df[name].dtype.name:
                min_value = df[name].min()
                max_value = df[name].max()
            feature = Feature(name, os.path.join(path, name + '.npy'), 'w', 
                             self.size, self.get_ftype(df[name].dtype.name),
                             min_value, max_value)
            if feature.get_type() == 'object':
                feature.count = len(df[name].unique())
            self.add_feature(feature)

        for f in self.features:
            if f.should_skip():
                print('Skipping feature: ', f.get_name(),
                      '. This feature is not loaded because it contains too many different values.')
        conv_features = [(i, f.get_cast(), f.array) for i, f in enumerate(self.features) if not f.should_skip()]
        with open(csv_filename, 'r') as csv:
            csv.readline()  # skip header
            i = 0  
            for line in csv:
                try:
                    values = line[:-1].split(',')
                    for j, cast, array in conv_features:
                        array[i] = cast(values[j])
                    i += 1
                    if i % 100000 == 0:
                        print(i, ' lines converted.')
                except Exception as e:
                    print(e)
                    names = [f.name for f in self.features if not f.should_skip()]
                    print(i, list(zip(names, self.get_binary_format()[1:], values)))
                    raise e
            print('Loaded ', i, ' lines from ', csv_filename, '.')
        self.save()
        self.open() 
        
    def flush(self):
        for f in self.features:
            f.flush()
        
    def close(self):
        for f in self.features:
            f.close()
        self.feature = []
        self.features_index = {} 
