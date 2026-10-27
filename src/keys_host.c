/* Test backend: the matrix is simulated by writing to host_matrix[]. */
#include "keys.h"
u8 host_matrix[8];
int host_reads;
int ql_read_row(int row) { host_reads++; return host_matrix[row & 7]; }
