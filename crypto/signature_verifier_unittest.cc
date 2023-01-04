// Copyright 2011 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "crypto/signature_verifier.h"

#include <stddef.h>
#include <stdint.h>

#include "base/numerics/safe_conversions.h"
#include "testing/gtest/include/gtest/gtest.h"

TEST(SignatureVerifierTest, BasicTest) {
  // The input data in this test comes from real certificates.
  //
  // tbs_certificate ("to-be-signed certificate", the part of a certificate that
  // is signed), signature, and algorithm come from the certificate of
  // bugs.webkit.org.
  //
  // public_key_info comes from the certificate of the issuer, Go Daddy Secure
  // Certification Authority.
  //
  // The bytes in the array initializers are formatted to expose the DER
  // encoding of the ASN.1 structures.

  // The data that is signed is the following ASN.1 structure:
  //    TBSCertificate  ::=  SEQUENCE  {
  //        ...  -- omitted, not important
  //        }
  const uint8_t tbs_certificate[1017] = {
      0x30, 0x82, 0x03, 0xf5,  // a SEQUENCE of length 1013 (0x3f5)
      0xa0, 0x03, 0x02, 0x01, 0x02, 0x02, 0x03, 0x43, 0xdd, 0x63, 0x30, 0x0d,
      0x06, 0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x01, 0x05, 0x05,
      0x00, 0x30, 0x81, 0xca, 0x31, 0x0b, 0x30, 0x09, 0x06, 0x03, 0x55, 0x04,
      0x06, 0x13, 0x02, 0x55, 0x53, 0x31, 0x10, 0x30, 0x0e, 0x06, 0x03, 0x55,
      0x04, 0x08, 0x13, 0x07, 0x41, 0x72, 0x69, 0x7a, 0x6f, 0x6e, 0x61, 0x31,
      0x13, 0x30, 0x11, 0x06, 0x03, 0x55, 0x04, 0x07, 0x13, 0x0a, 0x53, 0x63,
      0x6f, 0x74, 0x74, 0x73, 0x64, 0x61, 0x6c, 0x65, 0x31, 0x1a, 0x30, 0x18,
      0x06, 0x03, 0x55, 0x04, 0x0a, 0x13, 0x11, 0x47, 0x6f, 0x44, 0x61, 0x64,
      0x64, 0x79, 0x2e, 0x63, 0x6f, 0x6d, 0x2c, 0x20, 0x49, 0x6e, 0x63, 0x2e,
      0x31, 0x33, 0x30, 0x31, 0x06, 0x03, 0x55, 0x04, 0x0b, 0x13, 0x2a, 0x68,
      0x74, 0x74, 0x70, 0x3a, 0x2f, 0x2f, 0x63, 0x65, 0x72, 0x74, 0x69, 0x66,
      0x69, 0x63, 0x61, 0x74, 0x65, 0x73, 0x2e, 0x67, 0x6f, 0x64, 0x61, 0x64,
      0x64, 0x79, 0x2e, 0x63, 0x6f, 0x6d, 0x2f, 0x72, 0x65, 0x70, 0x6f, 0x73,
      0x69, 0x74, 0x6f, 0x72, 0x79, 0x31, 0x30, 0x30, 0x2e, 0x06, 0x03, 0x55,
      0x04, 0x03, 0x13, 0x27, 0x47, 0x6f, 0x20, 0x44, 0x61, 0x64, 0x64, 0x79,
      0x20, 0x53, 0x65, 0x63, 0x75, 0x72, 0x65, 0x20, 0x43, 0x65, 0x72, 0x74,
      0x69, 0x66, 0x69, 0x63, 0x61, 0x74, 0x69, 0x6f, 0x6e, 0x20, 0x41, 0x75,
      0x74, 0x68, 0x6f, 0x72, 0x69, 0x74, 0x79, 0x31, 0x11, 0x30, 0x0f, 0x06,
      0x03, 0x55, 0x04, 0x05, 0x13, 0x08, 0x30, 0x37, 0x39, 0x36, 0x39, 0x32,
      0x38, 0x37, 0x30, 0x1e, 0x17, 0x0d, 0x30, 0x38, 0x30, 0x33, 0x31, 0x38,
      0x32, 0x33, 0x33, 0x35, 0x31, 0x39, 0x5a, 0x17, 0x0d, 0x31, 0x31, 0x30,
      0x33, 0x31, 0x38, 0x32, 0x33, 0x33, 0x35, 0x31, 0x39, 0x5a, 0x30, 0x79,
      0x31, 0x0b, 0x30, 0x09, 0x06, 0x03, 0x55, 0x04, 0x06, 0x13, 0x02, 0x55,
      0x53, 0x31, 0x13, 0x30, 0x11, 0x06, 0x03, 0x55, 0x04, 0x08, 0x13, 0x0a,
      0x43, 0x61, 0x6c, 0x69, 0x66, 0x6f, 0x72, 0x6e, 0x69, 0x61, 0x31, 0x12,
      0x30, 0x10, 0x06, 0x03, 0x55, 0x04, 0x07, 0x13, 0x09, 0x43, 0x75, 0x70,
      0x65, 0x72, 0x74, 0x69, 0x6e, 0x6f, 0x31, 0x13, 0x30, 0x11, 0x06, 0x03,
      0x55, 0x04, 0x0a, 0x13, 0x0a, 0x41, 0x70, 0x70, 0x6c, 0x65, 0x20, 0x49,
      0x6e, 0x63, 0x2e, 0x31, 0x15, 0x30, 0x13, 0x06, 0x03, 0x55, 0x04, 0x0b,
      0x13, 0x0c, 0x4d, 0x61, 0x63, 0x20, 0x4f, 0x53, 0x20, 0x46, 0x6f, 0x72,
      0x67, 0x65, 0x31, 0x15, 0x30, 0x13, 0x06, 0x03, 0x55, 0x04, 0x03, 0x13,
      0x0c, 0x2a, 0x2e, 0x77, 0x65, 0x62, 0x6b, 0x69, 0x74, 0x2e, 0x6f, 0x72,
      0x67, 0x30, 0x81, 0x9f, 0x30, 0x0d, 0x06, 0x09, 0x2a, 0x86, 0x48, 0x86,
      0xf7, 0x0d, 0x01, 0x01, 0x01, 0x05, 0x00, 0x03, 0x81, 0x8d, 0x00, 0x30,
      0x81, 0x89, 0x02, 0x81, 0x81, 0x00, 0xa7, 0x62, 0x79, 0x41, 0xda, 0x28,
      0xf2, 0xc0, 0x4f, 0xe0, 0x25, 0xaa, 0xa1, 0x2e, 0x3b, 0x30, 0x94, 0xb5,
      0xc9, 0x26, 0x3a, 0x1b, 0xe2, 0xd0, 0xcc, 0xa2, 0x95, 0xe2, 0x91, 0xc0,
      0xf0, 0x40, 0x9e, 0x27, 0x6e, 0xbd, 0x6e, 0xde, 0x7c, 0xb6, 0x30, 0x5c,
      0xb8, 0x9b, 0x01, 0x2f, 0x92, 0x04, 0xa1, 0xef, 0x4a, 0xb1, 0x6c, 0xb1,
      0x7e, 0x8e, 0xcd, 0xa6, 0xf4, 0x40, 0x73, 0x1f, 0x2c, 0x96, 0xad, 0xff,
      0x2a, 0x6d, 0x0e, 0xba, 0x52, 0x84, 0x83, 0xb0, 0x39, 0xee, 0xc9, 0x39,
      0xdc, 0x1e, 0x34, 0xd0, 0xd8, 0x5d, 0x7a, 0x09, 0xac, 0xa9, 0xee, 0xca,
      0x65, 0xf6, 0x85, 0x3a, 0x6b, 0xee, 0xe4, 0x5c, 0x5e, 0xf8, 0xda, 0xd1,
      0xce, 0x88, 0x47, 0xcd, 0x06, 0x21, 0xe0, 0xb9, 0x4b, 0xe4, 0x07, 0xcb,
      0x57, 0xdc, 0xca, 0x99, 0x54, 0xf7, 0x0e, 0xd5, 0x17, 0x95, 0x05, 0x2e,
      0xe9, 0xb1, 0x02, 0x03, 0x01, 0x00, 0x01, 0xa3, 0x82, 0x01, 0xce, 0x30,
      0x82, 0x01, 0xca, 0x30, 0x09, 0x06, 0x03, 0x55, 0x1d, 0x13, 0x04, 0x02,
      0x30, 0x00, 0x30, 0x0b, 0x06, 0x03, 0x55, 0x1d, 0x0f, 0x04, 0x04, 0x03,
      0x02, 0x05, 0xa0, 0x30, 0x1d, 0x06, 0x03, 0x55, 0x1d, 0x25, 0x04, 0x16,
      0x30, 0x14, 0x06, 0x08, 0x2b, 0x06, 0x01, 0x05, 0x05, 0x07, 0x03, 0x01,
      0x06, 0x08, 0x2b, 0x06, 0x01, 0x05, 0x05, 0x07, 0x03, 0x02, 0x30, 0x57,
      0x06, 0x03, 0x55, 0x1d, 0x1f, 0x04, 0x50, 0x30, 0x4e, 0x30, 0x4c, 0xa0,
      0x4a, 0xa0, 0x48, 0x86, 0x46, 0x68, 0x74, 0x74, 0x70, 0x3a, 0x2f, 0x2f,
      0x63, 0x65, 0x72, 0x74, 0x69, 0x66, 0x69, 0x63, 0x61, 0x74, 0x65, 0x73,
      0x2e, 0x67, 0x6f, 0x64, 0x61, 0x64, 0x64, 0x79, 0x2e, 0x63, 0x6f, 0x6d,
      0x2f, 0x72, 0x65, 0x70, 0x6f, 0x73, 0x69, 0x74, 0x6f, 0x72, 0x79, 0x2f,
      0x67, 0x6f, 0x64, 0x61, 0x64, 0x64, 0x79, 0x65, 0x78, 0x74, 0x65, 0x6e,
      0x64, 0x65, 0x64, 0x69, 0x73, 0x73, 0x75, 0x69, 0x6e, 0x67, 0x33, 0x2e,
      0x63, 0x72, 0x6c, 0x30, 0x52, 0x06, 0x03, 0x55, 0x1d, 0x20, 0x04, 0x4b,
      0x30, 0x49, 0x30, 0x47, 0x06, 0x0b, 0x60, 0x86, 0x48, 0x01, 0x86, 0xfd,
      0x6d, 0x01, 0x07, 0x17, 0x02, 0x30, 0x38, 0x30, 0x36, 0x06, 0x08, 0x2b,
      0x06, 0x01, 0x05, 0x05, 0x07, 0x02, 0x01, 0x16, 0x2a, 0x68, 0x74, 0x74,
      0x70, 0x3a, 0x2f, 0x2f, 0x63, 0x65, 0x72, 0x74, 0x69, 0x66, 0x69, 0x63,
      0x61, 0x74, 0x65, 0x73, 0x2e, 0x67, 0x6f, 0x64, 0x61, 0x64, 0x64, 0x79,
      0x2e, 0x63, 0x6f, 0x6d, 0x2f, 0x72, 0x65, 0x70, 0x6f, 0x73, 0x69, 0x74,
      0x6f, 0x72, 0x79, 0x30, 0x7f, 0x06, 0x08, 0x2b, 0x06, 0x01, 0x05, 0x05,
      0x07, 0x01, 0x01, 0x04, 0x73, 0x30, 0x71, 0x30, 0x23, 0x06, 0x08, 0x2b,
      0x06, 0x01, 0x05, 0x05, 0x07, 0x30, 0x01, 0x86, 0x17, 0x68, 0x74, 0x74,
      0x70, 0x3a, 0x2f, 0x2f, 0x6f, 0x63, 0x73, 0x70, 0x2e, 0x67, 0x6f, 0x64,
      0x61, 0x64, 0x64, 0x79, 0x2e, 0x63, 0x6f, 0x6d, 0x30, 0x4a, 0x06, 0x08,
      0x2b, 0x06, 0x01, 0x05, 0x05, 0x07, 0x30, 0x02, 0x86, 0x3e, 0x68, 0x74,
      0x74, 0x70, 0x3a, 0x2f, 0x2f, 0x63, 0x65, 0x72, 0x74, 0x69, 0x66, 0x69,
      0x63, 0x61, 0x74, 0x65, 0x73, 0x2e, 0x67, 0x6f, 0x64, 0x61, 0x64, 0x64,
      0x79, 0x2e, 0x63, 0x6f, 0x6d, 0x2f, 0x72, 0x65, 0x70, 0x6f, 0x73, 0x69,
      0x74, 0x6f, 0x72, 0x79, 0x2f, 0x67, 0x64, 0x5f, 0x69, 0x6e, 0x74, 0x65,
      0x72, 0x6d, 0x65, 0x64, 0x69, 0x61, 0x74, 0x65, 0x2e, 0x63, 0x72, 0x74,
      0x30, 0x1d, 0x06, 0x03, 0x55, 0x1d, 0x0e, 0x04, 0x16, 0x04, 0x14, 0x48,
      0xdf, 0x60, 0x32, 0xcc, 0x89, 0x01, 0xb6, 0xdc, 0x2f, 0xe3, 0x73, 0xb5,
      0x9c, 0x16, 0x58, 0x32, 0x68, 0xa9, 0xc3, 0x30, 0x1f, 0x06, 0x03, 0x55,
      0x1d, 0x23, 0x04, 0x18, 0x30, 0x16, 0x80, 0x14, 0xfd, 0xac, 0x61, 0x32,
      0x93, 0x6c, 0x45, 0xd6, 0xe2, 0xee, 0x85, 0x5f, 0x9a, 0xba, 0xe7, 0x76,
      0x99, 0x68, 0xcc, 0xe7, 0x30, 0x23, 0x06, 0x03, 0x55, 0x1d, 0x11, 0x04,
      0x1c, 0x30, 0x1a, 0x82, 0x0c, 0x2a, 0x2e, 0x77, 0x65, 0x62, 0x6b, 0x69,
      0x74, 0x2e, 0x6f, 0x72, 0x67, 0x82, 0x0a, 0x77, 0x65, 0x62, 0x6b, 0x69,
      0x74, 0x2e, 0x6f, 0x72, 0x67};

  // RSA signature, a big integer in the big-endian byte order.
  const uint8_t signature[256] = {
      0x1e, 0x6a, 0xe7, 0xe0, 0x4f, 0xe7, 0x4d, 0xd0, 0x69, 0x7c, 0xf8, 0x8f,
      0x99, 0xb4, 0x18, 0x95, 0x36, 0x24, 0x0f, 0x0e, 0xa3, 0xea, 0x34, 0x37,
      0xf4, 0x7d, 0xd5, 0x92, 0x35, 0x53, 0x72, 0x76, 0x3f, 0x69, 0xf0, 0x82,
      0x56, 0xe3, 0x94, 0x7a, 0x1d, 0x1a, 0x81, 0xaf, 0x9f, 0xc7, 0x43, 0x01,
      0x64, 0xd3, 0x7c, 0x0d, 0xc8, 0x11, 0x4e, 0x4a, 0xe6, 0x1a, 0xc3, 0x01,
      0x74, 0xe8, 0x35, 0x87, 0x5c, 0x61, 0xaa, 0x8a, 0x46, 0x06, 0xbe, 0x98,
      0x95, 0x24, 0x9e, 0x01, 0xe3, 0xe6, 0xa0, 0x98, 0xee, 0x36, 0x44, 0x56,
      0x8d, 0x23, 0x9c, 0x65, 0xea, 0x55, 0x6a, 0xdf, 0x66, 0xee, 0x45, 0xe8,
      0xa0, 0xe9, 0x7d, 0x9a, 0xba, 0x94, 0xc5, 0xc8, 0xc4, 0x4b, 0x98, 0xff,
      0x9a, 0x01, 0x31, 0x6d, 0xf9, 0x2b, 0x58, 0xe7, 0xe7, 0x2a, 0xc5, 0x4d,
      0xbb, 0xbb, 0xcd, 0x0d, 0x70, 0xe1, 0xad, 0x03, 0xf5, 0xfe, 0xf4, 0x84,
      0x71, 0x08, 0xd2, 0xbc, 0x04, 0x7b, 0x26, 0x1c, 0xa8, 0x0f, 0x9c, 0xd8,
      0x12, 0x6a, 0x6f, 0x2b, 0x67, 0xa1, 0x03, 0x80, 0x9a, 0x11, 0x0b, 0xe9,
      0xe0, 0xb5, 0xb3, 0xb8, 0x19, 0x4e, 0x0c, 0xa4, 0xd9, 0x2b, 0x3b, 0xc2,
      0xca, 0x20, 0xd3, 0x0c, 0xa4, 0xff, 0x93, 0x13, 0x1f, 0xfc, 0xba, 0x94,
      0x93, 0x8c, 0x64, 0x15, 0x2e, 0x28, 0xa9, 0x55, 0x8c, 0x2c, 0x48, 0xd3,
      0xd3, 0xc1, 0x50, 0x69, 0x19, 0xe8, 0x34, 0xd3, 0xf1, 0x04, 0x9f, 0x0a,
      0x7a, 0x21, 0x87, 0xbf, 0xb9, 0x59, 0x37, 0x2e, 0xf4, 0x71, 0xa5, 0x3e,
      0xbe, 0xcd, 0x70, 0x83, 0x18, 0xf8, 0x8a, 0x72, 0x85, 0x45, 0x1f, 0x08,
      0x01, 0x6f, 0x37, 0xf5, 0x2b, 0x7b, 0xea, 0xb9, 0x8b, 0xa3, 0xcc, 0xfd,
      0x35, 0x52, 0xdd, 0x66, 0xde, 0x4f, 0x30, 0xc5, 0x73, 0x81, 0xb6, 0xe8,
      0x3c, 0xd8, 0x48, 0x8a};

  // The public key is specified as the following ASN.1 structure:
  //   SubjectPublicKeyInfo  ::=  SEQUENCE  {
  //       algorithm            AlgorithmIdentifier,
  //       subjectPublicKey     BIT STRING  }
  const uint8_t public_key_info[294] = {
      0x30, 0x82, 0x01, 0x22,  // a SEQUENCE of length 290 (0x122)
      // algorithm
      0x30, 0x0d,  // a SEQUENCE of length 13
      0x06, 0x09,  // an OBJECT IDENTIFIER of length 9
      0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x01, 0x01, 0x05,
      0x00,  // a NULL of length 0
      // subjectPublicKey
      0x03, 0x82, 0x01, 0x0f,  // a BIT STRING of length 271 (0x10f)
      0x00,                    // number of unused bits
      0x30, 0x82, 0x01, 0x0a,  // a SEQUENCE of length 266 (0x10a)
      // modulus
      0x02, 0x82, 0x01, 0x01,  // an INTEGER of length 257 (0x101)
      0x00, 0xc4, 0x2d, 0xd5, 0x15, 0x8c, 0x9c, 0x26, 0x4c, 0xec, 0x32, 0x35,
      0xeb, 0x5f, 0xb8, 0x59, 0x01, 0x5a, 0xa6, 0x61, 0x81, 0x59, 0x3b, 0x70,
      0x63, 0xab, 0xe3, 0xdc, 0x3d, 0xc7, 0x2a, 0xb8, 0xc9, 0x33, 0xd3, 0x79,
      0xe4, 0x3a, 0xed, 0x3c, 0x30, 0x23, 0x84, 0x8e, 0xb3, 0x30, 0x14, 0xb6,
      0xb2, 0x87, 0xc3, 0x3d, 0x95, 0x54, 0x04, 0x9e, 0xdf, 0x99, 0xdd, 0x0b,
      0x25, 0x1e, 0x21, 0xde, 0x65, 0x29, 0x7e, 0x35, 0xa8, 0xa9, 0x54, 0xeb,
      0xf6, 0xf7, 0x32, 0x39, 0xd4, 0x26, 0x55, 0x95, 0xad, 0xef, 0xfb, 0xfe,
      0x58, 0x86, 0xd7, 0x9e, 0xf4, 0x00, 0x8d, 0x8c, 0x2a, 0x0c, 0xbd, 0x42,
      0x04, 0xce, 0xa7, 0x3f, 0x04, 0xf6, 0xee, 0x80, 0xf2, 0xaa, 0xef, 0x52,
      0xa1, 0x69, 0x66, 0xda, 0xbe, 0x1a, 0xad, 0x5d, 0xda, 0x2c, 0x66, 0xea,
      0x1a, 0x6b, 0xbb, 0xe5, 0x1a, 0x51, 0x4a, 0x00, 0x2f, 0x48, 0xc7, 0x98,
      0x75, 0xd8, 0xb9, 0x29, 0xc8, 0xee, 0xf8, 0x66, 0x6d, 0x0a, 0x9c, 0xb3,
      0xf3, 0xfc, 0x78, 0x7c, 0xa2, 0xf8, 0xa3, 0xf2, 0xb5, 0xc3, 0xf3, 0xb9,
      0x7a, 0x91, 0xc1, 0xa7, 0xe6, 0x25, 0x2e, 0x9c, 0xa8, 0xed, 0x12, 0x65,
      0x6e, 0x6a, 0xf6, 0x12, 0x44, 0x53, 0x70, 0x30, 0x95, 0xc3, 0x9c, 0x2b,
      0x58, 0x2b, 0x3d, 0x08, 0x74, 0x4a, 0xf2, 0xbe, 0x51, 0xb0, 0xbf, 0x87,
      0xd0, 0x4c, 0x27, 0x58, 0x6b, 0xb5, 0x35, 0xc5, 0x9d, 0xaf, 0x17, 0x31,
      0xf8, 0x0b, 0x8f, 0xee, 0xad, 0x81, 0x36, 0x05, 0x89, 0x08, 0x98, 0xcf,
      0x3a, 0xaf, 0x25, 0x87, 0xc0, 0x49, 0xea, 0xa7, 0xfd, 0x67, 0xf7, 0x45,
      0x8e, 0x97, 0xcc, 0x14, 0x39, 0xe2, 0x36, 0x85, 0xb5, 0x7e, 0x1a, 0x37,
      0xfd, 0x16, 0xf6, 0x71, 0x11, 0x9a, 0x74, 0x30, 0x16, 0xfe, 0x13, 0x94,
      0xa3, 0x3f, 0x84, 0x0d, 0x4f,
      // public exponent
      0x02, 0x03,  // an INTEGER of length 3
      0x01, 0x00, 0x01};

  // We use the signature verifier to perform four signature verification
  // tests.
  crypto::SignatureVerifier verifier;

  // Test  1: feed all of the data to the verifier at once (a single
  // VerifyUpdate call).
  EXPECT_TRUE(verifier.VerifyInit(crypto::SignatureVerifier::RSA_PKCS1_SHA1,
                                  signature, public_key_info));
  verifier.VerifyUpdate(tbs_certificate);
  EXPECT_TRUE(verifier.VerifyFinal());

  // Test 2: feed the data to the verifier in three parts (three VerifyUpdate
  // calls).
  EXPECT_TRUE(verifier.VerifyInit(crypto::SignatureVerifier::RSA_PKCS1_SHA1,
                                  signature, public_key_info));
  verifier.VerifyUpdate(base::make_span(tbs_certificate, 256));
  verifier.VerifyUpdate(base::make_span(tbs_certificate + 256, 256));
  verifier.VerifyUpdate(
      base::make_span(tbs_certificate + 512, sizeof(tbs_certificate) - 512));
  EXPECT_TRUE(verifier.VerifyFinal());

  // Test 3: verify the signature with incorrect data.
  uint8_t bad_tbs_certificate[sizeof(tbs_certificate)];
  memcpy(bad_tbs_certificate, tbs_certificate, sizeof(tbs_certificate));
  bad_tbs_certificate[10] += 1;  // Corrupt one byte of the data.
  EXPECT_TRUE(verifier.VerifyInit(crypto::SignatureVerifier::RSA_PKCS1_SHA1,
                                  signature, public_key_info));
  verifier.VerifyUpdate(bad_tbs_certificate);
  EXPECT_FALSE(verifier.VerifyFinal());

  // Test 4: verify a bad signature.
  uint8_t bad_signature[sizeof(signature)];
  memcpy(bad_signature, signature, sizeof(signature));
  bad_signature[10] += 1;  // Corrupt one byte of the signature.
  EXPECT_TRUE(verifier.VerifyInit(crypto::SignatureVerifier::RSA_PKCS1_SHA1,
                                  bad_signature, public_key_info));
  verifier.VerifyUpdate(tbs_certificate);
  EXPECT_FALSE(verifier.VerifyFinal());

  // Test 5: import an invalid key.
  uint8_t bad_public_key_info[sizeof(public_key_info)];
  memcpy(bad_public_key_info, public_key_info, sizeof(public_key_info));
  bad_public_key_info[0] += 1;  // Corrupt part of the SPKI syntax.
  EXPECT_FALSE(verifier.VerifyInit(crypto::SignatureVerifier::RSA_PKCS1_SHA1,
                                   signature, bad_public_key_info));

  // Test 6: import a key with extra data.
  uint8_t long_public_key_info[sizeof(public_key_info) + 5];
  memset(long_public_key_info, 0, sizeof(long_public_key_info));
  memcpy(long_public_key_info, public_key_info, sizeof(public_key_info));
  EXPECT_FALSE(verifier.VerifyInit(crypto::SignatureVerifier::RSA_PKCS1_SHA1,
                                   signature, long_public_key_info));
}

