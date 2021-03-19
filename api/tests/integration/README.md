# Functional test for Indigo API
Indigo API functional tests are written in common subset of Pythn 2 and 3. Also we use them for testing Java and .NET
wrappers (via Jython and IronPython).

Tests use self-written engine that intercepts STDOUT and STDERR, writes it to "out" folder and compares with the 
reference files from "ref" folder.

## Tests code style

1. Tests should be independent. Test engine does not guarantee any deterministic execution order, and also allows to
   run any single test.
2. Tests should use common subset of Python 2 and 3. In the first case it is about using "print".
   Always use `print` with braces and only single argument: `print('{0} {1} {2}'.format('a', 'b', 'c'))`. 
   It is required because:
 * In Python 2 `print` is a keyword, and `print('a', 'b', 'c')` prints a tuple of `('a', 'b', 'c')`. 
 * In Python 3 `print` is a function, so `print('a', 'b', 'c')` prints formatted string `a b c`. 
 

## Example of Indigo API test:

```
import sys
# Folder with "env_indigo"
sys.path.append(os.path.normpath(os.path.join(os.path.abspath(__file__), '..', '..', '..', "common")))  
from env_indigo import * # Load Indigo and some useful functions

indigo = Indigo()
m = indigo.loadMolecule('C')
print("SMILES: {0}".format(m.smiles())) 
try:
    m.getAtom(2)
except IndigoException as e:
    print("Error: {0}".format(getIndigoExceptionText(e))) # Exception pringing should use getIndigoExceptionText, 
                                                          # since their format differs in Python, .NET and Java
```

* Import of env_indigo allows to load indigo-python, indigo-java or indigo-dotnet from `../indigo/api`, or 
  `../indigo/dist`. This is the same as importing Indigo, IndigoException, IndigoInchi, IndigoRenderer, Bingo, 
  BingoException, BingoObject.

## IndigoRenderer test example

```
import sys
sys.path.append(os.path.normpath(os.path.join(os.path.abspath(__file__), '..', '..', '..', "common")))
from env_indigo import *
from rendering import *

if not os.path.exists(joinPath("out")):
    os.makedirs(joinPath("out"))

indigo = Indigo()
renderer = IndigoRenderer(indigo)

mol = indigo.loadMolecule("CCNNCN")

# SVG testing
indigo.setOption("render-output-format", "svg")
renderer.renderToFile(mol, joinPath("out/rsite_highlighted.svg"))
print(checkImageSimilarity('rsite_highlighted.svg'))

# PNG testing
indigo.setOption("render-output-format", "png")
renderer.renderToFile(mol, joinPath('out/rsite_highlighted.png'))
print(checkImageSimilarity('rsite_highlighted.png'))
```
* `checkImageSimilarity()` compares the result in `indigo/api/tests/python/tests/rendering/out` with the reference from
  folder `indigo/api/tests/python/rendering/rendering/ref/{%os%}`, where {%os%} is `win`, `linux` or `mac`.
  This is required because we use native font engines and fonts a slightly different.

## Useful functions from env_indigo
* `isJython()` — returns `True`, if we use indigo-java
* `isIronPython()` — returns `True`, if we use indigo-dotnet
* `getPlatform()` — returns `win`, `linux` или `mac`, our current OS
* `getIndigoExceptionTest(IndigoException)` — return the text of `IndigoException` in the same format for
   Python, Java and .NET.
* `joinPath(*args)` — returns the absolute path of a file relative to current. For instance, 
   `joinPath('molecules/helma_smi')`, called in `indigo/api/tests/python/tests/basic/basic.py`, will return
   `indigo/api/tests/python.tests/basic/molecules/helma.smi`
