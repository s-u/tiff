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
          compression = c("LZW", "none", "PackBits", "RLE", "JPEG", "deflate"))
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
}
%\details{
%}
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
\note{
  Currently \code{writeTIFF} always uses the same number of planes as
  there are planes in the input image. For native images it is always
  four. Consequently, color maps are not used. The output always uses
  contiguous planar configuration (baseline TIFF). The output is tagged
  with a photometric tag of either RGB (3 or 4 planes) or zero-is-black
  (1 or 2 planes). If \code{what} is a list then the TIFF output will be
  a directory of the corresponding number of images.
}
\seealso{
  \code{\link{readTIFF}}
}
%\examples{
%}
\keyword{IO}
