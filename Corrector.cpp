#include "stdafx.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "corrector.h"

// Funcion auxiliar que inserta en orden alfabetico permitiendo duplicados
void InsertarSugerencia(const char *temp, char szPalabrasSugeridas[][TAMTOKEN], int &iNumSugeridas) 
{
    if (temp[0] == '\0') {
        return; // Rompemos la funcion para evitar corromper el arreglo de sugeridas con vacios
    }

    int izq = 0, der = iNumSugeridas - 1;
    int pos = iNumSugeridas;

    // Busqueda binaria para encontrar la posicion correcta
    while (izq <= der) {
        int medio = izq + (der - izq) / 2;
        int comp = strcmp(temp, szPalabrasSugeridas[medio]);
        
        // Si comp es 0 , no se rompe el ciclo
        // dejamos que se inserte a un lado del elemento duplicado.
        if (comp < 0) { // Siempre vamos a asumir que la palabra es la ultima a la derecha y trabajar ocn la izq
            der = medio - 1;
            pos = medio;
        } else {
            izq = medio + 1; // Tanto la igual dad como la dereche se trabajan en conjunto
        }
    }

    // Desplazar elementos para hacerle espacio a la nueva palabra
    for (int k = iNumSugeridas; k > pos; k--) {
        strcpy(szPalabrasSugeridas[k], szPalabrasSugeridas[k - 1]); // Desplazamos desde el maximo numero de posiciones hasta la pos esperada
    }                                                              // Recorriendo a la derecha
    
    // Copia segura limitando el tamaño max
    strncpy(szPalabrasSugeridas[pos], temp, TAMTOKEN - 1); // Se guarda la palabra en la poscicion indicada limitando el tamaño de la misma
    szPalabrasSugeridas[pos][TAMTOKEN - 1] = '\0'; // Cerramos el string
    iNumSugeridas++; // Aumentamos el numero de palabras sugeridas
}

void Diccionario(char *szNombre, char szPalabras[][TAMTOKEN], int iEstadisticas[], int &iNumElementos)
{
    FILE *origin; // Declaracion de la variable de archivo
    iNumElementos = 0; //Elementos en el diccionario
    origin = fopen(szNombre, "r"); //Apertura del archivo en modo lectura

    if (origin != NULL) { 
        char linea[1024]; // Tamaño de linea 1023 bytes
        const char *delimiters = " ,;().\r\t\n"; // Delimitadores donde se cortaran las palabras

        while(fgets(linea, sizeof(linea), origin) != NULL) { // Leemos linea a linea hasta el final del archivo
            char *palabra = strtok(linea, delimiters); // USo de funcion strtok para separar las palabras, se direge como puntero para
                                                    // modificar el arreglo original

            while (palabra != NULL) { // El ciclo maneja las palabras de la linea hasta que se acaben 
                char palabra_Limpia[TAMTOKEN] = "";  // Se declara palabra limpia vacia para evitar basura
                int Write_index = 0; // Puntero auxiliar para la escritura de la palabra
                
                for (int k = 0; palabra[k] != '\0' && Write_index < TAMTOKEN - 1; k++) { // Se evita que el puntero de escritura escriba en '\0'
                    unsigned char c = (unsigned char)palabra[k]; // Se declara sin signo para evitar probelmas con carcacteres especiales
                    
                    if (c >= 'A' && c <= 'Z') { 
                        palabra_Limpia[Write_index] = c + 32;
                        Write_index++; 
                    }
                    else {
                        palabra_Limpia[Write_index] = c;
                        Write_index++; 
                    }
                }
                palabra_Limpia[Write_index] = '\0'; // Se cierra el arreglo de la palabra

                if (strlen(palabra_Limpia) > 0) {
                    int izq = 0, der = iNumElementos - 1;
                    int pos = iNumElementos;
                    bool existe = false;

                    while (izq <= der) {
                        int medio = izq + (der - izq) / 2;
                        int comparacion = strcmp(palabra_Limpia, szPalabras[medio]);
                        if (comparacion == 0) {
                            iEstadisticas[medio]++;
                            existe = true;
                            break;
                        } else if (comparacion < 0) {
                            der = medio - 1;
                            pos = medio;
                        } else {
                            izq = medio + 1;
                        }
                    }

                    if (!existe && iNumElementos < 50000) { 
                        for(int j = iNumElementos; j > pos; j--) { // Se recorre el arreglo a la derecha hasta la posicion de insercion 
                            strcpy(szPalabras[j], szPalabras[j - 1]);
                            iEstadisticas[j] = iEstadisticas[j - 1];
                        }
                        strcpy(szPalabras[pos], palabra_Limpia); // Se copia la palabra nueva en la posicion correspondiente
                        iEstadisticas[pos] = 1;
                        iNumElementos++;
                    }
                }
                palabra = strtok(NULL, delimiters); // Volvemos a llamra a la funcion utilizando NULL para iniciar desde donde se quedo 
            }                                       // el puntero de la propia funcion en arreglo de linea 
        }
        fclose(origin);
    }
}

