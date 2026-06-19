#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#define MAX_PALAVRAS   2000 //txt ta com 1604
#define WORD_LEN       4 //palavras de 4 letras
#define INF INT_MAX

// Lista de adjacência 
typedef struct Aresta {
    int destino;
    int peso; // número de letras diferentes, sempre 1) 
    struct Aresta *prox;
} Aresta;

typedef struct {
    char palavra[WORD_LEN + 1]; 
    Aresta *lista;
    int grau;
} Vertice;

typedef struct {
    Vertice vertices[MAX_PALAVRAS];
    int n; //vertices
    int num_arestas;
    int num_lacos;
    int num_arestas_mult;
} Grafo;

//(min-heap) para Dijkstra
typedef struct {
    int vertice;
    int dist;
} HeapNode;

typedef struct {
    HeapNode *dados;
    int tamanho;
    int capacidade;
} MinHeap;










//remove \r\n + converte para maiúsculas 
void normaliza(char *s) {
    int len = strlen(s);
    while (len > 0 && (s[len-1] == '\n' || s[len-1] == '\r' || s[len-1] == ' '))
        s[--len] = '\0';
    for (int i = 0; s[i]; i++)
        s[i] = toupper((unsigned char)s[i]);
}

// Conta letras diferentes entre duas palavras de tamanho WORD_LEN 
int diff_letras(const char *a, const char *b) {
    int d = 0;
    for (int i = 0; i < WORD_LEN; i++)
        if (a[i] != b[i]) d++;
    return d;
}

// Adiciona aresta não direcionada
void adiciona_aresta(Grafo *g, int u, int v, int peso) {
    Aresta *e1 = (Aresta*)malloc(sizeof(Aresta));
    e1->destino = v;
    e1->peso = peso;
    e1->prox = g->vertices[u].lista;
    g->vertices[u].lista = e1;
    g->vertices[u].grau++;

    Aresta *e2 = (Aresta*)malloc(sizeof(Aresta));
    e2->destino = u;
    e2->peso = peso;
    e2->prox = g->vertices[v].lista;
    g->vertices[v].lista = e2;
    g->vertices[v].grau++;

    g->num_arestas++;
}


//MIN-HEAP

MinHeap* cria_heap(int cap) {
    MinHeap *h = (MinHeap*)malloc(sizeof(MinHeap));
    h->dados = (HeapNode*)malloc(cap * sizeof(HeapNode));
    h->tamanho = 0;
    h->capacidade = cap;
    return h;
}

void libera_heap(MinHeap *h) {
    free(h->dados);
    free(h);
}

void heap_swap(MinHeap *h, int i, int j) {
    HeapNode tmp = h->dados[i];
    h->dados[i] = h->dados[j];
    h->dados[j] = tmp;
}

void heap_push(MinHeap *h, int v, int d) {
    if (h->tamanho == h->capacidade) {
        h->capacidade *= 2;
        h->dados = (HeapNode*)realloc(h->dados, h->capacidade * sizeof(HeapNode));
    }
    int i = h->tamanho++;
    h->dados[i].vertice = v;
    h->dados[i].dist = d;
    while (i > 0) {
        int pai = (i - 1) / 2;
        if (h->dados[pai].dist <= h->dados[i].dist) break;
        heap_swap(h, pai, i);
        i = pai;
    }
}

HeapNode heap_pop(MinHeap *h) {
    HeapNode topo = h->dados[0];
    h->dados[0] = h->dados[--h->tamanho];
    int i = 0;
    while (1) {
        int menor = i, esq = 2*i+1, dir = 2*i+2;
        if (esq < h->tamanho && h->dados[esq].dist < h->dados[menor].dist) menor = esq;
        if (dir < h->tamanho && h->dados[dir].dist < h->dados[menor].dist) menor = dir;
        if (menor == i) break;
        heap_swap(h, i, menor);
        i = menor;
    }
    return topo;
}








//DIJKSTRA

void dijkstra(Grafo *g, int src, int *dist, int *prev) {
    for (int i = 0; i < g->n; i++) {
        dist[i] = INF;
        prev[i] = -1;
    }
    dist[src] = 0;

    MinHeap *h = cria_heap(g->n * 2 + 1);
    heap_push(h, src, 0);

    while (h->tamanho > 0) {
        HeapNode cur = heap_pop(h);
        int u = cur.vertice;
        if (cur.dist > dist[u]) continue;

        for (Aresta *e = g->vertices[u].lista; e; e = e->prox) {
            int v = e->destino;
            int nd = dist[u] + e->peso;
            if (nd < dist[v]) {
                dist[v] = nd;
                prev[v] = u;
                heap_push(h, v, nd);
            }
        }
    }
    libera_heap(h);
}






