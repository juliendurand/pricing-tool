from django.forms import modelform_factory, modelformset_factory, formset_factory, inlineformset_factory, ModelForm
from django.http import HttpResponseRedirect
from django.shortcuts import render
from django.urls import reverse, reverse_lazy
from django.views import generic

from .models import Dataset, Feature, FeatureStatus, Model, Run

import pypricing.dataset as ds

from . import tasks


def index(request):
    context = {}
    return render(request, 'pricing/index.html', context)


class DatasetListView(generic.ListView):
    model = Dataset
    nav = 'dataset'


class DatasetDetailView(generic.DetailView):
    model = Dataset
    nav = 'dataset'


class DatasetCreateView(generic.CreateView):
    model = Dataset
    fields = ['name', 'csvfile', 'fn_transform', 'fn_filter', 'fn_train']
    nav = 'dataset'

    def get_success_url(self):
        return self.model.get_absolute_url(self.object)

    def form_valid(self, form):
        self.object = form.save(commit=False)
        self.object.train_size = 100
        self.object.test_size = 10
        self.object.save()
        fields = ds.get_fields(self.object.get_data_filename())
        for field, field_status in fields:
            try:
                status = FeatureStatus(field_status)
            except Exception:
                status = FeatureStatus.Ignore
            f = Feature(dataset=self.object, name=field, status=status)
            f.save()
        return HttpResponseRedirect(self.get_success_url())


class DatasetDeleteView(generic.DeleteView):
    model = Dataset
    success_url = reverse_lazy('dataset_list')


def dataset_process(request, pk):
    tasks.process_dataset(pk)
    return HttpResponseRedirect(reverse('dataset_list'))


class FeatureUpdateView(generic.UpdateView):
    model = Feature
    fields = ['name', 'status']

    def get_success_url(self):
        return Dataset.get_absolute_url(self.object.dataset)


class ModelListView(generic.ListView):
    model = Model
    nav = 'model'


class ModelDetailView(generic.DetailView):
    model = Model
    nav = 'model'


class ModelCreateView(generic.CreateView):
    model = Model
    fields = ['name', 'dataset', 'loss', 'target', 'weight', ]
    nav = 'model'

    def get_success_url(self):
        return self.model.get_absolute_url(self)

    def form_valid(self, form):
        self.object = form.save(commit=False)
        self.object.save()
        return HttpResponseRedirect(self.get_success_url())


class ModelDeleteView(generic.DeleteView):
    model = Model
    success_url = reverse_lazy('model_list')


class RunDetailView(generic.DetailView):
    model = Run
    nav = 'model'
