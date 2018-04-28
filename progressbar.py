import time

start_time = 0

# Print iterations progress
def printProgressBar (iteration, total, prefix = '', suffix = '', decimals = 1, length = 100, fill = '█'):
    """
    Call in a loop to create terminal progress bar
    @params:
        iteration   - Required  : current iteration (Int)
        total       - Required  : total iterations (Int)
        prefix      - Optional  : prefix string (Str)
        suffix      - Optional  : suffix string (Str)
        decimals    - Optional  : positive number of decimals in percent complete (Int)
        length      - Optional  : character length of bar (Int)
        fill        - Optional  : bar fill character (Str)
    """
    global start_time
    if iteration == 0:
        start_time = time.time()
    percent = ("{0:." + str(decimals) + "f}").format(100 * (iteration / float(total)))
    filledLength = int(length * iteration // total)
    bar = fill * filledLength + '-' * (length - filledLength)
    elapsed_time = int(time.time() - start_time)
    m = str(elapsed_time // 60).zfill(2)
    s = str(elapsed_time % 60).zfill(2)
    print('\r%s |%s| %s%% %s in %sm%ss' % (prefix, bar, percent, suffix, m, s), end = '\r')
    # Print New Line on Complete
    if iteration == total:
        print()
