#include "stdafx.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "corrector.h"

// Función auxiliar que inserta ordenado alfabéticamente pero PERMITE duplicados
void InsertarSugerencia(const char *temp, char szPalabrasSugeridas[][TAMTOKEN], int &iNumSugeridas) 
{
    if (temp[0] == '\0') {
        return; 
    }

    int izq = 0, der = iNumSugeridas - 1;
    int pos = iNumSugeridas;

    // Búsqueda binaria para encontrar la posición correcta alfabéticamente
    while (izq <= der) {
        int medio = izq + (der - izq) / 2;
        int comp = strcmp(temp, szPalabrasSugeridas[medio]);
        
        // Si comp es 0 (son idénticas), no rompemos el ciclo, 
        // dejamos que se inserte a un lado del elemento duplicado.
        if (comp < 0) {
            der = medio - 1;
            pos = medio;
        } else {
            izq = medio + 1;
        }
    }

    // Desplazar elementos para hacerle espacio a la nueva palabra
    for (int k = iNumSugeridas; k > pos; k--) {
        strcpy(szPalabrasSugeridas[k], szPalabrasSugeridas[k - 1]);
    }
    
    // Copia segura limitando el tamaño máximo
    strncpy(szPalabrasSugeridas[pos], temp, TAMTOKEN - 1);
    szPalabrasSugeridas[pos][TAMTOKEN - 1] = '\0';
    iNumSugeridas++;
}

void Diccionario(char *szNombre, char szPalabras[][TAMTOKEN], int iEstadisticas[], int &iNumElementos)
{
    FILE *origin;
    iNumElementos = 0;
    origin = fopen(szNombre, "r");

    if (origin != NULL) { 
        char linea[1024];
        const char *delimiters = " ,;().\r\t\n"; 

        while(fgets(linea, sizeof(linea), origin) != NULL) { 
            char *palabra = strtok(linea, delimiters); 
            
            while (palabra != NULL) {
                char palabra_Limpia[TAMTOKEN] = ""; 
                int index = 0; 
                
                for (int k = 0; palabra[k] != '\0' && index < TAMTOKEN - 1; k++) {
                    unsigned char c = (unsigned char)palabra[k];
                    if (c >= 'A' && c <= 'Z') {
                        palabra_Limpia[index++] = c + 32; 
                    }
                    else {
                        palabra_Limpia[index++] = c; 
                    }
                }
                palabra_Limpia[index] = '\0'; 

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
                        for(int j = iNumElementos; j > pos; j--) {
                            strcpy(szPalabras[j], szPalabras[j - 1]);
                            iEstadisticas[j] = iEstadisticas[j - 1];
                        }
                        strcpy(szPalabras[pos], palabra_Limpia);
                        iEstadisticas[pos] = 1;
                        iNumElementos++;
                    }
                }
                palabra = strtok(NULL, delimiters); 
            }
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