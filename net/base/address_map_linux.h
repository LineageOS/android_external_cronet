// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef NET_BASE_ADDRESS_MAP_LINUX_H_
#define NET_BASE_ADDRESS_MAP_LINUX_H_

#include <map>
#include <unordered_set>

#include "base/containers/flat_map.h"
#include "base/functional/callback_forward.h"
#include "net/base/ip_address.h"
#include "net/base/net_export.h"
#include "third_party/abseil-cpp/absl/types/optional.h"

struct ifaddrmsg;

namespace net {

// Various components of //net need to access a real-time-updated AddressMap
// (see comments below). For example, AddressSorterPosix (used in DNS
// resolution) and GetNetworkList() (used in many places).
// The methods defined in this interface should be safe to call from any thread.
class NET_EXPORT AddressMapOwnerLinux {
 public:
  // A map from net::IPAddress to netlink's ifaddrmsg, which includes
  // information about the network interface that the IP address is associated
  // with (e.g. interface index).
  using AddressMap = std::map<IPAddress, struct ifaddrmsg>;

  AddressMapOwnerLinux() = default;

  AddressMapOwnerLinux(const AddressMapOwnerLinux&) = delete;
  AddressMapOwnerLinux& operator=(const AddressMapOwnerLinux&) = delete;

  virtual ~AddressMapOwnerLinux() = default;

  // These functions can be called on any thread. Implementations should use
  // locking if necessary.

  // Returns the current AddressMap.
  virtual AddressMap GetAddressMap() const = 0;
  // Returns set of interface indices for online interfaces.
  virtual std::unordered_set<int> GetOnlineLinks() const = 0;
};

}  // namespace net

#endif  // NET_BASE_ADDRESS_MAP_LINUX_H_
