#
# Copyright (C) 2024 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

""" This script collects all of the classes fully-qualified path provided in a JAR, applies
post-processing then bundles that list in a java-compilable file."""

import argparse
from pathlib import Path
import re
from zipfile import ZipFile


def parse_arguments(argv):
    parser = argparse.ArgumentParser()
    parser.add_argument(
        'jars', nargs='+',
        help='Path to JARs. Multiple jars can be specified.')
    parser.add_argument(
        '--prefix', required=True,
        help='Package prefix to use to append to the classes, '
             'for example "com.android.connectivity" (does not end with a dot).')
    parser.add_argument(
        '--output', required=True, help='Path to output java-compilable file.')
    parser.add_argument(
        '--prefix-excludes', action='append', default=[],
        help='Path to files listing classes that should not have their package prefixed. '
             'Can be repeated to specify multiple files.'
             'Each file should contain one full-match regex per line. Empty lines or lines '
             'starting with "#" are ignored.')
    parser.add_argument(
        '--excludes', action='append', default=[],
        help='Path to files listing classes that should be excluded '
             'Can be repeated to specify multiple files.'
             'Each file should contain one full-match regex per line. Empty lines or lines '
             'starting with "#" are ignored.')
    parser.add_argument(
        '--java-package', default="android.net.http",
        help='The package name of the generated java file.')
    return parser.parse_args(argv)


def _list_jar_classes(jar):
    with ZipFile(jar, 'r') as zip:
        files = zip.namelist()
        assert 'classes.dex' not in files, f'Jar file {jar} is dexed, ' \
                                           'expected an intermediate zip of .class files'
        class_suffix = '.class'
        return [f.removesuffix(class_suffix).replace('/', '.') for f in files
                if f.endswith(class_suffix) and not f.endswith('/package-info.class')]


def _get_excludes(path):
    out = []
    with open(path, 'r') as f:
        for line in f:
            stripped = line.strip()
            if not stripped or stripped.startswith('#'):
                continue
            out.append(re.compile(stripped))
    return out


def _write_classes_list(package_name, output_path, classes_list):
    with open(output_path, 'w') as outfile:
        outfile.write(
            f"""
package {package_name};

public class {Path(output_path).stem} {{
    public static final String[] ALL_CLASSES = {{{",".join(classes_list)}}};
}}
""")


def make_classes_list(args):
    exclude_regexes = []
    for exclude_file in args.prefix_excludes:
        exclude_regexes.extend(_get_excludes(exclude_file))

    exclude_collecting_regexes = []
    for exclude_file in args.excludes:
        exclude_collecting_regexes.extend(_get_excludes(exclude_file))

    processed_classes = []
    for jar in args.jars:
        jar_classes = _list_jar_classes(jar)
        jar_classes.sort()
        for clazz in jar_classes:
            if any(r.fullmatch(clazz) for r in exclude_collecting_regexes):
                continue

            if (clazz.startswith(args.prefix + '.') or
                    any(r.fullmatch(clazz) for r in exclude_regexes)):
                processed_classes.append(f'"{clazz}"')
            else:
                processed_classes.append(f'"{args.prefix}.{clazz}"')
    return processed_classes


def _main():
    # Pass in None to use argv
    args = parse_arguments(None)
    _write_classes_list(args.java_package, args.output, make_classes_list(args))


if __name__ == '__main__':
    _main()
