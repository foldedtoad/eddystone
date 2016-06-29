// Copyright 2015 Google Inc. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

package com.callender.eddystone.locator;

class Beacon {
  private static final String BULLET = "● ";
  final String deviceAddress;
  int rssi;

  // Used to remove devices from the listview when they haven't been seen in a while.
  long lastSeenTimestamp = System.currentTimeMillis();

  byte[] uidServiceData;

  class UidStatus {
    String uidValue;
    int txPower;

    String errTx;
    String errUid;
    String errRfu;
  }

  class FrameStatus {
    String nullServiceData;
    String tooShortServiceData;
    String invalidFrameType;

    public String getErrors() {
      StringBuilder sb = new StringBuilder();
      if (nullServiceData != null) {
        sb.append(BULLET).append(nullServiceData).append("\n");
      }
      if (tooShortServiceData != null) {
        sb.append(BULLET).append(tooShortServiceData).append("\n");
      }
      if (invalidFrameType != null) {
        sb.append(BULLET).append(invalidFrameType).append("\n");
      }
      return sb.toString().trim();
    }

    @Override
    public String toString() {
      return getErrors();
    }
  }

  boolean hasUidFrame;
  UidStatus uidStatus = new UidStatus();

  FrameStatus frameStatus = new FrameStatus();

  Beacon(String deviceAddress, int rssi) {
    this.deviceAddress = deviceAddress;
    this.rssi = rssi;
  }

  /**
   * Performs a case-insensitive contains test of s on the device address (with or without the
   * colon separators) and/or the UID value, and/or the URL value.
   */
  boolean contains(String s) {
    return s == null || s.isEmpty() ||
           deviceAddress.replace(":", "").toLowerCase().contains(s.toLowerCase()) ||
           (uidStatus.uidValue != null &&
            uidStatus.uidValue.toLowerCase().contains(s.toLowerCase()));
  }
}
