"""
Each function in this module has at least three arguments,
which are numpy arrays of the same size.

``function(y, y_pred, weights = None, **kwargs)``
where ``**kwargs`` are **named** extra-parameters.

"""

import numpy as np


def mean_absolute_error(y, y_hat, exposure):
    return np.sum(np.abs(y_hat - y)) / np.sum(exposure)


def root_mean_square_error(y, y_hat, exposure):
    return np.sqrt(np.sum(np.square(y - y_hat)) / np.sum(exposure))


def check_deviance(y, y_pred, weight=None):
    """
    checks to run at beginning of deviance
    """
    if isinstance(y_pred, (np.floating, float)):
        y_pred = np.repeat(y_pred, y.shape[0])
    assert y.shape[0] == y_pred.shape[0], \
        "y and y_pred must have the same size"
    if weight is not None:
        assert weight.shape[0] == y.shape[0], \
            "weight and y do not have same shape"
    return y_pred


def gaussian_deviance(y, y_pred, weight=None):
    """
    Deviance function for the gaussian/least squares model.

    Parameters
    ----------

    y : ndarray
        array containing the TRUE response
    y_pred : ndarray
        array containing the value predicted by the model
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


def poisson_deviance(y, y_pred, weight=None):
    """
    Deviance function for the poisson model.

    Parameters
    ----------

    y : ndarray
        array containing the TRUE response
    y_pred : ndarray
        array containing the value predicted by the model
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
    deviance_vector[bool_zeros] = \
        (y[bool_zeros] * np.log(y[bool_zeros] / y_pred[bool_zeros]) -
            y[bool_zeros] + y_pred[bool_zeros])
    deviance_vector[~bool_zeros] = - y[~bool_zeros] + y_pred[~bool_zeros]
    if weight is not None:
        deviance_vector = np.dot(weight, deviance_vector)

    return 2 * np.sum(deviance_vector)


def gamma_deviance(y, y_pred, weight=None):
    """
    Deviance function for gamma model.

    Parameters
    ----------

    y : ndarray
        array containing the TRUE response
    y_pred : ndarray
        array containing the value predicted by the model
    weight : ndarray, optional
        array containing the weight (default 1)

    Returns
    -------
    ndarray
        computed deviance

    """
    y_pred = check_deviance(y, y_pred, weight=weight)
    deviance_vector = -np.log(y / y_pred) + (y - y_pred) / y_pred
    if weight is not None:
        deviance_vector = np.dot(weight, deviance_vector)

    return 2 * np.sum(deviance_vector)


def binomial_deviance(y, y_pred, weight=None):
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
    deviance_vector = - (y * np.log(y_pred) + (1 - y) * np.log(1 - y_pred))
    if weight is not None:
        deviance_vector = np.dot(weight, deviance_vector)
    return 2 * np.sum(deviance_vector)


def gaussian_pseudo_r2(y, y_pred, weight=None):
    """
    PseudoR2 for a Normal model.

    Parameters
    ----------

    y : ndarray
        array containing the TRUE response
    y_pred : ndarray
        array containing the value predicted by the model
    weight : ndarray, optional
        array containing the weight (default 1)

    Returns
    -------
    ndarray
        computed pseudo_R2

    Notes
    -----
    Pseudo R2 is defined as :
        1 - (deviance(y,y_pred,weight) / deviance(y,mu,weight)
    where mu is the weighted mean of y

    """
    return 1 - (gaussian_deviance(y, y_pred, weight) /
                gaussian_deviance(y, np.ones(len(y)) *
                np.average(y, weights=weight), weight))


def poisson_pseudo_r2(y, y_pred, weight=None):
    """
    PseudoR2 for the Poisson model.

    Parameters
    ----------

    y : ndarray
        array containing the TRUE response
    y_pred : ndarray
        array containing the value predicted by the model
    weight : ndarray, optional
        array containing the weight (default 1)

    Returns
    -------
    ndarray
        computed pseudo_R2

    Notes
    -----
    Pseudo R2 is defined as :
        1 - (deviance(y,y_pred,weight) / deviance(y,mu,weight)
    where mu is the weighted mean of y

    """
    return 1 - (poisson_deviance(y, y_pred, weight) /
                poisson_deviance(y, np.ones(len(y)) *
                np.average(y, weights=weight), weight))


def gamma_pseudo_r2(y, y_pred, weight=None):
    """
    Pseudo R2 for gamma model


    Parameters
    ----------

    y : ndarray
        array containing the TRUE response
    y_pred : ndarray
        array containing the value predicted by the model
    weight : ndarray, optional
        array containing the weight (default 1)

    Returns
    -------
    ndarray
        computed pseudo_R2

    Notes
    -----
    Pseudo R2 is defined as :
        1 - (deviance(y,y_pred,weight) / deviance(y,mu,weight)
    where mu is the weighted mean of y

    """
    return 1 - (gamma_deviance(y, y_pred, weight) /
                gamma_deviance(y, np.ones(len(y)) *
                np.average(y, weights=weight), weight))


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
    Pseudo R2 is defined as :
        1 - (deviance(y,y_pred,weight) / deviance(y,mu,weight)
    where mu is the weighted mean of y

    """
    return 1 - (binomial_deviance(y, y_pred, weight) /
                binomial_deviance(y, np.ones(len(y)) *
                np.average(y, weights=weight), weight))


def area_lorentz_fast(y, y_pred, weight=None):
    '''
    Calcultate the weighted gini.

    Parameters
    ----------

    y : ndarray
        array containing the TRUE response
    y_pred : ndarray
        array containing the value predicted by the model
    weight : ndarray, optional
        array containing the weight (default 1)
    '''
    if y.shape[0] != y_pred.shape[0]:
        raise ValueError("y and y_pred must have the same length.")

    n_samples = y.shape[0]

    if weight is None:
        weight = np.ones(n_samples)

    # Id of each column
    obs_col, pred_col, w_col, rank_col = 0, 1, 2, 3

    # Order data following prediction
    ordered_data = np.column_stack((y, y_pred, weight, np.zeros(y.shape[0])))

    pred_order = np.argsort(ordered_data[:, pred_col])[::-1]
    ordered_data = ordered_data[pred_order, :]

    # Compute the rank
    ordered_data[:, rank_col] = np.cumsum(ordered_data[:, w_col]) - 1. / \
        2 * ordered_data[:, w_col]

    total_weight = np.sum(ordered_data[:, w_col])

    obs_sum = np.dot(ordered_data[:, w_col], ordered_data[:, obs_col])

    intermediate = ordered_data[:, 0] * ordered_data[:, 2] * ordered_data[:, 3]
    rank_obs_sum = intermediate.sum()

    # Compute the weighted Gini
    gini = 1 - (2 / (total_weight * obs_sum)) * rank_obs_sum

    return gini


def gini(y, y_pred, weights=None, normalize_gini=False):
    # compute the weighted Gini
    gini = area_lorentz_fast(y, y_pred, weights)
    if normalize_gini:
        # compute the weighted Gini for the "perfect model"
        gini_perfect_model = area_lorentz_fast(y, y, weights)
        # normalization
        gini = gini / gini_perfect_model
    return gini
