* ql_ipc.s - llamada a MT.IPCOM (trap #1, D0=$11). SIN PROBAR; ajustar directivas a as68 si hace falta.
* long ql_ipc(char *blk)  -> D1.B de la respuesta (0..255) o codigo de error negativo
        xdef    _ql_ipc
        section code
_ql_ipc:
        movem.l d2-d7/a2-a6,-(sp)
        move.l  48(sp),a3            ; 11 registros (44 bytes) + direccion de retorno (4) -> argumento
        moveq   #$11,d0              ; MT.IPCOM
        trap    #1
        tst.l   d0
        bne.s   .fin                 ; error: D0 ya es negativo
        moveq   #0,d0
        move.b  d1,d0                ; respuesta del IPC
.fin:
        movem.l (sp)+,d2-d7/a2-a6
        rts
