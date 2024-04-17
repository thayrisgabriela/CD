
#include <stdio.h>
#include <stdlib.h>

#include "smpl.h"

/* cada processo conta seu tempo */

/*---- aqui definimos os eventos ----*/
#define test 1
#define fault 2
#define recovery 3

/*---- declaramos agora o TipoProcesso ----*/
typedef struct {
    int id; /* identificador de facility SMPL */
            /* variáveis locais do processo são declaradas aqui */
} TipoProcesso;

TipoProcesso *processo;

int main(int argc, char *argv[]) {
    static int N, /* number of nodes is parameter */
        token,    /* node identifier, natural number */
        event, r, i;

    static char fa_name[5];

    if (argc != 2) {
        puts("Uso correto: tempo <num-processos>");
        exit(1);
    }

    N = atoi(argv[1]);
    smpl(0, "Um Exemplo de Simulação");
    reset();
    stream(1);

    /*----- inicializacao -----*/

    processo = (TipoProcesso *)malloc(sizeof(TipoProcesso) * N);

    int STATE[N][N];
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            if (i == j) {
                STATE[i][j] = 0;
            } else {
                STATE[i][j] = -1;
            }
        };
    }

    for (i = 0; i < N; i++) {
        memset(fa_name, '\0', 5);
        sprintf(fa_name, "%d", i);
        processo[i].id = facility(fa_name, 1);
        // printf("fa_name = %s, processo[%d].id = %d\n", fa_name, i,
        // processo[i].id);
    } /* end for */

    /*----- vamos escalonar os eventos iniciais -----*/

    for (i = 0; i < N; i++) {
        schedule(test, 30.0, i);
    }
    schedule(fault, 31.0, 1);
    // schedule(recovery, 92.0, 1);

    /*----- agora vem o loop principal do simulador -----*/

    while (time() < 150.0) {
        cause(&event, &token);
        switch (event) {
            case test: {
                int current_st = status(processo[token].id);
                if (current_st != 0)  // erro no processo atual
                    break;
                int prox_token = (token + 1) % N;
                printf("\nTempo: %5.1f\n", time());
                for (int i = 1; i < N; i++) {
                    printf("  Processo %d testando o %d\n", token, prox_token);
                    int st = status(processo[prox_token].id);
                    STATE[token][prox_token] = st;
                    if (st != 0) {
                        printf("    Erro no processo %d :(\n", prox_token);
                        // schedule(fault, 1.0, prox_token);
                        prox_token = (prox_token + 1) % N;
                    } else {
                        // printf("SUCESSO: processo %d irá testar no tempo
                        // %5.1f\n", prox_token, time());
                        printf("    SUCESSO!\n");
                        schedule(test, 30.0, prox_token);

                        for (int j = 0; j < N; j++) {
                            if (STATE[prox_token][j] != -1) {
                                STATE[token][j] = STATE[prox_token][j];
                            }
                        }
                        break;
                    }
                }

                break;
            }
            case fault:
                r = request(processo[token].id, token, 0);
                printf("\n--Processo %d falhou no tempo %5.1f\n", token, time());
                break;
            case recovery:
                release(processo[token].id, token);
                schedule(test, 1.0, token);
                printf("\n++Processo %d recuperou no tempo %5.1f\n", token,
                    time());
                break;
        } /* end switch */
    }     /* end while */

    printf("\n\n");
    for (int i = 0; i < N; i++) {
        printf("Processo %d: ", i);
        for (int j = 0; j < N; j++) {
            printf("%d ", STATE[i][j]);
        }
        printf("\n");
    }

} /* end main */
