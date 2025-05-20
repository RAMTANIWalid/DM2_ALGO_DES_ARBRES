#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Structures

typedef struct _noeud {
    char *mot;
    int nb_occ;
    struct _noeud *fg, *fd;
} Noeud, *ABRnois;

typedef struct _cell {
    Noeud *n;
    struct _cell *suivant;
} Cell, *Liste;

// Fonctions de base

Noeud *alloue_noeud(char *mot) {
    Noeud *n = malloc(sizeof(Noeud));
    if (!n) return NULL;
    n->mot = malloc(strlen(mot) + 1);
    if (!n->mot) {
        free(n);
        return NULL;
    }
    strcpy(n->mot, mot);
    n->nb_occ = 1;
    n->fg = n->fd = NULL;
    return n;
}

Cell *alloue_cell(Noeud *n) {
    Cell *c = malloc(sizeof(Cell));
    Noeud *tmp = alloue_noeud(n->mot);
    if (!c) return NULL;
    c->n = tmp;
    c->n->nb_occ = n->nb_occ;
    c->suivant = NULL;
    return c;
}

void libererNoeud(Noeud *n) {
    if (n) {
        free(n->mot);
        free(n);
    }
}

void libere_liste(Liste lst) {
    Cell *courant = lst;
    while (courant) {
        Cell *temp = courant;
        courant = courant->suivant;
        free(temp);
    }
}

void libere_arbre(ABRnois a) {
    if (!a) return;
    libere_arbre(a->fg);
    libere_arbre(a->fd);
    libererNoeud(a);
}

// Rotations

void rotation_gauche(ABRnois *r) {
    Noeud *d = (*r)->fd;
    (*r)->fd = d->fg;
    d->fg = *r;
    *r = d;
}

void rotation_droite(ABRnois *r) {
    Noeud *g = (*r)->fg;
    (*r)->fg = g->fd;
    g->fd = *r;
    *r = g;
}

// Insertion
ABRnois insererABRnois(ABRnois arbre, char *mot, int nb_occ) {
    if (!arbre) {
        Noeud *n = alloue_noeud(mot);
        if (n) n->nb_occ = nb_occ;
        return n;
    }
    int cmp = strcmp(mot, arbre->mot);
    if (cmp < 0) {
        arbre->fg = insererABRnois(arbre->fg, mot, nb_occ);
    } else if (cmp > 0) {
        arbre->fd = insererABRnois(arbre->fd, mot, nb_occ);
    } else {
        arbre->nb_occ += nb_occ;
    }
    if (arbre->fd && arbre->fd->nb_occ > arbre->nb_occ) {
        rotation_gauche(&arbre);
    }
    if (arbre->fg && arbre->fg->nb_occ > arbre->nb_occ) {
        rotation_droite(&arbre);
    }
    return arbre;
}

int insert_ABRnois(ABRnois *A, char *mot) {
    if (!A || !mot) return 0;
    *A = insererABRnois(*A, mot, 1);
    return 1;
}


// Suppression

ABRnois descendreNoeud(ABRnois *arbre, char *mot) {
    if (!*arbre) return NULL;
    int cmp = strcmp(mot, (*arbre)->mot);
    if (cmp == 0) {
        if ((*arbre)->fg && (*arbre)->fd) {
            if ((*arbre)->fg->nb_occ >= (*arbre)->fd->nb_occ) {
                rotation_droite(arbre);
                (*arbre)->fd = descendreNoeud(&(*arbre)->fd, mot);
            } else {
                rotation_gauche(arbre);
                (*arbre)->fg = descendreNoeud(&(*arbre)->fg, mot);
            }
        } else if ((*arbre)->fg) {
            rotation_droite(arbre);
            (*arbre)->fd = descendreNoeud(&(*arbre)->fd, mot);
        } else if ((*arbre)->fd) {
            rotation_gauche(arbre);
            (*arbre)->fg = descendreNoeud(&(*arbre)->fg, mot);
        } else {
            libererNoeud(*arbre);
            *arbre = NULL;
        }
        return *arbre;
    } else if (cmp < 0) {
        (*arbre)->fg = descendreNoeud(&(*arbre)->fg, mot);
    } else {
        (*arbre)->fd = descendreNoeud(&(*arbre)->fd, mot);
    }
    return *arbre;
}

