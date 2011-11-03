# -*- coding: cp1251 -*-

import os, sys
import glob
import fnmatch
import subprocess
import shutil
import difflib
import time
import re
import platform

sys.path.append('common')
from env_indigo import Indigo

indigo = Indigo()

class Logger(object):
    def __init__(self, output_file_name):
        self.terminal = sys.stdout
        self.log = open(output_file_name, "w")

    def write(self, message):
        self.terminal.write(message)
        self.log.write(message)  

    def flush(self):
        self.terminal.flush()
        self.log.flush()

def write_difference(fn_1, fn_2, fn_3):
    f_1 = open(fn_1, 'r')
    f_2 = open(fn_2, 'r')        
    lines_1 = f_1.readlines()
    lines_2 = f_2.readlines()
    f_2.close()
    f_1.close()    
    for i in xrange(len(lines_1)):
        lines_1[i] = lines_1[i].splitlines()[0]
    for i in xrange(len(lines_2)):
        lines_2[i] = lines_2[i].splitlines()[0]  
    d = difflib.Differ()  
    result = d.compare(lines_2, lines_1)    
    f_3 = open(fn_3, 'w')
    result_list = list(result)
    difference_counter = 0
    for line in result_list:
        if line[0] <> ' ' and line != '+ ' and line != '- ':
            f_3.write(line + '\n')
            difference_counter += 1
    f_3.close()    
    return difference_counter
      
output_dir_base = "output_base"
output_dir = "output"
pattern = ""
exclude_pattern = ""
nunit_report_name=""

for i in range(1, len(sys.argv), 2):
   if sys.argv[i] == '-p':
      pattern = sys.argv[i + 1]
   elif sys.argv[i] == '-e':
      exclude_pattern = sys.argv[i + 1]
   elif sys.argv[i] == '-o':
      output_dir = sys.argv[i + 1]
   elif sys.argv[i] == '-b':
      output_dir_base = sys.argv[i + 1]
   elif sys.argv[i] == '-n':
      nunit_report_name = sys.argv[i + 1]
   else:
      print("Unexpected options: %s" % (sys.argv[i]))
      exit()
   
if not os.path.exists(output_dir):
   os.makedirs(output_dir)
   
sys.stdout = Logger(output_dir + "/results.txt") 

print("Indigo version: " + indigo.version())
print("Platform: " + platform.platform())
print("Processor: " + platform.processor())
print("Python: " + sys.version)
print("")

# Collect tests and sort them
tests_dir = 'tests'
tests = []
for root, dirnames, filenames in os.walk(tests_dir):
   if root == '.':
      continue
   i = len(os.path.commonprefix([root, tests_dir])) 
   rel_root = root[i+1:]
   for filename in fnmatch.filter(filenames, '*.py'):
      tests.append((rel_root, filename))
tests.sort()
      
# Calcuate maximum lenthd of the test names
max_name_len = max([len(os.path.join(root, filename).replace('\\', '/')) for root, filename in tests])
# add small gap
max_name_len += 3

test_results = []    

# Execute tests in sorted order
for root, filename in tests:   
   test_dir = os.path.join(output_dir, root)
   if not os.path.exists(test_dir):
      os.makedirs(test_dir)
   test_name = os.path.join(root, filename).replace('\\', '/')
   # Check test name by input pattern
   if test_name != "" and not re.search(pattern, test_name):
      continue
   # exclude some files from test by pattern
   if exclude_pattern != "" and re.search(exclude_pattern, test_name):
      continue
   
   sys.stdout.write("%s" % test_name)
   sys.stdout.flush()
   
   spacer_len = max_name_len - len(test_name)
   
   test_root = os.path.join(tests_dir, root)

   t0 = time.time()
   p = subprocess.Popen([sys.executable, filename], cwd=test_root, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
   stdout, stderr = p.communicate()
   tspend = time.time() - t0
   
   output_file = os.path.join(test_dir, filename + ".out")
   output_file_handle = open(output_file, 'wb')
   output_file_handle.write(stdout)
   if len(stderr) > 0:
      output_file_handle.write("*** STDERR OUTPUT ***\n")
      output_file_handle.write(stderr)
   output_file_handle.close()
   
   base_dir = os.path.join(output_dir_base, root)
   base_output_file = os.path.join(base_dir, filename + ".out")
   
   base_exists = False
   ndiffcnt = 0
   if os.path.exists(base_output_file):
      diff_file = os.path.join(test_dir, filename + ".diff")
      # copy reference file
      new_ref_file = os.path.join(test_dir, filename + ".std")
      shutil.copy (base_output_file, new_ref_file)
      
      ndiffcnt = write_difference(output_file, new_ref_file, diff_file)
      # remove empty diff file
      if not ndiffcnt:
          os.remove(diff_file)
      base_exists = True
   
   spacer = '.'
   if stderr != "":
      msg = "[FAILED: stderr]"
   elif not base_exists:
      msg = "[NEW]"
   elif not ndiffcnt:
      msg = "[PASSED]"
      spacer = ' '
      spacer_len += 2
   else:
      msg = "[FAILED]"
      
   print("%s%s\t%.2f sec" % (spacer * spacer_len, msg, tspend))
   test_results.append((root, filename, msg, tspend))

if nunit_report_name != "":
   from generate_nunit_report import *
   generateNUnitReport(test_results, nunit_report_name)