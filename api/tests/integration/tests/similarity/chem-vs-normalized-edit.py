import sys
sys.path.append('../../common')
from env_indigo import *

from itertools import *

indigo = Indigo()
indigo.setOption("ignore-stereochemistry-errors", "1")
indigo.setOption("ignore-noncritical-query-features", "true")
indigo.setOption("similarity-type", "CHEM")

print("*** Difference between CHEM and Normilized-Edit ***")

smiles_file_path = joinPath("molecules", "pubchem_slice_100k.smiles")
num_control_mols = 10
num_mols = 10000

# the max accepted difference between `norm` and `chem`
similarity_threshold = 0.20
zero_similarity_threshold = 0.35
alarm_similarity_threshold = 0.40

all_mols = (m for m in indigo.iterateSmilesFile(smiles_file_path))
control_mols = islice(all_mols, num_control_mols)
mols = islice(all_mols, num_mols)

check_counter = 0
problems_counter = 0
zero_problems_counter = 0
alarm_problems_counter = 0

worst_problem_difference = 0
worst_problem_data = (None, None, 0.0, 0.0)

for (m1, m2) in product(control_mols, mols):
    f1 = m1.fingerprint('sim')
    f2 = m2.fingerprint('sim')

    norm = indigo.similarity(m1, m2, 'normalized-edit')
    chem = indigo.similarity(f1, f2, 'tversky 0.7 0.7')
    diff = abs(norm - chem)

    check_counter += 1

    if diff > similarity_threshold:
        problems_counter += 1

    if diff > zero_similarity_threshold:
        zero_problems_counter += 1

    if diff > alarm_similarity_threshold:
        alarm_problems_counter += 1

    if diff > worst_problem_difference:
        worst_problem_difference = diff
        worst_problem_data = (m1, m2, norm, chem)


print("    Molecule group size: %d" % num_mols)
print("    Control group size: %d" % num_control_mols)
print("    Number of pairs: %d" % check_counter)

DEBUG = False  # turn off for reproducible results
if DEBUG:
    print("    Out of %d pairs %d pairs were above the similarity threshold %f, which is %d%%" %
          (check_counter, problems_counter, similarity_threshold, round(100.0 * problems_counter / check_counter)))
    print("    Out of %d pairs %d pairs were above the zero similarity threshold %f, which is %d%%" %
          (check_counter, zero_problems_counter, zero_similarity_threshold, round(100.0 * zero_problems_counter / check_counter)))
    print("    %d pairs were above the alarm similarity threshold %f, which is %d%%" %
          (alarm_problems_counter, alarm_similarity_threshold, round(100.0 * alarm_problems_counter / check_counter)))

all_is_fine_flag = True

if 1.0 * alarm_problems_counter / check_counter > 0.0001:
    all_is_fine_flag = False
    (m1, m2, norm, chem) = worst_problem_data
    print("  ALARM!\n")
    print("    Too many pairs (%d) were above the alarm problem threshold %f\n" %
          (alarm_problems_counter, alarm_similarity_threshold))
    print("    mol1: %s\n" % (m1.canonicalSmiles()))
    print("    mol2: %s\n" % (m2.canonicalSmiles()))
    print("    normalized-edit sim: %f, chem sim: %f\n" % (norm, chem))
    print("    The difference %f is bigger than the alarm threshold %f\n" %
          (worst_problem_difference, alarm_similarity_threshold))

if 1.0 * zero_problems_counter / check_counter > 0.001:
    all_is_fine_flag = False
    print("  ALARM!\n")
    print("    Too many pairs (%d) were above the zero problem threshold %f\n" %
          (zero_problems_counter, zero_similarity_threshold))

if 1.0 * problems_counter / check_counter > 0.25:
    all_is_fine_flag = False
    print("  ALARM!\n")
    print("    Too many pairs (%d) were above the problem threshold %f\n" % (problems_counter, similarity_threshold))

if all_is_fine_flag:
    print("    All seems to be fine")