ABRnois supprimerABRnois(ABRnois *arbre, char *mot) {
    return descendreNoeud(arbre, mot);
}

// Analyse

int trouver_max_occ(ABRnois A) {
    if (!A) return 0;
    int max = A->nb_occ;
    int g = trouver_max_occ(A->fg);
    int d = trouver_max_occ(A->fd);
    if (g > max) max = g;
    if (d > max) max = d;
    return max;
}

void collecter_noeuds(ABRnois A, int nb_occ, Liste *lst) {
    if (!A) return;
    if (A->nb_occ == nb_occ) {
        Cell *c = alloue_cell(A);
        if (c) {
            c->suivant = *lst;
            *lst = c;
        }
    }
    collecter_noeuds(A->fg, nb_occ, lst);
    collecter_noeuds(A->fd, nb_occ, lst);
}

void trier_liste(Liste *lst) {
    if (!*lst || !(*lst)->suivant) return;
    for (Cell *i = *lst; i->suivant; i = i->suivant) {
        for (Cell *j = i->suivant; j; j = j->suivant) {
            if (strcmp(i->n->mot, j->n->mot) > 0) {
                Noeud *tmp = i->n;
                i->n = j->n;
                j->n = tmp;
            }
        }
    }
}

int longueur_liste(Liste lst) {
    int len = 0;
    while (lst) {
        len++;
        lst = lst->suivant;
    }
    return len;
}



int print_list(Liste lst) {
    printf("List of nodes with target occurrence (from root):\n");
    int t = 0;
    Cell *current = lst;
    while (current != NULL) {
        printf("  - Word: %s, Occurrences: %d\n", current->n->mot, current->n->nb_occ);
        current = current->suivant;
        t++;
    }
    if (lst == NULL) {
        printf("  (List is empty)\n");
    }
    return t;
}
void writeFrequentToFile(Liste lst, FILE *f, int totalWords) {
    if (!f) {
        perror("Erreur ouverture fichier");
        return;
    }
    for (Liste tmp = lst; tmp != NULL; tmp = tmp->suivant) {
        float percentage = (tmp->n->nb_occ * 100/ (float)totalWords);
        fprintf(f, "%s %.2f%\n", tmp->n->mot, percentage);
    }
}

int extrait_priorite_max(ABRnois *A, Liste *lst) {
    if (!A || !(*A)) return 0;
    int max_occ = (*A)->nb_occ;
    if (max_occ == 0) return 0;
    *lst = NULL;

    collecter_noeuds(*A, max_occ, lst);
    trier_liste(lst);
    for (Cell *c = *lst; c != NULL; c = c->suivant){
        // Exporte l'arbre en PDF
        supprimerABRnois(A, c->n->mot);
    
    }
    return max_occ;
}



/**
 * Cherche si un texte est présent dans argv (à partir de l'index 1).
 * @param argc Nombre d'éléments dans argv.
 * @param argv Tableau de chaînes de caractères.
 * @param text Chaîne de caractères à chercher.
 * @return L'index de la chaîne de caractères dans argv si elle est trouvée, 0 sinon.
 */
int text_in_argv(int argc, char *argv[], const char *text) {
    // On commence à 1 car argv[0] est généralement le nom du programme
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], text) == 0) {
            return i; // Trouvé
        }
    }
    return 0; // Non trouvé
}


// Partie dot (graphviz)
void ecrireDebut(FILE *f) {
    if (!f) return;
    fprintf(f, "digraph arbre {\n");
    fprintf(f, "node [ shape = record, height = .1]\n");
    fprintf(f, "edge [ tailclip = false, arrowtail = dot, dir = both ];\n");
}

