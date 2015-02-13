version <- function() {
    .Call("version")
}

canonicalSmiles <- function(data) {
    stopifnot(is.character(data))
    stopifnot(length(data) == 1)

    return(.Call("canonicalSmiles", data))
}

checkSub <- function(query, target, mode) {
    stopifnot(is.character(query), is.character(target), is.character(mode))
    stopifnot(length(query) == 1, length(target) == 1, length(mode) == 1)
    
    result <- .Call("checkSub", query, target, mode)
    if (result == -1)
       warning(paste("checkSub(\"", query, "\", \"", target, "\", \"", mode, "\") was failed", sep = "", collapse = NULL))

    return(result > 0)
}

fingerprint <- function(item, mode) {
    stopifnot(is.character(item), is.character(mode))
    stopifnot(length(item) == 1, length(mode) == 1)

    return(.Call("fingerprint", item, mode))
}

fingerprintQuery <- function(item, mode) {
    stopifnot(is.character(item), is.character(mode))
    stopifnot(length(item) == 1, length(mode) == 1)

    return(.Call("fingerprintQuery", item, mode))
}

molecularWeight <- function(item) {
    stopifnot(is.character(item))
    stopifnot(length(item) == 1)

    return(.Call("molecularWeight", item))
}