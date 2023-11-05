#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef struct Node {
    unsigned long rang;
    unsigned long x;
    unsigned long y;
    struct Node* parent;
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

// liste des racines des arbres
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
            if(pixels[i][j].rouge == 0) {
                fprintf(f, "%d %d %d", 0, 0, 0);
            } else {
                Node* current = pixels[i][j].node->parent;
                while(
                    (pixels[current->x][current->y].bleu == pixels[i][j].bleu && 
                    pixels[current->x][current->y].rouge == pixels[i][j].rouge &&
                    pixels[current->x][current->y].vert == pixels[i][j].vert) &&
                    current->parent != current
                ) {
                    current = current->parent;
                }
                pixels[i][j].bleu = pixels[current->x][current->y].bleu;
                pixels[i][j].rouge = pixels[current->x][current->y].rouge;
                pixels[i][j].vert = pixels[current->x][current->y].vert;
                fprintf(f, "%lu %lu %lu", pixels[i][j].rouge, pixels[i][j].vert, pixels[i][j].bleu);
            }
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
    node->parent = node;
    node->x = x;
    node->y = y;
    node->rang = 0;
    return node;
}

Node* FindSet(Node* node) {
    // On cherche la racine de l'arbre O(log(n))
    Node* par = node;
    while(par != par->parent) {
        par = par->parent;
    }
    return par;
}

void Union(Node* node1, Node* node2, unsigned long h, Pixel** pixels) {
    // on cherche les racines des arbres
    Node* rep1 = FindSet(node1);
    Node* rep2 = FindSet(node2);

    // on vérifie que les deux noeuds ne sont pas déjà dans la même liste
    if(rep1 == rep2) {
        return;
    }

    // sinon, on ajoute un arbre à l'autre
    Node* current = node1;
    rep2->parent = rep1;
    unsigned long r = rep2->rang + 1;
    do {
        current->rang = current->rang > r ? current->rang : r;
        r++;
        current = current->parent;
    } while(current != current->parent);
    // On enleve la racine de l'arbre qu'on a ajouté à l'autre
    for(int i=0; i<nbSets; i++) {
        if(sets[i] == rep2) {
            for(int j=i; j<nbSets-1; j++) {
                sets[j] = sets[j+1];
            }
            nbSets--;
            break;
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
                if(second == NULL) {
                    second = MakeSet(pixels[i+1][j].rouge, i+1, j, hauteur, largeur);
                    pixels[i+1][j].node = second;
                    sets[nbSets] = second;
                    nbSets++;
                }
                Union(head, second, hauteur, pixels);
            }
            if(j < largeur - 1 && pixels[i][j+1].rouge == pixels[i][j].rouge) {
                Node* second = pixels[i][j+1].node;
                if(second == NULL) {
                    second = MakeSet(pixels[i][j+1].rouge, i, j+1, hauteur, largeur);
                    pixels[i][j+1].node = second;
                    sets[nbSets] = second;
                    nbSets++;
                }
                Union(head, second, hauteur, pixels);
            }
            if(i < hauteur - 1 && j < largeur - 1 && pixels[i+1][j+1].rouge == pixels[i][j].rouge) {
                Node* second = pixels[i+1][j+1].node;
                if(second == NULL) {
                    second = MakeSet(pixels[i+1][j+1].rouge, i+1, j+1, hauteur, largeur);
                    pixels[i+1][j+1].node = second;
                    sets[nbSets] = second;
                    nbSets++;
                }
                Union(head, second, hauteur, pixels);
            }
            if(j > 0 && i < hauteur - 1 &&  pixels[i+1][j-1].rouge == pixels[i][j].rouge) {
                Node* second = pixels[i+1][j-1].node;
                if(second == NULL) {
                    second = MakeSet(pixels[i+1][j-1].rouge, i+1, j-1, hauteur, largeur);
                    pixels[i+1][j-1].node = second;
                    sets[nbSets] = second;
                    nbSets++;
                }
                Union(head, second, hauteur, pixels);
            }
            if(i > 0 && j < largeur - 1 && pixels[i-1][j+1].rouge == pixels[i][j].rouge) {
                Node* second = pixels[i-1][j+1].node;
                if(second == NULL) {
                    second = MakeSet(pixels[i-1][j+1].rouge, i-1, j+1, hauteur, largeur);
                    pixels[i-1][j+1].node = second;
                    sets[nbSets] = second;
                    nbSets++;
                }
                Union(head, second, hauteur, pixels);
            }
            if(i > 0 && pixels[i-1][j].rouge == pixels[i][j].rouge) {
                Node* second = pixels[i-1][j].node;
                if(second == NULL) {
                    second = MakeSet(pixels[i-1][j].rouge, i-1, j, hauteur, largeur);
                    pixels[i-1][j].node = second;
                    sets[nbSets] = second;
                    nbSets++;
                }
                Union(head, second, hauteur, pixels);
            }
            if(j > 0 && pixels[i][j-1].rouge == pixels[i][j].rouge) {
                Node* second = pixels[i][j-1].node;
                if(second == NULL) {
                    second = MakeSet(pixels[i][j-1].rouge, i, j-1, hauteur, largeur);
                    pixels[i][j-1].node = second;
                    sets[nbSets] = second;
                    nbSets++;
                }
                Union(head, second, hauteur, pixels);
            }
            if(j > 0 && i > 0 && pixels[i-1][j-1].rouge == pixels[i][j].rouge) {
                Node* second = pixels[i-1][j-1].node;
                if(second == NULL) {
                    second = MakeSet(pixels[i-1][j-1].rouge, i-1, j-1, hauteur, largeur);
                    pixels[i-1][j-1].node = second;
                    sets[nbSets] = second;
                    nbSets++;
                }
                Union(head, second, hauteur, pixels);
            }
        }
    }
    printf("Nombre d'ensembles = %lu\n", nbSets);
    for(int i=0; i<nbSets; i++) {
        // On genere un nombre aleatoire entre 0 et 255 pour chaque couleur et on l'applique a la tete de chaque ensemble
        int r = rand() % 256;
        int g = rand() % 256;
        int b = rand() % 256;
        Node* current = sets[i];
        pixels[current->x][current->y].rouge = r;
        pixels[current->x][current->y].vert = g;
        pixels[current->x][current->y].bleu = b;
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