void ecrireArbre(FILE *f, ABRnois A) {
    if (!A || !f) return;
    fprintf(f, "n%p [ label = \"<gauche> | <valeur> %s\\n(%d) | <droit>\"];\n", 
            (void*)A, A->mot, A->nb_occ);
    if (A->fg) {
        fprintf(f, "n%p:gauche:c -> n%p;\n", (void*)A, (void*)A->fg);
        ecrireArbre(f, A->fg);
    }
    if (A->fd) {
        fprintf(f, "n%p:droit:c -> n%p;\n", (void*)A, (void*)A->fd);
        ecrireArbre(f, A->fd);
    }
}

void ecrireFin(FILE *f) {
    if (!f) return;
    fprintf(f, "}\n");
}

void dessine(FILE *f, ABRnois a) {
    ecrireDebut(f);
    ecrireArbre(f, a);
    ecrireFin(f);
}

// Crée le PDF à partir de l'arbre
void creePDF(char *pdf, ABRnois a) {
    const char *dot = "arbre.dot";
    FILE *out = fopen(dot, "w");
    if (!out) {
        fprintf(stderr, "Erreur lors de l'ouverture du fichier DOT.\n");
        return;
    }
    dessine(out, a);
    fclose(out);

    char cmd[256];
    snprintf(cmd, sizeof(cmd), "dot -Tpdf %s -o %s", dot, pdf);
    system(cmd);  // Nécessite que Graphviz soit installé
}

// Exporte l'arbre sous forme de fichier PDF
// Retourne 0 en cas de succès, -1 en cas d'erreur
int exporte_arbre(char *nom_pdf, ABRnois A) {
    // Vérification des paramètres
    if (!nom_pdf || !A) return -1;

    // Exportation de l'arbre en PDF
    creePDF(nom_pdf, A);

    // Retour en cas de succès
    return 0;
}


// Fonction principale
int main(int argc, char *argv[]) {
    int total_n = 0;
    int highest_occ = 0;
    FILE *frequent = fopen(argv[1], "w");

    ABRnois racine = NULL; // Start with an empty tree
    int indice = 1;
    int pad = 0;
    int g, n, p = 1;
    g = text_in_argv(argc, argv, "-g");
    if(g){
       pad++;
    }
    n = text_in_argv(argc, argv, "-n");
    if(n){
        p = atoi(argv[n + 1]);
        printf("%d\n", p);
        pad++;
        pad++;
     }
    while (indice + 1 < argc - pad) {
        FILE *f = fopen(argv[indice + 1], "r");
        if (!f) {
            printf("Error opening file %s\n", argv[indice + 1]);
            return 1;
        }

        char line[1024];
        while (fgets(line, sizeof(line), f)) {
            // Remove newline character at the end of the line
            line[strcspn(line, "\n")] = '\0';

            char *token = strtok(line, " ");
            while (token) {
                // Convert the word to lowercase
                for (char *p = token; *p; p++) {
                    *p = tolower(*p);
                }
                // Insert the word into the ABR
                insert_ABRnois(&racine, token);
                total_n++;
                // option -g
                if(g){
                    char filename[256];
                    snprintf(filename, sizeof(filename), "insertion%d.pdf", total_n);
                    exporte_arbre(filename, racine);
                }
                // Get the next token
                token = strtok(NULL, " ");
            }
        }

        fclose(f);
        indice += 1;
    }
    exporte_arbre("final_tree.pdf", racine);
    while(racine && p > 0){
        Liste lst = NULL;
        int priorite = extrait_priorite_max(&racine, &lst);
        print_list(lst);
        int len = longueur_liste(lst);
        if (n)
            p -= len;
        writeFrequentToFile(lst, frequent, total_n);
        libere_liste(lst);
    }


    fclose(frequent);
    libere_arbre(racine); // Free the tree nodes
    printf("\nMemory freed.\n");

    return 0;
}