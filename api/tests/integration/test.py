#!/usr/bin/env python
import datetime
import difflib
import os
import re
import shutil
import sys
import time
from threading import Lock

import runpy
import traceback
import gc

if sys.platform == 'cli':
    import clr
    import System

    clr.AddReference("System.IO.FileSystem")
    clr.AddReference("System.Runtime.Extensions")
    from System.IO import FileInfo, File, Directory
    from System import Environment

base_root = os.path.normpath(os.path.abspath(os.path.dirname(__file__)))
sys.path.append(os.path.join(base_root, 'common'))
from env_indigo import (Indigo, getPlatform, isJython, isIronPython, dll_full_path, dir_exists,
                        makedirs, file_size, file_exists, open_file_utf8)
from util import overridePlatform
from thread_printer import ThreadPrinter, ThreadPrinterManager
from thread_pool import ThreadPool, cpu_count

stdout_thread_printer = ThreadPrinter()
stderr_thredd_printer = ThreadPrinter()
stdout_lock = Lock()


# class Logger(object):
#     def __init__(self, output_file_name):
#         self.terminal = sys.stdout
#         self.log = open(output_file_name, "w")
#
#     def write(self, message):
#         self.terminal.write(message)
#         self.log.write(message)
#
#     def flush(self):
#         self.terminal.flush()
#         self.log.flush()


def write_difference(fn_1, fn_2, fn_3):
    f_1 = open(fn_1, 'r')
    f_2 = open(fn_2, 'r')
    lines_1 = f_1.readlines()
    lines_2 = f_2.readlines()
    f_2.close()
    f_1.close()

    s1 = [s.splitlines()[0] for s in lines_1]
    s2 = [s.splitlines()[0] for s in lines_2]

    s = difflib.SequenceMatcher()
    s.set_seqs(s1, s2)

    f_3 = open(fn_3, 'w')

    difference_counter = 0
    for tag, i1, i2, j1, j2 in s.get_opcodes():
        if tag == 'equal':
            continue
        sub1 = s1[i1:i2]
        sub2 = s2[j1:j2]
        difference_counter += len(sub1) + len(sub2)
        # we cannot use zip_longest or map because code have to compatible for Python 2.4 .. 3.x
        while len(sub1) < len(sub2):
            sub1.append('')
        while len(sub2) < len(sub1):
            sub2.append('')
        for line1, line2 in zip(sub1, sub2):
            if line1:
                f_3.write("- " + line1 + "\n")
            if line2:
                f_3.write("+ " + line2 + "\n")
    f_3.close()
    # On Linux and macOS Jython and IronPython work for hours in some cases to build this diff. Use Python.
    if not isJython() and not isIronPython():
        with open("{}.html".format(fn_3), "w") as pretty_file:
            pretty_diff = difflib.HtmlDiff()
            html_cont = pretty_diff.make_file(lines_1, lines_2)
            pretty_file.write(html_cont)
    return difference_counter


def get_tests(root_path):
    result = []
    for path in os.listdir(root_path):
        if path.endswith('.py'):
            result.append((os.path.basename(root_path), path))
        elif dir_exists(os.path.join(root_path, path)):
            result += get_tests(os.path.join(root_path, path))
    return result


def main():
    indigo = Indigo()
    output_dir_base = os.path.join(base_root, "ref")
    output_dir = os.path.join(base_root, "out")
    pattern_list = []
    exclude_pattern_list = []
    junit_report_name = ""
    iterations = 1
    threads_number = cpu_count()

    if 'LABEL' in os.environ:
        binenv = os.environ['LABEL'].upper()
        python_exec = os.environ[binenv]
    else:
        python_exec = sys.executable

    for i in range(1, len(sys.argv), 2):
        if sys.argv[i] == '-p':
            pattern_list = sys.argv[i + 1].split(',')
        elif sys.argv[i] == '-e':
            exclude_pattern_list = sys.argv[i + 1].split(',')
        elif sys.argv[i] == '-o':
            output_dir = sys.argv[i + 1]
        elif sys.argv[i] == '-b':
            output_dir_base = sys.argv[i + 1]
        elif sys.argv[i] == '-i':
            iterations = int(sys.argv[i + 1])
        elif sys.argv[i] == '-j':
            junit_report_name = sys.argv[i + 1]
        elif sys.argv[i] == '-t':
            threads_number = int(sys.argv[i + 1])
        elif sys.argv[i] == '-exec':
            python_exec = sys.argv[i + 1]
        elif sys.argv[i] == '-platform':
            overridePlatform(sys.argv[i + 1])
        else:
            print("Unexpected options: %s" % (sys.argv[i]))
            exit()

    if not dir_exists(output_dir):
        makedirs(output_dir)

    # sys.stdout = Logger(output_dir + "/results.txt")

    if python_exec.startswith('"') and python_exec.endswith('"'):
        python_exec = python_exec[1:-1]

    print("Indigo version: " + indigo.version())
    print("Indigo library path: " + dll_full_path())
    print("Date & time: " + datetime.datetime.now().strftime("%d.%m.%Y %H:%M"))
    if sys.platform == 'cli':
        import System.Environment
    # print("Platform: {}".format(platform.platform() if sys.platform != 'cli' else System.Environment.OSVersion.ToString()))
    # print("Processor: {}".format(platform.processor() if sys.platform != 'cli' else 'x86_64' if System.Environment.Is64BitProcess else 'x86'))
    print("Python: " + sys.version.replace('\n', '\t'))
    print("Executable: " + python_exec)
    import socket
    print("Host name: " + socket.gethostname())  # platform.node())
    print("")
    del indigo
    # Collect tests and sort them
    tests_dir = os.path.join(base_root, 'tests')
    tests = sorted(get_tests(tests_dir))
    # Calcuate maximum lenthd of the test names
    max_name_len = max((len(item[0]) + len(item[1]) + 1 for item in tests))
    # add small gap
    max_name_len += 3

    test_results = []
    # Execute tests in sorted order
    test_args = []
    for root, filename in tests:
        test_dir = os.path.join(output_dir, root)
        if not dir_exists(test_dir):
            os.makedirs(test_dir)
        test_name = os.path.join(root, filename).replace('\\', '/')

        if test_name != "":
            # Check test name by input pattern
            if len(pattern_list):
                skip_test = True
                for pattern in pattern_list:
                    if re.search(pattern, test_name):
                        skip_test = False
                if skip_test:
                    continue
            # exclude some files from test by pattern
            if len(exclude_pattern_list):
                skip_test = False
                for exclude_pattern in exclude_pattern_list:
                    if re.search(exclude_pattern, test_name):
                        skip_test = True
                if skip_test:
                    continue

        for i in range(iterations):
            test_args.append(((filename, i, max_name_len, output_dir_base, root, test_dir,
                               test_name, tests_dir)))

    p = ThreadPool(threads_number)
    total_time = time.time()
    with ThreadPrinterManager(stdout=stdout_thread_printer, stderr=stderr_thredd_printer):
        test_results = tuple(p.imap_unordered(run_analyze_test, test_args))
    test_status = 0
    for test_result in test_results:
        if test_result[2] == '[FAILED]':
            test_status = test_status | 1
        if test_result[2] == '[ERROR]':
            test_status = test_status | 2
    total_time = time.time() - total_time
    print("\nTotal time: {:.2f} sec".format(total_time))

    if junit_report_name != "":
        from generate_junit_report import generate_junit_report
        generate_junit_report(test_results, junit_report_name)

    return test_status


