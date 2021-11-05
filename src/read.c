#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "common.h"

#include <Rinternals.h>

/* avoid protection issues with setAttrib where new symbols may trigger GC problems */
static void setAttr(SEXP x, const char *name, SEXP val) {
    PROTECT(val);
    setAttrib(x, Rf_install(name), val);
    UNPROTECT(1);
}

/* add information attributes according to the TIFF tags.
   Only a somewhat random set (albeit mostly baseline) is supported */
static void TIFF_add_info(TIFF *tiff, SEXP res) {
    uint32_t i32;
    uint16_t i16;
    float f;
    char *c = 0;

    if (TIFFGetField(tiff, TIFFTAG_IMAGEWIDTH, &i32))
	setAttr(res, "width", ScalarInteger(i32));
    if (TIFFGetField(tiff, TIFFTAG_IMAGELENGTH, &i32))
	setAttr(res, "length", ScalarInteger(i32));
    if (TIFFGetField(tiff, TIFFTAG_IMAGEDEPTH, &i32))
	setAttr(res, "depth", ScalarInteger(i32));
    if (TIFFGetField(tiff, TIFFTAG_BITSPERSAMPLE, &i16))
	setAttr(res, "bits.per.sample", ScalarInteger(i16));
    if (TIFFGetField(tiff, TIFFTAG_SAMPLESPERPIXEL, &i16))
	setAttr(res, "samples.per.pixel", ScalarInteger(i16));
    if (TIFFGetField(tiff, TIFFTAG_SAMPLEFORMAT, &i16)) {
	char uv[24];
	const char *name = 0;
	switch (i16) {
	case 1: name = "uint"; break;
	case 2: name = "int"; break;
	case 3: name = "float"; break;
	case 4: name = "undefined"; break;
	case 5: name = "complex int"; break;
	case 6: name = "complex float"; break;
	default:
	    snprintf(uv, sizeof(uv), "unknown (%d)", i16);
	    name = uv;
	}
	setAttr(res, "sample.format", mkString(name));
	
    }
    if (TIFFGetField(tiff, TIFFTAG_PLANARCONFIG, &i16)) {
	if (i16 == PLANARCONFIG_CONTIG)
	    setAttr(res, "planar.config", mkString("contiguous"));
	else if (i16 == PLANARCONFIG_SEPARATE)
	    setAttr(res, "planar.config", mkString("separate"));
	else {
	    char uv[24];
	    snprintf(uv, sizeof(uv), "unknown (%d)", i16);
	    setAttr(res, "planar.config", mkString(uv));
	}
    }

    if (TIFFGetField(tiff, TIFFTAG_ROWSPERSTRIP, &i32))
        setAttr(res, "rows.per.strip", ScalarInteger(i32));

    if (TIFFGetField(tiff, TIFFTAG_TILEWIDTH, &i32)) {
        setAttr(res, "tile.width", ScalarInteger(i32));
        TIFFGetField(tiff, TIFFTAG_TILELENGTH, &i32);
        setAttr(res, "tile.length", ScalarInteger(i32));
    }

    if (TIFFGetField(tiff, TIFFTAG_COMPRESSION, &i16)) {
	char uv[24];
	const char *name = 0;
	switch (i16) {
	case 1: name = "none"; break;
	case 2: name = "CCITT RLE"; break;
	case 32773: name = "PackBits"; break;
	case 3: name = "CCITT Group 3 fax"; break;
	case 4: name = "CCITT Group 4 fax"; break;
	case 5: name = "LZW"; break;
	case 6: name = "old JPEG"; break;
	case 7: name = "JPEG"; break;
	case 8: name = "deflate"; break;
	case 9: name = "JBIG b/w"; break;
	case 10: name = "JBIG color"; break;
	default:
	    snprintf(uv, sizeof(uv), "unknown (%d)", i16);
	    name = uv;
	}
	setAttr(res, "compression", mkString(name));
    }
    if (TIFFGetField(tiff, TIFFTAG_THRESHHOLDING, &i16))
	setAttr(res, "threshholding", ScalarInteger(i16));
    if (TIFFGetField(tiff, TIFFTAG_XRESOLUTION, &f))
	setAttr(res, "x.resolution", ScalarReal(f));
    if (TIFFGetField(tiff, TIFFTAG_YRESOLUTION, &f))
	setAttr(res, "y.resolution", ScalarReal(f));
    if (TIFFGetField(tiff, TIFFTAG_XPOSITION, &f))
	setAttr(res, "x.position", ScalarReal(f));
    if (TIFFGetField(tiff, TIFFTAG_YPOSITION, &f))
	setAttr(res, "y.position", ScalarReal(f));
    if (TIFFGetField(tiff, TIFFTAG_RESOLUTIONUNIT, &i16)) {
	const char *name = "unknown";
	switch (i16) {
	case 1: name = "none"; break;
	case 2: name = "inch"; break;
	case 3: name = "cm"; break;
	}	    
	setAttr(res, "resolution.unit", mkString(name));
    }
#ifdef TIFFTAG_INDEXED /* very recent in libtiff even though it's an old tag */
    if (TIFFGetField(tiff, TIFFTAG_INDEXED, &i16))
	setAttr(res, "indexed", ScalarLogical(i16));
#endif
    if (TIFFGetField(tiff, TIFFTAG_ORIENTATION, &i16)) {
	const char *name = "<invalid>";
	switch (i16) {
	case 1: name = "top.left"; break;
	case 2: name = "top.right"; break;
	case 3: name = "bottom.right"; break;
	case 4: name = "bottom.left"; break;
	case 5: name = "left.top"; break;
	case 6: name = "right.top"; break;
	case 7: name = "right.bottom"; break;
	case 8: name = "left.bottom"; break;
	}
	setAttr(res, "orientation", mkString(name));
    }
    if (TIFFGetField(tiff, TIFFTAG_COPYRIGHT, &c) && c)
	setAttr(res, "copyright", mkString(c));
    if (TIFFGetField(tiff, TIFFTAG_ARTIST, &c) && c)
	setAttr(res, "artist", mkString(c));
    if (TIFFGetField(tiff, TIFFTAG_DOCUMENTNAME, &c) && c)
	setAttr(res, "document.name", mkString(c));
    if (TIFFGetField(tiff, TIFFTAG_DATETIME, &c) && c)
	setAttr(res, "date.time", mkString(c));
    if (TIFFGetField(tiff, TIFFTAG_IMAGEDESCRIPTION, &c) && c)
	setAttr(res, "description", mkString(c));
    if (TIFFGetField(tiff, TIFFTAG_SOFTWARE, &c) && c)
	setAttr(res, "software", mkString(c));
    if (TIFFGetField(tiff, TIFFTAG_PHOTOMETRIC, &i16)) {
	char uv[24];
	const char *name = 0;
	switch (i16) {
	case 0: name = "white is zero"; break;
	case 1: name = "black is zero"; break;
	case 2: name = "RGB"; break;
	case 3: name = "palette"; break;
	case 4: name = "mask"; break;
	case 5: name = "separated"; break;
	case 6: name = "YCbCr"; break;
	case 8: name = "CIELAB"; break;
	case 9: name = "ICCLab"; break;
	case 10: name = "ITULab"; break;
	default:
	    snprintf(uv, sizeof(uv), "unknown (%d)", i16);
	    name = uv;
	}
	setAttr(res, "color.space", mkString(name));
    }
}

