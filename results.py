import os
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt

import metrics
import preprocessing


# print('deviance:', np.log(metrics.poisson_deviance(df.target, df.prediction,
#       df.exposure)))
# metrics.plot_lift_curve(df.target, df.prediction, df.exposure, n_band=20)


class Results:

    def __init__(self, path, name):
        self.metadata = preprocessing.Metadata(path, name)
        self.metadata.load()
        self.load_results()
        self.load_data()
        self.load_coeffs()
        self.testdata = self.data[self.df.row, :]
        self.train = {}
        self.train['mean'] = round(np.sum(self.df.target) /
                                   np.sum(self.df.exposure), 6)
        self.test = {}
        self.test['mean'] = round(np.sum(self.df.prediction) /
                                  np.sum(self.df.exposure), 6)
        self.test['rmse'] = round(metrics.root_mean_square_error(
                                  self.df.target,
                                  self.df.prediction,
                                  self.df.exposure), 6)
        self.test['gini'] = round(metrics.gini_emblem_fast(
                                  self.df.target,
                                  self.df.prediction,
                                  self.df.exposure), 6) * 100
        self.test['features'] = {}
        for f in self.metadata.features:
            self.test['features'][f] = {}
            relativities = self.calculate_relativities(f)
            self.test['features'][f]['relativities'] = {}
            self.test['features'][f]['relativities']['data'] = relativities

    def load_results(self):
        self.df = pd.read_csv("./data/results.csv")

    def load_data(self):
        file_path = self.metadata.get_feature_filename()
        shape = (self.metadata.size, self.metadata.count_features())
        self.data = np.memmap(file_path, dtype=np.dtype('u1'), shape=shape)

    def load_coeffs(self):
        self.df_coeffs = pd.read_csv('data/mrh/coeffs.csv').as_matrix()
        self.df_coeffs = np.exp(self.df_coeffs)

    def get_coeffs(self, feature_range):
        return self.df_coeffs[1 + np.array(feature_range)]

    def fill_missing_modalities(self, df, modalities):
        for i in range(len(modalities)):
            if i not in df.index:
                df.loc[i] = [0, 1, 1]
        return df

    def calculate_relativities(self, feature):
        df = self.df
        idx = self.metadata.get_feature_index(feature)
        modalities = self.metadata.get_modalities(feature)

        try:
            modalities = [float(m) for m in modalities]
            if sum([m - int(m) for m in modalities]) == 0:
                modalities = [int(m) for m in modalities]
        except Exception:
            # modalities are not integers -> that is perfectly ok
            pass

        df['f'] = self.testdata[:, idx]

        relativity = df.groupby(['f']).agg({
                                           'exposure': 'sum',
                                           'target': 'mean',
                                           'prediction': 'mean'
                                           })
        relativity = relativity.rename({
            'exposure': 'Exposure',
            'target': 'Target',
            'prediction': 'Prediction',
        }, axis=1)
        relativity.Exposure = relativity.Exposure.astype('int')
        relativity.Prediction /= df.target.mean()
        relativity.Prediction = relativity.Prediction.round(decimals=3)
        relativity.Target /= df.target.mean()
        relativity.Target = relativity.Target.round(decimals=3)
        relativity = self.fill_missing_modalities(relativity, modalities)
        relativity['Modalities'] = modalities
        relativity['Coefficients'] = self.get_coeffs(
            self.metadata.get_feature_range(feature)
        )
        relativity['Coefficients'] = relativity['Coefficients'] \
            .round(decimals=3)
        relativity = relativity.sort_values('Modalities')
        relativity = relativity.reset_index()
        relativity = relativity[['Modalities',
                                 'Coefficients',
                                 'Exposure',
                                 'Prediction',
                                 'Target']]
        return relativity

    def plot_relativities(self, feature, relativity, path):
        ar = np.arange(relativity.Prediction.size)

        # Create chart
        fig, ax1 = plt.subplots(figsize=(10, 8))

        # Exposure on first axis
        ax1.bar(ar, relativity.Exposure, color='#fffca0', edgecolor='grey')
        ax1.set_ylim(ymax=relativity.Exposure.max() * 3)
        ax1.set_xticks(ar)
        ax1.set_xticklabels(labels=relativity.Modalities)
        ax1.set_ylabel('Weight')

        # Relativities on second axis
        ax2 = ax1.twinx()
        ax2.set_title(feature)
        ax2.plot(ar, relativity.Prediction, color="#0f600e", marker=".")
        ax2.plot(ar, relativity.Target, color="#c242f4", marker=".")
        ax2.plot(ar, relativity.Coefficients, color="#93ff9e", marker="^")
        ax2.axhline(y=1, color='black', linewidth=1, linestyle="dotted")
        ax2.set_ylim(ymin=0)
        ax2.set_ylabel('Values')

        chart_path = os.path.join(path)
        if not os.path.exists(path):
            os.makedirs(path)
        filename = os.path.join(chart_path, 'img', 'relativity_' +
                                feature + '.png')
        # print('Saving relativity chart : ', filename)

        plt.savefig(filename)
        plt.close()

        return os.path.join('img', 'relativity_' + feature + '.png')

    def plot_lift_curve(self, path, n_band=10):
        y = self.df.target
        y_pred = self.df.prediction
        weight = self.df.exposure

        if weight is None:
            weight = np.ones(y.shape[0])

        d = {'pred': list(y_pred), 'obs': list(y), 'weights': list(weight)}
        d = pd.DataFrame(d)
        d = d.dropna(subset=['obs', 'pred'])
        d = d.sort_values('pred', ascending=True)
        d.index = list(range(0, len(y_pred)))
        exp_cum = [0]
        for k in range(0, len(y_pred)):
            exp_cum.append(exp_cum[-1] + d.ix[k, 'weights'])
        s = exp_cum[-1]
        j = s // n_band
        m_pred, m_obs, m_weight = [], [], []
        k, k2 = 0, 0

        for i in range(0, n_band):
            k = k2
            for p in range(k, len(y_pred)):
                if exp_cum[p] < ((i + 1) * j):
                    k2 += 1
            temp = d.ix[range(k, k2), ]
            m_pred.append(sum(temp['pred'] * temp['weights']) /
                          sum(temp['weights']))
            m_obs.append(sum(temp['obs'] * temp['weights']) /
                         sum(temp['weights']))
            m_weight.append(temp['weights'].sum())

        fig, ax1 = plt.subplots(figsize=(10, 8))
        ax2 = ax1.twinx()
        ax1.set_title('Lift Curve')
        ax1.set_ylim([0, max(m_weight) * 3])
        # the histogram of the weigths
        ax1.bar(range(0, n_band), m_weight, color='#fffca0',
                edgecolor='grey')
        ax2.plot(range(0, n_band), m_pred, linestyle='--',
                 marker='o', color='b')
        ax2.plot(range(0, n_band), m_obs, linestyle='--',
                 marker='o', color='r')
        ax2.legend(labels=['Predicted', 'Observed'], loc=2)
        ax1.set_xlabel('Band')
        ax2.set_ylabel('Y values')
        ax1.set_ylabel('Weight')
        path = os.path.join(path, 'img')
        if not os.path.exists(path):
            os.makedirs(path)
        filename = os.path.join(path, 'lift_curve.png')
        fig.savefig(filename, bbox_inches='tight')
        plt.close()
        return os.path.join('img', 'lift_curve.png')
