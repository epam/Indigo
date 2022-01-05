with open("abbreviations.xml") as inf:
    with open("abbreviations.inc", "w") as outf:
        outf.write("static const char *default_abbreviations_xml = ")
        for line in inf:
            outf.write('\n  "%s\\n"' % line[:-1].replace('"', '\\"'))
        outf.write(";\n")
