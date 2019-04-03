#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "mpi.h"


// Constantes:
int ROOT = 0;
int POS_ARRIBA = 0;
int POS_ABAJO = 1;
int POS_DERECHA = 2;
int POS_IZQUIERDA = 3;
int POS_INVALIDA = -1;

int es_primo(int numero){
    int conteoPrimo = 0, i;
    for(i=1;i<=numero;i++)
        if(numero%i==0)
            conteoPrimo++;
    return (conteoPrimo==2?1:0);
}

void obtener_movimientos_matriz_c( int fila, int columna, int n, int p, int indice, int movimientos[] ){
    // Primero se calcula las posiciones de arriba y abajo
    if ( fila == 0 ){
        movimientos[ POS_ARRIBA ] = POS_INVALIDA;
        movimientos[ POS_ABAJO ] = indice + n ;
    } else if (fila == (n/p) - 1 ){
        movimientos[ POS_ARRIBA ] = indice - n;
        movimientos[ POS_ABAJO ] = POS_INVALIDA;
    }else{
        movimientos[ POS_ARRIBA ] = indice - n;
        movimientos[ POS_ABAJO ] = indice + n ;
    }
    // Finalmente se calcula las posiciones de derecha e izquierda
    if ( columna % n == 0 ){
        movimientos[ POS_DERECHA ] = indice + 1;
        movimientos[ POS_IZQUIERDA ] = POS_INVALIDA;
    }else if ( columna % n == n - 1 ){
        movimientos[ POS_DERECHA ] = POS_INVALIDA;
        movimientos[ POS_IZQUIERDA ] = indice - 1;
    }else{
        movimientos[ POS_DERECHA ] = indice + 1;
        movimientos[ POS_IZQUIERDA ] = indice - 1;
    }
}


int contar_primos_totales_M_y_por_columnas_y_matriz_C( int vectorParteM[], int dimensionesParteM, int dimensionFila, int vectorConteoColumnas[], int cantidad_procesos, int superior[], int inferior[], int parte_C[] ){
    int indice, posResultado = 0, acumuladorPrimos = 0, fila_actual = 0, columna_actual = 0;
    int *vector_sub, *vector_inf;
    int movimientos[4] = {0};        
    for ( indice = 0; indice < dimensionFila; ++indice ) vectorConteoColumnas[indice] = 0;
    // Se recorre y se suman los primos por columna en el vector de resultados y el total de primos
    for ( indice = 0; indice < dimensionesParteM; ++ indice ){
        if ( es_primo( vectorParteM[indice] ) ){
            ++acumuladorPrimos;
            ++vectorConteoColumnas[posResultado];           
        } 
        posResultado = ( posResultado + 1 )% dimensionFila;
		// Se calcula mi parte de C 
		// Se calcula la fila actual
		columna_actual = indice % dimensionFila;			
		// Se calcula la columna actual
		fila_actual = indice / dimensionFila ;		
        // Se calcula el vector de posiciones para los momientos de esta entrada en C	
        obtener_movimientos_matriz_c(fila_actual, columna_actual, dimensionFila,cantidad_procesos, indice, movimientos);
        // Se calcula la entrada C actual segun el vector de movimientos		
        parte_C[indice] = vectorParteM[ indice ];						
        // ---> Sumar arriba:
        
        if (movimientos[POS_ARRIBA] != POS_INVALIDA)
            parte_C[indice] += vectorParteM[ movimientos[ POS_ARRIBA ] ];
        else
            parte_C[indice] += superior[ columna_actual ];
        // ---> Sumar abajo:        
        if ( movimientos[ POS_ABAJO ] != POS_INVALIDA ) 
            parte_C[indice] += vectorParteM[ movimientos[ POS_ABAJO] ];   
        else
            parte_C[indice] += inferior[columna_actual];   
        // ---> Sumar derecha:
        if ( movimientos[ POS_DERECHA ] != POS_INVALIDA )
            parte_C[indice] += vectorParteM[ movimientos[ POS_DERECHA ] ];        
        // ---> Sumar izquierda:
        if ( movimientos[ POS_IZQUIERDA ] != POS_INVALIDA )
            parte_C[indice] += vectorParteM[ movimientos[ POS_IZQUIERDA ] ];
        
    }
    return acumuladorPrimos;
}


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
    time_t t;
    srand( (unsigned) time (&t));
	int indice;
    for(indice = 0; indice < tamanno; ++indice)vector[indice] = (int)rand() % (valor_aleatorio_maximo + 1 - valor_aleatorio_minimo) + valor_aleatorio_minimo;
}

