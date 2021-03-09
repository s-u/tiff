readTIFF <- function(source, native=FALSE, all=FALSE, convert=FALSE, info=FALSE, indexed=FALSE, as.is=FALSE,
                     payload=TRUE) {
    if (payload) .Call(read_tiff,
          if (is.raw(source)) source else path.expand(source), native,
          if (is.numeric(all)) as.integer(all) else all, convert, info, indexed, as.is, TRUE)
    else { ## for payload=FALSE we have to extract the info from the attributes
       x <- .Call(read_tiff,
       		  if (is.raw(source)) source else path.expand(source), FALSE,
		  if (is.numeric(all)) as.integer(all) else all, FALSE, TRUE, FALSE, FALSE, FALSE)
       if (is.integer(x))
          attributes(x)
       else {
          x <- sapply(x, attributes)
	  if (is.matrix(x)) t(x) else x
       }
    }
}
