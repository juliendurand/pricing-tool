# Principes de tarification IARD

## La structure du Tarif

On distingue traditionnellement trois éléments dans la structure tarifaire :

1. le **tarif technique** : cout des sinistres ;
2. le **tarif de revient** : cout de production incluant les sinistres et les autres charges allouées à la branche par la comptabilité analytique ;
3. le **tarif commercial** : cout de production + marge.

Le **tarif technique**, également appelé **prime pure**, reflète le coût du risque, c'est à dire le coût des sinistres attendus sur la période de couverture. Le tarif technique dépend à la fois du profil du client et du produit vendu. Par exemple le niveau des franchises affecte de manière directe le montant des sinistres. En conséquence le tarif technique dépend du niveau de franchise choisi. Le tarif technique est obtenu à partir d'un modèle d'apprentissage sur les sinistres passés et d'une série d'ajustement pour tenir compte des hypothèses d'évolutions des principaux paramètres.

Le **tarif de revient** reflète le coût de production d'un contrat, c'est à dire la somme du coût des sinistres modélisés par le tarif technique et des autres charges imputées en comptabilité analytique. Ces charges comprennent notamment les frais généraux, les charges financières, les couts marketing, le frais d'acquisitions, les commissions des intermédiaires, ... Pour obtenir le tarif de revient il est nécessaire de modéliser les charges pour chaque contrat et d'ajouter le montant obtenu au tarif technique. Par construction le tarif de revient est toujours supérieur au tarif technique.

Le **tarif commercial** est le tarif réellement proposé au client. Il est généralement supérieur au tarif de revient (et donc au tarif technique) afin de dégager une marge pour l'assureur. Toutefois il peut être utile d'avoir sur certains segments un tarif commercial inférieur au tarif de revient. Par exemple un assureur pourrait envisager une stratégie consistant à proposer des tarifs d'assurance automobile très avantageux aux jeunes conducteurs qui ont une sinistralité élevée en pariant sur le fait que leur cout du risque va diminuer au fil des ans et que le contrat sera profitable à terme. Cela permet de renouveler le portefeuille en faisant payer la sinistralité des jeunes conducteurs par les clients plus expérimentés.

La **marge technique** d'un contrat est égale à la différence entre le tarif commercial et le tarif technique.

La  **marge opérationnelle** est égale à la différence entre le tarif commercial et le tarif de revient. La marge opérationnelle peut être négative pour un contrat.

@TODO : graphique sur les différentes primes

### Modèle Technique

Le modèle technique a pour objectif de prédire pour chaque contrat le cout des sinistres sur la période de couverture de référence, en général un an. Pour cela il faut réaliser un apprentissage statistique sur les contrats des années antérieures.

Il existe souvent des contraintes légales ou opérationnelles qui obligent à exclure certaines variables dans les processus de tarification opérationnels, à titre d'illustration voici deux exemples :

- contrainte légale : afin de respecter le principe d'égalité des genre le législateur français interdit de prendre en compte la variable "sexe" dans la tarification des contrats d'assurance ;
- contrainte opérationnelle : certaines variables peuvent être inconnues ou invérifiables à la souscription. Il est préférable de ne pas les utiliser dans le tarif opérationnel. Par exemple le nombre de kilomètres parcourus avec le véhicule est une information peu vérifiable et il convient de l'utiliser avec prudence.

Dans la pratique est-il donc possible de construire plusieurs modèles techniques pour différents usages :

- modèle technique non contraint : utilisé pour le suivi de l'activité et de la rentabilité réelle du portefeuille ;
- modèle technique contraint : utilisé pour les processus opérationnels de gestion des contrats : tarification, souscription et éventuellement résiliation.

Dans la suite nous nous intéresserons particulièrement aux méthodes de calcul du tarif technique.

### Modèle de Cout

Le modèle de cout doit estimer pour chaque contrat la part des coûts fixes et des coûts variables qui lui sont imputables en comptabilité analytique.

