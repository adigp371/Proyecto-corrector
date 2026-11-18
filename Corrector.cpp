#include "stdafx.h"
#include <string.h>
#include <ctype.h>
#include "corrector.h"

//Funciones publicas del proyecto
/*****************************************************************************************************************
	DICCIONARIO: Esta funcion crea el diccionario completo
	char *	szNombre				:	Nombre del archivo de donde se sacaran las palabras del diccionario	
	char	szPalabras[][TAMTOKEN]	:	Arreglo con las palabras completas del diccionario
	int		iEstadisticas[]			:	Arreglo con el numero de veces que aparecen las palabras en el diccionario
	int &	iNumElementos			:	Numero de elementos en el diccionario
******************************************************************************************************************/
#include <stdio.h>
#include <string.h>
#include <ctype.h>

void Diccionario(char *szNombre, char szPalabras[][TAMTOKEN], int iEstadisticas[], int &iNumElementos)
{
    FILE *origin;
    iNumElementos = 0;

    origin = fopen(szNombre, "r");

    if (origin != NULL) { 
        
        char linea[1024];
        const char *delimiters = " ,;().\r\t\n";

        // Leer un renglon del archivo
        while(fgets(linea, sizeof(linea), origin) != NULL) { 
            
            // Primero obtenemos la primera palabra del renglon
            char *palabra = strtok(linea, delimiters); 
            
            // Mientras haya palabras en el renglon, las procesamos
            while (palabra != NULL) {
                
                char palabra_Limpia[TAMTOKEN] = ""; 
                int index = 0; 
                
                // Normalizamos (Filtro especial para Español)
                for (int k = 0; palabra[k] != '\0' && index < TAMTOKEN - 1; k++) {
                    unsigned char c = (unsigned char)palabra[k];
                    
                    // Si es una letra normal (a-z, A-Z)
                    if (isalpha(c)) {
                        palabra_Limpia[index] = tolower(c);
                        index++;
                    }
                    // Si es una minus con acento, ñ o diéresis (á, é, í, ó, ú, ñ, ü)
                    else if (c == 225 || c == 233 || c == 237 || c == 243 || c == 250 || c == 241 || c == 252) {
                        palabra_Limpia[index] = c;
                        index++;
                    }
                    // Si es una mayus con acento, Ñ o diéresis (Á, É, Í, Ó, Ú, Ñ, Ü)
                    else if (c == 193 || c == 201 || c == 205 || c == 211 || c == 218 || c == 209 || c == 220) {
                        palabra_Limpia[index] = c + 32; // Convertimos a minúscula sumando 32
                        index++;
                    }

                }
                    // Cualquier otro carácter se ignora
                palabra_Limpia[index] = '\0'; 

                // Verificar su existencia
                if (strlen(palabra_Limpia) > 0) {
                    
                    int existe = 0;

                    for(int i = 0; i < iNumElementos; i++){
                        if(strcmp(palabra_Limpia, szPalabras[i]) == 0){
                            iEstadisticas[i]++;
                            existe = 1;
                            break;
                        }
                    }

                    // Incluir si es nueva
                    if(existe == 0){
                        int posicion = 0;

                        while(posicion < iNumElementos && strcmp(palabra_Limpia, szPalabras[posicion]) > 0){
                            posicion++;
                        }

                        // Desplazar palabras y estadísticas
                        for(int j = iNumElementos; j > posicion; j--){
                            strcpy(szPalabras[j], szPalabras[j - 1]);
                            iEstadisticas[j] = iEstadisticas[j - 1];
                        }

                        strcpy(szPalabras[posicion], palabra_Limpia);
                        iEstadisticas[posicion] = 1;

                        iNumElementos++;
                    }
                }
                
                // Seguir con la siguiente palabra del renglón
                palabra = strtok(NULL, delimiters); 
            }
        }
        
        fclose(origin);
    }
}

