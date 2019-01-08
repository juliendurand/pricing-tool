# Generated by Django 2.1.4 on 2019-01-08 22:20

from django.db import migrations, models
import django.db.models.deletion


class Migration(migrations.Migration):

    dependencies = [
        ('pricing', '0008_auto_20190108_1451'),
    ]

    operations = [
        migrations.AlterField(
            model_name='model',
            name='features',
            field=models.ManyToManyField(related_name='features', to='pricing.Feature'),
        ),
        migrations.AlterField(
            model_name='model',
            name='target',
            field=models.ForeignKey(limit_choices_to='belong_to', on_delete=django.db.models.deletion.CASCADE, related_name='target', to='pricing.Feature'),
        ),
    ]
