# tiff

[![CRAN](https://rforge.net/do/cransvg/tiff)](https://cran.r-project.org/package=tiff)
[![RForge](https://rforge.net/do/versvg/tiff)](https://RForge.net/tiff)
[![tiff check](https://github.com/s-u/tiff/actions/workflows/check.yml/badge.svg)](https://github.com/s-u/tiff/actions/workflows/check.yml)

__tiff__ R package supports TIFF (Tag Image File Format) image format. [readTIFF](https://rforge.net/doc/packages/tiff/readTIFF.html) reads one of more images from a TIFF file, [writeTIFF](https://rforge.net/doc/packages/tiff/writeTIFF.html) writes image data into a TIFF file. See the corresponsing [R documentation](https://rforge.net/doc/packages/tiff/00Index.html).

See also [tiff on RForge.net](https://rforge.net/tiff)

## Installation

On Windows and macOS you can install `tiff` directly from CRAN using

```r
install.packages("tiff")
```

On Linux, you need `libtiff` library and corresponding development files, e.g. on Debian/Ubuntu that is `libtiff-dev`, as well as all tools necessary to build R packages. Once you have it all, then you can use the same method as above.
