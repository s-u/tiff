library(testthat)

# Smoke tests

# Check the attributes read from Rlogo.tiff
check_attributes <- function(attrs) {
    expected = list(
        width = 100,
        length = 76,
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
    path = system.file("img", "Rlogo.tiff", package="tiff")
    img <- readTIFF(path, info=TRUE)

    expect_equal(dim(img), c(76, 100, 4))
    attrs = attributes(img)
    check_attributes(attrs)
    
    img <- readTIFF(path, info=TRUE, all=TRUE)
    expect_equal(length(img), 1)
    expect_equal(dim(img[[1]]), c(76, 100, 4))
    attrs = attributes(img[[1]])
    check_attributes(attrs)
    
    img <- readTIFF(path, info=TRUE, all=1)
    expect_equal(length(img), 1)
    expect_equal(dim(img[[1]]), c(76, 100, 4))
    attrs = attributes(img[[1]])
    check_attributes(attrs)
    
    img <- readTIFF(path, info=TRUE, all=2)
    expect_equal(length(img), 0)
})

test_that("readTIFFDirectory works", {
    path = system.file("img", "Rlogo.tiff", package="tiff")
    attrs <- readTIFFDirectory(path)
    check_attributes(attrs)
    
    attrs <- readTIFFDirectory(path, all=TRUE)
    expect_equal(length(attrs), 1) # attrs is a list of lists now
    check_attributes(attrs[[1]])
    
    attrs <- readTIFFDirectory(path, all=1)
    expect_equal(length(attrs), 1) # attrs is a list of lists now
    check_attributes(attrs[[1]])
    
    attrs <- readTIFFDirectory(path, all=2)
    expect_equal(length(attrs), 0) # attrs is an empty now, there is no image 2
})
