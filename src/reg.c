#include <stdlib.h>
#include <R_ext/Rdynload.h>
#include <Rinternals.h>

/* read.c */
extern SEXP read_tiff(SEXP sFn, SEXP sNative, SEXP sAll, SEXP sConvert, SEXP sInfo, SEXP sIndexed,
		      SEXP sOriginal, SEXP sPayload);
/* write.c */
extern SEXP write_tiff(SEXP image, SEXP where, SEXP sBPS, SEXP sCompr, SEXP sReduce);

static const R_CallMethodDef CAPI[] = {
    {"read_tiff",  (DL_FUNC) &read_tiff , 8},
    {"write_tiff", (DL_FUNC) &write_tiff, 5},
    {NULL, NULL, 0}
};

void R_init_tiff(DllInfo *dll)
{
    R_registerRoutines(dll, NULL, CAPI, NULL, NULL);
    R_useDynamicSymbols(dll, FALSE);
}
