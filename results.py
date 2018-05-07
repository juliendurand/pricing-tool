import os
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt

import preprocessing


# print('mean:', round(np.sum(df.prediction) / np.sum(df.exposure), 6), round(np.sum(df.target) / np.sum(df.exposure), 6))
# print('rmse:', round(metrics.root_mean_square_error(df.target, df.prediction, df.exposure), 6))
# print('gini:', metrics.gini_emblem_fast(df.target, df.prediction, df.exposure))
# print('deviance:', np.log(metrics.poisson_deviance(df.target, df.prediction, df.exposure)))
# metrics.plot_lift_curve(df.target, df.prediction, df.exposure, n_band=20)


class Results:

    def __init__(self, model, path, name):
        self.model = model
        self.metadata = preprocessing.Metadata(path, name)
        self.metadata.load()
        self.load_results()
        self.load_data()
        self.load_coeffs()
        self.test = {}
        self.train = {}
        for f in self.metadata.features:
            relativities = self.calculate_relativities(f)
            self.plot_relativities(f, relativities)

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

        df['f'] = self.data[df.row, :][:, idx]

        relativity = df.groupby(['f']).agg({
                                           'exposure': 'sum',
                                           'target': 'mean',
                                           'prediction': 'mean'
                                           })
        relativity.target /= df.target.mean()
        relativity.prediction /= df.target.mean()
        relativity = self.fill_missing_modalities(relativity, modalities)
        relativity['modalities'] = modalities
        relativity['one'] = 1
        relativity['coeffs'] = self.get_coeffs(
            self.metadata.get_feature_range(feature)
        )
        relativity = relativity.sort_values('modalities')
        return relativity

    def plot_relativities(self, feature, relativity):
        ar = np.arange(relativity.prediction.size)

        # Create chart
        fig, ax1 = plt.subplots(figsize=(10, 8))

        # Exposure on first axis
        ax1.bar(ar, relativity.exposure, color='#fffca0', edgecolor='grey')
        ax1.set_ylim(ymax=relativity.exposure.max() * 3)
        ax1.set_xticks(ar)
        ax1.set_xticklabels(labels=relativity.modalities)

        # Relativities on second axis
        ax2 = ax1.twinx()
        ax2.set_title(feature)
        ax2.plot(ar, relativity.prediction, color="#0f600e", marker=".")
        ax2.plot(ar, relativity.target, color="#c242f4", marker=".")
        ax2.plot(ar, relativity.coeffs, color="#93ff9e", marker="^")
        ax2.axhline(y=1, color='black', linewidth=1, linestyle="dotted")
        ax2.set_ylim(ymin=0)

        chart_path = os.path.join(self.metadata.path, self.metadata.name,
                                  self.model)
        if not os.path.exists(chart_path):
            os.makedirs(chart_path)
        filename = os.path.join(chart_path, 'relativity_' + feature + '.png')
        print('Saving relativity chart : ', filename)

        plt.savefig(filename)
        plt.close()

        return filename