void imprimir_matriz_cuadrada_memoria_continua_por_filas(int matriz[], int tamanno, char* nombre_matrix, FILE* archivo){
    fprintf(archivo,"\n%s:\n", nombre_matrix);
    int indice;
	for( indice = 0; indice < (tamanno*tamanno); ++indice){
        if(indice%tamanno == 0) fprintf(archivo, "\n");
        fprintf(archivo,"%4d", matriz[indice]);
    }
    fprintf(archivo, "\n\n");
}

void calcular_producto_parcial_matrices_cuadradas_memoria_continua_por_filas(int filas[], int matriz_columnas[], int numero_filas, int tamanno, int resultado_parcial[]){
    int indice_fila, indice_columna, indice_resultado_parcial, indice_fila_resultado_parcial;
    for(indice_resultado_parcial = 0; indice_resultado_parcial < (numero_filas*tamanno); ++indice_resultado_parcial){
        indice_fila = indice_resultado_parcial/tamanno;
        for(indice_fila_resultado_parcial = indice_fila*tamanno, indice_columna = indice_resultado_parcial%tamanno; indice_fila_resultado_parcial < indice_fila*tamanno+tamanno; ++indice_fila_resultado_parcial, indice_columna += tamanno)
            resultado_parcial[indice_resultado_parcial] += filas[indice_fila_resultado_parcial] * matriz_columnas[indice_columna];
    }
}

void calcular_reparticion_faltantes_M(int pocisionesInicialesFaltanteSuperior[], int pocisionesInicialesFaltanteInferior[], 
    int desplazamientoFaltanteSuperior[], int desplazamientoFaltanteInferior[], int cantidad_procesos, int n){
		int indice_proceso, desplazamiento;
        int filas_por_proceso = desplazamiento = n / cantidad_procesos;
        for(indice_proceso = 0; indice_proceso < cantidad_procesos; ++indice_proceso){
			if(indice_proceso == 0){
				pocisionesInicialesFaltanteSuperior[indice_proceso] = desplazamientoFaltanteSuperior[indice_proceso] = 0;
				if(cantidad_procesos > 1){
					pocisionesInicialesFaltanteInferior[indice_proceso] = (n*n)/cantidad_procesos;
					desplazamientoFaltanteInferior[indice_proceso] = n;
				}
				else pocisionesInicialesFaltanteInferior[indice_proceso] = desplazamientoFaltanteInferior[indice_proceso] = 0;
			}
			else if(indice_proceso == (cantidad_procesos-1)){
				pocisionesInicialesFaltanteSuperior[indice_proceso] = (n*n) - ((n*n)/cantidad_procesos) - n;
				desplazamientoFaltanteSuperior[indice_proceso] = n;
				pocisionesInicialesFaltanteInferior[indice_proceso] = 0;
				desplazamientoFaltanteInferior[indice_proceso] = 0;
			}
			else{
				pocisionesInicialesFaltanteSuperior[indice_proceso] =  (desplazamiento - 1)*n;
				desplazamientoFaltanteSuperior[indice_proceso] = n;
				pocisionesInicialesFaltanteInferior[indice_proceso] =  (desplazamiento +filas_por_proceso)*n;
				desplazamientoFaltanteInferior[indice_proceso] = n;
                desplazamiento+=filas_por_proceso;
			}                    
        }
}

void imprimir_primos_por_columna(int vector_primos[], int n, FILE *archivo){
	int y;
	fprintf(archivo, "\nP:\n\n");
	for (y = 0; y < n; ++ y) fprintf(archivo, "P[%d] = %d\n", y, vector_primos[y]);
}

