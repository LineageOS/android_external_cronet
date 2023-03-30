// Copyright 2022 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.net.impl;

import android.content.Context;
import android.os.Build;

import androidx.annotation.Nullable;
import androidx.annotation.VisibleForTesting;

import org.chromium.net.impl.CronetLogger.CronetSource;

/**
 * Takes care of instantiating the correct CronetLogger.
 */
public final class CronetLoggerFactory {
    private static final int SAMPLE_RATE_PER_SECOND = 1;

    private CronetLoggerFactory() {}

    private static final CronetLogger sDefaultLogger = new NoOpLogger();
    private static CronetLogger sTestingLogger;

    // Class that is packaged for Cronet telemetry.
    private static final String CRONET_LOGGER_IMPL_CLASS =
            "com.google.net.cronet.telemetry.CronetLoggerImpl";

    /**
     * Bypasses CronetLoggerFactory logic and always creates a NoOpLogger.
     * To be used only as a kill-switch for logging.
     * @return a NoOpLogger instance.
     */
    public static CronetLogger createNoOpLogger() {
        return sDefaultLogger;
    }

    /**
     * @return The correct CronetLogger to be used for logging.
     */
    public static CronetLogger createLogger(Context ctx, CronetSource source) {
        if (sTestingLogger != null) return sTestingLogger;

        // The CronetLoggerImpl only works from apiLevel 30
        if (!CronetManifest.isAppOptedInForTelemetry(ctx, source)
                || Build.VERSION.SDK_INT < Build.VERSION_CODES.R) {
            return sDefaultLogger;
        }

        Class<? extends CronetLogger> cronetLoggerImplClass = fetchLoggerImplClass();
        if (cronetLoggerImplClass == null) return sDefaultLogger;

        try {
            return cronetLoggerImplClass.getConstructor(int.class).newInstance(
                    SAMPLE_RATE_PER_SECOND);
        } catch (Exception e) {
            // Pass - since we dont want any failure, catch any exception that might arise.
        }
        return sDefaultLogger;
    }

    @VisibleForTesting
    public static void setLoggerForTesting(@Nullable CronetLogger testingLogger) {
        sTestingLogger = testingLogger;
    }

    private static Class<? extends CronetLogger> fetchLoggerImplClass() {
        ClassLoader loader = CronetLoggerFactory.class.getClassLoader();
        try {
            return loader.loadClass(CRONET_LOGGER_IMPL_CLASS).asSubclass(CronetLogger.class);
        } catch (ClassNotFoundException e) {
            return null;
        }
    }
}
