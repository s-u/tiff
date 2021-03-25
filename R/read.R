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
           as.data.frame(attributes(x), stringsAsFactors=FALSE)
       else {
           ## fetch tag from each image
           x <- lapply(x, attributes)
           ## construct a union of all tags
           u <- unique(unlist(lapply(x, names)))
           ## match tags from each image using NA for missing
           d <- do.call("rbind.data.frame", lapply(x, function(o) {
               o <- c(o, list(`NA`=NA))
               d <- o[match(u, names(o), length(o))]
               names(d)=u
               d
           }))
           ## clear row names so they are sequential
           row.names(d) <- NULL
           d
       }
    }
}