En théorie un assureur n'a pas intérêt à proposer un tarif inférieur au tarif technique plus les coûts variables affectés au contrat. En pratique comme la rentabilité d'un contrat s'apprécie sur une durée longue ce critère n'est pas déterminant et la distinction entre les coûts variables et les coûts fixes n'est pas obligatoire.

### Modèle Commercial

Le modèle de tarification commercial met en œuvre la stratégie commerciale de l'assureur. Il n'est pas nécessairement le reflet du modèle technique, toutefois il est très important de disposer d'outils permettant de mettre en regard le tarif commercial avec le tarif technique afin de garantir la maitrise des risques.

## L'approche actuarielle classique

La pratique actuarielle modélise les risques par apprentissage statistique. Dans cette approche, l'actuaire choisit une loi statistique qui sert de modèle pour représenter la réalité des processus observés. Cette loi est aussi appelée "loss function". L'apprentissage statistique est réalisé en recherchant les paramètres de la loi qui permettent d'obtenir le maximum de vraisemblance. En pratique l'algorithme d'apprentissage recherche les paramètres qui minimisent l'écart entre les observations et les prédictions.

 Toutefois la modélisation des risques IARD est difficile car la prime pure (coût des sinistres) ne suit pas une loi statistique simple. Les actuaires ont vérifié de manière empirique (@TODO mettre une référence) qu'il est possible de décomposer la prime pure en deux composantes indépendantes :

 1. la *fréquence* : nombre de sinistres observés, normalisés sur la période de référence (en général un an) ;
 2. le *coût moyen* des sinistres déclarés.

 Comme nous pouvons en pratique faire l'hypothèse de l'indépendance statistique de ces deux variables aléatoires (corrélation nulle), il est possible d'exprimer la prime pure comme le produit de la fréquence et du cout moyen :

```
PrimePure = Fréquence * Coût Moyen
```

La modélisation de la prime pure se fait donc en deux sous-parties parallèles : la modélisation de la fréquence et la modélisation des coûts moyens.

Il est utile de vérifier en pratique que pour chaque garantie modélisée l'hypothèse d'indépendance de la fréquence et des coûts moyens est vérifiée. Si ce n'est pas le cas il convient de rechercher l'origine de la corrélation entre les deux composantes et corriger ce biais. En effet si cette hypothèse n'est pas vérifiée le modèle technique final peut être fortement biaisé car les effets combinés seront sur ou sous-estimés.

## Modèle de fréquence

Le modèle de fréquence est entraîné sur la totalité de la base d'apprentissage, que le contrat soit ou non sinistré.

En supposant que pour un contrat donné la probabilité d'avoir un sinistre  à chaque instant suive un processus de Bernoulli avec probabilité constante, le nombre de sinistre observés sur une année (période de référence) obéit à une loi binomiale négative. Or lorsque l'on répète le processus à l'infini sur des périodes de temps de plus en plus courte la loi binomiale négative converge asymptotiquement vers une loi de Poisson.

Les actuaires font donc couramment l'hypothèse que la fréquence des sinistres suit une loi de Poisson.

Cette hypothèse est très importante et doit être vérifiée en pratique avant chaque modélisation. Pour cette vérification il est recommandé de :

1. vérifier que l'espérance et la variance de la distribution des fréquences observées sont égales (dans une marge de tolérance à définir, par exemple 10%) ;
2. vérifier que la courbe de la distribution théorique, paramétrée par l'espérance de la distribution observée, se superpose de manière convenable avec la distribution observée.

Si la loi de Poisson n'est pas vérifiée, et en particulier dans les situations où la variance est plus dispersée que l'espérance, il faut soit revoir la construction de la base d'apprentissage soit modéliser les fréquences avec une loi plus adaptée. @TODO (définir les lois plus adaptées)

## Modèle de coût moyen

