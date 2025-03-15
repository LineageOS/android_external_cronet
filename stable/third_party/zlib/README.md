This directory was created manually and not imported from Chromium. It was created to remove the
divergence we had where we had to rename `#include "third_party/zlib/zlib.h"` line to
`#include <zlib.h>`. This is due to the fact that we don't import Chromium's zlib and we use
zlib from AOSP. In order to workaround that, we redirect traffic from `third_party/zlib/zlib.h` to
AOSP's zlib.