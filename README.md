## Cronet (HttpEngine)

Cronet is Chrome's networking stack packaged into a client networking library for Android. It significantly improves performance thanks to highly optimized code and support of modern protocols like QUIC and HTTP/3.

Cronet is imported through copybara from Chromium. Please do not submit any changes to this repoistory or touch the Android.bp as they are auto-generated. Contact cronet-team@ for more information

## Repository Layout

See [go/cronet-structure-in-aosp](go/cronet-structure-in-aosp) for more additional details

#### AOSP-only

[android/](https://cs.android.com/android/platform/superproject/main/+/main:external/cronet/android/) only exists in AOSP and is not imported from Chromium. This contains tools and code that is usually developed in AOSP and is not related to Chromium (eg: HttpEngine API).

#### Third-party code

This include the [top-level third_party/](https://cs.android.com/android/platform/superproject/main/+/main:external/cronet/third_party/). It's important to note that there are some third-party code that lives under first-party code (eg: [QUICHE](https://cs.android.com/android/platform/superproject/main/+/main:external/cronet/net/third_party/quiche/) which lives under net/third_party). Those should be moved to the top-level third_party directory at some point but we will only do so once chromium has done that.


#### Rust Third-party code

We follow the same structure which Rust follows in AOSP where the crates live under [third_party/rust/chromium_crates_io/vendor](https://source.chromium.org/chromium/chromium/src/+/main:third_party/rust/chromium_crates_io/vendor/;l=1) but the BUILD.gn which defines the build target lives under third_party/rust/{library_name} (eg: [aho-corasick](https://source.chromium.org/chromium/chromium/src/+/main:third_party/rust/aho_corasick/)). For more information, see the [README.md](https://source.chromium.org/chromium/chromium/src/+/main:third_party/rust/README.md) in Chromium for Rust crates.
