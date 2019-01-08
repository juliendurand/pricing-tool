from enum import Enum

from django.db import models
from django.urls import reverse
from django.utils.text import slugify

import pypricing.dataset as ds


class FeatureStatus(Enum):
    Input = "Input"
    Target = "Target"
    Ignore = "Ignore"

    def __str__(self):
        return self.value


class LossFunction(Enum):
    Gaussian = "Gaussian"
    Logistic = "Logistic"
    Poisson = "Poisson"
    Gamma = "Gamma"

    def __str__(self):
        return self.value

fn_filter = "def data_filter(row):\n    return row['HAB_hab'] == 'A'\n"
fn_transform = "def data_transform(row):\n    row['POL_mtcapass'] = int(math.log(float(row['POL_mtcapass'])))\n"
fn_train = "def data_train(row):\n    return row['Random'] != '9' and row['Random'] != '10'\n"


class Dataset(models.Model):
    timestamp = models.DateTimeField(auto_now_add=True)
    name = models.CharField(max_length=200, unique=True)
    csvfile = models.FileField(upload_to='data/')
    train_size = models.BigIntegerField()
    test_size = models.BigIntegerField()
    fn_transform = models.TextField(default=fn_transform)
    fn_filter = models.TextField(default=fn_filter)
    fn_train = models.TextField(default=fn_train)

    def get_absolute_url(self):
        return reverse("dataset_detail", kwargs={'pk': self.pk})

    def get_data_filename(self):
        return str(self.csvfile)

    def get_number_lines(self):
        return ds.count_line(self.get_data_filename())

    def get_fields(self):
        return ds.get_fields(self.get_data_filename())

    def get_config(self):
        config = {
            'filename': str(self.csvfile),
            'transform': str(self.fn_transform),
            'filter': str(self.fn_filter),
            'train': str(self.fn_train),
            'features': [f.name for f in
                         self.feature_set.filter(status=FeatureStatus.Input)],
            'targets': [f.name for f in
                        self.feature_set.filter(status=FeatureStatus.Target)],
            'path': './dataset/' + slugify(self.name),
        }
        return config

    def __str__(self):
        return self.name


class Feature(models.Model):
    dataset = models.ForeignKey(Dataset, on_delete=models.CASCADE)
    name = models.CharField(max_length=50)
    status = models.CharField(
        max_length=10,
        blank=False,
        default=FeatureStatus.Input,
        choices=[(tag.name, tag.value) for tag in FeatureStatus]
    )

    class Meta:
        ordering = [models.functions.Lower('name')]

    def __str__(self):
        return self.dataset.name + ' / ' + self.name


class Modality(models.Model):
    feature = models.ForeignKey(Feature, on_delete=models.CASCADE)
    value = models.CharField(max_length=50)

    def __str__(self):
        return self.feature.dataset.name + ' / ' + self.feature.name \
            + ' / ' + self.value


class Model(models.Model):
    timestamp = models.DateTimeField(auto_now_add=True)
    name = models.CharField(max_length=200, unique=True)
    dataset = models.ForeignKey(Dataset, on_delete=models.CASCADE)
    loss = models.CharField(
        max_length=10,
        blank=False,
        default=LossFunction.Gaussian,
        choices=[(tag.name, tag.value) for tag in LossFunction]
    )
    features = models.ManyToManyField(Feature, related_name='features')
    target = models.ForeignKey(Feature,
                               on_delete=models.CASCADE,
                               related_name='target',
                               )
    weight = models.ForeignKey(Feature,
                               on_delete=models.CASCADE,
                               related_name='weight',
                               )

    def get_absolute_url(self):
        return reverse("model_detail", kwargs={'pk': self.object.pk})

    def __str__(self):
        return self.name


class Run(models.Model):
    timestamp = models.DateTimeField(auto_now_add=True)
    model = models.ForeignKey(Model, on_delete=models.CASCADE)
