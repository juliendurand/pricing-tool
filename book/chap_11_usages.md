# Usages

## Pilotage d'activité

### Calcul de la Prime Pure

### Expected Loss Ratio

#### Principes

L’ELR signifie « Expected Loss Ratio ». Il est calculé comme le ratio de la prime pure modélisée divisée par la prime commerciale observée.

```
ELR = Prime Pure / Prime Commerciale
```

L’ELR est une modélisation du ratio S/C. Le S/C est défini comme le ratio du coût des sinistres observés sur la cotisation acquise sur la période d’observation. L’ELR est donc un indicateur avancé de la rentabilité d’un contrat, d’un segment de client ou du portefeuille.

La prime pure modélisée est le cout statistique des sinistres pour une période donnée, avec les informations disponibles à un instant déterminé. Les informations utilisées pour le calcul de la prime pure modélisée doivent être identiques à celles utilisées lors de la phase de modélisation.

La prime commerciale observée est le prix hors taxe qui est facturé au client. La prime commerciale doit être observée au même moment que le risque utilisé pour le calcul de la prime pure modélisée.

L’ELR s’exprime par un nombre : 0.7 ou un pourcentage : 70%.  Il exprime la part de la prime commerciale nécessaire pour couvrir les sinistres. Par exemple un ELR de 70% signifie que 70% de la prime perçue sera nécessaire pour couvrir les sinistres. Un ELR de 1,3 ou 130% signifie qu’il faudra mobiliser 130% de la prime pour couvrir les sinistres attendus.

L’ELR est un indicateur instantané sans dimension. Ainsi l’ELR est indépendant de la devise utilisée et de la période de temps choisie. Cela découle du fait que l’ELR est un ratio de deux données qui sont homogènes à un prix sur une durée temporelle. En revanche il faut veiller à ce que la prime pure et la prime commerciale soient exprimées dans les mêmes unités de devise et de durée. Nous nous recommandons d'utiliser une prime annualisée.


L’ELR peut être calculé à différents niveaux de granularité : contrat, segment de client, portefeuille. Si l’ELR est calculé sur plusieurs contrats alors il faut diviser la somme des primes pures par la somme des primes commerciales (et non pas faire la moyenne des ELR des contrats individuels).

L’ELR peut également être calculé à tout instant de la vie du contrat, et en particulier, mais pas uniquement en date de souscription, résiliation, renouvellement à l'échéance, avenant.

ELR par garantie : il est possible de calculer un ELR par garantie ou un ELR total.

L’ELR a vocation à être suivi dans le temps pour :

- Evaluer la politique tarifaire de souscription ;
- Evaluer la politique tarifaire de renouvellement au terme ;
- Evaluer la politique tarifaire des avenants ;
- Suivre la rentabilité par segment ;
- Suivre la rentabilité par génération ;

#### Suivi par Segment

Le suivi de l’ELR par segment permet de suivre dans le temps la rentabilité attendue sur un segment de client. Pour cela il faut calculer l’ELR en vision « photo » à chaque fin de période par groupe de clients regroupés en segment. L’ELR du segment est égale à la somme des primes pures modélisées sur tous les contrats du segment divisée par la somme des primes commerciales observées sur ce même périmètre de contrat.

#### Suivi par fait de production

Le suivi de l’ELR par fait de production permet de suivre dans le temps la rentabilité attendue sur les contrats ayant réalisé un mouvement spécifique : Affaire Nouvelle, Résiliation, Changement de véhicule, …
Pour cela il faut calculer l’ELR en vision « photo » à chaque fin de période par groupe de clients ayant réalisés l’un des mouvements suivis. L’ELR du fait de production sur la période est égale à la somme des primes pures modélisées sur tous les contrats ayant eu le fait de production visé divisée par la somme des primes commerciales observées sur ce même périmètre de contrat.

#### Suivi par cohorte

Le suivi de l’ELR par cohorte permet de suivre l’évolution dans le temps d’une population donnée. Par exemple il est utile de suivre les contrats par génération.
Pour cela il faut calculer l’ELR en vision « photo » à chaque fin de période par groupe de clients appartenant à chaque cohorte. L’ELR de la cohorte sur la période est égale à la somme des primes pures modélisées sur tous les contrats de la cohorte divisée par la somme des primes commerciales observées sur cette cohorte.

## Valeur client

Modèle de Prime Commerciale

Modèle de Marge

Modèle de résiliation

## Politique Commerciale

### Affaire Nouvelle

### Renouvellement

### Avenant

### Résiliations
