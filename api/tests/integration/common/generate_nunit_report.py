from itertools import groupby
from xml.etree.ElementTree import Element, tostring
import sys

def indent(elem, level=0):
    i = "\n" + level * "  "
    if len(elem):
        if not elem.text or not elem.text.strip():
            elem.text = i + "  "
        if not elem.tail or not elem.tail.strip():
            elem.tail = i
        for elem in elem:
            indent(elem, level + 1)
        if not elem.tail or not elem.tail.strip():
            elem.tail = i
    else:
        if level and (not elem.tail or not elem.tail.strip()):
            elem.tail = i


def createTestCase(name, time, status):
    testCase = Element('test-case', name=name, attrib={'time': time})
    if status != "[PASSED]":
        failure = Element('failure')
        message = Element('message')
        message.text = status
        failure.append(message)
        testCase.append(failure)
    return testCase


def generateNUnitReport(testResults, reportFilename):
    xmlReport = Element('test-results')
    mainTestSuite = Element('test-suite', name='Indigo')

    for root, group in groupby(testResults, lambda x: x[0]):
        testSuite = Element('test-suite', attrib={'name': "Indigo." + root})
        results = Element('results')

        for root, filename, status, msg, tspend in group:
            results.append(createTestCase(root + "." + filename, str(tspend), status))

        testSuite.append(results)
        mainTestSuite.append(testSuite)

    xmlReport.append(mainTestSuite)

    indent(xmlReport)
    reportOutput = tostring(xmlReport, encoding='utf-8' if sys.version_info[0] < 3 else 'unicode')
    f = open(reportFilename, 'wt')
    f.write("<?xml version='1.0' encoding='UTF-8'?>\n")
    f.write(reportOutput)
    f.close()
