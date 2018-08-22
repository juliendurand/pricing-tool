---
title: La tarification IARD à l'ère du Big Data
author: Julien Durand
subject: Finance Data-Science Big-Data
keywords: Tarification Assurance IARD Data-Science Big-Data
---

TODO

Dire qu'en big data les algorithmes doivent être au plus linaires par rapport au nombre de variables utilisées.

# Introduction {-}

Les assureurs détiennent dans leur portefeuille des millions de contrats automobiles, habitations et professionnels. Ces activités, couramment regroupées sous le vocable IARD (Incendie, Accidents et Risques Divers) génèrent des volumes de données très importants. Pour améliorer leur compétitivité les compagnies d'assurance souhaitent modéliser l'impact de dizaines, voire centaines de variables. L'un des enjeux majeurs est de tarifer au juste prix chaque contrat en fonction de ses caractéristiques individuelles tout en assurant la mutualisation des risques au niveau du portefeuille. Pour avoir un ordre de grandeur, nous pouvons estimer qu'un assureur qui détient un portefeuille de quatre millions de contrats et qui souhaite mesurer l'impact de cent variables sur trois ans d'activité doit traiter 1,2 milliard de point de données. Cela représente 9,6 Go de donnée brute (en comptant une moyenne de 8 octets pour le stockage d'une donnée atomique). Ce volume croit avec la taille du portefeuille, la profondeur de l'historique et le nombre de variables étudiées. Les outils classiques de l'actuaire et du statisticien sont peu adaptés à des volumes aussi importants. La maitrise des technologies big data est donc un facteur clef de succès pour la tarification des contrats IARD.

Face à ce large volume de données, les principaux acteurs du secteur ont historiquement fait le choix de recourir à des solutions externes développées par des vendeurs spécialisés. Le bénéfice principal de cette approche est de fournir des résultats fiables en mutualisant les couts de recherche et développement. En revanche les algorithmes utilisés sont des "boites noires" et cela limite les capacités d'innovations pour exploiter de nouveaux axes de différenciations tarifaires. Les assureurs utilisent le même type de modélisation et la compétition se fait principalement sur la qualité et la quantité des données disponibles. Cela avantage les acteurs qui disposent des plus gros portefeuilles et des meilleurs systèmes d'informations.

Le big data et la data-science changent la donne en mettant à la disposition des actuaires des solutions en kit pour créer des modèles adaptés au business model et à la stratégie de leur compagnie.

L'objectif de ce livre est de fournir les bases méthodologiques et pratiques aux actuaires et data-scientistes qui souhaitent développer des outils de tarifications innovants tout en adhérant aux meilleures pratiques développées par la communauté actuarielle.

Les méthodes proposées sont mises en œuvre dans des exemples de code disponibles sur la plateforme de partage collaborative github.

Ce livre contient intentionnellement peu de formules mathématiques car l'objectif est de proposer un cadre méthodologique permettant aux praticiens de développer des algorithmes adaptés au contexte commercial et stratégique de leur activité.
