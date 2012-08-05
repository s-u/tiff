\name{readTIFF}
\alias{readTIFF}
\title{
Read a bitmap image stored in the TIFF format
}
\description{
Reads an image from a TIFF file/content into a raster array.
}
\usage{
readTIFF(source, native = FALSE, all = FALSE, convert = FALSE,
         info = FALSE, indexed = FALSE)
}
\arguments{
  \item{source}{Either name of the file to read from or a raw vector
  representing the TIFF file content.}
  \item{native}{determines the image representation - if \code{FALSE}
  (the default) then the result is an array, if \code{TRUE} then the
  result is a native raster representation (suitable for plotting).}
\item{all}{TIFF files can contain more than one image, if \code{all =
    TRUE} then all images are returenin a list, otherwsie only the first
  image is returned.}
\item{convert}{first convert the image into 8-bit RGBA samples and then
  to an array, see below for details.}
\item{info}{if set to \code{TRUE} then the resulting image(s) will also
  contain information from TIFF tags as attributes}
\item{indexed}{if set to \code{TRUE} then indexed images will be
  returned in the indexed form, i.e., as a matrix of integer indices
  referencing into a color map which is returned in the
  \code{"color.map"} attribute. This flag cannot be combined with
  \code{convert} or \code{native} and has no effect on images that are
  not indexed.}
}
\value{
If \code{native} is \code{FALSE} then an array of the dimensions height
x width x channels. If there is only one channel the result is a
matrix. The values are reals between 0 and 1 (except for 32-bit floating
point sample storage which are reals as-is). If \code{native} is
\code{TRUE} then an object of the class \code{nativeRaster} is
returned instead. The latter cannot be easily computed on but is the
most efficient way to draw using \code{rasterImage}.

If \code{all} is \code{TRUE} then the result is a list of the above with
one or more elements..
}
\details{
Most common files decompress into RGB (3 channels), RGBA (4 channels),
Grayscale (1 channel) or GA (2 channels). Note that G and GA images
cannot be directly used in \code{\link{rasterImage}} unless
\code{native} is set to \code{TRUE} because \code{rasterImage} requires
RGB or RGBA format (\code{nativeRaster} is always 8-bit RGBA).

TIFF images can have a wide range of internal representations, but only
the most common in image processing are directly supported (8-bit, 16-bit
integer and 32-bit float samples). Other formats (color maps, sub-8-bit
images, etc.) are only supported via \code{convert=TRUE} which uses the
built-in facilities of the TIFF library to convert the image into RGBA
format with 8-bit samples (i.e. total of 32-bit per pixel) and then
store the relevant components from there into real arrays. This is the
same path as used by \code{native=TRUE} and so differs only in the
output value. Note that conversion may result in different values than
direct acccess as it is intended mainly for viewing and not computation.
}
%\references{
%}
%\author{
%}
%\note{
%}
\seealso{
\code{\link{rasterImage}}, \code{\link{writeTIFF}}
}
\examples{
# read a sample file (R logo)
img <- readTIFF(system.file("img", "Rlogo.tiff", package="tiff"))

# read it also in native format
img.n <- readTIFF(system.file("img", "Rlogo.tiff", package="tiff"), native=TRUE)

# and also in converted
img.c <- readTIFF(system.file("img", "Rlogo.tiff", package="tiff"), convert=TRUE)

# if your R supports it, we'll plot it
if (exists("rasterImage")) { # can plot only in R 2.11.0 and higher
  plot(1:2, type='n')

  if (names(dev.cur()) == "windows") {
    # windows device doesn't support semi-transparency so we'll need
    # to flatten the image
    transparent <- img[,,4] == 0
    img <- as.raster(img[,,1:3])
    img[transparent] <- NA

    # interpolate must be FALSE on Windows, otherwise R will
    # try to interpolate transparency and fail
    rasterImage(img, 1.2, 1.27, 1.8, 1.73, interpolate=FALSE)

  } else {
    # any reasonable device will be fine using alpha
    rasterImage(img, 1.2, 1.27, 1.8, 1.73)
    rasterImage(img.n, 1.5, 1.5, 1.9, 1.8)
  }
}
}
\keyword{IO}