int main(int argc, char* argv[]) {
    int *A, *B, *M, *parte_A, *parte_M, *C, *parte_C, *parte_faltante_superior, *parte_faltante_inferior, *myP, *P; // Vectores para el conteo de los primos por fila;
    int n, cantidad_procesos, proceso_id, tp, myTp = 0; //  Acumulador para el conteo de primos en la matriz M
    double comienzo_tiempo_total, fin_tiempo_total, comienzo_tiempo_neto, fin_tiempo_neto;
    FILE* archivo = stdout;

    MPI_Init(&argc, &argv); // Inicializacion del ambiente para MPI.
    MPI_Comm_size(MPI_COMM_WORLD, &cantidad_procesos); // Se le pide al comunicador MPI_COMM_WORLD que almacene en cantidad_procesos el numero de procesos de ese comunicador.
    MPI_Comm_rank(MPI_COMM_WORLD, &proceso_id); // Se le pide al comunicador MPI_COMM_WORL que devuelva en la variable proceso_id la identificacion del proceso "llamador".

    // Se inicializan los vectores del Scatter variable
    int *pocisionesInicialesFaltanteSuperior = (int*)malloc(sizeof(int)*cantidad_procesos);
    int *pocisionesInicialesFaltanteInferior = (int*)malloc(sizeof(int)*cantidad_procesos);
    int *desplazamientoFaltanteSuperior = (int*)malloc(sizeof(int)*cantidad_procesos);
    int *desplazamientoFaltanteInferior = (int*)malloc(sizeof(int)*cantidad_procesos);

    if(proceso_id == ROOT){
        comienzo_tiempo_total = MPI_Wtime();

        // Se le pide al usuario el tamaño "n" de las matrices cuadradas A y B.
        n = preguntar_n(cantidad_procesos);

        comienzo_tiempo_neto = MPI_Wtime();

        // Se crean (aloja memoria) las matrices A y B con tamaño nxn.
        A = (int*)malloc(sizeof(int)*(n*n));
		B = (int*)malloc(sizeof(int)*(n*n));

        // Se llenan las matrices A y B con valores aleatorios.
        llenar_vector_aleatoreamente(A, (n*n), 0, 5);
        llenar_vector_aleatoreamente(B, (n*n), 0, 2);

        calcular_reparticion_faltantes_M(pocisionesInicialesFaltanteSuperior, pocisionesInicialesFaltanteInferior, 
            desplazamientoFaltanteSuperior, desplazamientoFaltanteInferior, cantidad_procesos, n);
    }

    // Necesito enviar a todos los procesos el valor de n, para que estos puedan reservar la memoria para sus partes de A y B.
    MPI_Bcast(&n, 1, MPI_INT, ROOT, MPI_COMM_WORLD);
    
    // Necesito repartir las matrices A y B.
    // B ya que es de la que se ocupan las columnas se va a repartir completa entre todos los procesos.
    if(proceso_id != ROOT) B = (int*)malloc(sizeof(int)*(n*n));
    MPI_Bcast(B, (n*n), MPI_INT, ROOT, MPI_COMM_WORLD);
    
    // A ya que solo se ocupan sus filas, se va a repartir sus filas entre los procesos, para así paralelisar el calculo de M.
    int numero_filas_parte_A = n/cantidad_procesos;
    int tamanno_parte_A = (numero_filas_parte_A)*n;
    parte_A = (int*)malloc(sizeof(int)*tamanno_parte_A);
    MPI_Scatter(A, tamanno_parte_A, MPI_INT, parte_A, tamanno_parte_A, MPI_INT, ROOT, MPI_COMM_WORLD);

    // Necesito que todos los procesos calculen su parte de M
    parte_M = (int*)calloc(tamanno_parte_A, sizeof(int));
    calcular_producto_parcial_matrices_cuadradas_memoria_continua_por_filas(parte_A, B, numero_filas_parte_A, n, parte_M);

    // Necesito armar la matriz M recuperando las partes calculadas por cada proceso
    M = (int*)malloc(sizeof(int)*(n*n));
    MPI_Gather(parte_M, tamanno_parte_A, MPI_INT, M, tamanno_parte_A, MPI_INT, ROOT, MPI_COMM_WORLD);

    // Repartir M
    parte_faltante_superior = (int*)calloc(n, sizeof(int));
    parte_faltante_inferior = (int*)calloc(n, sizeof(int));

    if(proceso_id == 0)MPI_Scatterv(M, desplazamientoFaltanteSuperior, pocisionesInicialesFaltanteSuperior, MPI_INT, parte_faltante_superior, 0, MPI_INT, ROOT, MPI_COMM_WORLD);
	else MPI_Scatterv(M, desplazamientoFaltanteSuperior, pocisionesInicialesFaltanteSuperior, MPI_INT, parte_faltante_superior, n, MPI_INT, ROOT, MPI_COMM_WORLD);
	if(proceso_id == cantidad_procesos-1) MPI_Scatterv(M, desplazamientoFaltanteInferior, pocisionesInicialesFaltanteInferior, MPI_INT, parte_faltante_inferior, 0, MPI_INT, ROOT, MPI_COMM_WORLD);
	else MPI_Scatterv(M, desplazamientoFaltanteInferior, pocisionesInicialesFaltanteInferior, MPI_INT, parte_faltante_inferior, n, MPI_INT, ROOT, MPI_COMM_WORLD);

    // Se inicializan los vectores de la parte de C de cada uno
    parte_C = (int*)calloc(tamanno_parte_A, sizeof(int));

    // Se realiza el conteo total de primos en M por cada proceso y el conteo por Columna
    // Se preparan los vectores
    if(proceso_id == ROOT)  P = (int*)malloc(sizeof(int)*n); // proceso 0 crea P
    myP = (int*)malloc(sizeof(int)*n); // todos los procesos crea su P para acumular
    
    // Se llama al metodo que hace todo el conteo
    myTp = contar_primos_totales_M_y_por_columnas_y_matriz_C(parte_M, tamanno_parte_A, n, myP, cantidad_procesos, parte_faltante_superior, parte_faltante_inferior, parte_C);

    // Se realiza el reduce con operacion de suma de los primos que cada proceso conto hacia el proc ROOT
    MPI_Reduce(&myTp,&tp,1,MPI_INT,MPI_SUM,ROOT,MPI_COMM_WORLD);
    
    // Se realiza el reduce de los resultados de las sumas
    MPI_Reduce(myP,P,n,MPI_INT,MPI_SUM,ROOT,MPI_COMM_WORLD);

    // Necesito armar la matriz C recuperando las partes calculadas por cada proceso
    C = (int*)malloc(sizeof(int)*(n*n));
    MPI_Gather(parte_C, tamanno_parte_A, MPI_INT, C, tamanno_parte_A, MPI_INT, ROOT, MPI_COMM_WORLD);

    // El proceso raíz imprime: n, cantidad_procesos, tp, tiempo_total, tiempo_neto, A, B, M, P y C
    if(proceso_id == ROOT) {
        fin_tiempo_neto = MPI_Wtime();
        fprintf(archivo, "\nEl \"n\" ingresado por el usuario fue: %d\n", n);
        fprintf(archivo, "\nLa cantidad de procesos ejecutados fue: %d\n", cantidad_procesos);
        fprintf(archivo, "\nEl total de primos de la matriz M es: %d\n", tp);
        fin_tiempo_total = MPI_Wtime();
        //Imprimir tiempo_total
        fprintf(archivo, "\nEl tiempo total de ejecucion del programa fue: %lf\n", (fin_tiempo_total-comienzo_tiempo_total));
        //Imprimir tiempo_neto
        fprintf(archivo, "\nEl tiempo neto (despues de leer y antes de escribir) de ejecucion del programa fue: %lf\n", (fin_tiempo_neto-comienzo_tiempo_neto));
        imprimir_matriz_cuadrada_memoria_continua_por_filas(A, n, "A", archivo);
        imprimir_matriz_cuadrada_memoria_continua_por_filas(B, n, "B", archivo);
        imprimir_matriz_cuadrada_memoria_continua_por_filas(M, n, "M", archivo);
        imprimir_primos_por_columna(P, n, archivo);
        imprimir_matriz_cuadrada_memoria_continua_por_filas(C, n, "C", archivo);

        // Solamente el proceso ROOT libera A y P
        free(A); 
        free(P);
    }

    // Se libera memoria.
    free(B);
    free(M);
    free(parte_A);
    free(parte_M);
    free(myP);

    MPI_Finalize(); // Se termina el ambiente MPI.
    return 0;
}