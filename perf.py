import matplotlib
import matplotlib.pyplot as plt
from matplotlib.ticker import MultipleLocator
import numpy as np


def mean_absolute_error(y_hat, y):
    return np.average(np.abs(y_hat - y))


def root_mean_square_error(y, y_hat):
    return np.sqrt(np.mean(np.square(y-y_hat)))


def deviance(y, y_hat):
    return 2 * np.sum( y * (np.log(y+1e-10) - np.log(y_hat))  + y_hat  - y)


def explained_deviance_ratio(y, y_hat):
    n = y.shape[0]
    return 1 - deviance(y, y_hat) / deviance(y, np.mean(y) * np.ones(n))


def gini(y, exposure):
    """Calculate the Gini coefficient of a numpy array."""
    n = y.shape[0] # number of array elements
    array = y / exposure
    array = np.sort(array) # values must be sorted
    index = np.argsort(array) # index per array element 
    return 2 * np.sum(np.inner(index, array)) / (n *  np.sum(array)) - (n + 1) / n


def saturated_gini(y, y_hat, exposure):
    return gini(y_hat, exposure) / gini(y, exposure)


def plot_gini(y, exposure):
    n = y.shape[0] #number of array elements
    array = y / exposure
    array = np.sort(array) #values must be sorted
    index = np.argsort(array) #index per array element 
    
    fig = plt.figure(figsize=(16, 8))
    ax = fig.add_subplot(111)
    line, = ax.plot(np.cumsum(array) / np.sum(array))
    ax = fig.add_subplot(111)
    line, = ax.plot(index, index / n)
    plt.show()

    
def plot_lift_curve(y, y_hat, exposure, n):
    size = y.shape[0]
    quantile_size = size / n
    y_r = np.zeros(n)
    y_hat_r = np.zeros(n)
    exposure_r = np.zeros(n)

    y_hat_sorted = np.argsort(y_hat)
    e_cumsum = np.cumsum(exposure[y_hat_sorted])
    e_cumsum_normalized = e_cumsum / e_cumsum[-1]

    for i in range(n):
        filter_quantile = (e_cumsum_normalized < (i + 1) / n) & (e_cumsum_normalized > i / n)
        nb_quantile = np.sum(filter_quantile)
        y_hat_r[i] = np.sum(y_hat[y_hat_sorted][filter_quantile]) / nb_quantile
        y_r[i] = np.sum(y[y_hat_sorted][filter_quantile]) /nb_quantile
        exposure_r[i] = np.sum(exposure[y_hat_sorted][filter_quantile]) 
        
    max_value = np.max(y_hat_r)
    
    width = 0.5
    
    fig = plt.figure(figsize=(16, 8))
    ax1 = fig.add_subplot(111)
    ax1.set_title('Lift Curve')
    ax1.set_xticks(np.arange(n) + width)
    ax1.set_xticklabels(np.arange(n) + 1)
    ax1.yaxis.set_major_locator(MultipleLocator(0.02))
    ax1.set_xlabel('Absolute value : Current Model')
    ax1.set_ylabel('Weighted Average')
    ax1.grid(color='grey', linestyle='-', linewidth=0.5)
    line1, = ax1.plot(np.arange(n) + width, y_hat_r, color='blue', marker='o', label='Current Model')
    line2, = ax1.plot(np.arange(n) + width, y_r, color='red', marker='s', label='Data')
    ax1.set_ylim([-max_value * 0.4, max_value * 1.2])
   
    ax2 = ax1.twinx()
    ax2.set_ylabel('Exposure')
    bar1 = ax2.bar(np.arange(n) + width / 2, exposure_r, width=width, color='yellow', label='Exposure')
    ax2.set_ylim([0, np.max(exposure_r) * 5])
    ax2.set_xlim([-0.5, n + 0.5])
    
    plt.legend(handles=[line1, line2, bar1], bbox_to_anchor=(0.25, 0.9))

    plt.show()
    

def plot_rescaled_predicted_values(feature, y_hat, y, exposure):
    unique_values = np.unique(feature)
    n = unique_values.shape[0]
    y_hat_result = np.zeros(n)
    y_result = np.zeros(n)
    exposure_result = np.zeros(n)
    for i, a in enumerate(unique_values):
        y_hat_result[i] = np.average(y_hat[feature == a])
        y_result[i] = np.average(y[feature == a])
        exposure_result[i] = np.average(exposure[feature == a])

    width = 0.5
    fig = plt.figure(figsize=(16, 8))
    ax1 = fig.add_subplot(111)
    ax1.grid(color='grey', linestyle='-', linewidth=0.5)
    ax1.set_title('Rescaled Predicted Values')
    #ax1.set_ylabel('Exposure')
    bar1 = ax1.bar(np.arange(n) + width / 2, exposure_result, width=width, color='yellow', label='Exposure')
    ax1.set_ylim([0, np.max(exposure_result) * 5])
    

    ax2 = ax1.twinx()

    line1, = ax2.plot(np.arange(n) + width, y_hat_result, color='green', marker='o', label='Fitted Average')
    line2, = ax2.plot(np.arange(n) + width, y_result, color='purple', marker='s', label='Observed Average')
    ax2.set_xticks(np.arange(n) + width)
    ax2.set_xticklabels(unique_values.astype('int32'))
    #ax2.yaxis.set_major_locator(MultipleLocator(0.1))
    #ax2.set_xlabel('Absolute value : Current Model')
    #ax2.set_ylabel('Weighted Average')
    ax2.set_xlim([-0.5, n + 0.5])

    plt.legend(handles=[line1, line2, bar1], bbox_to_anchor=(0.25, 0.9))

    plt.show()