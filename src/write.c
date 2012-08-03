#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"

#include <Rinternals.h>

SEXP write_tiff(SEXP image, SEXP where, SEXP sBPS, SEXP sCompr, SEXP sQual) {
    SEXP dims, img_list = 0;
    tiff_job_t rj;
    TIFF *tiff;
    FILE *f;
    int native = 0, raw_array = 0, bps = asInteger(sBPS), compression = asInteger(sCompr), img_index = 0, n_img = 1;
    float quality = asReal(sQual);
    uint32 width, height, planes = 1;

    if (TYPEOF(image) == VECSXP) {
	if ((n_img = LENGTH(image)) == 0) {
	    Rf_warning("empty image list, nothing to do");
	    return R_NilValue;
	}
	img_list = image;
    }

    if (bps != 8 && bps != 16 && bps != 32)
	Rf_error("currently bits.per.sample must be 8, 16 or 32");

    if (TYPEOF(where) == RAWSXP) {
	SEXP rv = allocVector(RAWSXP, INIT_SIZE);
	rj.rvtail = rj.rvlist = PROTECT(CONS(rv, R_NilValue));
	rj.data = (char*) RAW(rv);
	rj.len = LENGTH(rv);
	rj.ptr = 0;
	rj.rvlen = 0;
	rj.f = f = 0;
    } else {
	const char *fn;
	if (TYPEOF(where) != STRSXP || LENGTH(where) < 1) Rf_error("invalid filename");
	fn = CHAR(STRING_ELT(where, 0));
	f = fopen(fn, "wb");
	if (!f) Rf_error("unable to create %s", fn);
	rj.f = f;
    }

    tiff = TIFF_Open("wm", &rj);
    if (!tiff)
	Rf_error("cannot create TIFF structure");

    while (1) {
	if (img_list)
	    image = VECTOR_ELT(img_list, img_index++);

	if (inherits(image, "nativeRaster") && TYPEOF(image) == INTSXP)
	    native = 1;
	
	if (TYPEOF(image) == RAWSXP)
	    raw_array = 1;

	if (!native && !raw_array && TYPEOF(image) != REALSXP)
	    Rf_error("image must be a matrix or array of raw or real numbers");
	
	dims = Rf_getAttrib(image, R_DimSymbol);
	if (dims == R_NilValue || TYPEOF(dims) != INTSXP || LENGTH(dims) < 2 || LENGTH(dims) > 3)
	    Rf_error("image must be a matrix or an array of two or three dimensions");
	
	if (raw_array && LENGTH(dims) == 3) { /* raw arrays have either bpp, width, height or width, height dimensions */
	    planes = INTEGER(dims)[0];
	    width = INTEGER(dims)[1];
	    height = INTEGER(dims)[2];
	} else { /* others have width, height[, bpp] */
	    width = INTEGER(dims)[1];
	    height = INTEGER(dims)[0];
	    if (LENGTH(dims) == 3)
		planes = INTEGER(dims)[2];
	}
	
	if (planes < 1 || planes > 4)
	    Rf_error("image must have either 1 (grayscale), 2 (GA), 3 (RGB) or 4 (RGBA) planes");
	
	if (native) { /* nativeRaster should have a "channels" attribute if it has anything else than 4 channels */
	    SEXP cha = getAttrib(image, install("channels"));
	    if (cha != R_NilValue) {
		planes = asInteger(cha);
		if (planes < 1 || planes > 4)
		    planes = 4;
	    } else
		planes = 4;
	}
	if (raw_array) {
	    if (planes != 4)
		Rf_error("Only RGBA format is supported as raw data");
	    native = 1; /* from now on we treat raw arrays like native */
	}
	
	if (native) {
	    TIFFSetField(tiff, TIFFTAG_IMAGEWIDTH, width);
	    TIFFSetField(tiff, TIFFTAG_IMAGELENGTH, height);
	    TIFFSetField(tiff, TIFFTAG_PLANARCONFIG, 1);
	    TIFFSetField(tiff, TIFFTAG_BITSPERSAMPLE, 8);
	    TIFFSetField(tiff, TIFFTAG_SAMPLESPERPIXEL, 4);
	    TIFFSetField(tiff, TIFFTAG_ROWSPERSTRIP, height);
	    TIFFSetField(tiff, TIFFTAG_COMPRESSION, compression);
	    TIFFWriteEncodedStrip(tiff, 0, INTEGER(image), width * height * 4);
	}

	if (!img_list)
	    break;
	else if (img_index < n_img)
	    TIFFWriteDirectory(tiff);
    }
    TIFFClose(tiff);
    return R_NilValue;
}
