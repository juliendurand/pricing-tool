"""
Contains functions that are able to reproduce Emblem's metrics and plots.
Each function in this module has at least three arguments, which are numpy arrays of the same size

``function(y, y_pred, weights = None, **kwargs)``
where ``**kwargs`` are **named** extra-parameters.

"""

import numpy as np
from scipy import interpolate
import pandas as pd
import matplotlib.pyplot as plt


def check_deviance(y, y_pred, weight=None):
    """
    Robust checks to run at beginning of deviance
    """
    if isinstance(y_pred, (np.floating, float)):
        y_pred = np.repeat(y_pred,y.shape[0])
    assert y.shape[0] == y_pred.shape[0], "y and y_pred must have the same size"
    if weight is not None:
        assert weight.shape[0] == y.shape[0], "weight and y do not have same shape"
    return y_pred


def binomial_deviance(y, y_pred,weight=None):
    """
    Variance for the binomial model

    Parameters
    ----------

    y : ndarray
        array containing the TRUE response (either 0 or 1)
    y_pred : ndarray
        array containing the predicted probabilities by the model
    weight : ndarray, optional
        array containing the weight (default 1)

    Returns
    -------
    ndarray
        computed deviance

    """
    y_pred = check_deviance(y, y_pred, weight=weight)
    deviance_vector = - (y * np.log(y_pred) + (1-y) * np.log(1 - y_pred))
    if weight is not None:
        deviance_vector = np.dot(weight, deviance_vector)
    return 2 * np.sum(deviance_vector)


def gamma_deviance(y, y_pred, weight=None):
    """
    Deviance function for gamma model.
    Exactly the same as the one implement in Emblem.

    Parameters
    ----------

    y : ndarray
        array containing the TRUE response (either 0 or 1)
    y_pred : ndarray
        array containing the predicted probabilities by the model
    weight : ndarray, optional
        array containing the weight (default 1)

    Returns
    -------
    ndarray
        computed deviance


    """
    y_pred = check_deviance(y, y_pred, weight=weight)

    deviance_vector = -np.log(y/y_pred) + (y-y_pred)/y_pred
    if weight is not None:
        deviance_vector = np.dot(weight, deviance_vector)

    return 2 * np.sum(deviance_vector)


def poisson_deviance(y, y_pred, weight=None):
    """
    Deviance function for the poisson model.

    Parameters
    ----------

    y : ndarray
        array containing the TRUE response (either 0 or 1)
    y_pred : ndarray
        array containing the predicted probabilities by the model
    weight : ndarray, optional
        array containing the weight (default 1)

    Returns
    -------
    ndarray
        computed deviance

    """
    y_pred = check_deviance(y, y_pred, weight=weight)

    bool_zeros = y != 0
    deviance_vector = np.zeros(y.shape[0])
    deviance_vector[bool_zeros] = (y[bool_zeros] * np.log(y[bool_zeros] / y_pred[bool_zeros]) - y[bool_zeros] + y_pred[bool_zeros])
    deviance_vector[~bool_zeros] = - y[~bool_zeros] + y_pred[~bool_zeros]
    if weight is not None:
        deviance_vector = np.dot(weight, deviance_vector)

    return 2 * np.sum(deviance_vector)


def gaussian_deviance(y, y_pred, weight=None):
    """
    Deviance function for the gaussian/least squares model.

    Parameters
    ----------

    y : ndarray
        array containing the TRUE response (either 0 or 1)
    y_pred : ndarray
        array containing the predicted probabilities by the model
    weight : ndarray, optional
        array containing the weight (default 1)

    Returns
    -------
    ndarray
        computed deviance

    """
    y_pred = check_deviance(y, y_pred, weight=weight)
    deviance_vector = np.square(y - y_pred)
    if weight is not None:
        deviance_vector = np.dot(weight, deviance_vector)
    return 0.5 * np.sum(deviance_vector)


def gaussian_pseudo_r2(y, y_pred, weight=None):
    """
    PseudoR2 for a Normal model.

    Parameters
    ----------

    y : ndarray
        array containing the TRUE response (either 0 or 1)
    y_pred : ndarray
        array containing the predicted probabilities by the model
    weight : ndarray, optional
        array containing the weight (default 1)

    Returns
    -------
    ndarray
        computed pseudo_R2

    Notes
    -----
    Pseudo R2 is defined as 1 - (deviance(y,y_pred,weight)}deviance(y,mu,weight)
    where mu is the weighted mean of y

    """
    return 1 - (gaussian_deviance(y, y_pred, weight)
                / gaussian_deviance(y, np.ones(len(y))*np.average(y, weights=weight), weight))


