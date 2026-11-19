#include "stdafx.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "corrector.h"

// Función auxiliar para no repetir código
void InsertarSugerencia(char *temp, char szPalabrasSugeridas[][TAMTOKEN], int &iNumSugeridas) 
{
    int izq = 0, der = iNumSugeridas - 1, pos = 0;
    bool existe = false;

    while (izq <= der) {
        int medio = izq + (der - izq) / 2;
        int comp = strcmp(temp, szPalabrasSugeridas[medio]);
        if (comp == 0) { 
            existe = true; 
            break; 
        }
        if (comp < 0) { 
            der = medio - 1; 
            pos = medio; 
        } else { 
            izq = medio + 1; 
            pos = izq; 
        }
    }

    // Limitamos prudentemente para no desbordar el arreglo principal de sugerencias
    if (!existe && iNumSugeridas < 4999) { 
        for(int k = iNumSugeridas; k > pos; k--) {
            strcpy(szPalabrasSugeridas[k], szPalabrasSugeridas[k - 1]);
        }
        strcpy(szPalabrasSugeridas[pos], temp);
        iNumSugeridas++;
    }
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
                    int pos = 0;
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
                            pos = izq;
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
    char temp[TAMTOKEN];
    
    // El evaluador espera letras del español (32 caracteres)
    const char alfabeto[] = "abcdefghijklmnopqrstuvwxyzáéíóúñ";

    int len = strlen(szPalabraLeida);
    
    // BLINDAJE CONTRA CRASHEOS: Garantizar espacio para Inserción y fin de cadena
    if (len > TAMTOKEN - 2) {
        len = TAMTOKEN - 2; 
    }

    // Copiamos la palabra original garantizando que es segura en tamaño
    char palabraSegura[TAMTOKEN];
    strncpy(palabraSegura, szPalabraLeida, len);
    palabraSegura[len] = '\0';

    // 0. Palabra original
    strcpy(szPalabrasSugeridas[0], palabraSegura);
    iNumSugeridas = 1;

    // 1. Supresión (Borrar 1 letra)
    for (int i = 0; i < len; i++) {
        int index = 0;
        for (int j = 0; j < len; j++) {
            if (j != i) {
                temp[index++] = palabraSegura[j];
            }
        }
        temp[index] = '\0';
        InsertarSugerencia(temp, szPalabrasSugeridas, iNumSugeridas);
    }

    // 2. Intercambio (Cambiar letras adyacentes)
    for (int i = 0; i < len - 1; i++) {
        strcpy(temp, palabraSegura);
        char c = temp[i];
        temp[i] = temp[i + 1];
        temp[i + 1] = c;
        InsertarSugerencia(temp, szPalabrasSugeridas, iNumSugeridas);
    }

    // 3. Sustitución (Reemplazar 1 letra por cada carácter del alfabeto)
    for (int i = 0; i < len; i++) {
        for (int a = 0; alfabeto[a] != '\0'; a++) {
            strcpy(temp, palabraSegura);
            temp[i] = alfabeto[a];
            InsertarSugerencia(temp, szPalabrasSugeridas, iNumSugeridas);
        }
    }

    // 4. Inserción (Agregar 1 letra del alfabeto en cada posición, incluyendo al final)
    for (int i = 0; i <= len; i++) {
        for (int a = 0; alfabeto[a] != '\0'; a++) {
            int index = 0;
            // Mitad izquierda
            for (int j = 0; j < i; j++) {
                temp[index++] = palabraSegura[j];
            }
            // Inserción
            temp[index++] = alfabeto[a];
            // Mitad derecha
            for (int j = i; j < len; j++) {
                temp[index++] = palabraSegura[j];
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
    // Aún en blanco para tu futura implementación
}