//BFS pros componentes conexos

void bfs_componente(Grafo *g, int src, int *comp, int id) {
    int *fila = (int*)malloc(g->n * sizeof(int));
    int ini = 0, fim = 0;
    comp[src] = id;
    fila[fim++] = src;
    while (ini < fim) {
        int u = fila[ini++];
        for (Aresta *e = g->vertices[u].lista; e; e = e->prox) {
            if (comp[e->destino] == -1) {
                comp[e->destino] = id;
                fila[fim++] = e->destino;
            }
        }
    }
    free(fila);
}


//pegar o arquivo pro grafo

int carrega_grafo(Grafo *g, const char *arquivo) {
    FILE *f = fopen(arquivo, "r");
    if (!f) {
        printf("Erro ao abrir arquivo '%s'\n", arquivo);
        return 0;
    }

    g->n = 0;
    g->num_arestas = 0;
    g->num_lacos = 0;
    g->num_arestas_mult = 0;

    char linha[64];
    while (fgets(linha, sizeof(linha), f) && g->n < MAX_PALAVRAS) {
        normaliza(linha);
        if (strlen(linha) != WORD_LEN) continue;
        strncpy(g->vertices[g->n].palavra, linha, WORD_LEN);
        g->vertices[g->n].palavra[WORD_LEN] = '\0';
        g->vertices[g->n].lista = NULL;
        g->vertices[g->n].grau = 0;
        g->n++;
    }
    fclose(f);

    printf("Palavras carregadas: %d\n", g->n);

    //construção das arestas: O(n^2) 
    printf("Construindo arestas...\n");
    for (int i = 0; i < g->n; i++) {
        for (int j = i + 1; j < g->n; j++) {
            int d = diff_letras(g->vertices[i].palavra, g->vertices[j].palavra);
            if (d == 1) {

                // verificar se tem aresta multipla
                int ja_existe = 0;
                for (Aresta *e = g->vertices[i].lista; e; e = e->prox) {
                    if (e->destino == j) { ja_existe = 1; break; }
                }
                if (ja_existe) g->num_arestas_mult++;
                else adiciona_aresta(g, i, j, 1);
            }
        }
    }

    return 1;
}


// busca pela palavra indice

int busca_palavra(Grafo *g, const char *p) {
    for (int i = 0; i < g->n; i++)
        if (strcmp(g->vertices[i].palavra, p) == 0) return i;
    return -1;
}



void imprime_caminho(Grafo *g, int *prev, int dst) {
    if (dst == -1) return;
    if (prev[dst] == -1) {
        printf("%s", g->vertices[dst].palavra);
        return;
    }
    imprime_caminho(g, prev, prev[dst]);
    printf(" -> %s", g->vertices[dst].palavra);
}

