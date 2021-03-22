from __future__ import with_statement
import sys
if sys.version_info < (3, 0):
	from cStringIO import StringIO
else:
	from io import StringIO

import inspect
import os

class ThreadPrinter(object): 
    def __init__(self, lock): 
        self.fhs = {}
        self.lock = lock
        
    def write(self, value): 
        with self.lock:
            frm = inspect.stack()[1][1]
            splittedFromPath = frm.split(os.path.sep)            
            test_group = splittedFromPath[-2]
            test = splittedFromPath[-1]
            curTestName = '%s/%s' % (test_group, test)
            if curTestName in self.fhs:
                f = self.fhs[curTestName]            
            else:
                f = StringIO()
            f.write(value)
            self.fhs[curTestName] = f
        
    def getValueByTestName(self, testName):
        result = ''
        for key, value in self.fhs.items():
            key = key.replace('_modified', '')
            if key == testName:
                result = value.getvalue()
        return result