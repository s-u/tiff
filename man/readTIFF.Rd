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
         info = FALSE, indexed = FALSE, as.is = FALSE,
	 payload = TRUE)
}
\arguments{
  \item{source}{Either name of the file to read from or a raw vector
    representing the TIFF file content.}
  \item{native}{logical, determines the image representation - if
    \code{FALSE} (the default) then the result is an array, if
    \code{TRUE} then the result is a native raster representation
    (suitable for plotting).}
  \item{all}{logical scalar or integer vector. TIFF files can contain
    more than one image. If \code{all=TRUE} then all images are returned
    in a list of images. If \code{all} is a vector, it gives the
    (1-based) indices of images to return. Otherwise only the first
    image is returned.}
\item{convert}{logical, if \code{TRUE} then first convert the image into
  8-bit RGBA samples and then to an array, see below for details.}
\item{info}{logical, if set to \code{TRUE} then the resulting image(s)
  will also contain information from TIFF tags as attributes}
\item{indexed}{logical, if set to \code{TRUE} then indexed images will be
  returned in the indexed form, i.e., as a matrix of integer indices
  referencing into a color map which is returned in the
  \code{"color.map"} attribute. This flag cannot be combined with
  \code{convert} or \code{native} and has no effect on images that are
  not indexed.}
\item{as.is}{logical, if \code{TRUE} an attempt will be made to return
  the original integer values without re-scaling where possible}
\item{payload}{logical, if \code{FALSE} then only metadata about the
  image(s) is returned, but not the actual image. Implies
  \code{info=TRUE} and all image-related flags are ignored.}
}
\value{
If \code{native} is \code{FALSE} then an array of the dimensions height
x width x channels. If there is only one channel the result is a
matrix. The values are reals between 0 and 1 (except for 32-bit floating
point sample storage which are unscaled reals, and for indexed and
\code{as.is=TRUE} which are integers). If \code{native} is
\code{TRUE} then an object of the class \code{nativeRaster} is
returned instead. The latter cannot be easily computed on but is the
most efficient way to draw using \code{rasterImage}.

If \code{all} is \code{TRUE} or a vector of image indices,
then the result is a list of the above with
zero or more elements. If \code{all} is a vector of indices, the result
will have exactly the same length as \code{all}. If an index does not
appear in the file, the corresponding list entry will be \code{NULL}.

If \code{payload=FALSE} then the result is equivalent to
\code{info=TRUE} but without the image data and returned as a data frame.
If \code{all} is either \code{TRUE} or a vector then the result will
be a data frame with each row corresponding to one image.
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
\author{
  Simon Urbanek
  Kent Johnson
}
\note{
  Some non-standard formats such as 12-bit TIFFs are partially supported
  (there is no standard for packing order for TIFFs beoynd 8-bit so we
  assume big-endian packing similar to the default fill order and only
  support single channel or indexed).

  The \code{as.is=TRUE} option is experimental, cannot be used with
  \code{native} or \code{convert} and only works for integer storage
  TIFFs.
}
\seealso{
\code{\link{rasterImage}}, \code{\link{writeTIFF}}
}
\examples{
Rlogo <- system.file("img", "Rlogo.tiff", package="tiff")

# read a sample file (R logo)
img <- readTIFF(Rlogo)

# read it also in native format
img.n <- readTIFF(Rlogo, native=TRUE)

# and also in converted
img.c <- readTIFF(Rlogo, convert=TRUE)

# read all contained images
str(readTIFF(Rlogo, all=TRUE))

# pick some images
str(readTIFF(Rlogo, all=c(5, 1, 3)))

# only show information
str(readTIFF(Rlogo, payload=FALSE))

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
