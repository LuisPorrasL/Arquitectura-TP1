#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <string.h>
#include "mpi.h"


/*
Estudiantes:
    Luis Porras Ledezma B65477
    Jose Pablo Ramírez Méndez B65728
*/

// Constantes:
int ROOT = 0;
int POS_ARRIBA = 0;
int POS_ABAJO = 1;
int POS_DERECHA = 2;
int POS_IZQUIERDA = 3;
int POS_INVALIDA = -1;

/**
 * Recibe: <n>, un entero.
 * Realiza: determina si <n> es un número primo o no.
 * Retorna: 0 si <n> no es primo y 1 en caso contrario.
 **/
int es_primo(int n){
    int primo = 1, i;
    int root_n = (int)( pow(n,0.5) + 1 );
    for(i=2;i<root_n;i++)
        if( n%i ==0 ){
            primo = 0;
            break;
        }
    if (n <= 1 ) primo = 0;
    return primo;
}

/**
 * Recibe: 
 *      <fila>, fila actual por la que se esta recorriendo el arreglo 
 *      <columna>,columna actual por la que se esta recorriendo el arreglo
 *      <n>, el valor ingresado por el usuario para crear la matriz 
 *      <p>, la cantiadad de procesos creada
 *      <indice>, el valor de la posicion actual de la mattriz como vector de una dimension
 *      <movimientos>, vector de 4 posiciones que almacena la posicion de ARRIBA,ABAJO,DERECHA E IZQUIERDA validas para la posicion del indice actual.
 * Realiza: Permite obtener los movimientos para obtener el valor de una entrada de la matriz C segun la posición establecida por <fila> y <columna>
 * Retorna: Este metodo no retorna ningun valor, ya que modifica los valores a través del puntero del vector de <movimientos>
 **/
