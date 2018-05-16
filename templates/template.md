author: Julien Durand
version: 1
title: Technical Model Documentation

#{{title}}

Document written by {{author}}.

## Table of Content

[TOC]

## Dataset

TBW

## Model Selection

The gini curve below provides the gini for models with between 0 and 20 features :

{{result.get_gini_curve().to_html(classes=['table-condensed', 'table-striped'])}}

![Gini Curve]({{result.plot_gini_curve(path)}})

## Model Description

### Gini

The model has a gini of : {{result.gini()}}%.

### RMSE

The model has a root mean squared error of : {{result.rmse()}}.

### Lift Curve

![Lift Curve]({{result.plot_lift_curve(path, n_band=50)}})

<div class="pagebreak"></div>

### Relativities by feature

{% for feature in result.dataset.features %}

#### {{feature}}

{{result.calculate_relativities(feature).to_html(index=False, classes=['table-condensed', 'table-striped'])}}

![{{feature}} relativity chart]({{result.plot_relativities(feature, path)}})

<div class="pagebreak"></div>

{% endfor %}