def poisson_pseudo_r2(y, y_pred, weight=None):
    """
    PseudoR2 for the Poisson model.

    Parameters
    ----------

    y : ndarray
        array containing the TRUE response (either 0 or 1)
    y_pred : ndarray
        array containing the predicted probabilities by the model
    weight : ndarray, optional
        array containing the weight (default 1)

    Returns
    -------
    ndarray
        computed pseudo_R2

    Notes
    -----
    Pseudo R2 is defined as 1 - (deviance(y,y_pred,weight)}deviance(y,mu,weight)
    where mu is the weighted mean of y

    """
    return 1 - (poisson_deviance(y, y_pred, weight)
                / poisson_deviance(y, np.ones(len(y))*np.average(y, weights=weight), weight))


def gamma_pseudo_r2(y, y_pred, weight=None):
    """
    Pseudo R2 for gamma model

    Parameters
    ----------

    y : ndarray
        array containing the TRUE response (either 0 or 1)
    y_pred : ndarray
        array containing the predicted probabilities by the model
    weight : ndarray, optional
        array containing the weight (default 1)

    Returns
    -------
    ndarray
        computed pseudo_R2

    Notes
    -----
    Pseudo R2 is defined as 1 - (deviance(y,y_pred,weight)}deviance(y,mu,weight)
    where mu is the weighted mean of y

    """
    return 1 - (gamma_deviance(y, y_pred, weight)
                / gamma_deviance(y, np.ones(len(y))*np.average(y, weights=weight), weight))


def binomial_pseudo_r2(y, y_pred, weight=None):
    """
    PseudoR2 for the binomial model


    Parameters
    ----------

    y : ndarray
        array containing the TRUE response (either 0 or 1)
    y_pred : ndarray
        array containing the predicted probabilities by the model
    weight : ndarray, optional
        array containing the weight (default 1)

    Returns
    -------
    ndarray
        computed pseudo_R2

    Notes
    -----
    Pseudo R2 is defined as 1 - (deviance(y,y_pred,weight)}deviance(y,mu,weight)
    where mu is the weighted mean of y

    """
    return 1 - (binomial_deviance(y, y_pred, weight)
                / binomial_deviance(y, np.ones(len(y))*np.average(y, weights=weight), weight))


def area_lorentz_fast(y, y_pred, weight=None, resolution=5000, interpolation="constant", plot=False):
    '''
    Reproduces the weighted gini of emblem

    Parameters
    ----------

    y : ndarray
        array containing the TRUE response (either 0 or 1)
    y_pred : ndarray
        array containing the predicted probabilities by the model
    weight : ndarray, optional
        array containing the weight (default 1)
    resolution : int, optional
        the number of points in the plot of the lorenz curve (default is 5000)
    interpolation: {'linear', 'constant'}, optional
        type of interpolation for the lorenz curve (default "constant")
    plot: bool, optional
        compute or not interpolation of lorenz curve(default False)

    '''
    # Comments
    # --------
    #
    # constant piecewise interpolation is useful when the number of observed ones is
    # little (to underline the breakpoints),
    # linear has nicer smoothing properties

    if interpolation not in ["linear", "constant"]:
        raise NotImplementedError("interpolation available only for linear and constant")
    if y.shape[0] != y_pred.shape[0]:
        raise ValueError("y and y_pred must have the same length")

    n_samples = y.shape[0]

    if weight is None:
        weight = np.repeat([1. / n_samples], n_samples)

    # Id of each column
    obs_col, pred_col, w_col, rank_col = (0, 1, 2, 3)

    # Order data following prediction
    ordered_data = np.column_stack((y, y_pred,weight, np.zeros(y.shape[0])))

    pred_order = np.argsort(y_pred)[::-1]
    ordered_data = ordered_data[pred_order, :]

    # Compute the rank
    ordered_data[:, rank_col] = np.cumsum(ordered_data[:, w_col]) - 1. / 2 * ordered_data[:, w_col]

    total_weight = np.sum(ordered_data[:, w_col])

    obs_sum = np.dot(ordered_data[:, w_col], ordered_data[:, obs_col])

    intermediate = ordered_data[:,0]*ordered_data[:,2]*ordered_data[:,3]
    rank_obs_sum = intermediate.sum()

    # Compute the Gini coefficient
    gini = 1 - (2 / (total_weight * obs_sum)) * rank_obs_sum
    # until here, as in the old code
    if plot:
        # Determine the points to plot
        x_list = np.cumsum(ordered_data[:, w_col]) / total_weight
        y_list = np.cumsum(ordered_data[:, w_col] * ordered_data[:, obs_col]) / obs_sum
        x_interpolate = np.linspace(0, 1, num=resolution)

        #    this is for linear interpolation
        if interpolation == "linear":
            y_interpolate = np.interp(x_interpolate, x_list, y_list)
        elif interpolation == "constant":
            # this is for piecewise interpolation (better when 1s and 0s are little)
            f = interpolate.interp1d(x_list, y_list, kind='zero', bounds_error=False)
            y_interpolate = f(x_interpolate)

            # manually make 0 and 1s outside the range (bug in interp1d)
            y_interpolate[x_interpolate <= np.min(x_list)] = 0
            y_interpolate[x_interpolate >= np.max(x_list)] = 1

    # TODO: PLOT !

        return gini, y_interpolate
    else:
        return gini


