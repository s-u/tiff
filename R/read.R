readTIFF <- function(source, native=FALSE, all=FALSE, convert=FALSE, info=FALSE, indexed=FALSE, as.is=FALSE)
  .Call("read_tiff", if (is.raw(source)) source else path.expand(source), native, all, convert, info, indexed, as.is, PACKAGE="tiff")

readTIFFDirectory <- function(source, all=FALSE) {
  raw <- .Call("read_tiff_directory",
        if (is.raw(source)) source else path.expand(source), all,
        PACKAGE="tiff")
  if (all) Map(attributes, raw) else attributes(raw)
}
