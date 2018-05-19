author: Julien Durand
version: 1
title: Technical Model Documentation

#{{title}}

Document written by {{author}}.

## Table of Content

[TOC]

<div class="pagebreak"></div>

## Dataset

<table style="width: 500px;">
    <tr>
        <th></th>
        <th>Modeling Data</th>
        <th>Test Data</th>
    </tr>
    {% if result.config.loss == "poisson" %}
    <tr>
        <td># of observations</td>
        <td>{{result.train_data.shape[0] | thsep}}</td>
        <td>{{result.test_data.shape[0] | thsep}}</td>
    </tr>
    <tr>
        <td>Exposure</td>
        <td>{{result.weight[result.train_data_index].sum() | thsep}}</td>
        <td>{{result.weight[result.test_data_index].sum() | thsep}}</td>
    </tr>
    <tr>
        <td># of claims</td>
        <td>{{result.target[result.train_data_index].sum() | thsep}}</td>
        <td>{{result.target[result.test_data_index].sum() | thsep}}</td>
    </tr>
    <tr>
        <td>Avg Frequency</td>
        <td>{{(np.float(result.target[result.train_data_index].sum()) / np.float(result.weight[result.train_data_index].sum())) | percent}}</td>
        <td>{{(np.float(result.target[result.test_data_index].sum()) /
        np.float(result.weight[result.test_data_index].sum())) | percent}}</td>
    </tr>
    {% endif %}
    {% if result.config.loss == "gamma" %}
    <tr>
        <td># of observations</td>
        <td>{{result.train_data_index.shape[0] | thsep}}</td>
        <td>{{result.test_data_index.shape[0] | thsep}}</td>
    </tr>
    <tr>
        <td># of claims</td>
        <td>{{result.weight[result.train_data_index].sum() | thsep}}</td>
        <td>{{result.weight[result.test_data_index].sum() | thsep}}</td>
    </tr>
    <tr>
        <td>Min claim</td>
        <td>{{(result.target[result.train_data_index] / result.weight[result.train_data_index]).min() | euro}}</td>
        <td>{{(result.target[result.test_data_index] /
        result.weight[result.test_data_index]).min() | euro}}</td>
    </tr>
    <tr>
        <td>Average claim</td>
        <td>{{(np.float(result.target[result.train_data_index].sum()) / np.float(result.weight[result.train_data_index].sum())) | euro}}</td>
        <td>{{(np.float(result.target[result.test_data_index].sum()) /
        np.float(result.weight[result.test_data_index].sum())) | euro}}</td>
    </tr>
    <tr>
        <td>Max claim</td>
        <td>{{(result.target[result.train_data_index] / result.weight[result.train_data_index]).max() | euro}}</td>
        <td>{{(result.target[result.test_data_index] /
        result.weight[result.test_data_index]).max() | euro}}</td>
    </tr>
    {% endif %}
</table>

## Model Selection

### Gini Curve

The gini curve below provides the gini for models with between 0 and 20 features :

![Gini Curve]({{result.plot_gini_curve(path)}})

### Features

{{result.get_gini_curve(result.dataset.size).to_html(classes=['table-condensed', 'table-striped'])}}

<div class="pagebreak"></div>

## Selected Model ({{result.config.nb_features}} Features)

This section details the metrics for the selected model with {{result.config.nb_features}} features.

### Gini

The model has a gini of : {{result.gini()}}%.

### RMSE

The model has a root mean squared error of : {{result.rmse()}}.

### Features sorted by Gini Contribution

{{result.get_gini_curve(result.config.nb_features).to_html(classes=['table-condensed', 'table-striped'])}}

### Features sorted by Spread 100/0

{{result.get_gini_curve(result.config.nb_features).sort_values('Spread 100/0', ascending=False).to_html(classes=['table-condensed', 'table-striped'])}}

### Features sorted by Spread 95/5

{{result.get_gini_curve(result.config.nb_features).sort_values('Spread 95/5', ascending=False).to_html(classes=['table-condensed', 'table-striped'])}}

<div class="pagebreak"></div>

### Lift Curve

![Lift Curve]({{result.plot_lift_curve(path, n_band=50)}})

<div class="pagebreak"></div>

### Relativities by feature

{% for feature in result.gini_curve.Feature[1:] %}

#### {{feature}}

This feature is #{{loop.index}} and is {% if loop.index > np.int(result.config.nb_features) %}**not** included{% else %}**included**{% endif %} included in the selected model.

{{result.calculate_relativities(feature).to_html(index=False, classes=['table-condensed', 'table-striped'])}}

![{{feature}} relativity chart]({{result.plot_relativities(feature, path)}})

<div class="pagebreak"></div>

{% endfor %}
