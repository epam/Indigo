from __future__ import with_statement
import shutil
import time
import traceback
import os
import sys
from env_indigo import IndigoException
import difflib

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

def runTest(root, filename, output_dir, max_name_len, tests_dir, indigo, output_dir_base, test_results, lock):
    test_dir = os.path.join(output_dir, root)
    if not os.path.exists(test_dir):
        os.makedirs(test_dir)
    test_name = os.path.join(root, filename).replace('\\', '/')
    spacer_len = max_name_len - len(test_name)
    test_root = os.path.join(tests_dir, root)
    sys.path.append(test_root)       
    with lock:
        try:       
            beginTime = time.time()
            module = __import__(filename[:-3])
        except IndigoException, e:
            traceback.print_exc(file=sys.stderr)
        finally:
            totalTime = time.time() - beginTime
            if filename[:-3] in sys.modules:
                del sys.modules[filename[:-3]]
            sys.path.remove(test_root)
            
    stdout = sys.stdout.getValueByTestName(test_name)    
    stderr = sys.stderr.getValueByTestName(test_name)
    
    if indigo.version().endswith('-coverage') and hasattr(module, 'indigo'):
        indigoOutput = module.indigo
        for item in module.indigo._indigoCoverageDict:
            indigo._indigoCoverageDict[item] += indigoOutput._indigoCoverageDict[item]
        for item in module.indigo._indigoObjectCoverageDict:
            indigo._indigoObjectCoverageDict[item] += indigoOutput._indigoObjectCoverageDict[item]
        for type in module.indigo._indigoObjectCoverageByTypeDict:
            if not type in indigo._indigoObjectCoverageByTypeDict:
                indigo._indigoObjectCoverageByTypeDict[type] = {}
            for key, value in module.indigo._indigoObjectCoverageByTypeDict[type].items():
                if not key in indigo._indigoObjectCoverageByTypeDict[type]:
                    indigo._indigoObjectCoverageByTypeDict[type][key] = 0
                indigo._indigoObjectCoverageByTypeDict[type][key] += indigoOutput._indigoObjectCoverageByTypeDict[type][key]
                    
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
    sys.__stdout__.write("%s%s%s\t%.2f sec\n" % (test_name, spacer * spacer_len, msg, totalTime))
    test_results.append((root, filename, msg, totalTime))