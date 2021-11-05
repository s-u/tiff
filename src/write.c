#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "common.h"

#include <Rinternals.h>
#include <Rversion.h>

#define INIT_SIZE (256 * 1024)

#define HAS_ALPHA 0x01
#define IS_GRAY   0x02
#define IS_RGB    0x04
/* if neither GRAY/RGB is set then it's B/W */

int analyze_native(const unsigned int *what, int length) {
    int alpha = 0, ac = 0, i;
    for (i = 0; i < length; i++) {
	if (!alpha && (what[i] & 0xff000000) != 0xff000000) alpha = 1;
	if (ac < 2 && ((what[i] & 0xff) != ((what[i] >> 8) & 0xff) || (what[i] & 0xff) != ((what[i] >> 16) & 0xff))) ac = 2;
	if (ac == 0 && (what[i] & 0xffffff) != 0xffffff && (what[i] & 0xffffff)) ac = 1;
	if (ac == 2 && alpha) break; /* no need to continue */
    }
    return alpha | (ac << 1);
}

SEXP write_tiff(SEXP image, SEXP where, SEXP sBPS, SEXP sCompr, SEXP sReduce) {
    SEXP dims, img_list = 0;
    tiff_job_t rj;
    TIFF *tiff;
    FILE *f;
    int native = 0, raw_array = 0, bps = asInteger(sBPS), compression = asInteger(sCompr),
	reduce = asInteger(sReduce),
	img_index = 0, n_img = 1;
    uint32_t width, height, planes = 1;

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
	rj.alloc = INIT_SIZE;
	if (!(rj.data = malloc(rj.alloc)))
	    Rf_error("unable to allocate memory for the in-memory output buffer");
	rj.len = 0;
	rj.ptr = 0;
	rj.f = f = 0;
    } else {
	const char *fn;
	if (TYPEOF(where) != STRSXP || LENGTH(where) < 1) Rf_error("invalid filename");
	fn = CHAR(STRING_ELT(where, 0));
	f = fopen(fn, "w+b");
	if (!f) Rf_error("unable to create %s", fn);
	rj.f = f;
    }

    tiff = TIFF_Open("wm", &rj);
    if (!tiff) {
	if (!rj.f)
	    free(rj.data);
	Rf_error("cannot create TIFF structure");
    }

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
	
	TIFFSetField(tiff, TIFFTAG_IMAGEWIDTH, width);
	TIFFSetField(tiff, TIFFTAG_IMAGELENGTH, height);
	TIFFSetField(tiff, TIFFTAG_PLANARCONFIG, 1);
	TIFFSetField(tiff, TIFFTAG_SOFTWARE, "tiff package, R " R_MAJOR "." R_MINOR);
	if (native) {
	    TIFFSetField(tiff, TIFFTAG_BITSPERSAMPLE, 8);
	    if (reduce) {
		int an = analyze_native((const unsigned int*) INTEGER(image), width * height);
		if (an == (HAS_ALPHA | IS_RGB))
		    reduce = 0;
		else { /* we only reduce to RGB, GA or G */
		    int out_spp = ((an & HAS_ALPHA) ? 1 : 0 ) + ((an & IS_RGB) ? 3 : 1);
		    uint32_t i = 1, n = width * height;
		    tdata_t buf;
		    unsigned char *data8;
		    const unsigned int *nd = (const unsigned int*) INTEGER(image);
		    TIFFSetField(tiff, TIFFTAG_SAMPLESPERPIXEL, out_spp);
		    TIFFSetField(tiff, TIFFTAG_ROWSPERSTRIP, height);
		    TIFFSetField(tiff, TIFFTAG_COMPRESSION, compression);
		    TIFFSetField(tiff, TIFFTAG_PHOTOMETRIC, (out_spp > 2) ? PHOTOMETRIC_RGB : PHOTOMETRIC_MINISBLACK);
		    buf = _TIFFmalloc(width * height * out_spp);
		    data8 = (unsigned char*) buf;
		    if (out_spp == 1)
			for (i = 0; i < n; i++) /* G */
			    data8[i] = nd[i] & 255;
		    else if (out_spp == 2) {
			if (((const char*)&i)[0] == 1) /* little-endian */
			    for (i = 0; i < n; i++) { /* GA */
				*(data8++) = nd[i] & 255;
				*(data8++) = (nd[i] >> 24) & 255;
			    }
			else /* big-endian */
			    for (i = 0; i < n; i++) { /* GA */
				*(data8++) = (nd[i] >> 24) & 255;
				*(data8++) = nd[i] & 255;
			    }
		    } else if (out_spp == 3) {
			if (((const char*)&i)[0] == 1) /* little-endian */
			    for (i = 0; i < n; i++) { /* RGB */
				*(data8++) = nd[i] & 255;
				*(data8++) = (nd[i] >> 8) & 255;
				*(data8++) = (nd[i] >> 16) & 255;
			    }
			else /* big-endian */
			    for (i = 0; i < n; i++) { /* RGB */
				*(data8++) = (nd[i] >> 16) & 255;
				*(data8++) = (nd[i] >> 8) & 255;
				*(data8++) = nd[i] & 255;
			    }
		    }
		    TIFFWriteEncodedStrip(tiff, 0, buf, width * height * out_spp);
		    _TIFFfree(buf);
		}
	    }
	    if (!reduce) {
		TIFFSetField(tiff, TIFFTAG_SAMPLESPERPIXEL, 4);
		TIFFSetField(tiff, TIFFTAG_ROWSPERSTRIP, height);
		TIFFSetField(tiff, TIFFTAG_COMPRESSION, compression);
		TIFFSetField(tiff, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);
		TIFFWriteEncodedStrip(tiff, 0, INTEGER(image), width * height * 4);
	    }
	} else {
	    uint32_t x, y, pl;
	    tdata_t buf;
	    unsigned char *data8;
	    unsigned short *data16;
	    unsigned int *data32;
	    double *ra = REAL(image);
	    uint32_t i, N = LENGTH(image);
	    for (i = 0; i < N; i++) /* do a pre-flight check */
		if (ra[i] < 0.0 || ra[i] > 1.0) {
		    Rf_warning("The input contains values outside the [0, 1] range - storage of such values is undefined");
		    break;
		}
	    TIFFSetField(tiff, TIFFTAG_BITSPERSAMPLE, bps);
	    TIFFSetField(tiff, TIFFTAG_SAMPLESPERPIXEL, planes);
	    TIFFSetField(tiff, TIFFTAG_ROWSPERSTRIP, height);
	    TIFFSetField(tiff, TIFFTAG_COMPRESSION, compression);
	    TIFFSetField(tiff, TIFFTAG_PHOTOMETRIC, (planes > 2) ? PHOTOMETRIC_RGB : PHOTOMETRIC_MINISBLACK);
	    buf = _TIFFmalloc(width * height * planes * (bps / 8));
	    data8 = (unsigned char*) buf;
	    data16 = (unsigned short*) buf;
	    data32 = (unsigned int*) buf;
	    if (!buf)
		Rf_error("cannot allocate output image buffer");
	    if (bps == 8)
		for (y = 0; y < height; y++)
		    for (x = 0; x < width; x++)
			for (pl = 0; pl < planes; pl++)
			    data8[(x + y * width) * planes + pl] = (unsigned char) (ra[y + x * height + pl * width * height] * 255.0);
	    else if (bps == 16)
		for (y = 0; y < height; y++)
		    for (x = 0; x < width; x++)
			for (pl = 0; pl < planes; pl++)
			    data16[(x + y * width) * planes + pl] = (unsigned short) (ra[y + x * height + pl * width * height] * 65535.0);
	    else if (bps == 32)
		for (y = 0; y < height; y++)
		    for (x = 0; x < width; x++)
			for (pl = 0; pl < planes; pl++)
			    data32[(x + y * width) * planes + pl] = (unsigned int) (ra[y + x * height + pl * width * height] * 4294967295.0);
	    TIFFWriteEncodedStrip(tiff, 0, buf, width * height * planes * (bps / 8));
	    _TIFFfree(buf);
	}

	if (img_list && img_index < n_img)
	    TIFFWriteDirectory(tiff);
	else break;
    }
    if (!rj.f) {
	SEXP res;
	TIFFFlush(tiff);
	res = allocVector(RAWSXP, rj.len);
#if TIFF_DEBUG
	Rprintf("convert to raw %d bytes (ptr=%d, alloc=%d)\n", rj.len, rj.ptr, rj.alloc);
#endif
	memcpy(RAW(res), rj.data, rj.len);
	TIFFClose(tiff);
	return res;
    }
    TIFFClose(tiff);
    return ScalarInteger(n_img);
}
