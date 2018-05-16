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

TBW

## Model Description

### Gini

The model has a gini of : {{results.gini()}}%.

### RMSE

The model has a root mean squared error of : {{results.rmse()}}.

### Lift Curve

![Lift Curve]({{results.plot_lift_curve(path, n_band=50)}})

<div class="pagebreak"></div>

### Relativities by feature

{% for feature in results.metadata.features %}

#### {{feature}}

{{results.calculate_relativities(feature).to_html(index=False, classes=['table-condensed', 'table-striped'])}}

![{{feature}} relativity chart]({{results.plot_relativities(feature, path)}})

<div class="pagebreak"></div>

{% endfor %}