SEXP read_tiff(SEXP sFn, SEXP sNative, SEXP sAll, SEXP sConvert, SEXP sInfo, SEXP sIndexed, SEXP sOriginal,
	       SEXP sPayload) {
    SEXP res = R_NilValue, multi_res = R_NilValue, multi_tail = R_NilValue, dim = R_NilValue;
    const char *fn;
    int native = asInteger(sNative), all = (isLogical(sAll) && asInteger(sAll) > 0), n_img = 0,
	convert = (asInteger(sConvert) == 1), add_info = (asInteger(sInfo) == 1),
	indexed = (asInteger(sIndexed) == 1), original = (asInteger(sOriginal) == 1),
	info_only = (asInteger(sPayload) == 0);
    tiff_job_t rj;
    TIFF *tiff;
    FILE *f;
    int *pick = (isInteger(sAll) ? INTEGER(sAll) : 0);
    int picks = pick ? LENGTH(sAll) : 0;
    SEXP pick_res = pick ? PROTECT(allocVector(VECSXP, picks)) : 0;

    /* make sure people don't use vector logicals - they must use which() if that's what they want */
    if (!((isLogical(sAll) && LENGTH(sAll) == 1) || isInteger(sAll)))
	Rf_error("`all' must be either a scalar logical (TRUE/FALSE) or an integer vector");

    if (indexed && (convert || native))
	Rf_error("indexed and native/convert cannot both be TRUE as they are mutually exclusive");
    
    if (TYPEOF(sFn) == RAWSXP) {
	rj.data = (char*) RAW(sFn);
	rj.len = LENGTH(sFn);
	rj.alloc = rj.ptr = 0;
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

    int cur_dir = 0; /* 1-based image number */
    int nprot = 0;
    while (1) { /* loop over separate image in a directory if desired */
	int pick_index = -1; /* not picked */
	cur_dir++;

	/* If sAll is a numeric vector, only read images referenced in it */
	if (pick) {
	    /* We don't use match() as it re-builds the hash table on every call. */
	    int i = 0;
	    while (i < picks) {
		if (pick[i] == cur_dir) {
		    pick_index = i;
		    break;
		}
		i++;
	    }

	    if (pick_index == -1) { /* No match */
		if (TIFFReadDirectory(tiff)) /* skip */
		    continue;
		else
		    break;
	    }
	}


	if (info_only) {
	    /* dummy result object */
	    res = PROTECT(allocVector(INTSXP, 0));
	    TIFF_add_info(tiff, res);

	    if (isLogical(sAll) && asInteger(sAll) == 0) {
		UNPROTECT(1);
		TIFFClose(tiff);
		return res;
	    }
	    n_img++;
	    if (pick_index >= 0)
		SET_VECTOR_ELT(pick_res, pick_index, res);
	    else {
		if (multi_res == R_NilValue) {
		    multi_tail = multi_res = CONS(res, R_NilValue);
		    PROTECT(multi_res);
		    nprot++;
		} else {
		    SEXP q = CONS(res, R_NilValue);
		    SETCDR(multi_tail, q);
		    multi_tail = q;
		}
	    }
	    UNPROTECT(1);
	    if (!TIFFReadDirectory(tiff))
		break;
	    continue;
	}

	uint32_t imageWidth = 0, imageLength = 0, imageDepth;
	uint32_t tileWidth, tileLength;
	uint32_t x, y;
	uint16_t config, bps = 8, spp = 1, sformat = 1, out_spp;
	tdata_t buf;
	double *ra = 0;
	uint16_t *colormap[3] = {0, 0, 0};
	int is_float = 0;

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
	out_spp = spp;
	TIFFGetField(tiff, TIFFTAG_COLORMAP, colormap, colormap + 1, colormap + 2);
	if (TIFFGetField(tiff, TIFFTAG_SAMPLEFORMAT, &sformat) && sformat == SAMPLEFORMAT_IEEEFP) is_float = 1;
	if (spp == 1 && !indexed) { /* modify out_spp for colormaps */
	    if (colormap[2]) out_spp = 3;
	    else if (colormap[1]) out_spp = 2;
	}
#ifdef TIFF_DEBUG
	Rprintf("image %d x %d x %d, tiles %d x %d, bps = %d, spp = %d (output %d), config = %d, colormap = %s,\n",
		imageWidth, imageLength, imageDepth, tileWidth, tileLength, bps, spp, out_spp, config, colormap[0] ? "yes" : "no");
	Rprintf("      float = %d\n", is_float);
#endif
	
	if (native || convert) {
	    /* use built-in RGBA conversion - fortunately, libtiff uses exactly
	       the same RGBA representation as R ... *but* flipped y coordinate :( */
	    SEXP tmp = R_NilValue;
	    /* FIXME: TIFF handle leak in case this fails */
	    if (convert)
		PROTECT(tmp = allocVector(REALSXP, imageWidth * imageLength * out_spp));
	    res = PROTECT(allocVector(INTSXP, imageWidth * imageLength));
	    TIFFReadRGBAImage(tiff, imageWidth, imageLength, (uint32_t*) INTEGER(res), 0);

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
		uint16_t s;
		uint32_t *data = (uint32_t*) INTEGER(res);
		ra = REAL(tmp);
		for (x = 0; x < imageWidth; x++)
		    for (y = 0; y < imageLength; y++) {
			if (out_spp == 1) { /* single plane (gray) just take R */
			    ra[imageLength * x + y] = ((double)(data[x + y * imageWidth] & 255)) / 255.0;
			} else if (out_spp == 2 /* G+A */) { /* this is a bit odd as we need to copy R and A */
			    ra[imageLength * x + y] = ((double)(data[x + y * imageWidth] & 255)) / 255.0;
			    ra[imageWidth * imageLength + imageLength * x + y] = ((double)((data[x + y * imageWidth] >> 16) & 255)) / 255.0;
			} else /* 3-4 are simply sequential copies */
			    for (s = 0; s < out_spp; s++)
				ra[(imageLength * imageWidth * s) +
				   imageLength * x + y] =
				    ((double) ((data[x + y * imageWidth] >> (s * 8)) & 255)) / 255.0;
		    }
		UNPROTECT(1); /* res */
		res = tmp;
		dim = allocVector(INTSXP, (out_spp > 1) ? 3 : 2);
		INTEGER(dim)[0] = imageLength;
		INTEGER(dim)[1] = imageWidth;
		if (out_spp > 1)
		    INTEGER(dim)[2] = out_spp;
		setAttrib(res, R_DimSymbol, dim);
		if (add_info)
		    TIFF_add_info(tiff, res);
		UNPROTECT(1);
	    } else {
		SEXP R_ChannelsSymbol = Rf_install("channels");
		dim = allocVector(INTSXP, 2);
		INTEGER(dim)[0] = imageLength;
		INTEGER(dim)[1] = imageWidth;
		setAttrib(res, R_DimSymbol, dim);
		setAttrib(res, R_ClassSymbol, mkString("nativeRaster"));
		setAttrib(res, R_ChannelsSymbol, ScalarInteger(out_spp));
		if (add_info)
		    TIFF_add_info(tiff, res);
		UNPROTECT(1);
	    }
	    if (!all && !picks) {
		TIFFClose(tiff);
		return res;
	    }
	    n_img++;
	    if (pick_index >= 0)
		SET_VECTOR_ELT(pick_res, pick_index, res);
	    else {
		if (multi_res == R_NilValue) {
		    multi_tail = multi_res = CONS(res, R_NilValue);
		    PROTECT(multi_res);
		    nprot++;
		} else {
		    SEXP q = CONS(res, R_NilValue);
		    SETCDR(multi_tail, q);
		    multi_tail = q;
		}
	    }
	    if (!TIFFReadDirectory(tiff))
		break;
	    continue;
	} /* end native || convert */

	if (bps != 8 && bps != 16 && bps != 32 && ( bps != 12 || spp != 1 )) {
	    TIFFClose(tiff);
	    Rf_error("image has %d bits/sample which is unsupported in direct mode - use native=TRUE or convert=TRUE", bps);
	}

	if (original && is_float) {
	    TIFFClose(tiff);
	    Rf_error("as.is=TRUE is not supported for floating point images");
	}

	if (sformat == SAMPLEFORMAT_INT && !original)
	    Rf_warning("tiff package currently only supports unsigned integer or float sample formats in direct mode, but the image contains signed integer format - it will be treated as unsigned (use as.is=TRUE, native=TRUE or convert=TRUE depending on your intent)");

	/* FIXME: TIFF handle leak in case this fails */
	res = allocVector((spp == 1 && (original || (indexed && colormap[0]))) ? INTSXP : REALSXP, imageWidth * imageLength * out_spp);
	if (!(spp == 1 && (original || (indexed && colormap[0])))) ra = REAL(res);
	
#define DE12A(v) ((((unsigned int) v[0]) << 4) | (((unsigned int) v[1]) >> 4))
#define DE12B(v) (((((unsigned int) v[1]) & 0x0f) << 8) | ((unsigned int) v[2]))

	if (tileWidth == 0) {
	    tstrip_t strip;
	    tsize_t plane_offset = 0;
	    x = 0; y = 0; 
	    buf = _TIFFmalloc(TIFFStripSize(tiff));
#ifdef TIFF_DEBUG
	    Rprintf(" - %d x %d strips\n", TIFFNumberOfStrips(tiff), TIFFStripSize(tiff));
#endif
	    for (strip = 0; strip < TIFFNumberOfStrips(tiff); strip++) {
		tsize_t n = TIFFReadEncodedStrip(tiff, strip, buf, (tsize_t) -1);
		if (spp == 1) { /* config doesn't matter for spp == 1 */
		    if (colormap[0] && !indexed) {
			tsize_t i, step = bps / 8;
			int *ia = original ? INTEGER(res) : 0;
			if (bps == 12) step += 2;
			for (i = 0; i < n; i += step) {
			    unsigned int ci = 0, ci2 = 0;
			    const unsigned char *v = (const unsigned char*) buf + i;
			    if (bps == 8) ci = v[0];
			    else if (bps == 16) ci = ((const unsigned short int*)v)[0];
			    else if (bps == 32) ci = ((const unsigned int*)v)[0];
			    else if (bps == 12) {
				ci  = DE12A(v);
				ci2 = DE12B(v);
			    }
			    if (original) {
				ia[imageLength * x + y] = colormap[0][ci]; /* color maps are always 16-bit */
				/* FIXME: bps==12 is broken with color map lookup !!! */
				if (bps == 12)
				    ia[imageLength * ++x + y] = colormap[0][ci2];
				if (colormap[1]) {
				    ia[(imageLength * imageWidth) + imageLength * x + y] = colormap[1][ci];
				    if (colormap[2]) 
					ia[(2 * imageLength * imageWidth) + imageLength * x + y] = colormap[2][ci];
				}
			    } else {
				ra[imageLength * x + y] = ((double)colormap[0][ci]) / 65535.0; /* color maps are always 16-bit */
				if (bps == 12)
				    ra[imageLength * ++x + y] = ((double)colormap[0][ci2]) / 65535.0;
				if (colormap[1]) {
				    ra[(imageLength * imageWidth) + imageLength * x + y] = ((double)colormap[1][ci]) / 65535.0;
				    if (colormap[2]) 
					ra[(2 * imageLength * imageWidth) + imageLength * x + y] = ((double)colormap[2][ci]) / 65535.0;
				}
			    }
			    x++;
			    if (x >= imageWidth) {
				x -= imageWidth;
				y++;
			    }
			}
		    } else if (colormap[0] || original) { /* indexed requested */
			tsize_t i, step = bps / 8;
			int *ia = INTEGER(res), ix_base = original ? 0 : 1;
			if (bps == 12) step = 3;
			for (i = 0; i < n; i += step) {
			    int val = NA_INTEGER;
			    const unsigned char *v = (const unsigned char*) buf + i;
			    if (bps == 8) val = ix_base + v[0];
			    else if (bps == 16) val = ix_base + ((const unsigned short int*)v)[0];
			    else if (bps == 32) val = ix_base + ((const unsigned int*)v)[0];
			    else if (bps == 12) {
				ia[imageLength * x++ + y] = ix_base + DE12A(v);
				val = ix_base + DE12B(v);
			    }
			    ia[imageLength * x++ + y] = val;
			    if (x >= imageWidth) {
				x -= imageWidth;
				y++;
			    }
			}
		    } else { /* direct gray */
			tsize_t i, step = bps / 8;
			if (bps == 12) step = 3;
			for (i = 0; i < n; i += step) {
			    double val = NA_REAL;
			    const unsigned char *v = (const unsigned char*) buf + i;
			    if (bps == 8) val = ((double) v[0]) / 255.0;
			    else if (bps == 16) val = ((double) ((const unsigned short int*)v)[0]) / 65535.0;
			    else if (bps == 32) {
				if (is_float) val = (double) ((const float*)v)[0];
				else val = ((double) ((const unsigned int*)v)[0]) / 4294967296.0;
			    }
			    if (bps == 12) {
				ra[imageLength * x++ + y] = ((double) DE12A(v)) / 4096.0;
				val = ((double) DE12B(v)) / 4096.0;
			    }
			    ra[imageLength * x + y] = val;
			    x++;
			    if (x >= imageWidth) {
				x -= imageWidth;
				y++;
			    }
			}
		    }
		} else if (config == PLANARCONFIG_CONTIG) { /* interlaced */
		    tsize_t i, j, step = spp * bps / 8;
		    for (i = 0; i < n; i += step) {
			const unsigned char *v = (const unsigned char*) buf + i;
			if (bps == 8) {
			    for (j = 0; j < spp; j++)
				ra[(imageLength * imageWidth * j) + imageLength * x + y] = ((double) v[j]) / 255.0;
			} else if (bps == 16) {
			    for (j = 0; j < spp; j++)
				ra[(imageLength * imageWidth * j) + imageLength * x + y] = ((double) ((const unsigned short int*)v)[j]) / 65535.0;
			} else if (bps == 32 && !is_float) {
			    for (j = 0; j < spp; j++)
				ra[(imageLength * imageWidth * j) + imageLength * x + y] = ((double) ((const unsigned int*)v)[j]) / 4294967296.0;
			} else if (bps == 32 && is_float) {
			    for (j = 0; j < spp; j++)
				ra[(imageLength * imageWidth * j) + imageLength * x + y] = (double) ((const float*)v)[j];
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
			else if (bps == 32 && !is_float)
			    ra[plane_offset + imageLength * x + y] = ((double) ((const unsigned int*)v)[0]) / 4294967296.0;
			else if (bps == 32 && is_float)
			    ra[plane_offset + imageLength * x + y] = (double) ((const float*)v)[0];
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
	} else { /* tiled image */
	    if (indexed || colormap[0] || bps == 12)
		Rf_error("Indexed and 12-bit tiled images are not supported.");
	    
	    if (spp > 1 && config != PLANARCONFIG_CONTIG)
		Rf_error("Planar format tiled images are not supported");

#ifdef TIFF_DEBUG
	    Rprintf(" - %d x %d tiles\n", TIFFNumberOfTiles(tiff), TIFFTileSize(tiff));
#endif
	    x = 0; y = 0;
	    buf = _TIFFmalloc(TIFFTileSize(tiff));

	    for (y = 0; y < imageLength; y += tileLength)
		for (x = 0; x < imageWidth; x += tileWidth) {
		    tsize_t n = TIFFReadTile(tiff, buf, x, y, 0 /*depth*/, 0 /*plane*/);
		    if (spp == 1) {
			/* config doesn't matter for spp == 1 */
			/* direct gray */
			tsize_t i, step = bps / 8;
			uint32_t xoff=0, yoff=0;
			for (i = 0; i < n; i += step) {
			    double val = NA_REAL;
			    const unsigned char *v = (const unsigned char*) buf + i;
			    if (bps == 8) val = ((double) v[0]) / 255.0;
			    else if (bps == 16) val = ((double) ((const unsigned short int*)v)[0]) / 65535.0;
			    else if (bps == 32) {
				if (is_float) val = (double) ((const float*)v)[0];
				else val = ((double) ((const unsigned int*)v)[0]) / 4294967296.0;
			    }
			    if (x + xoff < imageWidth && y + yoff < imageLength)
				ra[imageLength * (x + xoff) + y + yoff] = val;
			    xoff++;
			    if (xoff >= tileWidth) {
				xoff -= tileWidth;
				yoff++;
			    }
			}
		    } else if (config == PLANARCONFIG_CONTIG) { /* interlaced */
			tsize_t i, j, step = spp * bps / 8;
			uint32_t xoff=0, yoff=0;
			for (i = 0; i < n; i += step) {
			    const unsigned char *v = (const unsigned char*) buf + i;
			    if (x + xoff < imageWidth && y + yoff < imageLength) {
				if (bps == 8) {
				    for (j = 0; j < spp; j++)
					ra[(imageLength * imageWidth * j) + imageLength * (x + xoff) + y + yoff] = ((double) v[j]) / 255.0;
				} else if (bps == 16) {
				    for (j = 0; j < spp; j++)
					ra[(imageLength * imageWidth * j) + imageLength * (x + xoff) + y + yoff] = ((double) ((const unsigned short int*)v)[j]) / 65535.0;
				} else if (bps == 32 && !is_float) {
				    for (j = 0; j < spp; j++)
					ra[(imageLength * imageWidth * j) + imageLength * (x + xoff) + y + yoff] = ((double) ((const unsigned int*)v)[j]) / 4294967296.0;
				} else if (bps == 32 && is_float) {
				    for (j = 0; j < spp; j++)
					ra[(imageLength * imageWidth * j) + imageLength * (x + xoff) + y + yoff] = (double) ((const float*)v)[j];
				}
			    }
			    xoff++;
			    if (xoff >= tileWidth) {
				xoff -= tileWidth;
				yoff++;
			    }
			} /* for i */
		    } /* PLANARCONFIG_CONTIG */
		} /* for x */
	}

	_TIFFfree(buf);

	PROTECT(res);
	dim = allocVector(INTSXP, (out_spp > 1) ? 3 : 2);
	INTEGER(dim)[0] = imageLength;
	INTEGER(dim)[1] = imageWidth;
	if (out_spp > 1)
	    INTEGER(dim)[2] = out_spp;
	setAttrib(res, R_DimSymbol, dim);
	if (colormap[0] && TYPEOF(res) == INTSXP && indexed) {
	    int nc = 1 << bps, i;
	    SEXP cm = allocMatrix(REALSXP, 3, nc);
	    double *d = REAL(cm);
	    for (i = 0; i < nc; i++) {
		d[3 * i] = ((double) colormap[0][i]) / 65535.0;
		d[3 * i + 1] = ((double) colormap[1][i]) / 65535.0;
		d[3 * i + 2] = ((double) colormap[2][i]) / 65535.0;
	    }
	    setAttr(res, "color.map", cm);
	}
	if (add_info)
	    TIFF_add_info(tiff, res);
	UNPROTECT(1);
	
	if (isLogical(sAll) && asInteger(sAll) == 0) {
	    TIFFClose(tiff);
	    return res;
	}
	n_img++;
	if (pick_index >= 0)
	    SET_VECTOR_ELT(pick_res, pick_index, res);
	else {
	    if (multi_res == R_NilValue) {
		multi_tail = multi_res = CONS(res, R_NilValue);
		PROTECT(multi_res);
		nprot++;
	    } else {
		SEXP q = CONS(res, R_NilValue);
		SETCDR(multi_tail, q);
		multi_tail = q;
	    }
	}
	if (!TIFFReadDirectory(tiff))
	    break;
    }
    TIFFClose(tiff);
    /* if picked, we already have the result list */
    if (pick) {
	UNPROTECT(nprot + 1 /* pick_res is the +1 */);
	return pick_res;
    }

    /* convert LISTSXP into VECSXP */
    PROTECT(res = allocVector(VECSXP, n_img));
    nprot++;
    {
	int i = 0;
	while (multi_res != R_NilValue) {
	    SET_VECTOR_ELT(res, i, CAR(multi_res));
	    i++;
	    multi_res = CDR(multi_res);
	}
    }
    UNPROTECT(nprot);
    return res;
}
