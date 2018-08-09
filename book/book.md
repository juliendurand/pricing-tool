---
title: La tarification IARD à l'ère du Big Data
author: Julien Durand
subject: Finance Data-Science Big-Data
keywords: Tarification Assurance IARD Data-Science Big-Data
---

\addcontentsline{toc}{chapter}{Introduction}
\chapter*[Introduction]{Introduction}

Les assureurs détiennent dans leur portefeuille des millions de contrats automobiles, habitations et professionnels. Ces activités, couramment regroupées sous le vocable IARD (Incendie, Accidents et Risques Divers) génèrent des volumes de données très importants. Pour améliorer leur compétitivité les compagnies d'assurance souhaitent modéliser l'impact de dizaines, voire centaines de variables. L'un des enjeux majeurs est de tarifer au juste prix chaque contrat en fonction de ses caractéristiques individuelles tout en assurant la mutualisation des risques au niveau du portefeuille. Pour avoir un ordre de grandeur, nous pouvons estimer qu'un assureur qui détient un portefeuille de quatre millions de contrats et qui souhaite mesurer l'impact de cent variables sur trois ans d'activité doit traiter 1,2 milliard de point de données. Cela représente 9,6 Go de donnée brute (en comptant une moyenne de 8 octets pour le stockage d'une donnée atomique). Ce volume croit avec la taille du portefeuille, la profondeur de l'historique et le nombre de variables étudiées. Les outils classiques de l'actuaire et du statisticien sont peu adaptés à des volumes aussi importants. La maitrise des technologies big data est donc un facteur clef de succès pour la tarification des contrats IARD.

Face à ce large volume de données, les principaux acteurs du secteur ont historiquement fait le choix de recourir à des solutions externes développées par des vendeurs spécialisés. Le bénéfice principal de cette approche est de fournir des résultats fiables en mutualisant les couts de recherche et développement. En revanche les algorithmes utilisés sont des "boites noires" et cela limite les capacités d'innovations pour exploiter de nouveaux axes de différenciations tarifaires. Les assureurs utilisent le même type de modélisation et la compétition se fait principalement sur la qualité et la quantité des données disponibles. Cela avantage les acteurs qui disposent des plus gros portefeuilles et des meilleurs systèmes d'informations.

Le big data et la data-science changent la donne en mettant à la disposition des actuaires des solutions en kit pour créer des modèles adaptés au business model et à la stratégie de leur compagnie.

L'objectif de ce livre est de fournir les bases méthodologiques et pratiques aux actuaires et data-scientistes qui souhaitent développer des outils de tarifications innovants tout en adhérant aux meilleures pratiques développées par la communauté actuarielle.

Les méthodes proposées sont mises en œuvre dans des exemples de code disponibles sur la plateforme de partage collaborative github.

Ce livre contient intentionnellement peu de formules mathématiques car l'objectif est de proposer un cadre méthodologique permettant aux praticiens de développer des algorithmes adaptés au contexte commercial et stratégique de leur activité.

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

Au sein d'une même garantie il est parfois possible d'isoler plusieurs risques de nature différente. Par exemple la garantie de responsabilité civile en assurance automobile couvre à la fois les dommages corporels et matériels des tiers, or le développement de ces deux types de sinistres est totalement distincts. Il convient donc, si possible de les modéliser séparément. En assurance habitation il peut être utile de modéliser de manière séparée les appartements et les maisons. De la même manière sur les garanties des professionnels il peut être utile de modéliser séparément certains secteurs d'activités tels que l'agriculture, l'industrie et le secteur tertiaire (ou tout autre distinction permettant de classer les sinistres dans des catégories homogènes).

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

Astuce : si vous devez réaliser une jointure spatiale avec un fichier contenant des polygones complexes (ex : fichier shapefile des zones de crues), il est souvent utile de commencer par triangulariser les polygones, en effet l'algorithme de recherche d'un point dans un polygone est beaucoup plus efficaces quand les polygones ont peu de cotés (le triangle est le plus simple des polygones).


# Préparation des Données

La phase de préparation des données est la phase la plus critique du processus de modélisation. Toute erreur dans cette étape sera transmise jusqu'en bout de chaine. Or il est souvent très difficile de détecter les erreurs commises dans la préparation des données, tandis qu'il est plus facile de détecter les erreurs de modélisation. Pour limiter les risques d'erreur il est recommandé de mettre en place un processus défensif :

- automatisation des traitements pour limiter le risque d'erreur manuelle ;
- documentation des traitements pour permettre la maintenance future ;
- tests automatisés de cohérence des données ;

En fonction de la maturité du système d'information vous devrez réaliser un nombre d'étapes variables car certaines étapes seront peut-être réalisées de manière automatisée par votre SI. Par exemple certains systèmes d'information proposent directement une vision unifiée des événements contrats et sinistres et vous n'aurez pas besoin de faire cette réconciliation.

L'objectif général est de constituer une base d'images couvrant chacune une période de risque homogène pour un contrat. Chaque image possède une date de début et de fin d'effet, la liste des garanties couvertes, les facteurs de risque et enfin pour chaque type de sinistre : le nombre et le montant total.

Dans le cas le plus général la préparation des données pour la modélisation suit un processus en seize étapes :

1. Définition du périmètre ;
2. Identification des features ;
3. Retraitement des features ;
4. Création de nouvelle features ;
5. Classification des features ;
6. Extraction des mouvements des contrats ;
7. Extraction des sinistres ;
8. Jointure des contrats et des sinistres ;
9. Création des images ;
10. Calcul de l'exposition ;
11. Filtrage des images ;
12. Discrétisation des facteurs ;
13. Optimisation du format de stockage ;
14. Création des metadata ;
15. Réconciliation comptable ;
16. Documentation.

Chacune de ces étapes est détaillée dans la suite de ce chapitre et vous devez vous assurer que chacune d'entre elle est bien documentée pour pouvoir auditer la phase de préparation des données.

## L'approche Big Data : Utilisation de MapReduce

Pour définir les différentes étapes, nous utiliserons un modèle de donnée fondé sur les séquences d'événements (Event Driven Processing) car ce modèle est très approprié pour la manipulation de données temporelles. Dans ce modèle on peut distinguer trois types d'opérateurs :

1. Générateurs d'événements ;
2. Processeurs ;
3. Stockage ;

Ces opérateurs sont chainés pour créer le flux d'information partant des bases sources, transitant à travers les processeurs et finalement stockés dans la base finale d'apprentissage. Cela permet de bien distinguer chaque étape de la préparation des données. Il possible en pratique de combiner certaines étapes afin de gagner en rapidité de calcul, toutefois nous trouvons préférable de bien isoler chaque opération. L'avantage obtenu est de faciliter la compréhension et l'identification des éventuelles erreurs puisque chaque étape peut être testée individuellement.

Tous les calculs nécessaires se prêtent particulièrement bien à la décomposition dans le paradigme de programmation MapReduce utilisés par les moteurs de calcul parallèle big data. Hadoop et Spark sont actuellement les solutions open-sources les plus populaires et peuvent servir très utilement de support à la mise en œuvre des traitements proposés.

## Définition du périmètre

Avant toute modélisation technique il est important de définir le périmètre de l'étude suivant plusieurs axes :

- produit ;
- garanties ;
- période temporelle ;
- canal de distribution ;
- ...

### Produit

TODO

### Garantie

TODO

### Période Temporelle

TODO
tradeoff entre :
- récence des données ;
- quantité ;
- complétude ;
- homogénéité ;


Année complète

Même période pour modèle de fréquence et de cout moyen pour la réconciliation technico-comptable.

### Canal de Distribution

TODO

## Identification des features

TODO

### Contrats

TODO

### Sinistres

TODO

### Devis

TODO

### Service Client

TODO

### Télématique

TODO

## Retraitement des features

TODO

### Valeurs manquantes

TODO

### Valeurs absurdes

TODO

### Valeurs avec disparité temporelle

TODO

## Création de nouvelles features

TODO

### Facteur aléatoire

TODO

### Facteur temporel

TODO

### Ages

TODO

### Interactions

TODO

### Autres features

TODO

### Features Catégorielles et Ordinales

TODO

### Classification des features

Une fois que vous aurez créé toutes les features nécessaires à l'apprentissage, vous obtiendrez probablement une liste de plusieurs dizaines, voire plusieurs centaines de variable. Nous vous recommandons d'y mettre de d'ordre en classant vos variables et en les renommant de telle sorte que leur utilisation devienne intuitive et cohérente. Voici une liste d'idées que vous pouvez mettre en œuvre :

- Etablir une convention de nommage pour vos variables. Cette convention va définir la langue, la casse et les séparateurs utilisés, voici quelques exemples courants :

    - CamelCase ;
    - snake_case ;
    - CAPITAL_CASE ;

- Grouper les features par catégorie et préfixer les noms de variables. Par exemple si vos features sont issues de 4 bases de données différentes (client, contrat, sinistre, devis) vous pouvez envisager une notation similaire à celle-ci :

    - client.feature_1, client.Feature_x... pour les variables de la base client ;
    - contrat.features_1, contrat.feature_x... pour les variables contrats ;
    - sinistre.feature_x... pour les variables sinistres ;
    - devis.feature_x... pour les variables devis ;
    - cible.nb_sinistres_rc, cible.cout_sinistres_rc... pour les variables cibles.

- Identifier les interactions de manière intuitive. Une interaction peut être définie par la combinaison de plusieurs variables séparées par un caractère spécial, par exemple '\*' : Client.nb_contrats_*_type_logement.

- Distinguer les variables catégorielles et ordinales :

    - client.type_logement&, avec un caractère & à la fin pour identifier les variables catégorielles ;
    - client.age#, avec un caractère # à la fin pour identifier les variables ordinales ;

Libre à vous de choisir les catégories et les conventions qui vous semblent les plus pertinentes !

## Extraction des mouvements contrats

La base contrats contient les informations sur les garanties et les facteurs de risques. Cette base peut se présenter sous plusieurs formes en fonction de l'architecture de votre système d'information : une base unique contenant pour chaque contrat tout son historique, en ensemble de vues "photos" du portefeuille prise avec une fréquence régulière (mensuelle par exemple), ou bien encore la séquence de tous les événements survenus sur le portefeuille.

Dans le cadre de l'architecture orienté événement, la base contrat est un générateur d'événement. L'objectif est de générer un nouvel événement à chaque fois que la nature du risque est modifiée. Ainsi le risque couvert est constant dans la période entre deux événements et nous pourrons utiliser cette période pour la phase d'apprentissage statistique. Vous devez donc créer un nouvel événement dans les situations suivantes :

- début de la période d'observation ;
- fin de la période d'observation ;
- prise d'effet d'une (ou plusieurs) garantie ;
- suspension d'une (ou plusieurs) garantie ;
- début d'une année (ou de début de la durée de référence du reserving) ;
- modification d'un ou plusieurs facteur de risque :
    - déménagement ;
    - changement de véhicule ;
    - modification de la composition familiale ;
    - ...

Le générateur d'événement contrat est un opérateur *map* dans un environnement big data. Il prend en entrée une ligne par contrat et génère un ou plusieurs événements contenant au moins les attributs suivants :

- type d'événement = RISQUE ;
- numéro du contrat (pour le rattachement des sinistres) ;
- date : date d'effet ;
- indicatrices de couverture pour chaque garantie ;
- facteurs de risque ;

## Extraction des sinistres

La base sinistre est le deuxième générateur d'événements. En général son exploitation est plus simple que celle des bases contrats. Ce générateur est également un opérateur *map* dans un environnement big data. Il prend en entrée une ligne par sinistre et génère un événement avec au minimum les attributs suivants :

- type d'événement = SINISTRE ;
- numéro de contrat (pour le rattachement des contrats) ;
- date : date de survenance (et non pas date de déclaration) ;
- indicatrice de sinistre pour chaque type de sinistre modélisé ;
- montant du sinistre pour chaque type de sinistre modélisé ;

Il est possible d'avoir plusieurs types de sinistres modélisés par garantie, par exemple si vous choisissez de modéliser séparément les sinistres attritionnels et les graves.

Vous devez également noter la date d'observation des sinistres afin de pouvoir par la suite appliquer le coefficient de développement à l'ultime issu du triangle de reserving pour la date d'observation des sinistres.

### Vérification de la distribution des sinistres

TODO

Courbe du nombre de sinistres par bande de coût

## Jointure des risques et des sinistres

Cette étape de traitement prend en entrée les événements de type 'RISQUE' créés à partir de la base de contrats et les fusionne avec les événements de type 'SINISTRE' générés par la base des sinistres. Les événements sont regroupés par numéro de contrat et triés par date. Ceci est une opération de *réduction* dans le paradigme MapReduce.

## Création des images

Nous souhaitons maintenant construire des images, c'est à dire des périodes de risque constant associées avec la sinistralité observée. Pour cela nous combinons les événements 'RISQUE' et 'SINISTRE' dans un nouveau type d'événement 'IMAGE'. Une image commence avec un événement 'RISQUE' et se termine avec l'événement 'RISQUE' suivant. Chaque événement 'RISQUE' est donc utilisé deux fois : pour déterminer la fin d'une image et le début de la suivante. Exception faite pour le premier et le dernier événements qui sont utilisé une seule fois chacun pour marquer respectivement le début de la première image et la fin de la dernière.

Les images contiennent également les informations agrégées de tous les événement sinistres dont la date de survenance est comprise entre le début et la fin de l'image.

Ainsi chaque image contient les attributs suivants :

- type d'événements = 'IMAGE' ;
- numéro de contrat ;
- date de début : date de l'événement 'RISQUE' de début de période ;
- date de fin : jour précédant la date de l'événement 'RISQUE' de fin de période ;
- indicatrices de couverture pour chaque garantie de l'événement 'RISQUE' de début de période ;
- facteurs de risque de l'événement 'RISQUE' de début de période ;
- nombre de sinistre par type de sinistre, agrégés sur tous les événements 'SINISTRE' ;
- montant total des sinistres par type de sinistre, agrégés sur tous les événements 'SINISTRE' ;

Ceci est un opérateur *map* dans un environnement big data.

Erreur possible : des sinistres déclarés en dehors de la période de couverture ou sur une garantie qui n'est pas souscrite.

## Calcul de l'exposition

Comme nous l'avons vu plus haut un même contrat est découpé en plusieurs images de durée inégales afin de garantir que chaque image reflète un risque constant.

Cette opération *map* ajoute, pour chaque image, sa durée d'exposition. L'exposition est la durée de couverture normalisée sur une année. Nous recommandons de calculer l'exposition comme la durée en jour de l'image divisée par 365. La durée de l'image doit être calculée en incluant la date de début et la date de fin. En effet la date de fin, par construction, précède d'un jour la date de début de l'image suivante.

Pour éviter d'avoir une exposition légèrement supérieure à 1 les années bissextiles nous recommandons de supprimer un jour d'exposition pour toutes les périodes contenant un 29 février.

## Filtrage des images

Vous devrez parfois exclure certaines images de la base de modélisation, par exemple :

- les images avec une exposition nulle ;
- les sinistres ouverts à une valeur forfaitaires ;
- ...

Vous devez conserver et identifier toutes les images écartées de la base de modélisation. En effet vous devrez par la suite les réintégrer dans les calculs de réconciliation comptable.

## Discrétisation des facteurs

Cette opération *map* permet de discrétiser les facteurs de risque évalués sur une échelle continue. Par exemple la longueur d'un véhicule doit être transformée en plage de longueur, exemple : [0cm-200cm], [200cm-240cm], [240cm-280cm] ...

A la fin de cette étape nous disposons d'une base de données au format texte qui est utilisable pour la modélisation statistique de la prime pure.


## Optimisation du stockage

La plupart des algorithmes de data-science utilisent en entrée des nombres en virgule flottante encodés sur 8 octets, soit 64 bits. Dans le cas des modélisations en tarification IARD nous utilisons des variables discrétisées et l'encodage standard sur 64 bits est inefficace.

Nous recommandons donc de limiter le nombre de modalités par feature à 256 modalités. Comme il est possible d'encoder 256 états dans un octet de 8 bits, nous pouvons donc encoder chaque facteur sur un octet. Le bénéfice est double : des économies de mémoire et de meilleurs temps de calcul.

Cette opération *map* est optionnelle mais très utile pour la modélisation de grands portefeuilles.

Les valeurs cibles et la variable "exposition" prennent des valeurs réelles continues et il est plus commode de conserver l'encodage standard en virgule flottante.

## Création des Metadata

Une fois votre base de données constituée, nous vous recommandons de constituer un fichier avec les metadata de la base de données afin de pouvoir les transmettre très facilement aux outils de modélisation. L'importance de cette étape est souvent sous-estimée car au moment de la création de la base les équipes ont toutes les données en tête. Toutefois cette étape est cruciale pour permettre l'industrialisation avec l'intégration des processus de préparation des données puis de modélisation. C'est aussi une étape essentielle pour la bonne documentation des travaux et garantir la capacité future à réutiliser et vérifier le travail réalisé. Nous vous recommandons d'utiliser un format structuré standard, par exemple le format JSON qui permet de garantir une très grande compatibilité. Voici une liste minimum des metadata que vous souhaiterez conserver dans un format structuré :

- date de création ;
- nom du projet ;
- description ;
- nombre de lignes ;
- nombre et liste des features ;
- nombre et liste des indicatrices de garantie ;
- nombre et liste des cibles ;
- nom de la variable "exposition" ;
- nombre et liste des modalités ordonnée par feature ;
- code numérique associé à chaque modalité ;

TODO schéma de la base finale

## Réconciliation comptable

La réconciliation comptable est une étape de validation importante. Elle permet de vous assurer que votre base d'apprentissage est cohérente avec vos données business.

Pour cela vous devez confronter vos données avec une source indépendante, idéalement une source comptable de référence. Vous devez vous assurer, pour chaque période comptable de modélisation (en général une année), que les principales statistiques sont égales :

- Exposition ;
- Nombre de sinistre ;
- Cout total ;

Pour cela vous devrez calculer ces éléments d'un coté sur les images incluses dans la base de modélisation, et de l'autre sur les images exclues et additionner les deux pour confronter le résultat à la vision comptable.

\clearpage

Nous vous recommandons d'utiliser, pour chaque garantie, un tableau similaire à celui de la figure ci-dessous :

![Tableau de Réconciliation comptable](img/img_datapre_reco.png){ height=60%  width=100% }
\

Points de vérification :

 - L'exposition, la charge totale et le nombre de sinistres, doivent être comparables pour chaque année sur :
    - le total de la base de modélisation, y compris les images exclues ;
    - la base comptable.

## Documentation

La documentation de la phase de préparation des données est souvent lacunaire car les équipes pensent, à tort, que cela ne crée pas de valeur. Nous avons pu constater que les équipes passent souvent trop rapidement à la phase de modélisation sans avoir sufisament documenté la constitution de la base. Ce qui semble un gain de temps à court terme, ce révèle souvent une perte de temps à long terme car il n'est pas rare de s'apercevoir en fin de modélisation que la base utilisée était biaisée et devoir recommencer le travail.

Mettre par écrit, de manière formalisée, toutes les étapes de la préparation de la base permet de partager les informations, de détecter les incohérences, et de les corriger. De plus c'est sur la base de cette documentation que le management et les auditeurs externes se forgeront une idée de la qualité et donc de la confiance qui doit être accordée au travail réalisé. D'une certaine manière vous devez considérer que la documentation est le véritable "livrable" de la phase de préparation des données.

Vous devez commencer à documenter les progrès dès le début de vos travaux et ensuite de manière continue jusqu'à l'achèvement complet. En procédant ainsi vous obtenez un double bénéfice. En premier lieu, cela facilite le travail de rédaction et de formalisation qui se réalise au quotidien et non pas à la fin. En second lieu la formalisation continue permet de partager les avancées, de détecter très tôt les éventuelles erreurs et permet de restituer tout le cheminement intellectuel qui a permis d'aboutir aux résultats proposés.

Pour obtenir une documentation de qualité, vous devez vous assurer que votre équipe ait les bons objectifs et la culture adaptée. Vous devez élaborer les objectifs de tels sorte que la documentation soit au cœur des résultats attendus. Par exemple vous pouvez formuler l'objectif : *la phase de data préparation est achevée lorsque l'équipe de seconde opinion valide la méthodologie sur la base de la documentation*.

### Check-list

Voici une check-list des éléments minimums qui doivent être documentés pour la phase de préparation des données :

- dictionnaire des données ;
- traitements réalisés avec la description textuelle et le code associé ;
- données d'entrées ;
- base(s) d'apprentissage générée(s) avec leurs metadata ;
- calcul de la réconciliation comptable ;
- distribution des sinistres en nombre et en coût.

# Apprentissage

## Modèles linéaire Généralisés

### Loi de Poisson

### Loi Gamma

## Méthodologie

### Création des bases

- test / train
- cross validation
- toujours utiliser l'année

### Normalisation

### Variables corrélées

## Optimisation convexe

### Algorithme d'ordre 1

- Descente du Gradient
- Descente par Coordonnées
- Descente du Gradient Stochastique
- IRS

### Algorithme d'ordre 2

- Newton-Raphson
- BFGS (et L-BFGS)

### Recommandation d'algorithme pour le big data

## Traitement des modalités à faible exposition

### Mise à zéro du coefficient linéaire

### Lissage par spline

## Interactions entre variables

# Sélection des variables

Nous disposons grâce au big data d'un nombre toujours plus grand de variables à tester pour nos modèle. Toutefois il est illusoire de vouloir toutes les utiliser dans un modèle de production. Il faut donc choisir le meilleur groupe de variables en fonction de votre stratégie commerciale. Réaliser une bonne sélection des variables est probablement l'étape la plus importante de la modélisation et cela se renforce à mesure que notre capacité à traiter de nouvelles variables s'accroît.

## Objectifs

Dès lors que vous disposez d'un grand nombre de variables (plusieurs dizaines), il est souvent possible d'obtenir des performances prédictives proches avec des variables différentes. Il est donc important de bien définir vos objectifs pour guider vos choix.

### Performance Predictive

La performance prédictive vient tout naturellement à l'esprit comme le premier critère de choix. C'est malheureusement parfois le seul qui est pris en compte. Le choix des variables doit souvent mettre en balance d'autres critère au depénd de quelques dizièmes de point de gini.

### Interprétabilité

Vous souhaiterez probablement communiquer de manière pédagogique sur les variables constituant vos modèles. Cela est d'autant plus facile que vos features sont faciles à interpréter. Imaginons par exemple que vous ayez le choix, en assurance automobile, entre deux variables véhicules :

   - la somme de la longueur et de la largeur du véhicule ;
   - le poids du véhicule ;

A capacité prédictive égale (ou très proche), il est préférable d'utiliser le poids du véhicule qui est plus facilement interprétable pour les novices.

### Sens business

Il est possible qu'une variable sans relation évidente avec le risque modélisé présente pourtant une forte corrélation avec la variable cible. Il est tentant de l'inclure dans le modèle car cela peut augmenter la capacité prédictive du modèle. Il est toutefois préférable de préférer des variables dont on comprend intuitivement l'impact sur la sinistralité.

### Robustesse

- Résistance aux outliers


### Simplicité

*"La simplicité est la sophistication suprême"* selon Léonard de Vinci. Cette maxime s'applique bien à la modélisation IARD. Pour une même puissance prédictive les modèles les plus simples sont les plus longs à réaliser.

Pour obtenir des modèles simples, voici quelques pistes :

  - diminuer le nombre de variables utilisées ;
  - diminuer le nombre de modalités utilisées ;
  - diminuer le nombre d'interactions utilisées.

### Disponibilité

De nombreuses variables ne sont pas disponible ou parfaitement connue au moment où l'on souhaiterait appliquer le modèle en production. Il faut privilégier les variables les plus facilement disponible.

### Crédibilité

Les variables reflètent de manière imparfaite la réalité, surtout si une partie prenante y trouve un avantage. Par exemple, il n'est pas possible de vérifier la durée d'assurance chez l'assureur précédent. Si la variable est utilisée dans les processus opérationnel, il y a un risque que les clients ou les intermédiaires déclarent des valeurs inexactes pour optimiser leur tarif.

Lors du choix d'une variable vous devez vous demander si le processus de collecte permet de garantir avec un degré de confiance suffisant la qualité de la donnée.

### Cout d'acquisition

La collecte et le stockage d'une donnée a toujours un coût. Vous devez essayer d'identifier les coûts associés à chaque variable et privilégier celles qui ont les coûts les plus faibles. Pour les variables les plus onéreuses vous pouvez balancer ce coût avec l'avantage compétitif procuré par l'intégration de la variable dans le modèle.

### Expérience client


vie privée

### Ethique

### Législation

### Stratégie

## Manuelle

## Forward Stepwise

## Backward Stepwise

## Greedy Process

## Pénalisation

### Lasso

### Ridge

## Métriques

### AIC

### BIC

### Norme 2

### CGini

### Spread 100/0

### Spread 95/5

## Chemin de sélection

## Recommandation d'algorithme pour le big data

# Evaluation d'un modèle

## Evaluation globale

### Réduction de la déviance

#### Déviance de la loi de Poisson

#### Déviance de la loi gamma

### Gini

#### Courbe de Lorentz

#### Coefficient de Gini

### RMSE

### Courbe de Gain

### Résidus

## Evéluation de chaque variable

### Spread

#### Spread 100/0

#### Spread 95/5

### Relativités

# Lissage spatial

## Méthodologie

## Algorithmes par agrégation

## Algorithme par proximité

## Validation

# Réconciliation comptable

Réintégration des sinistres exclus

Projections à l'ultime

Tendances et Inflation

Ajustement des niveaux de base

# Documentation

## Dictionnaire des données

## Traitements réalisés

## Distribution des données modélisées

## Modèles

- Relativities
- Base levels

## Métriques

## Courbe d'efficience

## Liste des variables

## Courbe de gain (lift curve)

## Variable par gain de gini

## Résidus

## Courbe des relativités par variable

## Documentation des bases levels

# Validation

## Check List

## Seconde Opinion

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

# Culture


# Leviers

## Data

### Quantity

Client interactions
Digital
Télématics
External

### Quality


## Modélisation

### Nombre de features

### Profondeur d'historique

### Recherche d'interaction

### Sélection des variables

### Sophistication des zoniers

## Culture

### Open Source

Revue par les pairs

### Documentation

## Business Analytics

ELR
Expected Frequency
Client Value
