/*
 * Copyright (C) 2023 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.android.tests.chromium.host;

import static com.android.tests.chromium.host.InstrumentationFlags.COMMAND_LINE_FLAGS_KEY;
import static com.android.tests.chromium.host.InstrumentationFlags.DUMP_COVERAGE_KEY;
import static com.android.tests.chromium.host.InstrumentationFlags.EXTRA_SHARD_NANO_TIMEOUT_KEY;
import static com.android.tests.chromium.host.InstrumentationFlags.LIBRARY_TO_LOAD_ACTIVITY_KEY;
import static com.android.tests.chromium.host.InstrumentationFlags.NATIVE_TEST_ACTIVITY_KEY;
import static com.android.tests.chromium.host.InstrumentationFlags.NATIVE_UNIT_TEST_ACTIVITY_KEY;
import static com.android.tests.chromium.host.InstrumentationFlags.RUN_IN_SUBTHREAD_KEY;
import static com.android.tests.chromium.host.InstrumentationFlags.STDOUT_FILE_KEY;
import static com.android.tests.chromium.host.InstrumentationFlags.TEST_RUNNER;

import com.android.ddmlib.MultiLineReceiver;
import com.android.tradefed.config.Option;
import com.android.tradefed.device.CollectingOutputReceiver;
import com.android.tradefed.device.DeviceNotAvailableException;
import com.android.tradefed.invoker.TestInformation;
import com.android.tradefed.log.LogUtil;
import com.android.tradefed.result.ITestInvocationListener;
import com.android.tradefed.testtype.GTest;
import com.android.tradefed.testtype.GTestListTestParser;
import com.android.tradefed.testtype.GTestResultParser;

import com.google.common.base.Strings;

import java.io.File;
import java.io.IOException;
import java.nio.file.Files;
import java.time.Duration;
import java.util.concurrent.TimeUnit;

public class ChromiumHostDrivenTest extends GTest {
    // This counter is used to retry
    private static final int retries = 1;
    private static final Duration testsTimeout = Duration.ofMinutes(30);
    // This contains the gtest logs that is printed to stdout.
    private static final String GTEST_OUTPUT_PATH = "/data/local/tmp/cronet_gtest_output.txt";
    private static final String CLEAR_CLANG_COVERAGE_FILES =
            "find /data/misc/trace -name '*.profraw' -delete";
    @Option(
            name = "dump-native-coverage",
            description = "Force APK under test to dump native test coverage upon exit"
    )
    private boolean mCoverage = false;
    @Option(
            name = "library-to-load",
            description = "Name of the .so file under test"
    )
    private String libraryToLoad = "";

    private String createRunAllTestsCommand() throws DeviceNotAvailableException {
        InstrumentationCommandBuilder builder = new InstrumentationCommandBuilder(TEST_RUNNER)
                .addArgument(NATIVE_TEST_ACTIVITY_KEY, NATIVE_UNIT_TEST_ACTIVITY_KEY)
                .addArgument(RUN_IN_SUBTHREAD_KEY, "1")
                .addArgument(EXTRA_SHARD_NANO_TIMEOUT_KEY, String.valueOf(testsTimeout.toNanos()))
                .addArgument(LIBRARY_TO_LOAD_ACTIVITY_KEY, libraryToLoad)
                .addArgument(STDOUT_FILE_KEY, GTEST_OUTPUT_PATH)
                .addArgument(COMMAND_LINE_FLAGS_KEY,
                        String.format("'%s'", getAllGTestFlags("")));
        if (mCoverage) {
            builder.addArgument(DUMP_COVERAGE_KEY, "true");
        }
        return builder.build();
    }

    private void printHostLogs(String cmd) {
        LogUtil.CLog.i(String.format("[Cronet] Library to be loaded: %s\n", libraryToLoad));
        LogUtil.CLog.i(String.format("[Cronet] Command used to run gtests: adb shell %s\n", cmd));
        LogUtil.CLog.i(String.format("[Cronet] Native-Coverage = %b", mCoverage));
    }

    @Override
    public void run(TestInformation testInfo, ITestInvocationListener listener)
            throws DeviceNotAvailableException {
        if (Strings.isNullOrEmpty(libraryToLoad)) {
            throw new IllegalStateException("No library provided to be loaded.");
        }
        String cmd = createRunAllTestsCommand();
        printHostLogs(cmd);
        getDevice().executeShellCommand(CLEAR_CLANG_COVERAGE_FILES);
        getDevice().executeShellCommand(cmd, new CollectingOutputReceiver(),
                testsTimeout.toMinutes(), TimeUnit.MINUTES, /* retryAttempts */ 1);
        try {
            parseAndReport(getDevice().pullFile(GTEST_OUTPUT_PATH), listener);
        } catch (IOException e) {
            throw new FailedReportingException("Failed to parse and report test results",
                    e.getCause());
        }
    }

    private void parseAndReport(File testResultsOutput, ITestInvocationListener listener)
            throws IOException {
        if (testResultsOutput == null) {
            throw new IOException(
                    String.format("Failed to retrieve %s from device", GTEST_OUTPUT_PATH));
        }
        // Loading all the lines is fine since this is done on the host-machine.
        String[] lines = Files.readAllLines(testResultsOutput.toPath()).toArray(String[]::new);
        MultiLineReceiver parser;
        // the parser automatically reports the test result back to the infra through the listener.
        if (isCollectTestsOnly()) {
            parser = new GTestListTestParser(libraryToLoad, listener);
        } else {
            parser = new GTestResultParser(libraryToLoad, listener);
        }
        parser.processNewLines(lines);
        parser.done();
    }
}