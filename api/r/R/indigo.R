checkResult = function(result){
  if (result < 0) {
    stop(.Call("r_indigoGetLastError"))
  }
  return(result)
}

checkResultFloat = function(result){
  if (result < -0.5) {
      stop(.Call("r_indigoGetLastError"))
  }
  return(result)
}

checkResultPtr = function(result){
  if (is.na(result)) {
      stop(.Call("r_indigoGetLastError"))
  }
  return(result)
}

Indigo = setRefClass(
            Class = "Indigo",
            fields = list(id = "numeric"))
            
Indigo$methods(
initialize = function(){
  id<<-(.Call("r_indigoAllocSessionId"))
})

Indigo$methods(
setSession = function(){
  .Call("r_indigoSetSessionId", id)
})

Indigo$methods(
  version = function(){
    setSession()
    return(checkResultPtr(.Call("r_indigoVersion")));
})

Indigo$methods(
  version = function(){
    setSession()
    return(checkResultPtr(.Call("r_indigoVersionInfo")));
})

Indigo$methods(
  finalize = function(){
    .Call("r_indigoReleaseSessionId", id)
})

Indigo$methods(
  loadMolecule = function(data){
    setSession()
    obj_id = checkResult(.Call("r_indigoLoadMolecule",  data))
    return(IndigoObject$new(id, obj_id))
  })
  
Indigo$methods(
  loadQueryMolecule = function(data){
    setSession()
    obj_id = checkResult(.Call("r_indigoLoadQueryMolecule",  data))
    return(IndigoObject$new(id, obj_id))
  })
  
Indigo$methods(
  setOption = function(option, value){
    setSession()
    obj_id = checkResult(.Call("r_indigoSetOption",  option, value))
  })

Indigo$methods(
  resetOptions = function(){
    setSession()
    obj_id = checkResult(.Call("r_indigoResetOptions"))
  })

IndigoObject = setRefClass(
                 Class = "IndigoObject",
                 fields = list(indigo_id = "numeric", obj_id = "numeric"))

IndigoObject$methods(
  initialize = function(new_indigo_id, new_obj_id){
    indigo_id<<-new_indigo_id
    obj_id<<-new_obj_id
  })

IndigoObject$methods(
  canonicalSmiles = function(){
    .Call("r_indigoSetSessionId", indigo_id)
    return(checkResultPtr(.Call("r_indigoCanonicalSmiles", obj_id)))
  })
  
IndigoObject$methods(
  fingerprint = function(mode){
    .Call("r_indigoSetSessionId", indigo_id)
    return(checkResult(.Call("r_indigoFingerprint", obj_id, mode)))
  })
  
IndigoObject$methods(
  molecularWeight = function(){
    .Call("r_indigoSetSessionId", indigo_id)
    return(checkResult(.Call("r_indigoMolecularWeight", obj_id)))
  })
  
IndigoObject$methods(
  aromatize = function(){
    .Call("r_indigoSetSessionId", indigo_id)
    return(checkResult(.Call("r_indigoAromatize", obj_id)))
  })

IndigoObject$methods(
  finalize = function(){
    .Call("r_indigoSetSessionId", indigo_id)
    (checkResult(.Call("r_indigoFree", obj_id)))
  })
  
#################################################################
#################################################################
  
version <- function() {
    .Call("r_indigoVersion")
}

versionInfo <- function() {
    .Call("r_indigoVersionInfo")
}

setOption <- function(option, value){
   stopifnot(is.character(option))
   stopifnot(length(option) == 1, length(value) == 1)
   
   obj_id = checkResult(.Call("r_indigoSetOption",  option, value))
}

resetOptions <- function(){
   obj_id = checkResult(.Call("r_indigoResetOptions"))
}

canonicalSmiles <- function(data) {
    stopifnot(is.character(data))
    stopifnot(length(data) == 1)

    return(.Call("canonicalSmiles", data))
}

smiles <- function(data) {
    stopifnot(is.character(data))
    stopifnot(length(data) == 1)

    return(.Call("smiles", data))
}

aromatize <- function(data) {
    stopifnot(is.character(data))
    stopifnot(length(data) == 1)

    return(.Call("aromatize", data))
}

aromatizeQuery <- function(data) {
    stopifnot(is.character(data))
    stopifnot(length(data) == 1)

    return(.Call("aromatizeQuery", data))
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

render <- function(item) {
    stopifnot(is.character(item))
    stopifnot(length(item) == 1)

    return(.Call("render", item))
}

renderQuery <- function(item) {
    stopifnot(is.character(item))
    stopifnot(length(item) == 1)

    return(.Call("renderQuery", item))
}

renderHighlightedTarget <- function(target, query) {
    stopifnot(is.character(target), is.character(query))
    stopifnot(length(target) == 1, length(query) == 1)

    return(.Call("renderHighlightedTarget", target, query))
}