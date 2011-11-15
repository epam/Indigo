from cStringIO import StringIO
import sys
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
            tests_folder = splittedFromPath[-3]
            test_group = splittedFromPath[-2]
            test = splittedFromPath[-1]
            #mod = inspect.getmodule(frm[0])
            #curThread = threading.currentThread()
            #if mod is None:
            #    raise RuntimeError('Cannot find the caller module. Stack: ' + str(frm))
            curTestName = '{0}/{1}'.format(test_group, test)            
            #curTestName = '{0}/{1}.py'.format(os.path.basename(os.path.split(mod.__file__)[0]), mod.__name__)                        
            if curTestName in self.fhs:
                f = self.fhs[curTestName]            
            else:
                f = StringIO()
            f.write(value)
            self.fhs[curTestName] = f
        
    def getValueByTestName(self, testName):
        result = ''
        for key, value in self.fhs.items():
            if key == testName:
                result = value.getvalue()
        return result