#include "stdafx.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "corrector.h"

// Función auxiliar para insertar palabras ordenadas
void InsertarSugerencia(char *temp, char szPalabrasSugeridas[][TAMTOKEN], int &iNumSugeridas) 
{
    // 1. Evitar sugerir cadenas vacías (como pasa al suprimir la única letra de una palabra)
    if (temp[0] == '\0') {
        return; 
    }

    // 2. Blindaje de seguridad para que la sugerencia nunca exceda el arreglo bidimensional
    temp[TAMTOKEN - 1] = '\0';

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

    // 3. Límite conservador para evitar desbordar el arreglo principal del evaluador
    if (!existe && iNumSugeridas < 4500) { 
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
    
    // Usamos un buffer más grande que TAMTOKEN temporalmente para evitar el Buffer Overflow local
    char temp[256]; 
    
    // Alfabeto utilizando códigos Hexadecimales de un solo byte (Evita errores de UTF-8)
    // \xE1=á, \xE9=é, \xED=í, \xF3=ó, \xFA=ú, \xF1=ñ
    const char alfabeto[] = "abcdefghijklmnopqrstuvwxyz\xE1\xE9\xED\xF3\xFA\xF1";

    int len = strlen(szPalabraLeida);
    
    // Garantizar que quede espacio para inserciones sin pasarse nunca de TAMTOKEN
    if (len > TAMTOKEN - 2) {
        len = TAMTOKEN - 2; 
    }

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

    // 3. Sustitución (Reemplazar 1 letra)
    for (int i = 0; i < len; i++) {
        for (int a = 0; alfabeto[a] != '\0'; a++) {
            strcpy(temp, palabraSegura);
            temp[i] = alfabeto[a];
            InsertarSugerencia(temp, szPalabrasSugeridas, iNumSugeridas);
        }
    }

    // 4. Inserción (Agregar 1 letra)
    for (int i = 0; i <= len; i++) {
        for (int a = 0; alfabeto[a] != '\0'; a++) {
            int index = 0;
            // Lado izquierdo
            for (int j = 0; j < i; j++) {
                temp[index++] = palabraSegura[j];
            }
            // Letra insertada
            temp[index++] = alfabeto[a];
            // Lado derecho
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
    // Pendiente para la siguiente fase
}