import sys
import fnmatch
import re
import platform
import os
from threading import Thread, currentThread, enumerate, Lock
sys.path.append('common')
from run_test import runTest
from thread_printer import ThreadPrinter
from env_indigo import *
test_results = []

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

def main():
    output_dir_base = "output_base"
    output_dir = "output"
    pattern = ""
    exclude_pattern = ""
    nunit_report_name = ""
    coverage_report_name = os.path.join(output_dir, 'coverage.txt')
    
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
        elif sys.argv[i] == '-wc':
            os.environ.pop('INDIGO_COVERAGE')
        else:
            print("Unexpected options: %s" % (sys.argv[i]))
            exit()
    
    indigo = Indigo()
    
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
        rel_root = root[i + 1:]
        for filename in fnmatch.filter(filenames, '*.py'):
            tests.append((rel_root, filename))
    tests.sort()
    
    # Calcuate maximum lenthd of the test names
    max_name_len = max([len(os.path.join(root, filename).replace('\\', '/')) for root, filename in tests])
    # add small gap
    max_name_len += 3   
    
    sys.stdout = ThreadPrinter()
    sys.stderr = ThreadPrinter()
    lock = Lock()
    for root, filename in tests:
        test_name = os.path.join(root, filename).replace('\\', '/')
        # Check test name by input pattern
        if test_name != "" and not re.search(pattern, test_name):
            continue
        # exclude some files from test by pattern
        if exclude_pattern != "" and re.search(exclude_pattern, test_name):
            continue
        #runTest(root, filename, output_dir, max_name_len, tests_dir, indigo, output_dir_base, test_results)
        t = Thread(name=test_name, target=runTest, args=(root, filename, output_dir, max_name_len, tests_dir, indigo, output_dir_base, test_results, lock))
        t.start()
    for thread in enumerate():
        if thread is not currentThread():
            thread.join()
    sys.stdout = sys.__stdout__
    sys.stderr = sys.__stderr__

    if indigo.version().endswith('-coverage'):
        from generate_coverage_report import generate_coverage_report
        generate_coverage_report(indigo, coverage_report_name)
    
    if nunit_report_name != "":
        from generate_nunit_report import generateNUnitReport
        generateNUnitReport(test_results, nunit_report_name)

if __name__ == '__main__':
    main()