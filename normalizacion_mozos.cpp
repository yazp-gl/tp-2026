#include <iostream>
#include <cstdio>
#include <cstring>

using namespace std;  
struct ComandaHistorica {
    char fecha[11];
    char nombreMozo[50];
    int codigoProducto;
    int cantidad;
    float comision;
    //char relleno[3];
};

struct Producto {
    int codigo;
    char descripcion[50];
    float precio;
    int stockActual;
};

struct Mozo {
    int idMozo;
    char nombre[50];
    char password[20];
    float totalComision;
};

struct Comanda {
    int idMozo;
    int codigoProducto;
    int cantidad;
    float comision;
};

const int K = 5;

void encriptarClave(const char* origen, char* destino, int k) {
    int i = 0;
    while (origen[i] != '\0') {
        destino[i] = origen[i] + k;
        i++;
    }
    destino[i] = '\0';
}

int buscarOAgregarMozo(Mozo mozos[], int &cantMozos, const char* nombre, float comision) {
    for (int i = 0; i < cantMozos; i++) {
        if (strcmp(mozos[i].nombre, nombre) == 0) {
            mozos[i].totalComision += comision;
            return mozos[i].idMozo;
        }
    }

    cantMozos++;
    mozos[cantMozos - 1].idMozo = cantMozos;
    strcpy(mozos[cantMozos - 1].nombre, nombre);
    mozos[cantMozos - 1].totalComision = comision;

    char clavePlana[20];
    sprintf(clavePlana, "%d", cantMozos);
    encriptarClave(clavePlana, mozos[cantMozos - 1].password, K);

    return cantMozos;
}

int main() {
    if (sizeof(ComandaHistorica) != 76 || sizeof(Producto) != 64) {
        cout << "Error: Tamaño de estructuras incorrecto." << endl;
        return 1;
    }

    FILE* fHist = fopen("comandas_historicas.dat", "rb");
    if (fHist == NULL) {
        cout << "Error: No se pudo abrir 'comandas_historicas.dat'." << endl;
        return 1;
    }

    Mozo mozos[100];
    int cantMozos = 0;
    ComandaHistorica regHist;

    while (fread(&regHist, sizeof(ComandaHistorica), 1, fHist) == 1) {
        buscarOAgregarMozo(mozos, cantMozos, regHist.nombreMozo, regHist.comision);
    }

    fclose(fHist);

    FILE* fMozos = fopen("mozos.dat", "wb");
    if (fMozos != NULL) {
        for (int i = 0; i < cantMozos; i++) {
            fwrite(&mozos[i], sizeof(Mozo), 1, fMozos);
        }
        fclose(fMozos);
    }

    cout << "Archivo 'mozos.dat' generado exitosamente." << endl;
    cout << "Total de mozos registrados: " << cantMozos << endl;

    return 0;
}
