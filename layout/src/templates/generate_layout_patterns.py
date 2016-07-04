import os

templates_file = open('layout_patterns.inc', 'w')

names = list(os.listdir('molecules'))

templates_file.write('// Templates from the following files:\n')
for fname in names:
    templates_file.write('//   {}\n'.format(fname))

templates_file.write('static const char* layout_templates[] =\n')
templates_file.write('{\n')

for fname in names:
    content = open(os.path.join('molecules', fname)).read().split("\n")
    # replace first line with file name
    content[0] = fname
    lines = ['  "' + line + '\\n"' for line in content]

    templates_file.write('\n'.join(lines))
    templates_file.write(',\n\n')

templates_file.write("};\n")
