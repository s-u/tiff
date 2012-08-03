readTIFF <- function(source, native=FALSE, all=FALSE, convert=FALSE, info=FALSE)
  .Call("read_tiff", if (is.raw(source)) source else path.expand(source), native, all, convert, info, PACKAGE="tiff")