Le modèle de cout moyen est réalisé seulement sur les contrats sinistrés, en effet les contrats non sinistrés n'ont pas d'observations pour le coût des sinistres.

Pour les contrats sinistrés, la variable à apprendre est la moyenne des sinistres observés sur la période.

Le cout d'un sinistre est supérieur à zéro et les actuaires font le plus souvent l'hypothèse d'une loi gamma.

Il est également utile de vérifier en pratique que l'hypothèse de modélisation est raisonnable en superposant la distribution des coûts moyens observée avec la loi gamma théorique issue des paramètres de la distribution observée.

## Classification des risques

Afin de réaliser une bonne modélisation il est important de catégoriser les risques dans des catégories homogènes afin de pouvoir isoler les facteurs de risques spécifiques à chaque type de risque et identifier la loi de probabilité réelle de chaque catégorie.

Un contrat d'assurance IARD couvre généralement plusieurs garanties. Par exemple un contrat Multi-Risque Habitation couvre l'assuré contre les dégâts des eaux, vol, incendie. Chaque garantie a des facteurs explicatifs différents et il est donc recommandé de modéliser chaque type de risque séparément.

Au sein d'une même garantie il est parfois possible d'isoler plusieurs risques de nature différente. Par exemple la garantie de responsabilité civile en assurance automobile couvre à la fois les dommages corporels et matériels des tiers, or le développement de ces deux types de sinistres est totalement distinct. Il convient donc, si possible de les modéliser séparément. En assurance habitation il peut être utile de modéliser de manière séparée les appartements et les maisons. De la même manière sur les garanties des professionnels il peut être utile de modéliser séparément certains secteurs d'activités tels que l'agriculture, l'industrie et le secteur tertiaire (ou tout autre distinction permettant de classer les sinistres dans des catégories homogènes).

Enfin, certaines garanties sont susceptibles de subir des typologies de sinistres distinctes. Par exemple pour les garanties "incendie", on peut constater généralement deux types de sinistres :

- des sinistres attritionnels relativement fréquents et peu graves : dommages limités à une pièce et quelques meubles ;
- des sinistres rares mais d'une gravité exceptionnelle : perte totale du logement et de son contenu.

Dans cette situation il est également recommandé de modéliser de manière distincte les différents types de sinistres. Dans l'exemple précédant il serait judicieux de modéliser séparément les sinistres attritionnels et les graves.

Le jugement du praticien est bien sûr nécessaire pour distinguer quand il est utile de distinguer les risques et de réaliser des modèles séparés et lorsque cela aboutirait à une complexité inutile au vu de la matérialité des risques.

Pour résumer il est recommandé de modéliser séparément :

- chaque type de garantie ;
- au sein de chaque garantie les risques différents ;
- lorsque nécessaire : les sinistres attritionnels et les sinistres graves ;

## Les modèles linéaires généralisés (GLM)

La loi de Poisson et la loi gamma appartiennent à la famille exponentielle. L'apprentissage statistique de modèles Poisson et gamma entrent dans le cadre théorique des modèles linéaires généralisés.

## Le signal géographique

Pour quasiment toutes les garanties IARD, l'adresse du contrat (ou autre adresse pertinente) fait partie des variables explicatives de la sinistralité. Il est donc crucial de pouvoir capter le signal géographique pour obtenir le modèle le plus performant. Pour cela il existe deux approches traditionnelles :

1. réaliser un *zonier* par **lissage spatial** des résidus afin de capturer les variations locales ;
2. **utiliser des données externes** les intégrer dans le processus d'apprentissage, soit directement dans les GLM, soit par l'intermédiaire d'un apprentissage sur les résidus.

### Lissage spatial