def gini_emblem_fast(y, y_pred, weights=None, normalize_gini=False, verbose=False):
    # We compute Gini coefficient for the model col_score
    gini_model = area_lorentz_fast(y, y_pred, weights)
    if verbose:
        print("Gini coefficient for prediction", " without normalization:", gini_model)
    # Emblem by default returns the non-normalized version of the Gini
    if normalize_gini:
        # We compute the gini coefficient for the "perfect model":
        gini_perfect_model = area_lorentz_fast(y, y, weights)
        if verbose:
            print("Gini coefficient of 'perfect' model:", gini_perfect_model)

        # We normalize the Gini coefficient:
        gini = gini_model / gini_perfect_model
        if verbose:
            print("The Gini coefficient for prediction", " after normalization:", gini)
        return gini

    else:
        # We don't normalize the Gini coefficient:
        return gini_model


def plot_lift_curve(y, y_pred, weight=None, n_band=10, title=None, path_plot_save='Results\\'):
    """

    Parameters
    ----------

    y : ndarray
        array containing the TRUE response (either 0 or 1)
    y_pred : ndarray
        array containing the predicted probabilities by the model
    weight : ndarray, optional
        array containing the weight (default 1)
    n_band : int, optional
        number of bands (default 10)
    title: str
        title of the plot, if None, plot is not saved.
    path_plot_save: str
        file path where the plot will be saved.

    Returns
    -------

    """
    if weight is None:
        weight = np.ones(y.shape[0])

    d = {'pred': list(y_pred), 'obs': list(y), 'weights': list(weight)}
    d = pd.DataFrame(d)
    d = d.dropna(subset=['obs', 'pred'])
    d = d.sort_values('pred', ascending=True)
    l = len(y_pred)
    d.index = list(range(0, l))
    exp_cum = [0]
    for k in range(0, l):
        exp_cum.append(exp_cum[-1] + d.ix[k, 'weights'])
    s = exp_cum[-1]
    j = s // n_band
    m_pred, m_obs, m_weight = [], [], []
    k, k2 = 0, 0

    for i in range(0, n_band):
        k = k2
        for p in range(k, l):
            if exp_cum[p] < ((i + 1) * j):
                k2 += 1
        temp = d.ix[range(k, k2), ]
        m_pred.append(sum(temp['pred'] * temp['weights']) / sum(temp['weights']))
        m_obs.append(sum(temp['obs'] * temp['weights']) / sum(temp['weights']))
        m_weight.append(temp['weights'].sum())

    fig, ax1 = plt.subplots()

    ax2 = ax1.twinx()

    ax1.set_xlabel('Band')
    ax1.set_ylabel('Y values')
    ax2.set_ylabel('Weight')
    ax2.set_ylim([0, max(m_weight) * 3])
    ax1.plot(range(0, n_band), m_pred, linestyle='--', marker='o', color='b')
    ax1.plot(range(0, n_band), m_obs, linestyle='--', marker='o', color='r')
    # the histogram of the weigths
    ax2.bar(range(0, n_band), m_weight, color='yellow', alpha=0.2)
    ax1.legend(labels=['Predicted', 'Observed'], loc=2)
    if title is not None:
        fig.savefig(path_plot_save + title + '.png', bbox_inches='tight')
