#!/bin/bash

# Copyright 2023 Google Inc. All rights reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# Script to generate `gn desc` json outputs that are used as an input to the
# gn2bp conversion tool.
# Inputs:
#  Arguments:
#   -d dir: The directory that points to a local checkout of chromium/src.
#   -r rev: The reference revision of upstream Chromium to use. Must match the
#           last revision that has been imported using import_cronet.sh.

set -e -x

OPTSTRING=d:r:

usage() {
    cat <<EOF
Usage: gen_gn_desc.sh -d dir -r rev
EOF
    exit 1
}


# Run this script inside a full chromium checkout.

OUT_PATH="out/cronet"

#######################################
# Apply patches in external/cronet.
# Globals:
#   ANDROID_BUILD_TOP
# Arguments:
#   chromium_dir, string
#######################################
function apply_patches() (
  local -r chromium_dir=$1
  local -r patch_root="${ANDROID_BUILD_TOP}/external/cronet/patches"

  cd "${chromium_dir}"

  local upstream_patches
  upstream_patches=$(ls "${patch_root}/upstream-next")
  local patch
  for patch in ${upstream_patches}; do
    git am --3way "${patch_root}/upstream-next/${patch}"
  done

  local local_patches
  local_patches=$(ls "${patch_root}/local")
  for patch in ${local_patches}; do
    git am --3way "${patch_root}/local/${patch}"
  done
)

#######################################
# Generate desc.json for a specified architecture.
# Globals:
#   OUT_PATH
# Arguments:
#   target_cpu, string
#   chromium_dir, string
#######################################
function gn_desc() (
  local -r target_cpu="$1"
  local -r chromium_dir="$2"
  local -a gn_args=(
    "target_os = \"android\""
    "enable_websockets = false"
    "disable_file_support = true"
    "is_component_build = false"
    "use_crash_key_stubs = true"
    "use_partition_alloc = false"
    "include_transport_security_state_preload_list = false"
    "use_platform_icu_alternatives = true"
    "default_min_sdk_version = 19"
    "enable_reporting = true"
    "use_hashed_jni_names = true"
    "enable_base_tracing = false"
    "is_cronet_build = true"
    "is_debug = false"
    "is_official_build = true"
    "use_nss_certs = false"
    "clang_use_default_sample_profile = false"
    "media_use_ffmpeg=false"
    "use_thin_lto=false"
    "enable_resource_allowlist_generation=false"
    "enable_jdk_library_desugaring=false"
    "exclude_unwind_tables=true"
    "symbol_level=1"
  )
  gn_args+=("target_cpu = \"${target_cpu}\"")

  # Only set arm_use_neon on arm architectures to prevent warning from being
  # written to json output.
  if [[ "${target_cpu}" = "arm" ]]; then
    gn_args+=("arm_use_neon = false")
  fi

  cd "${chromium_dir}"

  # Configure gn args.
  gn gen "${OUT_PATH}" --args="${gn_args[*]}"

  # Generate desc.json.
  local -r out_file="desc_${target_cpu}.json"
  gn desc "${OUT_PATH}" --format=json --all-toolchains "//*" > "${out_file}"
)

while getopts "${OPTSTRING}" opt; do
  case "${opt}" in
    d) chromium_dir="${OPTARG}" ;;
    r) rev="${OPTARG}" ;;
    ?) usage ;;
    *) echo "'${opt}' '${OPTARG}'"
  esac
done

if [ -z "${chromium_dir}" ]; then
  echo "-d argument required"
  usage
fi

if [ -z "${rev}" ]; then
  echo "-r argument required"
  usage
fi

apply_patches "${chromium_dir}"
gn_desc x86 "${chromium_dir}"
gn_desc x64 "${chromium_dir}"
gn_desc arm "${chromium_dir}"
gn_desc arm64 "${chromium_dir}"

