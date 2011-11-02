from itertools import groupby
from xml.dom.minidom import Document

def createTestSuite(doc, name):
   test_suite = doc.createElement("test-suite")
   test_suite.setAttribute("name", name)
   results = doc.createElement("results")
   test_suite.appendChild(results)
   return test_suite, results

def createTestCase(doc, name, time, status):
   test_case = doc.createElement("test-case")
   test_case.setAttribute("name", name)
   test_case.setAttribute("time", time)
   if status != "[PASSED]":
      failure = doc.createElement("failure")
      message = doc.createElement("message")
      message_text = doc.createTextNode(status)
      message.appendChild(message_text)
      failure.appendChild(message)
      test_case.appendChild(failure)
   return test_case

   
def generateNUnitReport(test_results, report_filename):
   doc = Document()

   xml_test_results = doc.createElement("test-results")
   doc.appendChild(xml_test_results)

   test_suite, test_suite_results = createTestSuite(doc, "Indigo")
   xml_test_results.appendChild(test_suite)
   
   for root, group in groupby(test_results, lambda x: x[0]):
      sub_test_suite, sub_test_suite_results = createTestSuite(doc, "Indigo." + root)
      test_suite_results.appendChild(sub_test_suite)
      
      for root, filename, msg, tspend in group:
         sub_test_suite_results.appendChild(createTestCase(doc, root + "." + filename, str(tspend), msg))
   
   open(report_filename, "w").write(doc.toprettyxml(indent="  "))   