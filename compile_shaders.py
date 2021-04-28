""" Compiles all shader files within the folder to a single C header file """

import os
import sys

_, folder = sys.argv
with open(f"{os.path.dirname(folder)}.h", "w") as ofile:
    print(ofile.name)
    for path, _, files in os.walk(folder):
        for file in files:
            with open(os.path.join(path, file), "r") as ifile:
                print(ifile.name)
                data = [f"// {ifile.name}\nconst char *{file.replace('.', '_')} =\n"] \
                    + [f"\"{line}\\n\"\n" for line in ifile.read().splitlines()] \
                    + ["\"\\0\";\n"]
                ofile.writelines(data)