void analisa_grafo(Grafo *g) {
    int *comp = (int*)malloc(g->n * sizeof(int));
    for (int i = 0; i < g->n; i++) comp[i] = -1;

    int num_comp = 0;
    for (int i = 0; i < g->n; i++)
        if (comp[i] == -1)
            bfs_componente(g, i, comp, num_comp++);

    int *tam   = (int*)calloc(num_comp, sizeof(int));
    int *gmax  = (int*)malloc(num_comp * sizeof(int));
    int *gmin  = (int*)malloc(num_comp * sizeof(int));
    int *imax  = (int*)malloc(num_comp * sizeof(int));
    int *imin  = (int*)malloc(num_comp * sizeof(int));

    for (int c = 0; c < num_comp; c++) {
        gmax[c] = -1;
        gmin[c] = INF;
        imax[c] = imin[c] = -1;
    }

    for (int i = 0; i < g->n; i++) {
        int c = comp[i];
        tam[c]++;
        if (g->vertices[i].grau > gmax[c]) { gmax[c] = g->vertices[i].grau; imax[c] = i; }
        if (g->vertices[i].grau < gmin[c]) { gmin[c] = g->vertices[i].grau; imin[c] = i; }
    }

    printf("\n========== ANALISE DO GRAFO ==========\n");
    printf("Vertices : %d\n", g->n);
    printf("Arestas  : %d\n", g->num_arestas);

    // Simples ou multigrafo
    printf("\n--- Tipo do grafo ---\n");
    if (g->num_lacos == 0 && g->num_arestas_mult == 0) {
        printf("Grafo SIMPLES (sem lacos e sem arestas multiplas)\n");
    } else {
        printf("MULTIGRAFO\n");
        printf("  Lacos            : %d\n", g->num_lacos);
        printf("  Arestas multiplas: %d\n", g->num_arestas_mult);
    }

    // mostra todos os componentes com grau max e min, como pede o PDF
    printf("\n--- Componentes Conexos: %d ---\n", num_comp);
    for (int c = 0; c < num_comp; c++) {
        printf("\nComponente %d:\n", c + 1);
        printf("  Tamanho (qtd de palavras) : %d\n", tam[c]);
        printf("  Palavra central   (grau max=%d): %s\n",
               gmax[c], g->vertices[imax[c]].palavra);
        printf("  Menos conectada   (grau min=%d): %s\n",
               gmin[c], g->vertices[imin[c]].palavra);
    }

    free(comp); free(tam); free(gmax); free(gmin); free(imax); free(imin);
}

void menu_dijkstra(Grafo *g) {
    char pa[16], pb[16];
    printf("\nDigite a palavra de ORIGEM  (4 letras): ");
    if (!fgets(pa, sizeof(pa), stdin)) return;
    normaliza(pa);

    printf("Digite a palavra de DESTINO (4 letras): ");
    if (!fgets(pb, sizeof(pb), stdin)) return;
    normaliza(pb);

    if (strlen(pa) != WORD_LEN || strlen(pb) != WORD_LEN) {
        printf("Palavras devem ter exatamente 4 letras.\n");
        return;
    }

    int src = busca_palavra(g, pa);
    int dst = busca_palavra(g, pb);

    if (src < 0) { printf("Palavra '%s' nao encontrada na base.\n", pa); return; }
    if (dst < 0) { printf("Palavra '%s' nao encontrada na base.\n", pb); return; }

    int *dist = (int*)malloc(g->n * sizeof(int));
    int *prev = (int*)malloc(g->n * sizeof(int));

    dijkstra(g, src, dist, prev);

    printf("\n--- Dijkstra: %s -> %s ---\n", pa, pb);
    if (dist[dst] == INF) {
        printf("Nao existe caminho entre '%s' e '%s' (componentes diferentes).\n", pa, pb);
    } else {
        printf("Distancia (numero de transformacoes): %d\n", dist[dst]);
        printf("Caminho: ");
        imprime_caminho(g, prev, dst);
        printf("\n");
    }

    free(dist);
    free(prev);
}


int main(int argc, char *argv[]) {
    const char *arquivo = (argc > 1) ? argv[1] : "base_de_palavras_4_letras.txt";

    Grafo *g = (Grafo*)calloc(1, sizeof(Grafo));
    if (!g) { fprintf(stderr, "Memoria insuficiente.\n"); return 1; }
    printf("Carregando base: %s\n", arquivo);

    if (!carrega_grafo(g, arquivo)) {
        free(g);
        return 1;
    }

    int opcao;
    do {
        printf("\n========== MENU ==========\n");
        printf("1. Analisar grafo (componentes, graus, tipo)\n");
        printf("2. Encontrar caminho entre duas palavras (Dijkstra)\n");
        printf("0. Sair\n");
        printf("Opcao: ");

        char buf[16];
        if (!fgets(buf, sizeof(buf), stdin)) break;
        opcao = atoi(buf);

        switch (opcao) {
            case 1: analisa_grafo(g); break;
            case 2: menu_dijkstra(g); break;
            case 0: printf("Encerrando.\n"); break;
            default: printf("Opcao invalida.\n");
        }
    } while (opcao != 0);

    for (int i = 0; i < g->n; i++) {
        Aresta *e = g->vertices[i].lista;
        while (e) { Aresta *t = e->prox; free(e); e = t; }
    }
    free(g);
    return 0;
}

//gcc -O2 -o grafo_palavras grafo_palavras.c
//./grafo_palavras base_de_palavras_4_letras.txt