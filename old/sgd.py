from math import sqrt, log, exp

import numpy as np

import dataset


def moving_average(a, n=3) :
    cum = np.cumsum(a, dtype=float)
    ret = cum + 0
    for i in range(len(ret)):
        if i < n:
            ret[i] = cum[i] / (i+1)
        else:
            ret[i] = (cum[i] - cum[i-n])/n
    return ret


def dataset_iterator(filename, size, features, target, exposure=None, rolling=False):
    ds = dataset.Dataset(filename)
    data_array = [ds.get_feature(f).normalize() for f in features]
    target_array = ds.get_column(target)
    exposure_array = ds.get_column(exposure)
    i = 0
    while True:
        if not rolling and i >= size:
            break
        if i>= size:
            i = 0
        if exposure:
            yield (np.array([f[i] for f in data_array]), target_array[i], exposure_array[i])
        else:
            yield (np.array([f[i] for f in data_array]), target_array[i])
        i += 1

def fit_online(filename, features, target, size=4459542, fit_intercept=True, nb_adaptative_steps=1000, 
               moving_average_size=1000, family=None, p1=0, p2=0, max_iterations = 1000000, 
               exposure=False, average_size=100000):
    
    xy_iterator = dataset_iterator(filename, 4459542, features, target, exposure=exposure, rolling=True) 
    
    a = 0.1
    alphas = np.zeros(nb_adaptative_steps + 1)
    intercept = 0
    p = len(next(xy_iterator)[0])
    b = np.zeros(p)
    errors = np.zeros(max_iterations)
    exposure = 1
    
    for i in range(1, max_iterations):
        if exposure:
            x, y, exposure = next(xy_iterator)
        else:
            x, y = next(xy_iterator)
        y_pred = np.inner(x, b) + intercept + log(exposure)
        if family == 'poisson':
            y_pred = exp(y_pred)
        e = y_pred - y
        e_regularized = e + np.sum(p1 * np.abs(b) + p2 * np.square(b))
        if fit_intercept:
            intercept -= a * e * 10
        b -= (a * e) * x + p1 * np.sign(b) + p2 * b
        errors[i-1] = abs(e)

        if i <= nb_adaptative_steps:
            y_pred = np.inner(x, b) + intercept + log(exposure)
            if family == 'poisson':
                y_pred = exp(y_pred)
            e_updated = y_pred - y + np.sum(p1 * np.abs(b) + p2 * np.square(b))
            if abs(e_updated) > abs(e_regularized):
                a *= (1 - 0.999**min(i, 1000))
            else:
                a *= 1.1
            alphas[i-1] = a
            if i == nb_adaptative_steps:
                a = np.average(alphas)
                alphas[i] = a
            
        if i> 100000 and i % 10000 == 0:
            log_trailing_error = np.average(errors[i-moving_average_size:i])
            log_trailing_error_before = np.average(errors[i-max(i // 10, 10000):i-max(i // 10, 10000) + moving_average_size])
            if log_trailing_error >= log_trailing_error_before:
                break
    
    a = np.average(alphas)
    nb_betas = max(i, average_size)
    intercepts = 0
    betas = np.zeros(p)
    for j in range(nb_betas):
        if exposure:
            x, y, exposure = next(xy_iterator)
        else:
            x, y = next(xy_iterator)
        y_pred = np.inner(x, b) + intercept + log(exposure)
        if family == 'poisson':
            y_pred = exp(y_pred)
        e = y_pred - y
        if fit_intercept:
            intercept -= a * e
        b -= (a * e) * x + p1 * np.sign(b) + p2 * b
        intercepts += intercept
        betas += b 
    intercepts /= nb_betas
    betas  /= nb_betas
    
    return (i, intercepts, betas, alphas, errors[:i])