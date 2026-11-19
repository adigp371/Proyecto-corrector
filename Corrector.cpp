#include "stdafx.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "corrector.h"

void Diccionario(char *szNombre, char szPalabras[][TAMTOKEN], int iEstadisticas[], int &iNumElementos)
{
    FILE *origin;
    iNumElementos = 0;
    origin = fopen(szNombre, "r");

    if (origin != NULL) { 
        char linea[1024];
        // Ampliación de delimitadores para evitar fusiones con signos de puntuación extendidos
        const char *delimiters = " ,;().\r\t\n¿?¡!-\":_"; 

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
                    else if (c == 193 || c == 201 || c == 205 || c == 211 || c == 218 || c == 209 || c == 220) {
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

                    if (!existe) {
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
    int izq, der, medio, comp, pos;
    bool existe;

    strcpy(szPalabrasSugeridas[0], szPalabraLeida);
    iNumSugeridas = 1;

    int len = strlen(szPalabraLeida);
    if (len >= TAMTOKEN) len = TAMTOKEN - 1; 

    // Supresión
    for (int i = 0; i < len; i++) {
        int index = 0;
        for (int j = 0; j < len; j++) {
            if (j != i) temp[index++] = szPalabraLeida[j];
        }
        temp[index] = '\0';
        
        izq = 0; der = iNumSugeridas - 1; pos = 0; existe = false;
        while (izq <= der) {
            medio = izq + (der - izq) / 2;
            comp = strcmp(temp, szPalabrasSugeridas[medio]);
            if (comp == 0) { existe = true; break; }
            if (comp < 0) { der = medio - 1; pos = medio; }
            else { izq = medio + 1; pos = izq; }
        }
        if (!existe && iNumSugeridas < 5000) { // Prevención de buffer overflow
            for(int k = iNumSugeridas; k > pos; k--) strcpy(szPalabrasSugeridas[k], szPalabrasSugeridas[k - 1]);
            strcpy(szPalabrasSugeridas[pos], temp);
            iNumSugeridas++;
        }
    }

    // Intercambio
    for (int i = 0; i < len - 1; i++) {
        strcpy(temp, szPalabraLeida);
        char c = temp[i];
        temp[i] = temp[i + 1];
        temp[i + 1] = c;
        
        izq = 0; der = iNumSugeridas - 1; pos = 0; existe = false;
        while (izq <= der) {
            medio = izq + (der - izq) / 2;
            comp = strcmp(temp, szPalabrasSugeridas[medio]);
            if (comp == 0) { existe = true; break; }
            if (comp < 0) { der = medio - 1; pos = medio; }
            else { izq = medio + 1; pos = izq; }
        }
        if (!existe && iNumSugeridas < 5000) { // Prevención de buffer overflow
            for(int k = iNumSugeridas; k > pos; k--) strcpy(szPalabrasSugeridas[k], szPalabrasSugeridas[k - 1]);
            strcpy(szPalabrasSugeridas[pos], temp);
            iNumSugeridas++;
        }
    }
}

void ListaCandidatas(
	char szPalabrasSugeridas[][TAMTOKEN], int iNumSugeridas,
	char szPalabras[][TAMTOKEN], int iEstadisticas[], int iNumElementos,
	char szListaFinal[][TAMTOKEN], int iPeso[], int &iNumLista)
{
    // Implementación pendiente
}