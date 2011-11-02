import itertools as it
import sys
sys.path.append('../../common')
from env_indigo import *

indigo = Indigo()
def transform(molset, reaction):
   output_reactions = indigo.reactionProductEnumerate(reaction, [ molset ])
   for out_rxn in output_reactions.iterateArray():
      for mol in out_rxn.iterateProducts():
         prod = mol.clone()
         prod.setName(out_rxn.name())
         yield prod
def transformBySmiles (reaction_smiles, molecules_smiles):
   print("***")
   print("Reaction: %s" % (reaction_smiles))
   print("Monomers:")
   molset = []
   for m, idx in zip(molecules_smiles, range(len(molecules_smiles))):
      mol = indigo.loadMolecule(m)
      mol.setName("mol_%d" % (idx))
      print("    %s: %s" % (mol.name(), m))
      molset.append(mol)
      
   print("Products:")
   reaction = indigo.loadQueryReaction(reaction_smiles)
   #reaction = indigo.loadReactionSmarts(reaction_smiles)
   try:
      res = []
      for mol in transform(molset, reaction):
         res.append((mol.canonicalSmiles(), mol.name()))
      for smiles, name in sorted(res, reverse=True):
         print("    %s: %s" % (name, smiles))
   except IndigoException, e:
      msg = "Error: %s" % (getIndigoExceptionText(e))
      print(msg)
      
transformBySmiles("O[*:1]>>[*:1][H]", [ "C(O)C=C(O)C(O)", "OC(=C)C1=CC=CC=C1O" ])
transformBySmiles("[H]C([*:2])([*:3])C([*:5])([*:6])[*:7]>>C([*:2])([*:3])=C([*:5])([*:6])", [ "C1(Cl)C(Cl)C(Cl)C(Cl)C(Cl)C1(Cl)" ])
transformBySmiles("[H]C([H,*:2])([H,*:3])C([H,*:5])([H,*:6])[H,*:7]>>C([H,*:2])([H,*:3])=C([H,*:5])([H,*:6])", [ "C1(Cl)C(Cl)C(Cl)C(Cl)C(Cl)C1(Cl)" ])
transformBySmiles("[H]C([H,*:2])([H,*:3])C([H,*:5])([H,*:6])[H,*:7]>>C([*:2])([*:3])=C([*:5])([H,*:6])", [ "C1(Cl)C(Cl)C(Cl)C(Cl)C(Cl)C1(Cl)" ])
transformBySmiles("[*:1]C([*:2])=[*:3]>>[*:1]C=[*:3]", [ "C=1(Cl)C=CC=CC1" ])
transformBySmiles("[*:2]c(:[*:1]):[*:3]>>[*:1]:c:[*:3]", [ "C=1(Cl)C=CC=CC1" ])
transformBySmiles("[*:1]>>[*;+:1]", [ "C", "[H]" ])
transformBySmiles("[*:1]>>[*;+0:1]", [ "[C+]", "[H]" ])
transformBySmiles("[*:1]>>[O,N;+:1]", [ "O", "N", "C" ])
# Bug?: Incorrect AAM
transformBySmiles("[H,*:2]>>N[*:2]", [ "C", ])
transformBySmiles("[H,*:1]>>N[H,*:1]", [ "C" ])
# Bug!: array: invalid index 14 (size=14)
transformBySmiles("[H,*:1]>>[*:1]", [ "C[2H]", ])
transformBySmiles("[H,*:1]>>[H,*:1]", [ "C[2H]" ])
transformBySmiles("[H,*:1]>>[*:1]", [ "[H]", ])
transformBySmiles("[H,*:1]>>[H,*:1]", [ "[H]" ])
