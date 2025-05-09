#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct _noeud {
    char * mot;
    int nb_occ;
    struct _noeud * fg, * fd;
} Noeud, * ABRnois;



typedef struct _cell {
    Noeud * n ;
    struct _cell * suivant ;
} Cell , * Liste;


// Alloue un nouveau noeud
Noeud * alloue_noeud(char * mot) {
    Noeud *n = malloc(sizeof(Noeud));
    if (!n) return NULL;
    n->mot = malloc(strlen(mot) + 1);
    if (!n->mot) {
        free(n);
        return NULL;
    }
    strcpy(n->mot, mot);
    n->nb_occ = 1;
    n->fg = NULL;
    n->fd = NULL;
    return n;
}

// Libère la mémoire allouée pour l'arbre
void libere_arbre(ABRnois a) {
    if (!a) return;
    libere_arbre(a->fg);
    libere_arbre(a->fd);
    free(a->mot);
    free(a);
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


void rotation_gauche(ABRnois * r) {
// Effectue une rotation gauche sur l'arbre binaire A.
    Noeud *d = (*r)->fd;
    (*r)->fd = d->fg;
    d->fg = *r;
    *r = d;
}


void *rotation_droite(ABRnois * r) {
// Effectue une rotation droite sur l'arbre binaire A.
    Noeud *g = (*r)->fg;
    (*r)->fg = g->fd;
    g->fd = *r;
    *r = g;
}



// Inserts a word into the binary search tree.
// If the word does not exist in the tree, it is added as a new node.
// If the word already exists, its occurrence count is incremented.
// Performs left or right rotations if necessary to maintain tree balance.
// Returns 1 if a new node is inserted, 0 if the word already existed.

int insert_ABRnois(ABRnois * A, char * mot) {
    if (*A == NULL) {
        *A = alloue_noeud(mot);
        return 1;
    } else if (strcmp(mot, (*A)->mot) < 0) {
        int res = insert_ABRnois(&(*A)->fg, mot);
        if(res == 1) {
            if((*A)->fg &&(*A)->nb_occ < (*A)->fg->nb_occ) {
                rotation_droite(&(*A));
                return 1;
            }else {
                return res;
            }
        }
    } else if (strcmp(mot, (*A)->mot) > 0) {
        int res = insert_ABRnois(&(*A)->fd, mot);
        if(res == 1) {
            if((*A)->fd &&(*A)->nb_occ < (*A)->fd->nb_occ) {
                rotation_gauche(&(*A));
                return 1;
            }
            else {
                return res;
            }
        }
    } else {
        (*A)->nb_occ++;
        return 1;
    }
}




void add_to_list_sorted(Liste *lst, Noeud *node) {
    Cell *new_cell = malloc(sizeof(Cell));
    if (!new_cell) return; // Handle allocation failure

    new_cell->n = node;
    new_cell->suivant = NULL; // Initialize next pointer

    // Case 1: List is empty or new node should be the new head
    if (*lst == NULL || strcmp(node->mot, (*lst)->n->mot) < 0) {
        new_cell->suivant = *lst;
        *lst = new_cell;
        return;
    }

    // Case 2: Find the correct position to insert
    Cell *current = *lst;
    while (current->suivant != NULL && strcmp(node->mot, current->suivant->n->mot) >= 0) {
        current = current->suivant;
    }

    // Insert the new node
    new_cell->suivant = current->suivant;
    current->suivant = new_cell;
}
// Helper function to collect nodes with a target occurrence count
void collect_nodes_with_occ(ABRnois a, int target_occ, Liste *lst) {
    if (a == NULL) {
        return;
    }
    if (a->nb_occ == target_occ) {
        add_to_list_sorted(lst, a);
    }else return ;
     // In-order traversal
    collect_nodes_with_occ(a->fg, target_occ, lst);

    collect_nodes_with_occ(a->fd, target_occ, lst);
}

// Function to extract elements with the highest occurrence into a list
// Returns the maximum occurrence count, or -1 if the tree is empty.
int extrait_priorite_max(ABRnois * A, Liste * lst) {
    *lst = NULL; // Initialize the list to empty

    if (*A == NULL) {
        return 0; // Indicate empty tree
    }

    int priorite_max = (*A)->nb_occ;

    if (priorite_max > 0) { // Only collect if max occurrence is positive
        collect_nodes_with_occ(*A, priorite_max, lst);
    }

    return priorite_max;
}




// Function to print the contents of the list
void print_list(Liste lst) {
    printf("List of nodes with target occurrence (from root):\n");
    Cell *current = lst;
    while (current != NULL) {
        printf("  - Word: %s, Occurrences: %d\n", current->n->mot, current->n->nb_occ);
        current = current->suivant;
    }
    if (lst == NULL) {
        printf("  (List is empty)\n");
    }
}

// Function to free the memory allocated for the list cells
void libere_liste(Liste lst) {
    Cell *current = lst;
    Cell *next;
    while (current != NULL) {
        next = current->suivant;
        // Note: We do NOT free current->n->mot or current->n
        // because these nodes belong to the tree and will be freed by libere_arbre.
        free(current);
        current = next;
    }
}

// Fonction principale
int main() {
    ABRnois racine = NULL; // Start with an empty tree
    Liste max_occ_list = NULL;
    int extracted_count;

    printf("Inserting words...\n");

    // Insert words to create a tree with varying occurrences
    insert_ABRnois(&racine, "apple"); // occ 1
    insert_ABRnois(&racine, "banana"); // occ 1
    insert_ABRnois(&racine, "apple"); // occ 2
    insert_ABRnois(&racine, "cherry"); // occ 1
    insert_ABRnois(&racine, "date"); // occ 1
    insert_ABRnois(&racine, "banana"); // occ 2
    insert_ABRnois(&racine, "fig"); // occ 1
    insert_ABRnois(&racine, "grape"); // occ 1
    insert_ABRnois(&racine, "apple"); // occ 3 (root)
    insert_ABRnois(&racine, "date"); // occ 2
    insert_ABRnois(&racine, "fig"); // occ 2
    insert_ABRnois(&racine, "banana"); // occ 3

    printf("Finished insertions.\n");
    exporte_arbre("final_tree.pdf", racine);
    printf("Final tree exported to final_tree.pdf\n");

    // The root node should be "apple" with occ 3 based on the insertion order and balancing.
    // Let's verify the root's occurrence before calling the function.
    if (racine != NULL) {
        printf("\nRoot node: %s, Occurrence: %d\n", racine->mot, racine->nb_occ);
    } else {
        printf("\nTree is empty.\n");
    }


    // Extract nodes with the root's occurrence
    extracted_count = extrait_priorite_max(&racine, &max_occ_list);

    if (extracted_count > 0) {
        printf("\nNumber of nodes extracted with root's occurrence (%d): %d\n", racine->nb_occ, extracted_count);
        print_list(max_occ_list);
    } else {
        printf("\nNo nodes extracted with the root's occurrence, or tree is empty.\n");
    }


    // Libération mémoire
    libere_liste(max_occ_list); // Free the list cells
    libere_arbre(racine); // Free the tree nodes
    printf("\nMemory freed.\n");

    return 0;
}