void ClonaPalabras(char *szPalabraLeida, char szPalabrasSugeridas[][TAMTOKEN], int &iNumSugeridas) 
{
    iNumSugeridas = 0;
    char temp[256]; 
    
    // Alfabeto utilizando códigos Hexadecimales de un solo byte 
    // \xE1=á, \xE9=é, \xED=í, \xF3=ó, \xFA=ú, \xF1=ñ
    const char alfabeto[] = "abcdefghijklmnopqrstuvwxyz\xE1\xE9\xED\xF3\xFA\xF1";

    int len = strlen(szPalabraLeida);
    if (len > TAMTOKEN - 2) {
        len = TAMTOKEN - 2; 
    }

    // 0. Palabra original
    InsertarSugerencia(szPalabraLeida, szPalabrasSugeridas, iNumSugeridas);

    // 1. Supresión (Borrar 1 letra)
    for (int i = 0; i < len; i++) {
        int index = 0;
        for (int j = 0; j < len; j++) {
            if (j != i) {
                temp[index++] = szPalabraLeida[j];
            }
        }
        temp[index] = '\0';
        InsertarSugerencia(temp, szPalabrasSugeridas, iNumSugeridas);
    }

    // 2. Intercambio (Cambiar letras adyacentes)
    for (int i = 0; i < len - 1; i++) {
        strncpy(temp, szPalabraLeida, len);
        temp[len] = '\0';
        char c = temp[i];
        temp[i] = temp[i + 1];
        temp[i + 1] = c;
        InsertarSugerencia(temp, szPalabrasSugeridas, iNumSugeridas);
    }

    // 3. Sustitución (Reemplazar 1 letra)
    for (int i = 0; i < len; i++) {
        for (int a = 0; alfabeto[a] != '\0'; a++) {
            strncpy(temp, szPalabraLeida, len);
            temp[len] = '\0';
            temp[i] = alfabeto[a];
            InsertarSugerencia(temp, szPalabrasSugeridas, iNumSugeridas);
        }
    }

    // 4. Inserción (Agregar 1 letra)
    for (int i = 0; i <= len; i++) {
        for (int a = 0; alfabeto[a] != '\0'; a++) {
            int index = 0;
            for (int j = 0; j < i; j++) {
                temp[index++] = szPalabraLeida[j];
            }
            temp[index++] = alfabeto[a];
            for (int j = i; j < len; j++) {
                temp[index++] = szPalabraLeida[j];
            }
            temp[index] = '\0';
            InsertarSugerencia(temp, szPalabrasSugeridas, iNumSugeridas);
        }
    }
}

void ListaCandidatas(
	char szPalabrasSugeridas[][TAMTOKEN], int iNumSugeridas,
	char szPalabras[][TAMTOKEN], int iEstadisticas[], int iNumElementos,
	char szListaFinal[][TAMTOKEN], int iPeso[], int &iNumLista)
{
    iNumLista = 0;

    for (int i = 0; i < iNumSugeridas; i++) {
        int izq = 0, der = iNumElementos - 1;
        int idxEncontrado = -1;

        while (izq <= der) {
            int medio = izq + (der - izq) / 2;
            int cmp = strcmp(szPalabrasSugeridas[i], szPalabras[medio]);
            
            if (cmp == 0) {
                idxEncontrado = medio;
                break;
            } else if (cmp < 0) {
                der = medio - 1;
            } else {
                izq = medio + 1;
            }
        }

        // Si la palabra clonada sí existe en el diccionario general
        if (idxEncontrado != -1) {
            // Evitar duplicados en la lista final (aquí sí es necesario)
            bool yaExiste = false;
            for (int j = 0; j < iNumLista; j++) {
                if (strcmp(szListaFinal[j], szPalabrasSugeridas[i]) == 0) {
                    yaExiste = true;
                    break;
                }
            }

            if (!yaExiste) {
                int pos = iNumLista;
                while (pos > 0) {
                    if (iEstadisticas[idxEncontrado] > iPeso[pos - 1]) {
                        // Desplazar hacia abajo
                    } else if (iEstadisticas[idxEncontrado] == iPeso[pos - 1] && 
                               strcmp(szPalabrasSugeridas[i], szListaFinal[pos - 1]) < 0) {
                        // Desplazar hacia abajo por orden alfabético si empatan en peso
                    } else {
                        break;
                    }
                    strcpy(szListaFinal[pos], szListaFinal[pos - 1]);
                    iPeso[pos] = iPeso[pos - 1];
                    pos--;
                }
                
                strcpy(szListaFinal[pos], szPalabrasSugeridas[i]);
                iPeso[pos] = iEstadisticas[idxEncontrado];
                iNumLista++;
            }
        }
    }
}