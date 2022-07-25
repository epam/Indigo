# Copyright (C) from 2009 to Present EPAM Systems.
#
# This file is part of Indigo toolkit.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

SALTS = [
    # inorganics with quaternary atom
    "[X4&!#1&!#6](~[X1&!#6])(~[X1&!#6])(~[X1&!#6])~[X1&!#6]",
    "[X4&!#1&!#6](~[X2H&!#6])(~[X2H&!#6])(~[X2H&!#6])~[X1&!#6]",
    "[X4&!#1&!#6](~[X2H&!#6])(~[X2H&!#6])(~[X1&!#6])~[X1&!#6]",
    "[X4&!#1&!#6](~[X2H&!#6])(~[X1&!#6])(~[X1&!#6])~[X1&!#6]",
    # inorganics with tertiary atom
    "[X3&!#1&!#6](~[X1&!#6])(~[X1&!#6])~[X1&!#6]",
    "[X3&!#1&!#6](~[X2H&!#6])(~[X2H&!#6])~[X1&!#6]",
    "[X3&!#1&!#6](~[X2H&!#6])(~[X1&!#6])~[X1&!#6]",
    # inorganics with secondary atom
    "[X2&!#1&!#6](~[X1&!#6])~[X1&!#6]",
    "[X2&!#1&!#6](~[X2H&!#6])~[X1&!#6]",
    # inorganics with primary atom
    "[X1&!#1&!#6]~[X1&!#6]",
    # single-element ions
    "[X0+1]",
    "[X0+2]",
    "[X0+3]",
    "[X0+4]",
    "[X0-1]",
    "[X0-2]",
]

IONS = [
    # metal cations + ammonium
    "[NH4+]",
    "[Li+]",
    "[Na+]",
    "[K+]",
    "[Rb+]",
    "[Cs+]",
    "[Be+2]",
    "[Mg+2]",
    "[Ca+2]",
    "[Sr+2]",
    "[Ba+2]",
    "[Al+3]",
    "[Ga+3]",
    "[Ge+4]",
    "[Sn+2]",
    "[Sn+4]",
    "[Pb+2]",
    "[Pb+4]",
    "[Sb+3]",
    "[Bi+3]",
    "[Sc+3]",
    "[Y+3]",
    "[Ti+4]",
    "[Zr+4]",
    "[V+4]",
    "[V+5]",
    "[Nb+5]",
    "[Cr+2]",
    "[Cr+3]",
    "[Mo+2]",
    "[Mo+3]",
    "[Mo+4]",
    "[Mn+2]",
    "[Fe+2]",
    "[Fe+3]",
    "[Co+2]",
    "[Ni+2]",
    "[Ru+3]",
    "[Co+2]",
    "[Co+3]",
    "[Rh+3]",
    "[Ni+2]",
    "[Ni+3]",
    "[Pd+2]",
    "[Cu+2]",
    "[Ag+]",
    "[Zn+2]",
    "[Cd+2]",
    # hydracids anions
    "[F-]",
    "[Cl-]",
    "[Br-]",
    "[I-]",
    "[S-2]",
    "[C-]#N",
    # inorganic oxyacids
    "B(=O)[O-]",
    "B([O-])([O-])[O-]",
    "C(=O)([O-])[O-]",
    "[O-][Si](=O)[O-]",
    "[O-][Si]([O-])([O-])[O-]",
    "N(=O)[O-]",
    "[N+](=O)([O-])[O-]",
    "[O-]P(=O)=O",
    "[O-]P(=O)([O-])[O-]",
    "[O-][As](=O)=O",
    "[O-][As](=O)([O-])[O-]",
    "[O-]S(=O)[O-]",
    "[O-]S(=O)(=O)[O-]",
    "[O-][Se](=O)(=O)[O-]",
    "[O-][Te](=O)[O-]",
    "[O-][Te]([O-])([O-])([O-])([O-])[O-]",
    "[O-][Al]=O",
    "[O-][Sn](=O)[O-]",
    # TODO: add [Al(OH)4]-, [Sn(OH)6]2- ???
    "[O-][Cr](=O)(=O)[O-] ",
    "[O-][Cr](=O)(=O)O[Cr](=O)(=O)[O-]",
    "[O-][Mn](=O)(=O)=O",
    "[O-]Cl",
    "[O-]Cl=O",
    "[O-]Cl(=O)=O",
    "[O-]Cl(=O)(=O)=O",
    "[O-]Br",
    "[O-]Br=O",
    "[O-]Br(=O)=O",
    "[O-]Br(=O)(=O)=O",
    "[O-]I(=O)=O",
    # acyclic monocarboxylic saturated acids and it`s halogen derivatives
    "C(=O)[O-]",
    "CC(=O)[O-]",
    "CCC(=O)[O-]",
    "CCCC(=O)[O-]",
    "CCCCC(=O)[O-]",
    "CCCCCCCCCCCCCCCC(=O)[O-]",
    "CCCCCCCCCCCCCCCCCC(=O)[O-]",
    "C(C(=O)[O-])Cl",
    "C(C(=O)[O-])(Cl)Cl",
    "C(=O)(C(Cl)(Cl)Cl)[O-]",
    # acyclic monocarboxylic unsaturated acids, cyclic acids
    # and it`s halogen derivatives
    "C=CC(=O)[O-]",
    "CC(=C)C(=O)[O-]",
    r"CCCCCCCC/C=C\CCCCCCCC(=O)[O-]",
    r"CCCCC/C=C\C/C=C\CCCCCCCC(=O)[O-]",
    r"CC/C=C\C/C=C\C/C=C\CCCCCCCC(=O)[O-]",
    "C/C=C/C(=O)[O-]",
    "C1=CC=C(C=C1)C(=O)[O-]",
    "C1=CC=C(C=C1)CC(=O)[O-]",
    "C1=CC=C(C=C1)CC(=O)[O-]",
    # polycarboxylic acids and it`s halogen derivatives
    "C(=O)(C(=O)[O-])[O-]",
    "C(CCC(=O)[O-])CC(=O)[O-]",
    "C(CCCC(=O)[O-])CCCC(=O)[O-]",
    "C(C(=O)[O-])C(=O)[O-]",
    "C1=CC(=CC=C1C(=O)[O-])C(=O)[O-]",
    # acids with additional functional group
    "CC(C(=O)[O-])O",
    "C(C(C(=O)[O-])O)(C(=O)[O-])O",
    "C(C(=O)[O-])C(CC(=O)[O-])(C(=O)[O-])O",
    "C(CC(=O)[O-])C(=O)[O-]",
    "C([C@H]([C@H]([C@@H]([C@H](C(=O)[O-])O)O)O)O)O",
    "C1=CC=C(C(=C1)C(=O)O)[O-]",
    "CC(=O)OC1=CC=CC=C1C(=O)[O-]",
]
