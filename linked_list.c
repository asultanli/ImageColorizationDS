#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef struct Node {
    struct Node* representant;
    unsigned long size;
    unsigned long x;
    unsigned long y;
    struct Node* next;
    struct Node* tail;
} Node;

typedef struct Pixel {
    unsigned long rouge;
    unsigned long vert;
    unsigned long bleu;
    struct Node* node;
} Pixel;

Pixel** Read(char fichier[], unsigned long* l, unsigned long* h) {
    FILE* f = fopen(fichier, "rb");

    // Details du fichier
    char magic[3];
    unsigned long largeur, hauteur, max;

    if(f == NULL) {
        printf("Erreur lors de l'ouverture du fichier");
        return NULL;
    }

    // On lit les 3 premiers caractères pour vérifier le type de fichier
    fscanf(f, "%s", magic);
    if(strcmp(magic, "P1") != 0) {
        printf("Le fichier n'est pas au format PBM");
        return NULL;
    }

    // On lit les dimensions du fichier
    fscanf(f, "%lu %lu", &largeur, &hauteur);
    *l = largeur;
    *h = hauteur;

    // On alloue la mémoire pour le tableau de pixels
    Pixel** pixels = malloc(sizeof(Pixel*) * hauteur);

    for(unsigned long i = 0; i < hauteur; i++) {
        pixels[i] = malloc(largeur * sizeof(Pixel));
    }

    // On lit les pixels
    for(unsigned long i = 0; i < hauteur; i++) {
        for(unsigned long j = 0; j < largeur; j++) {
            char couleur;
            fscanf(f, "%c", &couleur);
            if(couleur == ' ' || couleur == '\n') {
                j--;
                continue;
            }
            Pixel p = { ((couleur-'0')+1)%2, ((couleur-'0')+1)%2, ((couleur-'0')+1)%2, NULL };
            pixels[i][j] = p;
        }
    }
    fclose(f);

    return pixels;
}

// liste des listes chaînées
Node** sets = NULL;
unsigned long nbSets = 0;

void Write(char fichier[], Pixel** pixels, unsigned long largeur, unsigned long hauteur) {
    FILE* f = fopen(fichier, "wb");

    if(f == NULL) {
        printf("Erreur lors de l'ouverture du fichier");
        return;
    }

    // On écrit les 3 premiers caractères pour vérifier le type de fichier
    fprintf(f, "P3\n");

    // On écrit les dimensions du fichier et le max
    fprintf(f, "%lu %lu\n255\n", largeur, hauteur);

    // On écrit les pixels
    for(unsigned long i = 0; i < hauteur; i++) {
        unsigned long co = 0;
        for(unsigned long j = 0;j < largeur; j++) {
            fprintf(f, "%lu %lu %lu", pixels[i][j].rouge, pixels[i][j].vert, pixels[i][j].bleu);
            co++;
            // Puisque les lignes sone limitées à 70 caractères, on les sépare lorsqu'on atteint 70 caractères
            // On prend 11 charactères par pixel (3 pour chaque couleur + 2 espaces) et on ajoute 1 pour le retour à la ligne
            if(co%5==0){
                fprintf(f, "\n");
            } else {
                fprintf(f, " ");
            }
        }
        fprintf(f, "\n");
    }
    fclose(f);
}

Pixel** Generate(unsigned long largeur, unsigned long hauteur) {
    Pixel** pixels = malloc(sizeof(int*) * hauteur);

    for(unsigned long i = 0; i < hauteur; i++) {
        pixels[i] = malloc(sizeof(int) * largeur);
    }

    for(unsigned long i = 0; i < hauteur; i++) {
        for(unsigned long j = 0; j < largeur; j++) {
            int color = rand() % 2;
            Pixel p = { color, color, color };
            pixels[i][j] = p;
        }
    }

    return pixels;
}

Node* MakeSet(unsigned long couleur, unsigned long x, unsigned long y, unsigned long h, unsigned long l) {

    Node* node = malloc(sizeof(Node));
    node->next = NULL;
    node->tail = node;
    node->representant = node;
    node->x = x;
    node->y = y;
    node->size = 1;
    return node;
}