/*****************************************************************************************************************
	ClonaPalabras: toma una palabra y obtiene todas las combinaciones y permutaciones requeridas por el metodo
	char *	szPalabraLeida,						// Palabra a clonar
	char	szPalabrasSugeridas[][TAMTOKEN], 	//Lista de palabras clonadas
	int &	iNumSugeridas)						//Numero de elementos en la lista
******************************************************************************************************************/
void    ClonaPalabras(
    char *  szPalabraLeida,                     
    char    szPalabrasSugeridas[][TAMTOKEN],    
    int &   iNumSugeridas)                      
{
    char temp[TAMTOKEN]; 

    strcpy(szPalabrasSugeridas[0], szPalabraLeida);
    strcpy(temp, szPalabraLeida);
    iNumSugeridas = 1;

	// Supresion de un caracter
    for(int i = 0; i < strlen(szPalabraLeida); i++){
        char temp_supr[TAMTOKEN];
        int index_supr = 0;

        for(int j = 0; j < strlen(szPalabraLeida); j++){
            if(j != i){
                temp_supr[index_supr] = temp[j];
                index_supr++;
            }
        }
        temp_supr[index_supr] = '\0';

        // Verificar si la palbra existe
        int existe = 0;
        for(int m = 0; m < iNumSugeridas; m++){
            if(strcmp(temp_supr, szPalabrasSugeridas[m]) == 0){
                existe = 1;
                break; // Si ya existe, no se hace nada
            }
        }

        // En caso de ser nueva, ingresamos
        if(existe == 0){
            int posicion = 0;

            // Orden alfabetico
            while(posicion < iNumSugeridas && strcmp(temp_supr, szPalabrasSugeridas[posicion]) > 0){
                posicion++;
            }

            for(int k = iNumSugeridas; k > posicion; k--){
                strcpy(szPalabrasSugeridas[k], szPalabrasSugeridas[k - 1]);
            }

            // Insertamos la palabra nueva
            strcpy(szPalabrasSugeridas[posicion], temp_supr);
            
            iNumSugeridas++;
        }
    }

	// Intercambio de caracateres
    for(int i = 0; i < strlen(szPalabraLeida) - 1; i++){
        char temp_cambio[TAMTOKEN];
        strcpy(temp_cambio, temp);

        char CaracterTemp = temp_cambio[i];
        temp_cambio[i] = temp_cambio[i + 1];
        temp_cambio[i + 1] = CaracterTemp;

         // Verificar si la palbra existe
        int existe = 0;
        for(int m = 0; m < iNumSugeridas; m++){
            if(strcmp(temp_cambio, szPalabrasSugeridas[m]) == 0){
                existe = 1;
                break; // Si ya existe, no se hace nada
            }
        }

        // En caso de ser nueva, ingresamos
        if(existe == 0){
            int posicion = 0;

            // Orden alfabetico
            while(posicion < iNumSugeridas && strcmp(temp_cambio, szPalabrasSugeridas[posicion]) > 0){
                posicion++;
            }

            for(int k = iNumSugeridas; k > posicion; k--){
                strcpy(szPalabrasSugeridas[k], szPalabrasSugeridas[k - 1]);
            }

            // Insertamos la palabra nueva
            strcpy(szPalabrasSugeridas[posicion], temp_cambio);
            
            iNumSugeridas++;
        }
    }



}

/*****************************************************************************************************************
	ListaCandidatas: Esta funcion recupera desde el diccionario las palabras validas y su peso
	Regresa las palabras ordenadas por su peso
	char	szPalabrasSugeridas[][TAMTOKEN],	//Lista de palabras clonadas
	int		iNumSugeridas,						//Lista de palabras clonadas
	char	szPalabras[][TAMTOKEN],				//Lista de palabras del diccionario
	int		iEstadisticas[],					//Lista de las frecuencias de las palabras
	int		iNumElementos,						//Numero de elementos en el diccionario
	char	szListaFinal[][TAMTOKEN],			//Lista final de palabras a sugerir
	int		iPeso[],							//Peso de las palabras en la lista final
	int &	iNumLista)							//Numero de elementos en la szListaFinal
******************************************************************************************************************/

void	ListaCandidatas		(
	char	szPalabrasSugeridas[][TAMTOKEN],	//Lista de palabras clonadas
	int		iNumSugeridas,						//Lista de palabras clonadas
	char	szPalabras[][TAMTOKEN],				//Lista de palabras del diccionario
	int		iEstadisticas[],					//Lista de las frecuencias de las palabras
	int		iNumElementos,						//Numero de elementos en el diccionario
	char	szListaFinal[][TAMTOKEN],			//Lista final de palabras a sugerir
	int		iPeso[],							//Peso de las palabras en la lista final
	int &	iNumLista)							//Numero de elementos en la szListaFinal
{

	
}