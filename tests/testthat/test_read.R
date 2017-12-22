library(testthat)

# Smoke tests

# Check the attributes read from Rlogo.tiff
check_attributes <- function(attrs) {
    expected <- list(
        width = 100,
        length = 76,
        bits.per.sample = 8L,
        samples.per.pixel = 4L,
        sample.format = "uint",
        planar.config = "contiguous",
        rows.per.strip = 76,
        compression = "LZW",
        resolution.unit = "inch",
        orientation = "top.left",
        color.space = "RGB")

    expected_double <- list(
        x.resolution = 300,
        y.resolution = 300)

    for (n in names(expected))
        expect_equal(attrs[[n]], expected[[n]], info=n)
    for (n in names(expected_double))
        expect_equal(attrs[[n]], expected_double[[n]], tolerance=0.001, info=n)
}

test_that("readTIFF works", {
    # Don't duplicate the examples...
    path <- system.file("img", "Rlogo.tiff", package="tiff")
    img <- readTIFF(path, info=TRUE)

    expect_equal(dim(img), c(76, 100, 4))
    attrs <- attributes(img)
    check_attributes(attrs)

    img <- readTIFF(path, info=TRUE, all=TRUE)
    expect_equal(length(img), 1)
    expect_equal(dim(img[[1]]), c(76, 100, 4))
    attrs <- attributes(img[[1]])
    check_attributes(attrs)

    img <- readTIFF(path, info=TRUE, all=1)
    expect_equal(length(img), 1)
    expect_equal(dim(img[[1]]), c(76, 100, 4))
    attrs <- attributes(img[[1]])
    check_attributes(attrs)

    img <- readTIFF(path, info=TRUE, all=2)
    expect_equal(length(img), 0)
})

test_that("readTIFFDirectory works", {
    path <- system.file("img", "Rlogo.tiff", package="tiff")
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

test_that("Reading tiles works", {
    # Some tests that only work on @PKI-Kent's dev box :-/
    root <- rprojroot::find_testthat_root_file()
    data_folder <- file.path(root, '..', '..', '..', 'tiffData')
    skip_if_not(file.exists(data_folder))

    # These files all contain multiple images including tiled images
    ## 32-bit float ##
    path <- file.path(data_folder, 'Float32_Tiled.tif')
    info <- readTIFFDirectory(path, all=TRUE)
    expect_equal(length(info), 5)

    expect_equivalent(
            info[[1]][c('width', 'length',
                        'bits.per.sample', 'samples.per.pixel',
                        'tile.width')],
            c(3728, 2784, 32, 1, 512))
    expect_equal(info[[1]][['sample.format']], 'float')

    # Check reading multiple images
    images <- readTIFF(path, all=c(1, 3))
    expect_equal(length(images), 2)
    expect_equal(dim(images[[1]]), c(2784, 3728))
    expect_equal(dim(images[[2]]), c(348, 466, 3)) # RGB thumbnail

    ## 8-bit RGB ##
    path <- file.path(data_folder, 'RGB_Tiled.tif')
    info <- readTIFFDirectory(path, all=TRUE)
    expect_equal(length(info), 6)

    # The third image is the smallest tiled image; check it
    expect_equivalent(
            info[[3]][c('width', 'length', 
                        'bits.per.sample', 'samples.per.pixel', 
                        'tile.width')],
            c(2784, 2080, 8, 3, 512))
    expect_equal(info[[3]][['sample.format']], 'uint')
    images <- readTIFF(path, all=3)

    # all != FALSE returns a list
    expect_equal(length(images), 1)
    expect_equal(dim(images[[1]]), c(2080, 2784, 3))

    ## 8-bit grayscale ##
    path <- file.path(data_folder, 'UInt8_Tiled.tif')
    info <- readTIFFDirectory(path, all=TRUE)
    expect_equal(length(info), 9)

    expect_equivalent(
            info[[3]][c('width', 'length',
                        'bits.per.sample', 'samples.per.pixel',
                        'tile.width')],
            c(2784, 2080, 8, 1, 512))
    expect_equal(info[[3]][['sample.format']], 'uint')

    images <- readTIFF(path, all=3)
    expect_equal(length(images), 1)
    expect_equal(dim(images[[1]]), c(2080, 2784))
})
