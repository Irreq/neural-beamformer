#!/usr/bin/python3

"""Script for converting JSON file to config file in a specific language"""

import json

import datetime

import sys


USAGE ="Usage: python3 config_maker.py /path/to/config.json /path/to/config.*"


def eval_expression(expr: str, table: dict):
    items = expr.split(" ")

    if len(items) == 1:
        first = items[0]

        if first in table:
            new_value = str(table[first])
            item = eval_expression(new_value, table)
        else:
            item = first

        return f"{item} "
    else:
        return f"{eval_expression(items[0], table)} " \
               f'{eval_expression(" ".join(items[1:]), table)}'


class ConfigBuilder(object):
    def __init__(self, file):
        self.file = file

    def is_expression(self, value):
        return isinstance(value, str)

    def write(self, json_data):
        with open(self.file, "w+") as config:
            config.write(self.header())
            for key, value in json_data.items():
                if self.is_expression(value):
                    try:
                        output = eval(eval_expression(value, json_data))

                    except NameError:
                        output = f'"{value}"'
                else:
                    output = json.dumps(value)

                config.write(self.data(key, output))

            config.write(self.footer())

            config.close()

    def header(self):
        return "#This is default header\n"
    def data(self, key: str, output: object) -> str:
        return f"{key} = {output}\n"
    def footer(self):
        return "#This is default footer\n"
    

class C(ConfigBuilder):
    def __init__(self, file):
        super().__init__(file)

    def header(self):
        now = datetime.datetime.now() #.isoformat()
        return  f"""
#ifndef CONFIG_H
#define CONFIG_H

// NOTICE!

// DO NOT EDIT THIS FILE!
// THIS FILE IS AUTO-GENERATED BY A SCRIPT. IF YOU WANT TO EDIT THE CONFIGURATIONS
// CHANGE THE VALUES IN: "config.json".

// GENERATED: {now}

"""

    def data(self, key: str, output: object) -> str:
        return f"#define {key} {output}\n"
    
    def footer(self):
        return "\n#endif\n"
    
class Python(ConfigBuilder):
    def __init__(self, file):
        super().__init__(file)

    def header(self):
        now = datetime.datetime.now() #.isoformat()
        return  f"""#!/usr/bin/python3

# NOTICE!

# DO NOT EDIT THIS FILE!
# THIS FILE IS AUTO-GENERATED BY A SCRIPT. IF YOU WANT TO EDIT THE CONFIGURATIONS
# CHANGE THE VALUES IN: "config.json".

# GENERATED: {now}

"""

    def data(self, key: str, output: object) -> str:
        return f"{key} = {output}\n"
    
    def footer(self):
        return ""

class Cython(ConfigBuilder):
    def __init__(self, file):
        super().__init__(file)

    def header(self):
        now = datetime.datetime.now() #.isoformat()
        return  f"""# cython: language_level=3
# distutils: language=c

# NOTICE!

# DO NOT EDIT THIS FILE!
# THIS FILE IS AUTO-GENERATED BY A SCRIPT. IF YOU WANT TO EDIT THE CONFIGURATIONS
# CHANGE THE VALUES IN: "config.json".

# GENERATED: {now}

"""

    def data(self, key: str, output: object) -> str:
        return f"DEF {key} = {output}\n"
    
    def footer(self):
        return ""


if __name__ == '__main__':
    import argparse
    parser = argparse.ArgumentParser(description="Generate configuration files")

    parser.add_argument("in_file", help="target config file")
    parser.add_argument("out", help="output directory")

    parser.add_argument("--python", action="store_true", help="generate python config file")
    parser.add_argument("--cython", action="store_true", help="generate cython config file")
    parser.add_argument("--c", action="store_true", help="generate c config file")

    # Parse the command line arguments
    args = parser.parse_args()

    print(args.in_file)

    with open(args.in_file, "r") as json_file:
        json_data = json.load(json_file)

    languages = {
            "python": (Python, "/config.py"),
            "cython": (Cython, "/config.pxd"),
            "c": (C, "/config.h")
            }

    for (language, (writer, path)) in languages.items():
        if getattr(args, language, False):
            print(f"Generating {language} configuration at: {args.out}{path}")
            writer(args.out+path).write(json_data)

