import sys
from threading import Thread
from difflib import unified_diff
import errno

sys.path.append('../../common')
from env_indigo import *

ext = 'svg'
thread_count = 100
a = {}

if not os.path.exists(joinPath("out/threads")):
    try:
        os.makedirs(joinPath("out/threads"))
    except OSError as e:
        if e.errno != errno.EEXIST:
            raise


def threadFunction(i):
    indigo = Indigo()
    renderer = IndigoRenderer(indigo)
    mol = indigo.loadMolecule("C")
    indigo.setOption("render-output-format", ext)
    renderer.renderToFile(mol, joinPath('out/threads/thread_%s.%s' % (i, ext)))
    with open(joinPath('out/threads/thread_%s.%s' % (i, ext)), 'r') as f:
        a[i] = f.read()
    renderer.Dispose()

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
            # check number of lines because SVG have different id in different iterations with the same meaning:
            # -<g id="surface45">
            # +<g id="surface13">
            if len(a[i].split("\n")) != len(a[i - 1].split("\n")):
                result = unified_diff(a[i].splitlines(), a[i-1].splitlines())
                print('\n'.join(result))

runThreads()
