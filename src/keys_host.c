/* Backend de pruebas: la matriz se simula escribiendo en host_matrix[]. */
#include "keys.h"
u8 host_matrix[8];
int host_reads;
int ql_read_row(int row) { host_reads++; return host_matrix[row & 7]; }
