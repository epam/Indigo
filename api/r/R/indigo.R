version <- function() {
    .Call("version")
}

canonicalSmiles <- function(data) {
    .Call("canonicalSmiles", data)
}

checkSub <- function(query, target, mode) {
    (.Call("checkSub", query, target, mode) != 0)
}

fingerprint <- function(item, mode) {
    .Call("fingerprint", item, mode)
}

molecularWeight <- function(item) {
    .Call("molecularWeight", item)
}
