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
const int MAX_DIAS = 100;       
const int MAX_COMANDAS = 5000;

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

    int buscarIdMozo(Mozo mozos[], int cant, const char* nombre) {
    for (int i = 0; i < cant; i++) {
        if (strcmp(mozos[i].nombre, nombre) == 0) {
            return mozos[i].idMozo;
        }
    }
    return -1;
}
int fechaYaGuardada(char fechas[][11], int cant, const char* fecha) {
    for (int i = 0; i < cant; i++) {
        if (strcmp(fechas[i], fecha) == 0) {
            return 1;
        }
    }
    return 0;
}


int buscarProducto(FILE* f, int cantRegistros, int codigo, Producto &p) {
    int inicio = 0;
    int fin = cantRegistros - 1;
 
    while (inicio <= fin) {
        int medio = (inicio + fin) / 2;
        fseek(f, medio * sizeof(Producto), SEEK_SET);
        fread(&p, sizeof(Producto), 1, f);
 
        if (p.codigo == codigo) {
            return medio;
        }
        if (p.codigo < codigo) {
            inicio = medio + 1;
        } else {
            fin = medio - 1;
        }
    }
    return -1;
}

void ordenarPorMozo(Comanda v[], int n) {
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - 1 - i; j++) {
            if (v[j].idMozo > v[j + 1].idMozo) {
                Comanda aux = v[j];
                v[j] = v[j + 1];
                v[j + 1] = aux;
            }
        }
    }
}

oid generarPlanillasYStock(Mozo mozos[], int cantMozos) {

    FILE* fHist = fopen("comandas_historicas.dat", "rb");
    if (fHist == NULL) {
        cout << "Error: No se pudo abrir 'comandas_historicas.dat'." << endl;
        return;
    }
    char fechas[MAX_DIAS][11];
    int cantFechas = 0;
    ComandaHistorica h;
    while (fread(&h, sizeof(ComandaHistorica), 1, fHist) == 1) {
        if (fechaYaGuardada(fechas, cantFechas, h.fecha) == 0 && cantFechas < MAX_DIAS) {
            strcpy(fechas[cantFechas], h.fecha);
            cantFechas++;
        }
    }

FILE* fInv = fopen("inventario.dat", "r+b");
    if (fInv == NULL) {
        cout << "Error: No se pudo abrir 'inventario.dat'." << endl;
        fclose(fHist);
        return;
    }
    fseek(fInv, 0, SEEK_END);
    int cantProductos = ftell(fInv) / sizeof(Producto);

for (int d = 0; d < cantFechas; d++) {
        Comanda dia[MAX_COMANDAS];
        int n = 0;

seek(fHist, 0, SEEK_SET);            
        while (fread(&h, sizeof(ComandaHistorica), 1, fHist) == 1) {
            if (strcmp(h.fecha, fechas[d]) == 0) {
 
                int id = buscarIdMozo(mozos, cantMozos, h.nombreMozo);
                if (id == -1 || n == MAX_COMANDAS) {
                    cout << "Venta ignorada del " << h.fecha << " (mozo " << h.nombreMozo << ")" << endl;
                } else {
                    dia[n].idMozo = id;
                    dia[n].codigoProducto = h.codigoProducto;
                    dia[n].cantidad = h.cantidad;
                    dia[n].comision = h.comision;
                    n++;
 
                    // Descuento del stock
                    Producto p;
                    int pos = buscarProducto(fInv, cantProductos, h.codigoProducto, p);
                    if (pos == -1) {
                        cout << "Producto " << h.codigoProducto << " no existe en el inventario" << endl;
                    } else {
                        if (p.stockActual < h.cantidad) {
                            cout << "Producto " << p.codigo << " sin stock suficiente, queda en 0" << endl;
                            p.stockActual = 0;
                        } else {
                            p.stockActual = p.stockActual - h.cantidad;
                        }
                        fseek(fInv, pos * sizeof(Producto), SEEK_SET);
                        fwrite(&p, sizeof(Producto), 1, fInv);
                    }
                }
            }
        }
 
        ordenarPorMozo(dia, n);
 
        char nombreArchivo[40];                 
        strcpy(nombreArchivo, "comandas_");
        strcat(nombreArchivo, fechas[d]);
        strcat(nombreArchivo, ".dat");
 
        FILE* fDia = fopen(nombreArchivo, "wb");
        if (fDia == NULL) {
            cout << "Error: No se pudo crear '" << nombreArchivo << "'." << endl;
        } else {
            fwrite(dia, sizeof(Comanda), n, fDia);
            fclose(fDia);
        }
    }
 
    fclose(fHist);
    fclose(fInv);
 
    cout << "Se generaron " << cantFechas << " planillas diarias y se actualizo 'inventario.dat'." << endl;
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
    generarPlanillasYStock(mozos, cantMozos); 

    return 0;
}

