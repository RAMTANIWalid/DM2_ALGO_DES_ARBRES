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
    }
     // In-order traversal
    collect_nodes_with_occ(a->fg, target_occ, lst);

    collect_nodes_with_occ(a->fd, target_occ, lst);
}

// Function to extract elements with the highest occurrence into a list
// Returns the maximum occurrence count, or -1 if the tree is empty.
int extrait_priorite_max(ABRnois * A, Liste * lst, int priorite_max){
    *lst = NULL; // Initialize the list to empty

    if (*A == NULL) {
        return 0; // Indicate empty tree
    }

    if (priorite_max > 0) { // Only collect if max occurrence is positive
        collect_nodes_with_occ(*A, priorite_max, lst);
    }

    return priorite_max;
}




// Function to print the contents of the list
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


void writeFrequentToFile(Liste lst, FILE *f, int totalWords) {
    if (!f) {
        perror("Erreur ouverture fichier");
        return;
    }

    for (Liste tmp = lst; tmp != NULL; tmp = tmp->suivant) {
        double percentage = (tmp->n->nb_occ / (double)totalWords) * 100.0;
        fprintf(f, "%s %.2f%%\n", tmp->n->mot, percentage);
    }
}
// Fonction principale
int main(int argc, char *argv[]) {
    int total_n = 0;
    int highest_occ = 0;
    printf("arvg[1] : %s\n", argv[1]);
    FILE *frequent = fopen(argv[1], "w");

    ABRnois racine = NULL; // Start with an empty tree
    int indice = 1;
    int pad = 0
    if(strcmp(argv[argc -1], "-g") == 0){
       pad++;
    }
    if(strcmp(argv[argc - 2], "-n") == 0){
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
            if(strcmp(argv[argc - 1], "-g") == 0){
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

    Liste lst = NULL;
    highest_occ = extrait_priorite_max(&racine, &lst, racine->nb_occ);
    printf("highrdt_occ = %d\n", highest_occ);
    printf("total_n = %d\n", total_n);
    // Print the list of words with their frequencies
    int p ;
    if(strcmp(argv[argc - 2], "-n") == 0){
        p = (int )argv[argc - 2];
    }
    while(racine){
        libere_liste(lst);
        if(highest_occ == 0) break;
        lst = NULL;
        extrait_priorite_max(&racine, &lst, highest_occ);
        int t = print_list(lst);
        if(strcmp(argv[argc - 2], "-n") == 0){
            if(p > 0){
                writeFrequentToFile(lst, frequent, total_n);
                p -= t;
            }
            else
                break;
        }else
            writeFrequentToFile(lst, frequent, total_n);
        highest_occ--;

    }
    fclose(frequent);
    libere_arbre(racine); // Free the tree nodes
    printf("\nMemory freed.\n");

    return 0;
}
