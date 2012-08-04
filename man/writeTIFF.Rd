\name{writeTIFF}
\alias{writeTIFF}
\title{
  Write one or more bitmap images in TIFF format
}
\description{
  Writes images into a TIFF file or a raw vector representing such.
}
\usage{
writeTIFF(what, where, bits.per.sample = 8L,
          compression = c("LZW", "none", "PackBits", "RLE", "JPEG", "deflate"),
          reduce = TRUE)
}
\arguments{
  \item{what}{either an image or a list of images. An image is a real matrix
    or array of three dimensions, or an object of the class \code{"nativeRaster"}.}
  \item{where}{file name or a raw vector}
  \item{bits.per.sample}{number of bits per sample (numeric
    scalar). Supported values in this version are 8, 16, and 32.}
  \item{compression}{desired compression algorithm (string). Optionally,
    it can be specified as a numeric value corresponding to the
    compression TIFF tag, but it needs to be also supported by the
    underlying TIFF library}
  \item{reduce}{if \code{TRUE} then \code{writeTIFF} will attempt to
    reduce the number of planes in native rasters by analyzing the
    image to choose one of RGBA, RGB, GA or G formats, whichever uses
    the least planes without any loss. Otherwise the image is always
    saved with four planes (RGBA).}
}
\value{
  If \code{where} is a raw vector then the value is the raw vector
  containg the TIFF contents, otherwise a scalar integer specifying the
  number of images written in the file.
}
%\references{
%}
\author{
Simon Urbanek
}
\details{
  By default \code{writeTIFF} uses the same number of planes as
  there are planes in the input image. For native images it is always
  four unless \code{reduce = TRUE} is set (see above). Consequently,
  color maps are not used. The output always uses contiguous planar
  configuration (baseline TIFF). The output is tagged with a photometric
  tag of either RGB (3 or 4 planes) or zero-is-black (1 or 2 planes). If
  \code{what} is a list then the TIFF output will be a directory of the
  corresponding number of images (in TIFF speak - not to be confused
  with file directories).
}
\seealso{
  \code{\link{readTIFF}}
}
\examples{
img <- readTIFF(system.file("img", "Rlogo.tiff", package="tiff"))
# write without the alpha channel
tiff <- writeTIFF(img[,,-4], raw(0))
# read as native
i2 <- readTIFF(tiff, native=TRUE)
# write reduced - should be the same as tiff
t2 <- writeTIFF(i2, raw(0), reduce=TRUE)
}
\keyword{IO}
