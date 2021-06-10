import difflib
import imp
import os
import shutil
import sys
import time
import traceback
import re

from env_indigo import getPlatform


def write_difference(fn_1, fn_2, fn_3):
    def repl(matchobj):
        result = str(round(float(matchobj.group(0)), 1))
        if result == '-0.0':
            result = ' 0.0'
        return result

    if sys.version_info[0] < 3:
        with open(fn_1, 'r') as f:
            lines_1 = f.readlines()
        with open(fn_2, 'r') as f:
            lines_2 = f.readlines()
    else:
        with open(fn_1, 'r', encoding='utf-8') as f:
            lines_1 = f.readlines()
        with open(fn_2, 'r', encoding='utf-8') as f:
            lines_2 = f.readlines()

    for i in range(len(lines_1)):
        lines_1[i] = lines_1[i].splitlines()[0]
        lines_1[i] = re.sub('([-|+]{0,1}\d+\.\d+)', repl, lines_1[i])
    for i in range(len(lines_2)):
        lines_2[i] = lines_2[i].splitlines()[0]
        lines_2[i] = re.sub('([-|+]{0,1}\d+\.\d+)', repl, lines_2[i])
    d = difflib.Differ()
    result = d.compare(lines_2, lines_1)
    result_list = list(result)
    difference_counter = 0
    with open(fn_3, 'w') as f:
        for line in result_list:
            if line[0] != ' ' and line != '+ ' and line != '- ':
                f.write(line + '\n')
                difference_counter += 1
    return difference_counter


def filterOldText(oldText):
    importLineList = []
    testLineList = []
    for line in oldText:
        if line in ["sys.path.append('../../common')",
                    "sys.path.append(os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), 'common'))",
                    "import sys",
                    'import os',
                    "from env_indigo import *",
                    ]:
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
    newTextLineList.append("import os")
    newTextLineList.append("import sys")
    newTextLineList.append("sys.path.append(os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), 'common'))")
    newTextLineList.append('from env_indigo import *\n')
    newTextLineList.append('def runIndigoTest():')
    newTextLineList.append('    indigo = None')

    for line in testLineList:
        newTextLineList.append('    %s' % line)
    newTextLineList.append('    return indigo')

    newTextLineList.append("\nif __name__ == '__main__':")
    newTextLineList.append("    runIndigoTest()")
    return '\n'.join(newTextLineList)


def manageText(oldText):
    importLineList, testLineList = filterOldText(oldText.split('\n'))
    result = generateNewText(importLineList, testLineList)
    return result


def runTest(root, filename, output_dir, max_name_len, tests_dir, indigo, output_dir_base, test_results, writeResults, resultQueue):
    try:
        originalTestName = filename[:-3]
        modifiedTestName = '%s_modified' % (originalTestName)
        test_dir = os.path.join(output_dir, root)
        if not os.path.exists(test_dir):
            import errno
            try:
                os.makedirs(test_dir)
            except OSError as e:
                if e.errno != errno.EEXIST:
                    raise
        test_name = os.path.join(root, filename).replace('\\', '/')
        spacer_len = max_name_len - len(test_name)
        originalFp, originalPathName, originalDescription = imp.find_module(originalTestName, [os.path.join(tests_dir, root)])
        originalTestText = originalFp.read()
        originalFp.close()
        modifiedTestText = manageText(originalTestText)
        modifiedPathName = '%s_modified.py' % (originalPathName[:-3])
        with open(modifiedPathName, 'wt') as modifiedFp:
            modifiedFp.write(modifiedTestText)
        stdout = ''
        stderr = ''
        indigoOutput = None
        modifiedFp, modifiedPathName, modifiedDescription = imp.find_module(modifiedTestName, [os.path.join(tests_dir, root)])
        beginTime = time.time()
        module = imp.load_module(modifiedTestName, modifiedFp, modifiedPathName, modifiedDescription)
        try:
            indigoOutput = module.runIndigoTest()
        except BaseException:
            stderr = traceback.format_exc()
        finally:
            totalTime = time.time() - beginTime
            os.remove(modifiedPathName)
            modifiedFp.close()
        stdout = sys.stdout.getValueByTestName(test_name)
        if indigo.version().endswith('-coverage') and indigoOutput:
            for item in indigoOutput._indigoCoverageDict:
                indigo._indigoCoverageDict[item] += indigoOutput._indigoCoverageDict[item]
            for item in indigoOutput._indigoObjectCoverageDict:
                indigo._indigoObjectCoverageDict[item] += indigoOutput._indigoObjectCoverageDict[item]
            for type in indigoOutput._indigoObjectCoverageByTypeDict:
                if type not in indigo._indigoObjectCoverageByTypeDict:
                    indigo._indigoObjectCoverageByTypeDict[type] = {}
                for key, value in indigoOutput._indigoObjectCoverageByTypeDict[type].items():
                    if key not in indigo._indigoObjectCoverageByTypeDict[type]:
                        indigo._indigoObjectCoverageByTypeDict[type][key] = 0
                    indigo._indigoObjectCoverageByTypeDict[type][key] += indigoOutput._indigoObjectCoverageByTypeDict[type][key]

        output_file = os.path.join(test_dir, filename + ".out")
        if sys.version_info[0] < 3:
            with open(output_file, 'wt') as output_file_handle:
                output_file_handle.write(stdout)
                if len(stderr) > 0:
                    output_file_handle.write("*** STDERR OUTPUT ***\n")
                    output_file_handle.write(stderr)
        else:
            with open(output_file, 'wt', encoding='utf-8') as output_file_handle:
                output_file_handle.write(stdout)
                if len(stderr) > 0:
                    output_file_handle.write("*** STDERR OUTPUT ***\n")
                    output_file_handle.write(stderr)

        base_dir = os.path.join(output_dir_base, root)
        base_output_file = os.path.join(base_dir, filename + ".out")
        base_exists = False
        ndiffcnt = 0
        diff_file = None
        if os.path.exists(base_output_file):
            diff_file = os.path.join(test_dir, filename + ".diff")
            # copy reference file
            system_name = getPlatform()
            if system_name and os.path.exists(os.path.join(base_dir, system_name, filename + '.out')):
                base_output_file = os.path.join(base_dir, system_name, filename + '.out')
            new_ref_file = os.path.join(test_dir, filename + ".std")
            if not shutil._samefile(base_output_file, new_ref_file):
                shutil.copy(base_output_file, new_ref_file)

            ndiffcnt = write_difference(output_file, new_ref_file, diff_file)
            if not ndiffcnt:
                os.remove(diff_file)
            base_exists = True

        spacer = '.'
        if stderr != "":
            msg = "[FAILED: stderr]"
            spacer_2_len = 0
        elif not base_exists:
            msg = "[NEW]"
            spacer_2_len = 11
        elif not ndiffcnt:
            msg = "[PASSED]"
            spacer = ' '
            spacer_2_len = 8
        else:
            spacer_2_len = 8
            msg = "[FAILED]"
        writeResults("%s%s%s\t%s%.2f sec\n" % (test_name, spacer * spacer_len, msg, ' ' * spacer_2_len, totalTime))
        if diff_file and os.path.exists(diff_file):
            with open(diff_file, 'rt') as f:
                msg += '\n\nDiff:\n' + f.read()
        resultQueue.put((root, filename, msg, totalTime))
    except BaseException:
        traceback.print_exc(file=sys.__stderr__)
