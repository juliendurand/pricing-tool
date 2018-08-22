# Optimisation

## Traitement des modalités à faible exposition

### Mise à zéro des coefficients

### Lissage par spline

## Régularisation

#### Fused Lasso

Le Fused Lasso^[Robert Tibshirani, Michael Saunders, Saharon Rosset, Ji Zhu et Keith Knight, « Sparsity and smoothness via the fused lasso », Journal of the Royal Statistical Society: Series B (Statistical Methodology), vol. 67, n°1,‎ 2005, p. 91-108.] permet de lisser les coefficients pour les variables ordinales (discrètes). Le principe est que deux modalités successives auront des valeurs "proches". Par exemple, pour la variable `AGE_CLIENT`, la pénalisation Fused Lasso s'assure que pour chaque âge le coefficient soit lissé avec ses deux voisins, ce qui limite le sur-apprentissage et augmente la généralité du modèle.
\begin{equation}
\min _{\beta _{j}}\sum _{i=1}^{n}f_{loss}(y_{i},\sum _{j=0}^{p}\beta _{j}x_{i,j})+\lambda_{f} \sum_{j=2}^{n} \mid\beta_{j} - \beta_{j-1}\mid_{1}
\end{equation}

### Pénalisation Ridge

La pénalisation ridge (ou pénalisation de Tikhonov du nom de son inventeur, le mathématicien russe Andreï Nikolaïev Tikhonov) utilise la norme $l_{2}$, telle que $\mid\mid\beta\mid\mid_{2} = \sqrt{\sum _{j=1}^{p}{\beta _{j}^{2}}}$. Un paramètre $\lambda_{2}\ge0$ contrôle le niveau de contraction des coefficients.
\begin{equation}
\min _{\beta _{j}}\sum _{i=1}^{n}f_{loss}(y_{i},\sum _{j=0}^{p}\beta _{j}x_{i,j})+\lambda_{2} \mid\mid\beta\mid\mid_{2}
\end{equation}

### Pénalisation Elastic-Net

La pénalisation Elastic-Net essaie de tirer parti des qualités respectives des méthodes lasso et Ridge.
\begin{equation}
\min _{\beta _{j}}\sum _{i=1}^{n}f_{loss}(y_{i},\sum _{j=0}^{p}\beta _{j}x_{i,j})+\lambda_{1} \mid\mid\beta\mid\mid_{1}+\lambda_{2} \mid\mid\beta\mid\mid_{2}
\end{equation}
