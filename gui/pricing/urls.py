
from django.urls import path

from . import views

urlpatterns = [
    path('', views.index, name='index'),

    path('dataset/', views.DatasetListView.as_view(),
         name='dataset_list'),
    path('dataset/<int:pk>/', views.DatasetDetailView.as_view(),
         name='dataset_detail'),
    path('dataset/create', views.DatasetCreateView.as_view(),
         name='dataset_create'),
    path('dataset/<int:pk>/delete/', views.DatasetDeleteView.as_view(),
         name='dataset_delete'),
    path('dataset/<int:pk>/process/', views.dataset_process,
         name='dataset_process'),

    path('feature/update/<int:pk>/', views.FeatureUpdateView.as_view(),
         name='feature_update'),

    path('model/', views.ModelListView.as_view(),
         name='model_list'),
    path('model/<int:pk>/', views.ModelDetailView.as_view(),
         name='model_detail'),
    path('model/create', views.ModelCreateView.as_view(),
         name='model_create'),
    path('model/<int:pk>/update/', views.ModelUpdateView.as_view(),
         name='model_update'),
    path('model/<int:pk>/delete/', views.ModelDeleteView.as_view(),
         name='model_delete'),
    path('model/<int:pk>/run/', views.run,
         name='run'),

    path('feature/switch/<int:pk>/<int:mf>/', views.switchFeature,
         name='switch'),

    path('run/<int:pk>/', views.RunDetailView.as_view(),
         name='run_detail'),
]
