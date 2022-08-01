import os
import sys

sys.path.append(
    os.path.normpath(
        os.path.join(os.path.abspath(__file__), "..", "..", "..", "common")
    )
)
from env_indigo import *

print("***** Get options check *****")
indigo = Indigo()
renderer = IndigoRenderer(indigo)

indigo.setOption("render-comment", "test comment")
print(indigo.getOption("render-comment"))

print("***** Color *****")
print(indigo.getOption("render-background-color"))
print(indigo.getOptionType("render-background-color"))
indigo.setOption("render-background-color", "255, 255.3, 255.0")
print(indigo.getOption("render-background-color"))

print("***** XY *****")
print(indigo.getOption("render-image-size"))
print(indigo.getOptionType("render-image-size"))
indigo.setOption("render-image-size", "-1, -1.2")
print(indigo.getOption("render-image-size"))

if isIronPython():
    renderer.Dispose()
    indigo.Dispose()
