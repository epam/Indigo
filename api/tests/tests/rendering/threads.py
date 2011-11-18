import sys
from threading import Thread
from difflib import unified_diff
sys.path.append('../../common')
from env_indigo import *

ext = 'svg'
thread_count = 2
a = {}

if not os.path.exists(joinPath('out/threads')):
    os.makedirs(joinPath('out/threads'))

def threadFunction(i):
    indigo = Indigo()
    renderer = IndigoRenderer(indigo)
    mol = indigo.loadMolecule("C")
    indigo.setOption("render-output-format", ext)
    renderer.renderToFile(mol, joinPath('out/threads/thread_%s.%s' % (i, ext)))
    f =  open(joinPath('out/threads/thread_%s.%s' % (i, ext)), 'r')
    a[i] = f.read()
    f.close()

def runThreads():
    threads_list = []
    for i in range(thread_count):
        t = Thread(target=threadFunction, args=(i,))    
        t.start()
        threads_list.append(t)
            
    for t in threads_list:
        t.join()
    
    for i in range(1, thread_count):
        if a[i] != a[i - 1]:
            result = unified_diff(a[i].splitlines(), a[i-1].splitlines())
            print(''.join(result))

runThreads()