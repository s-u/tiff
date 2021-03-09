readTIFF <- function(source, native=FALSE, all=FALSE, convert=FALSE, info=FALSE, indexed=FALSE, as.is=FALSE)
    .Call(read_tiff,
          if (is.raw(source)) source else path.expand(source), native,
          if (is.numeric(all)) as.integer(all) else all, convert, info, indexed, as.is)
