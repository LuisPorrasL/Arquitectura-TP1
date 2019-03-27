#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"

int preguntar_n(int cantidad_procesos){
    int n = 0;
    while(!n){
        printf("\nDigite el tamaño \"n\" de las matrices cuadradas A y B.\nRecuerde que \"n\" debe ser multiplo de %d y mayor que 0.\n", cantidad_procesos);
        scanf("%d", &n);
        if(n%cantidad_procesos != 0 || n == 0){
            printf("\nPor favor digite un valor de \"n\" valido.\n\n");
            n = 0;
        }
    }
    return n;
}

void llenar_vector_aleatoreamente(int vector[], int tamanno, int valor_aleatorio_minimo, int valor_aleatorio_maximo){
    for(int indice = 0; indice < tamanno; ++indice){
        vector[indice] = (int)rand() % (valor_aleatorio_maximo + 1 - valor_aleatorio_minimo) + valor_aleatorio_minimo;
    }
}

void imprimir_matriz_memoria_continua_por_filas(int matriz[], int tamanno){
    for(int indice = 0; indice < (tamanno*tamanno); ++indice){
        if(indice%tamanno == 0){
            printf("\n");
        }
        printf("%2d", matriz[indice]);
    }
    printf("\n");
}

void imprimir_matriz_memoria_continua_por_columnas(int matriz[], int tamanno){
    printf("\n");
    for(int fila = 0; fila < tamanno; ++fila){
        for(int columna = fila; columna < (tamanno*tamanno); columna += tamanno){
            printf("%2d", matriz[columna]);
        }
        printf("\n");
    }
}

int main(int argc, char* argv[]) {
    int *A, *B, *M;
    int n, cantidad_procesos, proceso_id, tamanno_nombre_procesador;
    char nombre_procesador[MPI_MAX_PROCESSOR_NAME];


    MPI_Init(&argc, &argv); // Inicializacion del ambiente para MPI.
    MPI_Comm_size(MPI_COMM_WORLD, &cantidad_procesos); // Se le pide al comunicador MPI_COMM_WORLD que almacene en cantidad_procesos el numero de procesos de ese comunicador.
    MPI_Comm_rank(MPI_COMM_WORLD, &proceso_id); // Se le pide al comunicador MPI_COMM_WORL que devuelva en la variable proceso_id la identificacion del proceso "llamador".
    
    
    MPI_Get_processor_name(nombre_procesador,&tamanno_nombre_procesador);
    fprintf(stdout,"Proceso %d de %d en %s\n", proceso_id, cantidad_procesos, nombre_procesador); //Cada proceso despliega su identificacion y el nombre de la computadora en la que corre.
    MPI_Barrier(MPI_COMM_WORLD);


    if(proceso_id == 0){
        // Se le pide al usuario el tamaño "n" de las matrices cuadradas A y B.
        n = preguntar_n(cantidad_procesos);

        //Se crean (aloja memoria) las matrices A y B con tamaño nxn.
        A = (int*)malloc(sizeof(int)*(n*n));
		B = (int*)malloc(sizeof(int)*(n*n));

        //Se llenan las matrices A y B con valores aleatorios.
        llenar_vector_aleatoreamente(A, (n*n), 0, 5);
        llenar_vector_aleatoreamente(B, (n*n), 0, 2);

        imprimir_matriz_memoria_continua_por_filas(A, n);
        imprimir_matriz_memoria_continua_por_filas(B, n);
    }


    MPI_Finalize(); // Se termina el ambiente MPI.
    return 0;
}

