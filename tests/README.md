# Tests Hexodus

Framework `tester.hpp` repris du projet NovaC fourni par l'auteur, sans dépendance externe.

```sh
make test
make test-sanitize
```

GCC 13+ avec libstdc++ prenant en charge `<format>`, C++20, Linux.
Les tests n'ouvrent aucun terminal, n'accèdent pas à evdev et ne demandent pas root.
`TEST(Suite, Nom)`, `CHECK`, `CHECK_EQ`, `CHECK_NEAR` : ajouter un fichier `.cpp` dans tests/ suffit.
Le runner renvoie un code non nul en cas d'échec, y compris avec NDEBUG.

11 cas : conservation RGB indépendamment de l'alpha, reconstruction des 16 masques,
stabilité de l'encodage, séquences ANSI normales/brillantes/truecolor,
occlusion opaque dans les deux ordres, alpha nul, clipping proche.
Les adaptateurs friend ne changent pas l'API publique et permettent des tests sans ioctl.
L'encodeur est devenu static car il n'utilise aucun état ; son algorithme reste identique.

`testMassAlgebra.cpp` utilise maintenant le même framework : 12 cas supplémentaires
pour les vecteurs, matrices, transformations, coordonnées homogènes et exceptions.
Il inclut uniquement les headers, sans inclure de `.cpp` ni définir un second main.

```sh
make test-algebra       # 12 tests d'algèbre uniquement
make testMassAlgebra    # alias compatible
make -j4 test           # 23 tests, toutes suites réunies
```

Validation GCC 13.3 : compilation release et 23/23 tests réussis,
dont 12/12 dans le binaire d'algèbre isolé. Reconstruction incrémentale vérifiée.
Les conventions existantes sont conservées (vecteurs-lignes, sens des rotations,
addition vecteur + point refusée). Point - point produit bien un vecteur.

Le binaire ASan + UBSan compile, mais son exécution est bloquée par LeakSanitizer ; aucun résultat sanitizer validé.
LeakSanitizer ne peut pas fonctionner dans l'environnement de validation (accès /proc restreint).
Sur une machine normale, `make test-sanitize` conserve la détection des fuites par défaut.

Pas de validation visuelle en vraie console Linux ni dans les différents émulateurs.
