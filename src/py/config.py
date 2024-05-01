import os


class Config:
    '''
    Configuration for a glm regression.
    '''

    def __init__(self, config_filename):
        self.filename = config_filename
        with open(config_filename) as config_file:
            self.label = config_file.readline().strip()
            self.dataset_filename = config_file.readline().strip()
            self.name = config_file.readline().strip()
            self.loss = config_file.readline().strip()
            self.target = config_file.readline().strip()
            self.weight = config_file.readline().strip()
            self.nb_features = config_file.readline().strip()
            self.excludes = [x.strip() for x in config_file.readlines()]

    def get_dataset_filename(self):
        return self.dataset_filename

    def get_result_path(self):
        return os.path.join("result", self.name)

    def get_doc_path(self):
        doc_path = os.path.join("doc", self.name)
        if not os.path.exists(doc_path):
            os.makedirs(doc_path)
        return doc_path

    def get_doc_filename(self):
        doc_path = self.get_doc_path()
        get_doc_filename = os.path.join(doc_path, self.name + '.html')
        return get_doc_filename
