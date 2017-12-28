\name{readTIFFDirectory}
\alias{readTIFFDirectory}
\title{
Read information from an image stored in the TIFF format
}
\description{
Reads informational tags from a TIFF image file/content into a list.
}
\usage{
readTIFFDirectory(source, all = FALSE)
}
\arguments{
  \item{source}{Either name of the file to read from or a raw vector
  representing the TIFF file content.}
\item{all}{TIFF files can contain more than one image. If \code{all =
    TRUE} then information about all images is returned in a list
    of lists. If \code{all} is a vector, it gives the (1-based) 
    indices of images to return. Otherwise only information about the first
  image is returned.}
}
\value{
A list of
informational values from the first image directory of \code{source}.

If \code{all} is \code{TRUE} or a vector of image indices,
then the result is a list of the above with
one or more elements..
}
\author{
Simon Urbanek
Kent Johnson
}
\seealso{
\code{\link{readTIFF}}
}
\examples{
# read directory information from a sample file (R logo)
readTIFFDirectory(system.file("img", "Rlogo.tiff", package="tiff"))
}
\keyword{IO}