def run_analyze_test(args):
    filename, i, max_name_len, output_dir_base, root, test_dir, test_name, tests_dir = args
    out_message = test_name
    spacer_len = max_name_len - len(test_name)
    out_filename = os.path.join(test_dir, filename + "_" + str(i) + '.out')
    err_filename = os.path.join(test_dir, filename + "_" + str(i) + '.err')
    module_filename = os.path.join(tests_dir, root, filename)

    tspent = run_test_module(module_filename)

    with open(out_filename, 'wb') as f:
        f.write(stdout_thread_printer.read_and_clean())

    with open(err_filename, 'wb') as f:
        f.write(stderr_thredd_printer.read_and_clean())

    failed_stderr = False
    if not file_size(err_filename):
        os.remove(err_filename)
    else:
        failed_stderr = True
    base_dir = os.path.join(output_dir_base, root)
    base_output_file = os.path.join(base_dir, filename + ".out")
    base_exists = False
    ndiffcnt = 0
    diff_file = None
    if file_exists(base_output_file):
        diff_file = os.path.join(test_dir, filename + ".diff")
        # copy reference file
        if isJython() and file_exists(os.path.join(base_dir, "jython", filename + '.out')):
            base_output_file = os.path.join(base_dir, "jython", filename + '.out')
        elif isIronPython() and file_exists(os.path.join(base_dir, "iron", filename + '.out')):
            base_output_file = os.path.join(base_dir, "iron", filename + '.out')
        else:
            sn = getPlatform()
            if sn and file_exists(os.path.join(base_dir, sn, filename + '.out')):
                base_output_file = os.path.join(base_dir, sn, filename + '.out')
        new_ref_file = os.path.join(test_dir, filename + ".std")
        if not os.path.normpath(os.path.abspath(base_output_file)) == os.path.normpath(os.path.abspath(new_ref_file)):
            if not sys.platform == 'cli':
                shutil.copy(base_output_file, new_ref_file)
            else:
                import clr
                clr.AddReference("System.IO.FileSystem")
                from System.IO import File
                File.Copy(base_output_file, new_ref_file, True)

        ndiffcnt = write_difference(new_ref_file, out_filename, diff_file)
        # remove empty diff file
        if not ndiffcnt:
            os.remove(diff_file)
        base_exists = True
    spacer = '.'
    msg = ''
    if failed_stderr:
        test_status = "[ERROR]" if root != 'todo' else '[TODO]'
    elif not base_exists:
        test_status = "[NEW]"
    elif not ndiffcnt:
        test_status = "[PASSED]"
        spacer = ' '
        spacer_len += 2
    else:
        test_status = "[FAILED]" if root != 'todo' else '[TODO]'
    out_message += "{}{}    {:.2f} sec".format(spacer * spacer_len, test_status, tspent)
    if diff_file and file_exists(diff_file):
        f = open(diff_file, 'rt')
        msg = 'Diff:\n' + f.read()
        f.close()
    if err_filename and file_exists(err_filename):
        f = open(err_filename, 'rt')
        msg = 'Error:\n' + f.read()
        f.close()
    with stdout_lock:
        sys.__stdout__.write(out_message + '\n')
    test_result = (root, filename, test_status, msg, tspent)
    return test_result


def run_test_module(module_filename):
    t0 = time.time()
    try:
        runpy.run_path(module_filename, run_name='__main__')
        gc.collect()
    except:
        sys.stderr.write(traceback.format_exc())
    tspent = time.time() - t0
    return tspent


if __name__ == "__main__":
    result = main()
    if result:
        sys.exit(result)
