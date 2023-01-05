# Copyright 2016 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Wrapper around actool to compile assets catalog.

The script compile_xcassets.py is a wrapper around actool to compile
assets catalog to Assets.car that turns warning into errors. It also
fixes some quirks of actool to make it work from ninja (mostly that
actool seems to require absolute path but gn generates command-line
with relative paths).

The wrapper filter out any message that is not a section header and
not a warning or error message, and fails if filtered output is not
empty. This should to treat all warnings as error until actool has
an option to fail with non-zero error code when there are warnings.
"""

import argparse
import os
import re
import shutil
import subprocess
import sys
import tempfile

# Pattern matching a section header in the output of actool.
SECTION_HEADER = re.compile('^/\\* ([^ ]*) \\*/$')

# Name of the section containing informational messages that can be ignored.
NOTICE_SECTION = 'com.apple.actool.compilation-results'

# Map special type of asset catalog to the corresponding command-line
# parameter that need to be passed to actool.
ACTOOL_FLAG_FOR_ASSET_TYPE = {
    '.appiconset': '--app-icon',
    '.launchimage': '--launch-image',
}

def FixAbsolutePathInLine(line, relative_paths):
  """Fix absolute paths present in |line| to relative paths."""
  absolute_path = line.split(':')[0]
  relative_path = relative_paths.get(absolute_path, absolute_path)
  if absolute_path == relative_path:
    return line
  return relative_path + line[len(absolute_path):]


def FilterCompilerOutput(compiler_output, relative_paths):
  """Filers actool compilation output.

  The compiler output is composed of multiple sections for each different
  level of output (error, warning, notices, ...). Each section starts with
  the section name on a single line, followed by all the messages from the
  section.

  The function filter any lines that are not in com.apple.actool.errors or
  com.apple.actool.document.warnings sections (as spurious messages comes
  before any section of the output).

  See crbug.com/730054, crbug.com/739163 and crbug.com/770634 for some example
  messages that pollute the output of actool and cause flaky builds.

  Args:
    compiler_output: string containing the output generated by the
      compiler (contains both stdout and stderr)
    relative_paths: mapping from absolute to relative paths used to
      convert paths in the warning and error messages (unknown paths
      will be left unaltered)

  Returns:
    The filtered output of the compiler. If the compilation was a
    success, then the output will be empty, otherwise it will use
    relative path and omit any irrelevant output.
  """

  filtered_output = []
  current_section = None
  data_in_section = False
  for line in compiler_output.splitlines():
    match = SECTION_HEADER.search(line)
    if match is not None:
      data_in_section = False
      current_section = match.group(1)
      continue
    if current_section and current_section != NOTICE_SECTION:
      if not data_in_section:
        data_in_section = True
        filtered_output.append('/* %s */\n' % current_section)

      fixed_line = FixAbsolutePathInLine(line, relative_paths)
      filtered_output.append(fixed_line + '\n')

  return ''.join(filtered_output)


def CompileAssetCatalog(output, platform, target_environment, product_type,
                        min_deployment_target, inputs, compress_pngs,
                        partial_info_plist):
  """Compile the .xcassets bundles to an asset catalog using actool.

  Args:
    output: absolute path to the containing bundle
    platform: the targeted platform
    product_type: the bundle type
    min_deployment_target: minimum deployment target
    inputs: list of absolute paths to .xcassets bundles
    compress_pngs: whether to enable compression of pngs
    partial_info_plist: path to partial Info.plist to generate
  """
  command = [
      'xcrun',
      'actool',
      '--output-format=human-readable-text',
      '--notices',
      '--warnings',
      '--errors',
      '--minimum-deployment-target',
      min_deployment_target,
  ]

  if compress_pngs:
    command.extend(['--compress-pngs'])

  if product_type != '':
    command.extend(['--product-type', product_type])

  if platform == 'mac':
    command.extend([
        '--platform',
        'macosx',
        '--target-device',
        'mac',
    ])
  elif platform == 'ios':
    if target_environment == 'simulator':
      command.extend([
          '--platform',
          'iphonesimulator',
          '--target-device',
          'iphone',
          '--target-device',
          'ipad',
      ])
    elif target_environment == 'device':
      command.extend([
          '--platform',
          'iphoneos',
          '--target-device',
          'iphone',
          '--target-device',
          'ipad',
      ])
    elif target_environment == 'catalyst':
      command.extend([
          '--platform',
          'macosx',
          '--target-device',
          'ipad',
          '--ui-framework-family',
          'uikit',
      ])

  # Scan the input directories for the presence of asset catalog types that
  # require special treatment, and if so, add them to the actool command-line.
  for relative_path in inputs:

    if not os.path.isdir(relative_path):
      continue

    for file_or_dir_name in os.listdir(relative_path):
      if not os.path.isdir(os.path.join(relative_path, file_or_dir_name)):
        continue

      asset_name, asset_type = os.path.splitext(file_or_dir_name)
      if asset_type not in ACTOOL_FLAG_FOR_ASSET_TYPE:
        continue

      command.extend([ACTOOL_FLAG_FOR_ASSET_TYPE[asset_type], asset_name])

  # Always ask actool to generate a partial Info.plist file. If no path
  # has been given by the caller, use a temporary file name.
  temporary_file = None
  if not partial_info_plist:
    temporary_file = tempfile.NamedTemporaryFile(suffix='.plist')
    partial_info_plist = temporary_file.name

  command.extend(['--output-partial-info-plist', partial_info_plist])

  # Dictionary used to convert absolute paths back to their relative form
  # in the output of actool.
  relative_paths = {}

  # actool crashes if paths are relative, so convert input and output paths
  # to absolute paths, and record the relative paths to fix them back when
  # filtering the output.
  absolute_output = os.path.abspath(output)
  relative_paths[output] = absolute_output
  relative_paths[os.path.dirname(output)] = os.path.dirname(absolute_output)
  command.extend(['--compile', os.path.dirname(os.path.abspath(output))])

  for relative_path in inputs:
    absolute_path = os.path.abspath(relative_path)
    relative_paths[absolute_path] = relative_path
    command.append(absolute_path)

  try:
    # Run actool and redirect stdout and stderr to the same pipe (as actool
    # is confused about what should go to stderr/stdout).
    process = subprocess.Popen(command,
                               stdout=subprocess.PIPE,
                               stderr=subprocess.STDOUT)
    stdout = process.communicate()[0].decode('utf-8')

    # If the invocation of `actool` failed, copy all the compiler output to
    # the standard error stream and exit. See https://crbug.com/1205775 for
    # example of compilation that failed with no error message due to filter.
    if process.returncode:
      for line in stdout.splitlines():
        fixed_line = FixAbsolutePathInLine(line, relative_paths)
        sys.stderr.write(fixed_line + '\n')
      sys.exit(1)

    # Filter the output to remove all garbage and to fix the paths. If the
    # output is not empty after filtering, then report the compilation as a
    # failure (as some version of `actool` report error to stdout, yet exit
    # with an return code of zero).
    stdout = FilterCompilerOutput(stdout, relative_paths)
    if stdout:
      sys.stderr.write(stdout)
      sys.exit(1)

  finally:
    if temporary_file:
      temporary_file.close()


def Main():
  parser = argparse.ArgumentParser(
      description='compile assets catalog for a bundle')
  parser.add_argument('--platform',
                      '-p',
                      required=True,
                      choices=('mac', 'ios'),
                      help='target platform for the compiled assets catalog')
  parser.add_argument('--target-environment',
                      '-e',
                      default='',
                      choices=('simulator', 'device', 'catalyst'),
                      help='target environment for the compiled assets catalog')
  parser.add_argument(
      '--minimum-deployment-target',
      '-t',
      required=True,
      help='minimum deployment target for the compiled assets catalog')
  parser.add_argument('--output',
                      '-o',
                      required=True,
                      help='path to the compiled assets catalog')
  parser.add_argument('--compress-pngs',
                      '-c',
                      action='store_true',
                      default=False,
                      help='recompress PNGs while compiling assets catalog')
  parser.add_argument('--product-type',
                      '-T',
                      help='type of the containing bundle')
  parser.add_argument('--partial-info-plist',
                      '-P',
                      help='path to partial info plist to create')
  parser.add_argument('inputs',
                      nargs='+',
                      help='path to input assets catalog sources')
  args = parser.parse_args()

  if os.path.basename(args.output) != 'Assets.car':
    sys.stderr.write('output should be path to compiled asset catalog, not '
                     'to the containing bundle: %s\n' % (args.output, ))
    sys.exit(1)

  if os.path.exists(args.output):
    if os.path.isfile(args.output):
      os.unlink(args.output)
    else:
      shutil.rmtree(args.output)

  CompileAssetCatalog(args.output, args.platform, args.target_environment,
                      args.product_type, args.minimum_deployment_target,
                      args.inputs, args.compress_pngs, args.partial_info_plist)


if __name__ == '__main__':
  sys.exit(Main())