// The following RSA-PSS tests were generated via the following OpenSSL
// commands:
//
// clang-format off
// openssl genrsa -f4 -out key.pem 2048
// openssl rsa -in key.pem -pubout -outform der | xxd -i > spki.txt
// openssl rand -out message 50
// xxd -i message > message.txt
// openssl dgst -sign key.pem -sha256 -sigopt rsa_padding_mode:pss -sigopt rsa_pss_saltlen:32 < message | xxd -i > sig-good.txt
// openssl dgst -sign key.pem -sha256 -sigopt rsa_padding_mode:pss -sigopt rsa_pss_saltlen:33 < message | xxd -i > sig-bad-saltlen.txt
// clang-format on

namespace {

// This is the public key corresponding to the following private key.
//
// -----BEGIN RSA PRIVATE KEY-----
// MIIEowIBAAKCAQEArg5NXFRQQ5QU7dcqqIjZwL4qy4AaJNSPfSPvXmFbK0hDXdp6
// PdOZ2Wd+lQLZwb7ZQ2ZdqHVK3kZ2sVUlFmngIoEXNhVg+gW2zGPZ1YemwBMdZ/NW
// V2xTX7Y3RrdR/kSccd9ByRTHKb+BCJ2XN5pHu91+LFchahW0lVPHz9DkBPUCThM2
// I4ZosM3+AcO93RrrcbiQdpuY60Lfg9ZX7+1clM7zhiuOjWNY+FLN4+j4Ec8isiis
// /V1LQyxRZ2t29kto47UJKu0Li7gUvEE1PS/nXBVgEqcSEBBKXa4ahsTqKWJAwvEH
// xaH1t2qhVO1IHcf9FSv5k1T47H7XMLpO2OCPrwIDAQABAoIBAQCXA4exTOHa0Dcc
// aGv1j87GAPimWX3VaKsaGzyKuZNdSTRR0MXwsI+yZa4Y4UFHbSuZ483s499SXPaM
// Q2CLQs8ZgME/xmq+YojIavXL4wcVbUA9OY43CaCI0VLCQzmbj7HgxqCQMzvdh+8P
// J5PUxUHpyHG5TNuL7EsiqG8bapT7ip2+IpKrKjr18gn3k0k9mLNJxK9Qr+CJphwo
// eJgq0Kcjx3bfgDEpPzyvdd+J3e+jclOTYbk2HwJ0FVCrfgJedHFIWUytZoM5783g
// knXzgDyKs65aUDjc/opidXp3WOqfNJUPSiofPYPdYQ26UI0vztL5MBkWCpl+d/55
// BqxCdDlhAoGBANm90DFUca+7LdnDgj8mWtUIzr+XVzSD9tzOIpcjiPwEnxk8RHrM
// aMHCAKZpbsnX/ikdc2I1OsirPgNFh1q30xgL7oCadzxlwfXnEM0Nff8RJKtN+yI6
// +nRoOCDGCHBsaa2wyMYRbnanyRDLPIOP4eGQ6Hz/LQJBvhjRyTXlrUU/AoGBAMyj
// ec1ySnlJ2S0JqPBCk14dRsEs/zStgFz1Wdmk7TMRBPUMyhWf8JwNrU5Ppm9biJMo
// MKwkiFjzv/us4ne3wFRsTiKj4uiIwfji7/N2VpbEXSDGtonrX7hES4wQ/s+qr8XJ
// 8ykHrZ9rPOY2lBhxOo+VYE3U6aspAY/qwK8WyumRAoGAMdl+/Iw0quLTkHNuMj75
// tKQbkUl4sZE0x0B6Mtfz2J7GPeTKWMLLiPB9bZvdvWAx0//mFqnRF3f87orQfjhv
// n6W7qL20ZqN1UHLiKc/Y9LhcCMwFnsSZ6mSh1P8Bl5t6ZkV+8bmz7H5lTe75n7Ul
// JZsjXtqc11NtzgjZY/l9PckCgYAH6ZI+FVs30VkqWqNDlu9nxi4ELh84BDVgYsQ0
// nCHnxZKxfusZZvPAtO6shnvi9mETf4xSO59iARq9OnQPOPWgzgc/Y6LUZuVJIE0y
// 1rKGZdVL/SL1tjofP9TD96xCj1D4jtRuE7Ps5BKYvCeBwm8HOjldCQx357/9to/4
// tSLnYQKBgG7STr94Slb3/BzzMxdMLCum1PH73/+IFxu1J0cXxYLP2zEhcSgqGIwm
// aMgdu1L9eE6lJM99AWTEkpcUz8UFwNt+hZW+lZZpex77RMgRl9VCiwid1BNBSU/o
// +lT1mlpDrPCNOge9n6Qvy5waBugB8uNS86w0UImYiKZr+8IQ4EdE
// -----END RSA PRIVATE KEY-----
const uint8_t kPSSPublicKey[] = {
    0x30, 0x82, 0x01, 0x22, 0x30, 0x0d, 0x06, 0x09, 0x2a, 0x86, 0x48, 0x86,
    0xf7, 0x0d, 0x01, 0x01, 0x01, 0x05, 0x00, 0x03, 0x82, 0x01, 0x0f, 0x00,
    0x30, 0x82, 0x01, 0x0a, 0x02, 0x82, 0x01, 0x01, 0x00, 0xae, 0x0e, 0x4d,
    0x5c, 0x54, 0x50, 0x43, 0x94, 0x14, 0xed, 0xd7, 0x2a, 0xa8, 0x88, 0xd9,
    0xc0, 0xbe, 0x2a, 0xcb, 0x80, 0x1a, 0x24, 0xd4, 0x8f, 0x7d, 0x23, 0xef,
    0x5e, 0x61, 0x5b, 0x2b, 0x48, 0x43, 0x5d, 0xda, 0x7a, 0x3d, 0xd3, 0x99,
    0xd9, 0x67, 0x7e, 0x95, 0x02, 0xd9, 0xc1, 0xbe, 0xd9, 0x43, 0x66, 0x5d,
    0xa8, 0x75, 0x4a, 0xde, 0x46, 0x76, 0xb1, 0x55, 0x25, 0x16, 0x69, 0xe0,
    0x22, 0x81, 0x17, 0x36, 0x15, 0x60, 0xfa, 0x05, 0xb6, 0xcc, 0x63, 0xd9,
    0xd5, 0x87, 0xa6, 0xc0, 0x13, 0x1d, 0x67, 0xf3, 0x56, 0x57, 0x6c, 0x53,
    0x5f, 0xb6, 0x37, 0x46, 0xb7, 0x51, 0xfe, 0x44, 0x9c, 0x71, 0xdf, 0x41,
    0xc9, 0x14, 0xc7, 0x29, 0xbf, 0x81, 0x08, 0x9d, 0x97, 0x37, 0x9a, 0x47,
    0xbb, 0xdd, 0x7e, 0x2c, 0x57, 0x21, 0x6a, 0x15, 0xb4, 0x95, 0x53, 0xc7,
    0xcf, 0xd0, 0xe4, 0x04, 0xf5, 0x02, 0x4e, 0x13, 0x36, 0x23, 0x86, 0x68,
    0xb0, 0xcd, 0xfe, 0x01, 0xc3, 0xbd, 0xdd, 0x1a, 0xeb, 0x71, 0xb8, 0x90,
    0x76, 0x9b, 0x98, 0xeb, 0x42, 0xdf, 0x83, 0xd6, 0x57, 0xef, 0xed, 0x5c,
    0x94, 0xce, 0xf3, 0x86, 0x2b, 0x8e, 0x8d, 0x63, 0x58, 0xf8, 0x52, 0xcd,
    0xe3, 0xe8, 0xf8, 0x11, 0xcf, 0x22, 0xb2, 0x28, 0xac, 0xfd, 0x5d, 0x4b,
    0x43, 0x2c, 0x51, 0x67, 0x6b, 0x76, 0xf6, 0x4b, 0x68, 0xe3, 0xb5, 0x09,
    0x2a, 0xed, 0x0b, 0x8b, 0xb8, 0x14, 0xbc, 0x41, 0x35, 0x3d, 0x2f, 0xe7,
    0x5c, 0x15, 0x60, 0x12, 0xa7, 0x12, 0x10, 0x10, 0x4a, 0x5d, 0xae, 0x1a,
    0x86, 0xc4, 0xea, 0x29, 0x62, 0x40, 0xc2, 0xf1, 0x07, 0xc5, 0xa1, 0xf5,
    0xb7, 0x6a, 0xa1, 0x54, 0xed, 0x48, 0x1d, 0xc7, 0xfd, 0x15, 0x2b, 0xf9,
    0x93, 0x54, 0xf8, 0xec, 0x7e, 0xd7, 0x30, 0xba, 0x4e, 0xd8, 0xe0, 0x8f,
    0xaf, 0x02, 0x03, 0x01, 0x00, 0x01,
};

const uint8_t kPSSMessage[] = {
    0x1e, 0x70, 0xbd, 0xeb, 0x24, 0xf2, 0x9d, 0x05, 0xc5, 0xb5,
    0xf4, 0xca, 0xe6, 0x1d, 0x01, 0x97, 0x29, 0xf4, 0xe0, 0x7c,
    0xfd, 0xcc, 0x97, 0x8d, 0xc2, 0xbb, 0x2d, 0x9b, 0x6b, 0x45,
    0x06, 0xbd, 0x2c, 0x66, 0x10, 0x42, 0x73, 0x8d, 0x88, 0x9b,
    0x18, 0xcc, 0xcb, 0x7e, 0x43, 0x23, 0x06, 0xe9, 0x8f, 0x8f,
};

const uint8_t kPSSSignatureGood[] = {
    0x12, 0xa7, 0x6d, 0x9e, 0x8a, 0xea, 0x28, 0xe0, 0x3f, 0x6f, 0x5a, 0xa4,
    0x1b, 0x6a, 0x0a, 0x14, 0xba, 0xfa, 0x84, 0xf6, 0xb7, 0x3c, 0xc9, 0xd6,
    0x84, 0xab, 0x1e, 0x77, 0x88, 0x53, 0x95, 0x43, 0x8e, 0x73, 0xe4, 0x21,
    0xab, 0x69, 0xb2, 0x0c, 0x73, 0x4d, 0x98, 0x42, 0xbd, 0x65, 0xa2, 0x95,
    0x0d, 0x76, 0xb2, 0xbd, 0xe5, 0x9a, 0x6e, 0x9f, 0x72, 0x7f, 0xdd, 0x1e,
    0x9f, 0xda, 0xc8, 0x2e, 0xa3, 0xe6, 0x28, 0x03, 0x98, 0x5c, 0x13, 0xa7,
    0x7d, 0x4e, 0xde, 0xea, 0x35, 0x1b, 0x35, 0x7e, 0xaa, 0x14, 0xf9, 0xfb,
    0xac, 0x61, 0xd0, 0x44, 0x20, 0xd5, 0x52, 0x5b, 0x92, 0x8f, 0xe7, 0x37,
    0xa2, 0x72, 0x7d, 0xe6, 0x0d, 0x81, 0x63, 0xcc, 0x0f, 0xbd, 0xde, 0x25,
    0xe3, 0x3f, 0x89, 0x1b, 0x39, 0x64, 0xfa, 0x21, 0x1d, 0x0f, 0x9b, 0x8a,
    0xc1, 0xad, 0x03, 0x49, 0x96, 0xff, 0x9f, 0x2d, 0x83, 0xee, 0x2d, 0x2a,
    0x1e, 0xc5, 0x73, 0x9f, 0x5b, 0xde, 0xcb, 0xaf, 0x02, 0xbd, 0xc5, 0x9b,
    0x78, 0xb9, 0x8e, 0x01, 0x75, 0x3c, 0xc9, 0x6e, 0x7d, 0x3e, 0x61, 0x62,
    0xc4, 0x8c, 0x9e, 0x76, 0xed, 0x52, 0x5e, 0x80, 0x89, 0xa7, 0x75, 0x5e,
    0xc6, 0x34, 0x97, 0x22, 0x40, 0xb5, 0x0c, 0x77, 0x09, 0x8c, 0xa8, 0xe9,
    0xf6, 0x8d, 0xc0, 0x10, 0x78, 0x92, 0xa9, 0xc6, 0x68, 0xa3, 0x57, 0x6e,
    0x73, 0xb5, 0x73, 0x8d, 0x8e, 0x21, 0xb1, 0xf3, 0xd0, 0x0a, 0x40, 0x68,
    0xfc, 0x3c, 0xeb, 0xd3, 0x48, 0x4a, 0x44, 0xbd, 0xc0, 0x40, 0x5d, 0x9b,
    0x40, 0x6f, 0x45, 0x98, 0x2b, 0xae, 0x58, 0xe8, 0x9d, 0x34, 0x49, 0xd2,
    0xec, 0xdc, 0xd5, 0x98, 0xb4, 0x87, 0x8a, 0xcc, 0x41, 0x3e, 0xd7, 0xe6,
    0x21, 0xd6, 0x4c, 0x89, 0xf1, 0xf4, 0x77, 0x40, 0x3f, 0x9a, 0x28, 0x25,
    0x55, 0x7c, 0xf5, 0x0c,
};

const uint8_t kPSSSignatureBadSaltLength[] = {
    0x6e, 0x61, 0xbe, 0x8a, 0x82, 0xbd, 0xed, 0xc6, 0xe4, 0x33, 0x91, 0xa4,
    0x43, 0x57, 0x51, 0x7e, 0xa8, 0x18, 0xbf, 0x20, 0x98, 0xbc, 0x04, 0x50,
    0x06, 0x1b, 0x0b, 0xb6, 0x43, 0xde, 0x58, 0x7f, 0x6b, 0xa5, 0x5e, 0x9d,
    0xd1, 0x75, 0x03, 0xf5, 0x19, 0x8d, 0xdb, 0x2c, 0xd2, 0x9a, 0xf9, 0xbd,
    0x82, 0x8d, 0x32, 0x9d, 0x7d, 0x70, 0x6f, 0x81, 0x95, 0x60, 0x1d, 0x62,
    0x72, 0xf3, 0x95, 0x5b, 0x7a, 0x66, 0x7f, 0x45, 0x94, 0x0c, 0x07, 0xc8,
    0xa7, 0x64, 0x38, 0x57, 0x1a, 0x64, 0x64, 0xf1, 0xe0, 0x45, 0xfe, 0x00,
    0x11, 0x90, 0x57, 0x95, 0x15, 0x21, 0x10, 0x85, 0xc0, 0xbe, 0x53, 0x5b,
    0x3b, 0xa3, 0x57, 0x99, 0x2b, 0x94, 0x6b, 0xbf, 0xa5, 0x55, 0x7d, 0x5a,
    0xcb, 0xa2, 0x73, 0x6b, 0x5f, 0x7b, 0x3f, 0x10, 0x90, 0xd1, 0x26, 0x72,
    0x5e, 0xad, 0xd1, 0x34, 0xe8, 0x8a, 0x33, 0xeb, 0xd2, 0xbf, 0x54, 0x92,
    0xeb, 0x7c, 0xb9, 0x97, 0x80, 0x5a, 0x46, 0xc4, 0xbd, 0xf5, 0x7e, 0xd6,
    0x20, 0x90, 0x92, 0xcb, 0x37, 0x85, 0x9d, 0x81, 0x0a, 0xd0, 0xa5, 0x73,
    0x17, 0x7e, 0xe2, 0x91, 0xef, 0x35, 0x55, 0xc9, 0x5e, 0x87, 0x84, 0x11,
    0xa4, 0x36, 0xf0, 0x2a, 0xa7, 0x7a, 0x83, 0x1d, 0x7a, 0x90, 0x69, 0x22,
    0x5d, 0x3b, 0x30, 0x48, 0x46, 0xd2, 0xd3, 0x49, 0x23, 0x64, 0xa4, 0x6d,
    0xd1, 0xef, 0xb9, 0x1b, 0xa4, 0xd1, 0x92, 0xdd, 0x8c, 0xb2, 0xaa, 0x9f,
    0x6a, 0x2c, 0xc9, 0xdb, 0xa7, 0x35, 0x66, 0x92, 0x8b, 0x73, 0x11, 0x70,
    0x2b, 0xf4, 0x34, 0x3f, 0x9e, 0x15, 0x3e, 0xc0, 0xac, 0x78, 0x6f, 0x74,
    0x8a, 0x6b, 0xe4, 0xf2, 0x7b, 0x10, 0xca, 0x01, 0x3a, 0x3a, 0x88, 0x39,
    0x34, 0xa8, 0x52, 0x4a, 0x76, 0x50, 0xef, 0xdb, 0x91, 0x3c, 0x4a, 0x5c,
    0xe5, 0x43, 0x6f, 0x8e,
};

}  // namespace