Node* FindSet(Node* node) {
    return node->representant;
}

void Union(Node* node1, Node* node2, unsigned long h, int where, Pixel** pixels) {
    // on cherche les représentants des deux noeuds
    Node* rep1 = FindSet(node1);
    Node* rep2 = FindSet(node2);
    // on vérifie que les deux noeuds ne sont pas déjà dans la même liste
    if(rep1 == rep2) {
        return;
    }

    // sinon, on ajoute la liste chaînée la plus petite à la plus grande
    if(rep1->size >= rep2->size) {
        // on ajoute la liste chaînée de rep2 à rep1
        Node* current = rep2;
        rep1->size += rep2->size;
        while(current != NULL) {
            current->representant = rep1;
            current->size = rep1->size;
            current = current->next;
        }
        rep1->tail->next = rep2;
        rep1->tail = rep2->tail;
        rep1->representant = rep1;
        // On enlève la liste chaînée de rep2 de la liste des listes chaînées
        for(int i=0; i<nbSets; i++) {
            if(sets[i] == rep2) {
                for(int j=i; j<nbSets-1; j++) {
                    sets[j] = sets[j+1];
                }
                nbSets--;
                break;
            }
        }
    } else {
        // sinon, on ajoute rep1 à rep2
        Node* current = rep1;
        rep2->size += rep1->size;
        while(current != NULL) {
            current->representant = rep2;
            current->size = rep2->size;
            current = current->next;
        }
        rep2->tail->next = rep1;
        rep2->tail = rep1->tail;
        rep2->representant = rep2;
        // On enlève la liste chaînée de rep1 de la liste des listes chaînées
        for(int i=0; i<nbSets; i++) {
            if(sets[i] == rep1) {
                for(int j=i; j<nbSets-1; j++) {
                    sets[j] = sets[j+1];
                }
                nbSets--;
                break;
            }
        }
    }
}

