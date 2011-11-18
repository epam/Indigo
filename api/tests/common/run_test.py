import difflib
import imp
import os
import shutil
import sys
import time
import traceback

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

def filterOldText(oldText):
    importLineList = []
    testLineList = []
    for line in oldText:
        if line in [
                    "sys.path.append('../../common')",
                    "import sys",
                    "from env_indigo import *",
                    ""]:
            continue        
        elif line.startswith('import') or line.startswith('from'):
            importLineList.append(line)
        else:
            testLineList.append(line)
    return importLineList, testLineList

def generateNewText(importLineList, testLineList):
    newTextLineList = []
    if sys.version_info[0] < 3 and sys.version_info[1] >= 6:
        newTextLineList.append('from __future__ import print_function')
    for line in importLineList:
        newTextLineList.append(line)
    newTextLineList.append("import sys")    
    newTextLineList.append("sys.path.append('../../common')")
    newTextLineList.append('from env_indigo import *\n')
    newTextLineList.append('def runIndigoTest():')
    newTextLineList.append('    indigo = None')
    
    for line in testLineList:
        newTextLineList.append('    %s' % line)
    newTextLineList.append('    return indigo')
    return '\n'.join(newTextLineList)

def manageText(oldText):
    importLineList, testLineList = filterOldText(oldText.split('\n'))
    result = generateNewText(importLineList, testLineList)
    return result

def runTest(root, filename, output_dir, max_name_len, tests_dir, indigo, output_dir_base, test_results):    
    try:
        test_dir = os.path.join(output_dir, root)
        if not os.path.exists(test_dir):
            os.makedirs(test_dir)
        test_name = os.path.join(root, filename).replace('\\', '/')
        spacer_len = max_name_len - len(test_name)
        test_root = os.path.join(tests_dir, root)    
        pathName = None
        fp, pathname, description = imp.find_module(filename[:-3], [os.path.join(tests_dir, root)])
        originalTestText = fp.read()
        fp.close()
        modifiedTestText = manageText(originalTestText)
        fp = open(pathname, 'wt')
        fp.write(modifiedTestText)
        fp.close()
        fp = open(pathname, 'U')
        pathName = pathname            
        beginTime = time.time()
        stdout = ''
        stderr = ''
        indigoOutput = None
        module = imp.load_module(filename[:-3], fp, pathname, description)
        try:
            indigoOutput = module.runIndigoTest()
        except Exception, e:
            stderr = traceback.format_exc()
        finally:
            totalTime = time.time() - beginTime                
            fp.close()
            fp = open(pathName, 'wt')
            fp.write(originalTestText)
            fp.close()
          
        stdout = sys.stdout.getValueByTestName(test_name)
        if indigo.version().endswith('-coverage') and indigoOutput:
            for item in indigoOutput._indigoCoverageDict:
                indigo._indigoCoverageDict[item] += indigoOutput._indigoCoverageDict[item]
            for item in indigoOutput._indigoObjectCoverageDict:
                indigo._indigoObjectCoverageDict[item] += indigoOutput._indigoObjectCoverageDict[item]
            for type in indigoOutput._indigoObjectCoverageByTypeDict:
                if not type in indigo._indigoObjectCoverageByTypeDict:
                    indigo._indigoObjectCoverageByTypeDict[type] = {}
                for key, value in indigoOutput._indigoObjectCoverageByTypeDict[type].items():
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
    except Exception, e:
        traceback.print_exc(file=sys.__stderr__)
