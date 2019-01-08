# Generated by Django 2.1.4 on 2018-12-18 22:34

from django.db import migrations, models
import pricing.models


class Migration(migrations.Migration):

    dependencies = [
        ('pricing', '0002_auto_20181218_2231'),
    ]

    operations = [
        migrations.AlterField(
            model_name='feature',
            name='status',
            field=models.CharField(choices=[('Input', 'Input'), ('Target', 'Target')], default=pricing.models.FeatureStatus('Input'), max_length=10),
        ),
        migrations.AlterField(
            model_name='model',
            name='loss',
            field=models.CharField(choices=[('Gaussian', 'Gaussian'), ('Logistic', 'Logistic'), ('Poisson', 'Poisson'), ('Gamma', 'Gamma')], default=pricing.models.LossFunction('Gaussian'), max_length=10),
        ),
    ]