void Colorie(char file[]) {
    unsigned long largeur, hauteur;
    Pixel** pixels = Read(file, &largeur, &hauteur);
    printf("Lecture reussie\n");

    // on parcourt les pixels de l'image
    sets = malloc(hauteur*largeur*sizeof(Node*));
    for(unsigned long i=0; i<hauteur; i++){
        for(unsigned long j=0; j<largeur; j++) {
            if(pixels[i][j].rouge == 0) {
                continue;
            }
            Node* head = pixels[i][j].node;
            if(pixels[i][j].node == NULL) {
                head = MakeSet(pixels[i][j].rouge, i, j, hauteur, largeur);
                sets[nbSets] = head;
                nbSets++;
                pixels[i][j].node = head;
            }
            if(i < hauteur - 1 && pixels[i+1][j].rouge == pixels[i][j].rouge) {
                Node* second = pixels[i+1][j].node;
                if(second == NULL || (second != head && second->representant != head->representant)) {
                    if(second == NULL) {
                        second = MakeSet(pixels[i+1][j].rouge, i+1, j, hauteur, largeur);
                        pixels[i+1][j].node = second;
                        sets[nbSets] = second;
                        nbSets++;
                    }
                    Union(head, second, hauteur, 0, pixels);
                }
            }
            if(j < largeur - 1 && pixels[i][j+1].rouge == pixels[i][j].rouge) {
                Node* second = pixels[i][j+1].node;
                 if(second == NULL || (second != head && second->representant != head->representant)) {
                    if(second == NULL) {
                        second = MakeSet(pixels[i][j+1].rouge, i, j+1, hauteur, largeur);
                        pixels[i][j+1].node = second;
                        sets[nbSets] = second;
                        nbSets++;
                    }
                    Union(head, second, hauteur, 1, pixels);
                }
            }
            if(i < hauteur - 1 && j < largeur - 1 && pixels[i+1][j+1].rouge == pixels[i][j].rouge) {
                Node* second = pixels[i+1][j+1].node;
                if(second == NULL || (second != head && second->representant != head->representant)) {
                    if(second == NULL) {
                        second = MakeSet(pixels[i+1][j+1].rouge, i+1, j+1, hauteur, largeur);
                        pixels[i+1][j+1].node = second;
                        sets[nbSets] = second;
                        nbSets++;
                    }
                    Union(head, second, hauteur, 2, pixels);
                }
            }
            if(i < hauteur - 1 && j > 0 && pixels[i+1][j-1].rouge == pixels[i][j].rouge) {
                Node* second = pixels[i+1][j-1].node;
                 if(second == NULL || (second != head && second->representant != head->representant)) {
                    if(second == NULL) {
                        second = MakeSet(pixels[i+1][j-1].rouge, i+1, j-1, hauteur, largeur);
                        pixels[i+1][j-1].node = second;
                        sets[nbSets] = second;
                        nbSets++;
                    }
                }
                Union(head, second, hauteur, 3, pixels);
            }
            if(j < largeur - 1 && i > 0 && pixels[i-1][j+1].rouge == pixels[i][j].rouge) {
                Node* second = pixels[i-1][j+1].node;
                if(second == NULL || (second != head && second->representant != head->representant)) {
                    if(second == NULL) {
                        second = MakeSet(pixels[i-1][j+1].rouge, i-1, j+1, hauteur, largeur);
                        pixels[i-1][j+1].node = second;
                        sets[nbSets] = second;
                        nbSets++;
                    }
                    Union(head, second, hauteur, 4, pixels);
                }
            }
            if(i > 0 && pixels[i-1][j].rouge == pixels[i][j].rouge) {
                Node* second = pixels[i-1][j].node;
                 if(second == NULL || (second != head && second->representant != head->representant)) {
                    if(second == NULL) {
                        second = MakeSet(pixels[i-1][j].rouge, i-1, j, hauteur, largeur);
                        pixels[i-1][j].node = second;
                        sets[nbSets] = second;
                        nbSets++;
                    }
                    Union(head, second, hauteur, 5, pixels);
                }
            }
            if(j > 0 && pixels[i][j-1].rouge == pixels[i][j].rouge) {
                Node* second = pixels[i][j-1].node;
                if(second == NULL || (second != head && second->representant != head->representant)) {
                    if(second == NULL) {
                        second = MakeSet(pixels[i][j-1].rouge, i, j-1, hauteur, largeur);
                        pixels[i][j-1].node = second;
                        sets[nbSets] = second;
                        nbSets++;
                    }
                    Union(head, second, hauteur, 5, pixels);
                }
            }
            if(j > 0 && i > 0 && pixels[i-1][j-1].rouge == pixels[i][j].rouge) {
                Node* second = pixels[i-1][j-1].node;
                if(second == NULL || (second != head && second->representant != head->representant)) {
                    if(second == NULL) {
                        second = MakeSet(pixels[i-1][j-1].rouge, i-1, j-1, hauteur, largeur);
                        pixels[i-1][j-1].node = second;
                        sets[nbSets] = second;
                        nbSets++;
                    }
                    Union(head, second, hauteur, 5, pixels);
                }
            }
        }
    }
    printf("Nombre d'ensembles = %lu\n", nbSets);
    for(int i=0; i<nbSets; i++) {
        // generate random color
        int r = rand() % 256;
        int g = rand() % 256;
        int b = rand() % 256;
        Node* current = sets[i];
        while(current != NULL) {
            pixels[current->x][current->y].rouge = r;
            pixels[current->x][current->y].vert = g;
            pixels[current->x][current->y].bleu = b;
            current = current->next;
        }
    }
    char *name = strtok(file, ".");
    char new_name[100];
    sprintf(new_name, "%s_colorised.ppm", name);
    Write(new_name, pixels, largeur, hauteur);
}

int main(int argc, char** argv) {
    unsigned long largeur, hauteur;
    Colorie(argv[1]);
    return 0;
}

