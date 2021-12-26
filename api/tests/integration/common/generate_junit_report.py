import sys
from itertools import groupby
from xml.etree.ElementTree import Element, tostring


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


def create_test_case(group_name, name, time, status, msg):
    test_case = Element(
        "testcase", name=name, attrib={"time": time, "classname": group_name}
    )
    if status != "[PASSED]":
        if status in ("[TODO]", "[NEW]"):
            error_type = "system-out"
            failure = Element(error_type)
            failure.text = msg
        else:
            error_type = (
                "failure" if status.startswith("[FAILED]") else "error"
            )
            failure = Element(error_type, attrib={"message": status})
            failure.text = msg
        test_case.append(failure)
    return test_case


def generate_junit_report(test_results, report_filename):
    xml_report = Element("testsuites", attrib={"name": "indigo"})
    total_time = 0.0
    total_tests = 0
    total_errors = 0
    total_failures = 0

    for root, group in groupby(test_results, lambda x: x[0]):
        test_suite = Element("testsuite", attrib={"name": root})
        group_time = 0.0
        group_tests = 0
        group_errors = 0
        group_failures = 0

        for group_root, filename, status, msg, tspend in group:
            group_time += tspend
            group_tests += 1
            if status not in ("[PASSED]", "[TODO]"):
                if status.startswith("[FAILED]"):
                    group_failures += 1
                else:
                    group_errors += 1

            test_suite.append(
                create_test_case(
                    group_root,
                    filename.replace(".py", ""),
                    str(tspend),
                    status,
                    msg,
                )
            )
        test_suite.attrib["time"] = str(group_time)
        test_suite.attrib["tests"] = str(group_tests)
        test_suite.attrib["errors"] = str(group_errors)
        test_suite.attrib["failures"] = str(group_failures)

        xml_report.append(test_suite)
        total_time += group_time
        total_tests += group_tests
        total_errors += group_errors
        total_failures += group_failures

    xml_report.attrib["time"] = str(total_time)
    xml_report.attrib["tests"] = str(total_tests)
    xml_report.attrib["errors"] = str(total_errors)
    xml_report.attrib["failures"] = str(total_failures)

    indent(xml_report)
    report_output = tostring(
        xml_report, encoding="utf-8" if sys.version_info[0] < 3 else "unicode"
    )
    f = open(report_filename, "wt")
    f.write("<?xml version='1.0' encoding='UTF-8'?>\n")
    f.write(report_output)
    f.close()
