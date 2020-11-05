import os
import sys
sys.path.append(os.path.normpath(os.path.join(os.path.abspath(__file__), '..', '..', '..', "common")))
from env_indigo import *

print("***** Get options check *****")
indigo = Indigo()
renderer = IndigoRenderer(indigo)

print('***** String *****')
print(indigo.getOption("filename-encoding"))
print(indigo.getOptionType("filename-encoding"))
indigo.setOption("filename-encoding", "UTF-8")
print(indigo.getOption("filename-encoding"))

indigo.setOption("render-comment", "test comment")
print(indigo.getOption("render-comment"))

print('***** Integer *****')
print(indigo.getOptionInt("max-embeddings"))
print(indigo.getOptionType("max-embeddings"))
indigo.setOption("max-embeddings", 100)
print(indigo.getOptionInt("max-embeddings"))
print(indigo.getOption("max-embeddings"))


print('***** Boolean *****')
print(indigo.getOptionBool("ignore-noncritical-query-features"))
print(indigo.getOptionType("ignore-noncritical-query-features"))
indigo.setOption("ignore-noncritical-query-features", True)
print(indigo.getOptionBool("ignore-noncritical-query-features"))
print(indigo.getOption("ignore-noncritical-query-features"))


print('***** Float *****')
print('{:.3f}'.format(indigo.getOptionFloat("layout-horintervalfactor")))
print(indigo.getOptionType("layout-horintervalfactor"))

if isIronPython():
	from System import Single
	indigo.setOption("layout-horintervalfactor", Single(21.333))
else:
	indigo.setOption("layout-horintervalfactor", 21.333)

print('{:.3f}'.format(indigo.getOptionFloat("layout-horintervalfactor")))
print(indigo.getOption("layout-horintervalfactor"))


print('***** Color *****')
print(indigo.getOption("render-background-color"))
print(indigo.getOptionType("render-background-color"))
indigo.setOption("render-background-color", "255, 255.3, 255.0")
print(indigo.getOption("render-background-color"))


print('***** XY *****')
print(indigo.getOption("render-image-size"))
print(indigo.getOptionType("render-image-size"))
indigo.setOption("render-image-size", "-1, -1.2")
print(indigo.getOption("render-image-size"))