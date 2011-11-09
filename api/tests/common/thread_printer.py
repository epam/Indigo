import threading
from cStringIO import StringIO
import sys

class ThreadPrinter(object): 
    def __init__(self): 
        self.fhs = {}
         
    def write(self, value): 
        curThread = threading.currentThread()
        curTestName = curThread.name            
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