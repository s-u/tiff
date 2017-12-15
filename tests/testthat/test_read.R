library(testthat)

# Smoke tests

# Check the attributes read from Rlogo.tiff
check_attributes <- function(attrs) {
    expected = list(
        bits.per.sample = 8L, 
        samples.per.pixel = 4L, 
        sample.format = "uint", 
        planar.config = "contiguous", 
        compression = "LZW", 
        resolution.unit = "inch", 
        orientation = "top.left", 
        color.space = "RGB")

    expected_double = list(
        x.resolution = 300, 
        y.resolution = 300)    
    
    for (n in names(expected))
        expect_equal(attrs[[n]], expected[[n]], info=n)
    for (n in names(expected_double))
        expect_equal(attrs[[n]], expected_double[[n]], tolerance=0.001, info=n)
}

test_that("readTIFF works", {
    # Don't duplicate the examples...
    img <- readTIFF(system.file("img", "Rlogo.tiff", package="tiff"), info=TRUE)

    expect_equal(dim(img), c(76, 100, 4))
    
    attrs = attributes(img)
    check_attributes(attrs)
})

test_that("readTIFFDirectory works", {
    path = system.file("img", "Rlogo.tiff", package="tiff")
    attrs <- readTIFFDirectory(path)
    check_attributes(attrs)
    
    attrs <- readTIFFDirectory(path, all=TRUE)
    expect_equal(length(attrs), 1) # attrs is a list of lists now
    check_attributes(attrs[[1]])
})