Le lissage spatial est une méthode très attractive car elle permet de capturer le signal géographique sans avoir besoin d'ajouter de variables géographiques. Le lissage consiste à observer le voisinage d'un contrat pour en déduire la correction géographique locale à appliquer au modèle général. Le lissage réalisé sur la totalité du territoire permet de constituer un *zonier* qui découpe le territoire en zones auxquelles sont associées un coefficient local à intégrer dans le modèle GLM. Deux méthodes sont souvent utilisées en pratique pour réaliser ces zoniers :

- en suivant un découpage administratif : région, département, canton, ville, arrondissement, IRIS (zone administrative de l'INSEE regroupant environ 2000 habitants) ;
- par proximité géographique : méthode des plus proches voisins.

### Données externes

Il existe de nombreuses données géographiques externes fournies par des agences publiques ou des vendeurs spécialisés. Ces bases de données fournissent pour chaque point d'un territoire les valeurs associées. Par exemple il est possible d'obtenir la densité de population, le nombre d'habitant de la commune, le pourcentage d'immeubles collectifs...

Les jointures spatiales permettent d'associer à chaque contrat les données géographiques et de les inclure dans la base de modélisation. Il existe ensuite deux approches principales pour intégrer ces variables géographiques dans les modèles :

1. Inclure les variables directement dans les bases d'apprentissage des modèles GLM de fréquences et de coûts ;
2. Construire un zonier avec des méthodes d'apprentissage non linéaire sur les résidus des modèles (linéaires) de fréquence et de coûts ;

Si le choix numéro 2 est fait, c'est à dire construire un zonier sur les résidus des modèles linéaires, il est important d'utiliser un algorithme capable de capter les non linéarités sinon cela revient à la méthode 1 qui est plus simple à mettre en œuvre.

### Géocodage

Pour réaliser une jointure spatiale de la base d'apprentissage avec des données externe il faut tout d'abord procéder au géocodage des adresses. Le géocodage consiste à transformer une adresse texte en une position géographique dans un système géographique de référence, par exemple les coordonnées WGS84 utilisé par GPS.

Pour réaliser le géocodage de la base d'apprentissage il est possible d'utiliser les APIs de Google, Microsoft, Apple... Toutefois ces APIs sont limités en nombre d'adresse et leur coût est souvent prohibitif lorsque l'on souhaite géocoder des millions de contrats.

Il est également possible d'utiliser des API de géocodage fournies par des vendeurs spécialisés ou pour certains pays d'utiliser des bases d'adresses publique. C'est le cas de la France qui mets à disposition l'intégralité des adresses française dans une base de données librement utilisable, la BANO (Base d'Adresses Nationale Ouverte) :


```
https://www.data.gouv.fr/fr/datasets/
    base-d-adresses-nationale-ouverte-bano/
```

Pour géocoder les adresses française, l'auteur vous recommande l'outil GeoAPI qu'il a conçu en collaboration avec Paulo de Vilhena :

```
https://github.com/DeVilhena-Paulo/Geocoding
```

### Jointure Spatiale

Une jointure spatiale permet d'extraire les valeurs d'une base de données géographique à partir d'une clef déterminée par une adresse. Il est possible d'utiliser plusieurs types de clefs et c'est la structure de la base de données géographiques qui guide le choix de la clef. Voici une liste non exhaustive des clefs de jointure géographique possibles :

- code postal ;
- numéro de commune ;
- longitude et latitude ;
- x,y dans une projection de cartographie (ex : Lambert93 en France) ;
- clef propriétaire créé à partir de l'adresse ;
- ...

La base de données PostgreSQL, avec le plugin PostGIS est un outil vivement recommandé pour réaliser les jointures spatiales.

Astuce : si vous devez réaliser une jointure spatiale avec un fichier contenant des polygones complexes (ex : fichier shapefile des zones de crues), il est souvent utile de commencer par triangulariser les polygones, en effet l'algorithme de recherche d'un point dans un polygone est beaucoup plus efficace quand les polygones ont peu de cotés (le triangle est le plus simple des polygones).
