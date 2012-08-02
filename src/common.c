#include "common.h"
#include <stdlib.h>
#include <string.h>

#include <Rinternals.h>

static int need_init = 1;

static char txtbuf[2048];

static TIFF *last_tiff; /* this to avoid leaks */

static void TIFFWarningHandler_(const char* module, const char* fmt, va_list ap) {
    /* we can't pass it directly since R has no vprintf entry point */
    vsnprintf(txtbuf, sizeof(txtbuf), fmt, ap);
    Rf_warning("%s: %s", module, txtbuf);
}

static void TIFFErrorHandler_(const char* module, const char* fmt, va_list ap) {
    /* we can't pass it directly since R has no vprintf entry point */
    vsnprintf(txtbuf, sizeof(txtbuf), fmt, ap);
    /* we have to close the TIFF that caused it as it will not
       come back -- recursive calls won't work under errors
       but that is hopefully unlikely/impossible */
    if (last_tiff)
	TIFFClose(last_tiff); /* this will also reset last_tiff */
    Rf_error("%s: %s", module, txtbuf);
}

static void init_tiff() {
    if (need_init) {
	TIFFSetWarningHandler(TIFFWarningHandler_);
	TIFFSetErrorHandler(TIFFErrorHandler_);
	need_init = 0;
    }
}

static tsize_t TIFFReadProc_(thandle_t usr, tdata_t buf, tsize_t length) {
    tiff_job_t *rj = (tiff_job_t*) usr;
    tsize_t to_read = length;
    if (rj->f)
	return fread(buf, 1, to_read, rj->f);
    if (to_read > (rj->len - rj->ptr))
	to_read = (rj->len - rj->ptr);
    if (to_read > 0) {
	memcpy(buf, rj->data + rj->ptr, to_read);
	rj->ptr += to_read;
    }
    return to_read;
}

static tsize_t TIFFWriteProc_(thandle_t usr, tdata_t buf, tsize_t length) {
    Rf_error("TIFFWriteProc called in readTIFF");
    return -1;
}

static toff_t  TIFFSeekProc_(thandle_t usr, toff_t offset, int whence) {
    tiff_job_t *rj = (tiff_job_t*) usr;
    if (rj->f) {
	int e = fseeko(rj->f, offset, whence);
	if (e != 0) {
	    Rf_warning("fseek failed on a file in TIFFSeekProc");
	    return -1;
	}
	return ftello(rj->f);
    }
    if (whence == SEEK_CUR)
	offset += rj->ptr;
    else if (whence == SEEK_END)
	offset += rj->len;
    else if (whence != SEEK_SET) {
	Rf_warning("invalid `whence' argument to TIFFSeekProc callback called by libtiff");
	return -1;
    }
    if (offset < 0 || offset > rj->len) {
	Rf_warning("libtiff attempted to seek beyond the data end");
	return -1;
    }
    return (toff_t) (rj->ptr = offset);
}

static int     TIFFCloseProc_(thandle_t usr) {
    tiff_job_t *rj = (tiff_job_t*) usr;
    if (rj->f)
	fclose(rj->f);
    last_tiff = 0;
    return 0;
}

static toff_t  TIFFSizeProc_(thandle_t usr) {
    tiff_job_t *rj = (tiff_job_t*) usr;
    if (rj->f) {
	off_t cur = ftello(rj->f), end;
	fseek(rj->f, 0, SEEK_END);
	end = ftello(rj->f);
	fseeko(rj->f, cur, SEEK_SET);
	return end;
    }
    return (toff_t) rj->len;
}

static int     TIFFMapFileProc_(thandle_t usr, tdata_t* map, toff_t* off) {
    Rf_warning("libtiff attempted to use TIFFMapFileProc on non-file which is unsupported");
    return -1;
}

static void    TIFFUnmapFileProc_(thandle_t usr, tdata_t map, toff_t off) {
    Rf_warning("libtiff attempted to use TIFFUnmapFileProc on non-file which is unsupported");
}

/* actual interface */
TIFF *TIFF_Open(const char *mode, tiff_job_t *rj) {
    if (need_init) init_tiff();
#if AGGRESSIVE_CLEANUP
    if (last_tiff)
	TIFFClose(last_tiff);
#endif
    return (last_tiff = 
	    TIFFClientOpen("pkg:tiff", mode, (thandle_t) rj, TIFFReadProc_, TIFFWriteProc_, TIFFSeekProc_,
			   TIFFCloseProc_, TIFFSizeProc_, TIFFMapFileProc_, TIFFUnmapFileProc_)
	    );
}