void obtener_movimientos_matriz_c( int fila, int columna, int n, int p, int indice, int movimientos[] ){
    // Primero se calcula las posiciones de arriba y abajo
    if ( fila == 0 ){
        movimientos[ POS_ARRIBA ] = POS_INVALIDA;
        if ((n/p) == 1)
            movimientos[ POS_ABAJO ] = POS_INVALIDA;
        else
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

/**
 * Recibe: 
 *      <vectorParteM>, puntero al vector que contiene la direccion de memoria de la parte de M calculada por el proceso
 *      <dimencionesParteM>, cantidad de elementos asignados segun la reparticion en M para este proceso
 *      <dimensionFila>, valor n digitado por el usuario para crear las matrices
 *      <vectorConteoColumnas>, puntero al vector de dimension <dimensionFila> para contar la cantidad de primos por columna en la parte M
 *      <cantidad_procesos>, valor entero de la cantidad de procesos creada obtenida por el llamado de MPI
 *      <superior>, puntero al vector de la parte superior de M para el proceso logre calcular su parte de C correspondiente
 *      <inferior>, puntero al vector de la parte inferior de M para el proceso logre calcular su parte de C correspondiente
 *      <parte_C>, puntero a un vector de dimensiones <dimencionesParteM> usado para almacenar la parte C correspondiente a un proceso
 * Realiza: Realiza el conteo de primos parcial totales en M y por columnas en M, Tambien calcula la parteC correspondiente a un proceso
 * Retorna: La cantidad totoal parcial de enteros en <vectorParteM> correspondiente
 **/
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

/**
 * Recibe: <cantidad_procesos> de tipo entero.
 * Realiza: pide al usuario un valor "n", hasta que el usuario digita uno válido, es decir, un multiplo de <cantidad_procesos>.
 * Retorna: un entero con el "n" que digitó el usuario.
 **/
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

/**
 * Recibe:  <vector> puntero a un arreglo de enteros.
 *          <tamanno> entero con el tamaño de <vector>.
 *          <valor_aleatorio_minimo> entero con el valor aleatorio minimo para <vector>.
 *          <valor_aleatorio_maximo> entero con el valor aleatorio maximo para <vector>.
 * Realiza: llena <vector> con valores entre <valor_aleatorio_minimo> y <valor_aleatorio_maximo>.
 * Retorna: nada.
 **/
void llenar_vector_aleatoreamente(int vector[], int tamanno, int valor_aleatorio_minimo, int valor_aleatorio_maximo){
    time_t t;
    srand( (unsigned) time (&t));
	int indice;
    for(indice = 0; indice < tamanno; ++indice)vector[indice] = (int)rand() % (valor_aleatorio_maximo + 1 - valor_aleatorio_minimo) + valor_aleatorio_minimo;
}

/**
 * Recibe:  <matriz> de tipo entero.
 *          <tamanno> de tipo entero, con el tamaño de la fila o columna (pues es cuadrada) de la matriz.
 *          <nombre_matrix> de tipo char*, con el nombre de la matriz a imprimir.
 * Realiza: imprime <matriz> con el nombre <nombre_matrix> en pantalla si <tamanno> <= a 100 y en un archivo con nombre <nombre_matrix>.txt.
 * Retorna: nada.
 **/
void imprimir_matriz_cuadrada_memoria_continua_por_filas(int matriz[], int tamanno, char* nombre_matrix){
    FILE* archivo = stdout;
    int imprimir_archivo = tamanno > 30;
    if(imprimir_archivo){
        char nombre_archivo[6] = {0};
        strcat(nombre_archivo, nombre_matrix);
        archivo = fopen(strcat(nombre_archivo, ".txt"), "w");
    }
    fprintf(archivo,"\n%s:\n", nombre_matrix);
    int indice;
	for( indice = 0; indice < (tamanno*tamanno); ++indice){
        if(indice%tamanno == 0) fprintf(archivo, "\n");
        fprintf(archivo,"%6d", matriz[indice]);
    }
    fprintf(archivo, "\n\n");
    if(imprimir_archivo) fclose(archivo);
}

/**
 * Recibe:  <filas> puntero a un arreglo de enteros.
 *          <matriz_columnas> puntero a una matriz cuadrada de enteros en memoria continua, con las columnas a multiplicar.
 *          <numero_filas> de tipo entero, con la cantidada de filas en <filas>.
 *          <tamanno> de tipo entero con el tamaño de las filas de <filas>.
 *          <resultado_parcial> puntero a un arreglo de enteros, en el que se va a guardar el resultado del producto parcial de matrices.
 * Realiza: calcula el producto matriz parcial, entre <filas> y las columnas de <matriz_columnas>. 
 * Retorna: nada.
 **/
void calcular_producto_parcial_matrices_cuadradas_memoria_continua_por_filas(int filas[], int matriz_columnas[], int numero_filas, int tamanno, int resultado_parcial[]){
    int indice_fila, indice_columna, indice_resultado_parcial, indice_fila_resultado_parcial;
    for(indice_resultado_parcial = 0; indice_resultado_parcial < (numero_filas*tamanno); ++indice_resultado_parcial){
        indice_fila = indice_resultado_parcial/tamanno;
        for(indice_fila_resultado_parcial = indice_fila*tamanno, indice_columna = indice_resultado_parcial%tamanno; indice_fila_resultado_parcial < indice_fila*tamanno+tamanno; ++indice_fila_resultado_parcial, indice_columna += tamanno)
            resultado_parcial[indice_resultado_parcial] += filas[indice_fila_resultado_parcial] * matriz_columnas[indice_columna];
    }
}

/**
 * Recibe:  <pocisionesInicialesFaltanteSuperior> puntero a un arreglo de enteros, donde se van a guardar las posiciones para el scatter variable del faltante superior.
 *          <pocisionesInicialesFaltanteInferior> puntero a un arreglo de enteros, donde se van a guardar las posiciones para el scatter variable del faltante inferior.
 *          <desplazamientoFaltanteSuperior> puntero a un arreglo de enteros, donde se van a guardar los desplazamientos para el scatter variable del faltante superior.
 *          <desplazamientoFaltanteInferior> puntero a un arreglo de enteros, donde se van a guardar los desplazamientos para el scatter variable del faltante inferior.
 *          <cantidad_procesos> entero con la cantidad de procesos ejecutandose.
 *          <n> entero indicado por el usuario, representa el tamaño de todos los arreglos recibidos.
 * Realiza: crea los dos arreglos necesarios para realizar un MPI_Scatter (el de posiciones y el de desplazamientos), 
 *  para los 2 scartters variables que se van a realizar para repartir las filas que le faltan a los procesos para calcular C.
 * Retorna: nada.
 **/
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

/**
 * Recibe:  <vector_primos> puntero del arreglo de enteros a imprimir. <n> de tipo entero con el tamaño del arreglo <vector_primos>.
 * Realiza: imprimer <vector_primos> como una columna, en pantalla si n <= 30, en un archivo llamado "P.txt" en caso contrario.
 * Retorna: nada.
 **/
void imprimir_primos_por_columna(int vector_primos[], int n){
	FILE* archivo = stdout;
    int y, imprimir_archivo = n > 30;
    if(imprimir_archivo) archivo = fopen("P.txt", "w");
	fprintf(archivo, "\nP:\n\n");
	for (y = 0; y < n; ++ y) fprintf(archivo, "P[%d] = %d\n", y, vector_primos[y]);
    if(imprimir_archivo)fclose(archivo);
}

/**
 * Recibe: <argc> un entero con la cantidad de argumentos en <argv>. <argv> un puntero a un arreglo de char* con los argumentos con los que se llamó al programa.
 * Realiza: el calculo en paralelo de M, P, y C.
 * Retorna: un entero con el resultado de la ejecución del programa, ok = 0, not ok = !0.
 **/
int main(int argc, char* argv[]) {
    int *A, *B, *M, *parte_A, *parte_M, *C, *parte_C, *parte_faltante_superior, *parte_faltante_inferior, *myP, *P; // Vectores para el conteo de los primos por fila;
    int n, cantidad_procesos, proceso_id, tp, myTp = 0; //  Acumulador para el conteo de primos en la matriz M
    double comienzo_tiempo_total, fin_tiempo_total, comienzo_tiempo_neto, fin_tiempo_neto;

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
    if(proceso_id == ROOT) M = (int*)malloc(sizeof(int)*(n*n));
    MPI_Gather(parte_M, tamanno_parte_A, MPI_INT, M, tamanno_parte_A, MPI_INT, ROOT, MPI_COMM_WORLD);

    // Repartir M con las partes faltantes para el calculo parcial de C
    parte_faltante_superior = (int*)calloc(n, sizeof(int));
    parte_faltante_inferior = (int*)calloc(n, sizeof(int));

    // Se deben realizar 2 scatters variables ya que se decidió dar a cada proceso solo la fila superior y la fila inferior que necesitan para calcular C, sin repartir nuevamente lo que estos ya conocen de M (i.e. el grupo de filas que ellos calcularon de M).
    // Se reparte a los procesos la fila superior del grupo de filas de M que ellos (los procesos) ya conocen, en caso del primer proceso se le da un arreglo de tamaño 0.
    if(proceso_id == 0)
        MPI_Scatterv(M, desplazamientoFaltanteSuperior, pocisionesInicialesFaltanteSuperior, MPI_INT, parte_faltante_superior, 0, MPI_INT, ROOT, MPI_COMM_WORLD);
	else 
        MPI_Scatterv(M, desplazamientoFaltanteSuperior, pocisionesInicialesFaltanteSuperior, MPI_INT, parte_faltante_superior, n, MPI_INT, ROOT, MPI_COMM_WORLD);
    // Se reparte a los procesos la fila inferior del grupo de filas de M que ellos (los procesos) ya conocen, en caso del ultimo proceso se le da un arreglo de tamaño 0.
    if(proceso_id == cantidad_procesos-1) 
        MPI_Scatterv(M, desplazamientoFaltanteInferior, pocisionesInicialesFaltanteInferior, MPI_INT, parte_faltante_inferior, 0, MPI_INT, ROOT, MPI_COMM_WORLD);
	else 
        MPI_Scatterv(M, desplazamientoFaltanteInferior, pocisionesInicialesFaltanteInferior, MPI_INT, parte_faltante_inferior, n, MPI_INT, ROOT, MPI_COMM_WORLD);

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
    if(proceso_id == ROOT) C = (int*)malloc(sizeof(int)*(n*n));    
    MPI_Gather(parte_C, tamanno_parte_A, MPI_INT, C, tamanno_parte_A, MPI_INT, ROOT, MPI_COMM_WORLD);

    // El proceso raíz imprime: n, cantidad_procesos, tp, tiempo_total, tiempo_neto, A, B, M, P y C
    if(proceso_id == ROOT) {
        fin_tiempo_neto = MPI_Wtime();
        printf("\nEl \"n\" ingresado por el usuario fue: %d\n", n);
        printf("\nLa cantidad de procesos ejecutados fue: %d\n", cantidad_procesos);
        printf("\nEl total de primos de la matriz M es: %d\n", tp);
        imprimir_matriz_cuadrada_memoria_continua_por_filas(A, n, "A");
        imprimir_matriz_cuadrada_memoria_continua_por_filas(B, n, "B");
        imprimir_matriz_cuadrada_memoria_continua_por_filas(M, n, "M");
        imprimir_primos_por_columna(P, n);
        imprimir_matriz_cuadrada_memoria_continua_por_filas(C, n, "C");
        fin_tiempo_total = MPI_Wtime();
        //Imprimir tiempo_total
        printf("\nEl tiempo total de ejecucion del programa fue: %lf\n", (fin_tiempo_total-comienzo_tiempo_total));
        //Imprimir tiempo_neto
        printf("\nEl tiempo neto (despues de leer y antes de escribir) de ejecucion del programa fue: %lf\n", (fin_tiempo_neto-comienzo_tiempo_neto));
        // Solamente el proceso ROOT libera A,P,C Y M
        free(A); 
        free(P);
        free(C);
        free(M);
    }
    
    // Se libera memoria.
    free(B);
    free(parte_A);
    free(parte_M);
    free(myP);
    free(parte_C);
    free(parte_faltante_superior);
    free(parte_faltante_inferior);

    MPI_Finalize(); // Se termina el ambiente MPI.
    return 0;
}