TEST(SignatureVerifierTest, VerifyRSAPSS) {
  // Verify the test vector.
  crypto::SignatureVerifier verifier;
  ASSERT_TRUE(verifier.VerifyInit(crypto::SignatureVerifier::RSA_PSS_SHA256,
                                  kPSSSignatureGood, kPSSPublicKey));
  verifier.VerifyUpdate(kPSSMessage);
  EXPECT_TRUE(verifier.VerifyFinal());

  // Verify the test vector byte-by-byte.
  ASSERT_TRUE(verifier.VerifyInit(crypto::SignatureVerifier::RSA_PSS_SHA256,
                                  kPSSSignatureGood, kPSSPublicKey));
  for (uint8_t b : kPSSMessage) {
    verifier.VerifyUpdate(base::make_span(&b, 1));
  }
  EXPECT_TRUE(verifier.VerifyFinal());

  // The bad salt length does not verify.
  ASSERT_TRUE(verifier.VerifyInit(crypto::SignatureVerifier::RSA_PSS_SHA256,
                                  kPSSSignatureBadSaltLength, kPSSPublicKey));
  verifier.VerifyUpdate(kPSSMessage);
  EXPECT_FALSE(verifier.VerifyFinal());

  // Corrupt the message.
  std::vector<uint8_t> message(std::begin(kPSSMessage), std::end(kPSSMessage));
  message[0] ^= 1;
  ASSERT_TRUE(verifier.VerifyInit(crypto::SignatureVerifier::RSA_PSS_SHA256,
                                  kPSSSignatureGood, kPSSPublicKey));
  verifier.VerifyUpdate(message);
  EXPECT_FALSE(verifier.VerifyFinal());

  // Corrupt the signature.
  std::vector<uint8_t> signature(std::begin(kPSSSignatureGood),
                                 std::end(kPSSSignatureGood));
  signature[0] ^= 1;
  ASSERT_TRUE(verifier.VerifyInit(crypto::SignatureVerifier::RSA_PSS_SHA256,
                                  signature, kPSSPublicKey));
  verifier.VerifyUpdate(kPSSMessage);
  EXPECT_FALSE(verifier.VerifyFinal());
}
