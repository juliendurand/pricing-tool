split dataset in metadata
basic test/train stats : observation, exposure, min, mean, max
distribution of test/train
ideal poisson distribution
save result in own folder /results/name
list of coeffs by feature included
list of features sorted by gini and spread 100/0 and spread 95/5
get pred for n feature on demand
simple categorical smoothing
spline interpolation
complete documentation
lasso/ridge penalty ?


C++ interface:
--------------

coeffs, path = fit(filename)
ypred = predict(coeffs)
