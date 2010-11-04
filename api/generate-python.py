import sys
import re

file = open(sys.argv[1], "r")

p = re.compile('CEXPORT (void|int|qword|float|const char \*)\s*([^\s]*)\s*\((.*)\)')
p2 = re.compile("(int \*|int|qword|float|const char \*).*")

while file:
  line = file.readline()
  #rint line
  if not line:
    break
  m = p.match(line)
  if not m:
    continue
  fname = m.group(2)
  if fname == "indigoSetErrorHandler":
    continue
  if fname == "indigoToBuffer":
    continue
  if fname == "indigoRenderWriteHDC":
    continue
  if m.group(3) == "" or m.group(3) == "void":
    args = []
  else:
    args = re.split(", ", m.group(3))

  if args == []:
    argtypes = "None"
  else:
    argtypes = "["
    for i in xrange(len(args)):
      if i > 0:
        argtypes += ", "
      m2 = p2.match(args[i])
      if m2.group(1) == "int":
        argtypes += "c_int"
      elif m2.group(1) == "qword":
        argtypes += "c_ulonglong"
      elif m2.group(1) == "float":
        argtypes += "c_float"
      elif m2.group(1) == "const char *":
        argtypes += "c_char_p"
      elif m2.group(1) == "int *":
        argtypes += "POINTER(c_int)"
    argtypes += "]"

  if m.group(1) == "void":
    restype = "None"
  elif m.group(1) == "int":
    restype = "c_int"
  elif m.group(1) == "qword":
    restype = "c_ulonglong"
  elif m.group(1) == "float":
    restype = "c_float"
  elif m.group(1) == "const char *":
    restype = "c_char_p"
      
  print "    self._lib." + fname + ".restype = " + restype
  print "    self._lib." + fname + ".argtypes = " + argtypes
