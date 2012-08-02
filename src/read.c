#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"

#include <Rinternals.h>

SEXP read_tiff(SEXP sFn, SEXP sNative, SEXP sAll, SEXP sConvert) {
    SEXP res = R_NilValue, multi_res = R_NilValue, multi_tail = R_NilValue, dim;
    const char *fn;
    int native = asInteger(sNative), all = (asInteger(sAll) == 1), n_img = 0, convert = (asInteger(sConvert) == 1);
    tiff_job_t rj;
    TIFF *tiff;
    FILE *f;
    
    if (TYPEOF(sFn) == RAWSXP) {
	rj.data = (char*) RAW(sFn);
	rj.len = LENGTH(sFn);
	rj.ptr = 0;
	rj.f = f = 0;
    } else {
	if (TYPEOF(sFn) != STRSXP || LENGTH(sFn) < 1) Rf_error("invalid filename");
	fn = CHAR(STRING_ELT(sFn, 0));
	f = fopen(fn, "rb");
	if (!f) Rf_error("unable to open %s", fn);
	rj.f = f;
    }

    tiff = TIFF_Open("rmc", &rj); /* no mmap, no chopping */
    if (!tiff)
	Rf_error("Unable to open TIFF");

    while (1) { /* loop over separate image in a directory if desired */
	uint32 imageWidth = 0, imageLength = 0, imageDepth;
	uint32 tileWidth, tileLength;
	uint32 x, y;
	uint16 config, bps = 8, spp = 1;
	tdata_t buf;
	double *ra;

	TIFFGetField(tiff, TIFFTAG_IMAGEWIDTH, &imageWidth);
	TIFFGetField(tiff, TIFFTAG_IMAGELENGTH, &imageLength);
	if (!TIFFGetField(tiff, TIFFTAG_IMAGEDEPTH, &imageDepth)) imageDepth = 0;
	if (TIFFGetField(tiff, TIFFTAG_TILEWIDTH, &tileWidth))
	    TIFFGetField(tiff, TIFFTAG_TILELENGTH, &tileLength);
	else /* no tiles */
	    tileWidth = tileLength = 0;
	TIFFGetField(tiff, TIFFTAG_PLANARCONFIG, &config);
	TIFFGetField(tiff, TIFFTAG_BITSPERSAMPLE, &bps);
	TIFFGetField(tiff, TIFFTAG_SAMPLESPERPIXEL, &spp);
	Rprintf("image %d x %d x %d, tiles %d x %d, bps = %d, spp = %d, config = %d\n", imageWidth, imageLength, imageDepth, tileWidth, tileLength, bps, spp, config);
	
	if (native || convert) {
	    /* use built-in RGBA conversion - fortunately, libtiff uses exactly
	       the same RGBA representation as R ... *but* flipped y coordinate :( */
	    SEXP tmp = R_NilValue;
	    /* FIXME: TIFF handle leak in case this fails */
	    if (convert)
		PROTECT(tmp = allocVector(REALSXP, imageWidth * imageLength * spp));
	    res = allocVector(INTSXP, imageWidth * imageLength);

	    TIFFReadRGBAImage(tiff, imageWidth, imageLength, (uint32*) INTEGER(res), 0);
	    if (!all) TIFFClose(tiff);
	    PROTECT(res);
	    /* TIFF uses flipped y-axis, so we need to invert it .. argh ... */
	    if (imageLength > 1) {
		int *line = INTEGER(allocVector(INTSXP, imageWidth));
		int *src = INTEGER(res), *dst = INTEGER(res) + imageWidth * (imageLength - 1), ls = imageWidth * sizeof(int);
		int *el = src + imageWidth * (imageLength / 2);
		while (src < el) {
		    memcpy(line, src, ls);
		    memcpy(src, dst,  ls);
		    memcpy(dst, line, ls);
		    src += imageWidth;
		    dst -= imageWidth;
		}
	    }
	    if (convert) {
		uint16 s;
		uint32 *data = (uint32*) INTEGER(res);
		ra = REAL(tmp);
		for (x = 0; x < imageWidth; x++)
		    for (y = 0; y < imageLength; y++) {
			if (spp == 1) { /* single plane (gray) just take R */
			    ra[imageLength * x + y] = ((double)(data[x + y * imageWidth] & 255)) / 255.0;
			} else if (spp == 2 /* G+A */) { /* this is a bit odd as we need to copy R and A */
			    ra[imageLength * x + y] = ((double)(data[x + y * imageWidth] & 255)) / 255.0;
			    ra[imageWidth * imageLength + imageLength * x + y] = ((double)((data[x + y * imageWidth] >> 16) & 255)) / 255.0;
			} else /* 3-4 are simply sequential copies */
			    for (s = 0; s < spp; s++)
				ra[(imageLength * imageWidth * s) +
				   imageLength * x + y] =
				    ((double) ((data[x + y * imageWidth] >> (s * 8)) & 255)) / 255.0;
		    }
		UNPROTECT(1); /* res */
		res = tmp;
		dim = allocVector(INTSXP, (spp > 1) ? 3 : 2);
		INTEGER(dim)[0] = imageLength;
		INTEGER(dim)[1] = imageWidth;
		if (spp > 1)
		    INTEGER(dim)[2] = spp;
		setAttrib(res, R_DimSymbol, dim);
		UNPROTECT(1);
	    } else {
		dim = allocVector(INTSXP, 2);
		INTEGER(dim)[0] = imageLength;
		INTEGER(dim)[1] = imageWidth;
		setAttrib(res, R_DimSymbol, dim);
		setAttrib(res, R_ClassSymbol, mkString("nativeRaster"));
		setAttrib(res, install("channels"), ScalarInteger(spp));
		UNPROTECT(1);
	    }
	    if (!all)
		return res;
	    n_img++;
	    if (multi_res == R_NilValue) {
		multi_tail = multi_res = CONS(res, R_NilValue);
		PROTECT(multi_res);
	    } else {
		SEXP q = CONS(res, R_NilValue);
		SETCDR(multi_tail, q);
		multi_tail = q;
	    }
	    if (!TIFFReadDirectory(tiff))
		break;
	    continue;
	}
	
	if (bps != 8 && bps != 16 && bps != 32) {
	    TIFFClose(tiff);
	    Rf_error("image has %d bits/sample which is unsupported in direct mode - use native=TRUE or convert=TRUE", bps);
	}

	/* FIXME: TIFF handle leak in case this fails */
	res = allocVector(REALSXP, imageWidth * imageLength * spp);
	ra = REAL(res);
	
	if (tileWidth == 0) {
	    tstrip_t strip;
	    tsize_t plane_offset = 0;
	    x = 0; y = 0; 
	    buf = _TIFFmalloc(TIFFStripSize(tiff));
	    Rprintf(" - %d x %d strips\n", TIFFNumberOfStrips(tiff), TIFFStripSize(tiff));
	    for (strip = 0; strip < TIFFNumberOfStrips(tiff); strip++) {
		tsize_t n = TIFFReadEncodedStrip(tiff, strip, buf, (tsize_t) -1);
		if (config == PLANARCONFIG_CONTIG) { /* interlaced */
		    tsize_t i, j, step = spp * bps / 8;
		    for (i = 0; i < n; i += step) {
			const unsigned char *v = (const unsigned char*) buf + i;
			if (bps == 8) {
			    for (j = 0; j < spp; j++)
				ra[(imageLength * imageWidth * j) + imageLength * x + y] = ((double) v[j]) / 255.0;
			} else if (bps == 16) {
			    for (j = 0; j < spp; j++)
				ra[(imageLength * imageWidth * j) + imageLength * x + y] = ((double) ((const unsigned short int*)v)[j]) / 65535.0;
			} else if (bps == 32) {
			    for (j = 0; j < spp; j++)
				ra[(imageLength * imageWidth * j) + imageLength * x + y] = ((double) ((const unsigned int*)v)[j]) / 4294967296.0;
			}
			x++;
			if (x >= imageWidth) {
			    x -= imageWidth;
			    y++;
			}
		    }
		} else { /* separate */
		    tsize_t step = bps / 8, i;
		    for (i = 0; i < n; i += step) {
			const unsigned char *v = (const unsigned char*) buf + i;
			if (bps == 8)
			    ra[plane_offset + imageLength * x + y] = ((double) v[0]) / 255.0;
			else if (bps == 16) 
			    ra[plane_offset + imageLength * x + y] = ((double) ((const unsigned short int*)v)[0]) / 65535.0;
			else if (bps == 32)
			    ra[plane_offset + imageLength * x + y] = ((double) ((const unsigned int*)v)[0]) / 4294967296.0;
		    }
		    x++;
		    if (x >= imageWidth) {
			x -= imageWidth;
			y++;
			if (y >= imageLength) {
			    y -= imageLength;
			    plane_offset += imageWidth * imageLength;
			}
		    }
		}
	    }
	} else {
	    Rf_error("tile-based images are currently supported on with native=TRUE or convert=TRUE");
	    buf = _TIFFmalloc(TIFFTileSize(tiff));
	    
	    for (y = 0; y < imageLength; y += tileLength)
		for (x = 0; x < imageWidth; x += tileWidth)
		    TIFFReadTile(tiff, buf, x, y, 0 /*depth*/, 0 /*plane*/);
	}
	
	_TIFFfree(buf);

	if (!all)
	    TIFFClose(tiff);
	PROTECT(res);
	dim = allocVector(INTSXP, (spp > 1) ? 3 : 2);
	INTEGER(dim)[0] = imageLength;
	INTEGER(dim)[1] = imageWidth;
	if (spp > 1)
	    INTEGER(dim)[2] = spp;
	setAttrib(res, R_DimSymbol, dim);
	UNPROTECT(1);
	
	if (!all)
	    return res;
	n_img++;
	if (multi_res == R_NilValue) {
	    multi_tail = multi_res = CONS(res, R_NilValue);
	    PROTECT(multi_res);
	} else {
	    SEXP q = CONS(res, R_NilValue);
	    SETCDR(multi_tail, q);
	    multi_tail = q;
	}
	if (!TIFFReadDirectory(tiff))
	    break;
    }
    TIFFClose(tiff);
    /* convert LISTSXP into VECSXP */
    PROTECT(res = allocVector(VECSXP, n_img));
    {
	int i = 0;
	while (multi_res != R_NilValue) {
	    SET_VECTOR_ELT(res, i, CAR(multi_res));
	    i++;
	    multi_res = CDR(multi_res);
	}
    }
    UNPROTECT(2);
    return res;
}
