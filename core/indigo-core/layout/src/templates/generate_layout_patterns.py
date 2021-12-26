import os


def process_ctab(fname, input, output, replace_header=True):
    content = input.split("\n")
    # replace first line with file name
    if replace_header:
        content[0] = fname
    lines = ['  "' + line.strip("\n").strip("\r") + '\\n"' for line in content]

    output.write("\n".join(lines))
    output.write(",\n\n")


names = sorted(list(os.listdir("molecules")), key=str.lower)

with open("layout_patterns.inc", "w") as templates_file:
    templates_file.write("// Templates from the following files:\n")
    for fname in names:
        templates_file.write("//   {}\n".format(fname))

    templates_file.write("static const char* layout_templates[] =\n")
    templates_file.write("{\n")

    for fname in names:
        if fname.endswith("mol"):
            print(fname)
            with open(os.path.join("molecules", fname)) as content:
                process_ctab(fname, content.read(), templates_file)
        elif fname.endswith("sdf"):
            print(fname)
            with open(os.path.join("molecules", fname)) as sdf:
                reader = sdf.read()
                for i, content in enumerate(reader.split("$$$$")):
                    # print(content)
                    process_ctab(fname, content.strip(), templates_file, False)
        else:
            print("unknown molecule format")

    templates_file.write("};\n")
