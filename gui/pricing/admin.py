from django.contrib import admin

from .models import Dataset, Model, Feature, Modality, Run

admin.site.register(Dataset)
admin.site.register(Model)
admin.site.register(Feature)
admin.site.register(Modality)
admin.site.register(Run)
