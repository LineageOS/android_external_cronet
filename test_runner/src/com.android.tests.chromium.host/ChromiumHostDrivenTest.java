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

import com.android.tradefed.config.Option;
import com.android.tradefed.device.CollectingOutputReceiver;
import com.android.tradefed.device.DeviceNotAvailableException;
import com.android.tradefed.result.FileInputStreamSource;
import com.android.tradefed.result.LogDataType;
import com.android.tradefed.testtype.DeviceJUnit4ClassRunner;
import com.android.tradefed.testtype.junit4.BaseHostJUnit4Test;

import com.google.common.base.Strings;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.TemporaryFolder;
import org.junit.runner.RunWith;

import java.io.File;
import java.io.FileWriter;
import java.time.Duration;
import java.util.concurrent.TimeUnit;

@RunWith(DeviceJUnit4ClassRunner.class)
public class ChromiumHostDrivenTest extends BaseHostJUnit4Test {
    private static final String CHROMIUM_PACKAGE = "org.chromium.native_test";
    private static final String NATIVE_TEST_ACTIVITY_KEY = String.format("%s.%s", CHROMIUM_PACKAGE,
            "NativeTestInstrumentationTestRunner.NativeTestActivity");
    private static final String RUN_IN_SUBTHREAD_KEY = String.format("%s.%s", CHROMIUM_PACKAGE,
            "NativeTest.RunInSubThread");
    private static final String NATIVE_UNIT_TEST_ACTIVITY_KEY = String.format("%s.%s",
            CHROMIUM_PACKAGE,
            "NativeUnitTestActivity");
    private static final String COMMAND_LINE_FLAGS_KEY = String.format("%s.%s", CHROMIUM_PACKAGE,
            "NativeTest.CommandLineFlags");
    private static final String EXTRA_SHARD_NANO_TIMEOUT_KEY = String.format("%s.%s",
            CHROMIUM_PACKAGE,
            "NativeTestInstrumentationTestRunner.ShardNanoTimeout");
    private static final String DUMP_COVERAGE_KEY = String.format("%s.%s", CHROMIUM_PACKAGE,
            "NativeTestInstrumentationTestRunner.DumpCoverage");
    private static final String LIBRARY_TO_LOAD_ACTIVITY_KEY = String.format("%s.%s",
            CHROMIUM_PACKAGE,
            "NativeTestInstrumentationTestRunner.LibraryUnderTest");
    private static final String STDOUT_FILE_KEY = String.format("%s.%s", CHROMIUM_PACKAGE,
            "NativeTestInstrumentationTestRunner.StdoutFile");
    private static final String TEST_RUNNER = String.format("%s/%s", CHROMIUM_PACKAGE,
            "org.chromium.build.gtest_apk.NativeTestInstrumentationTestRunner");
    private static Duration testsTimeout = Duration.ofMinutes(15);
    // This contains the gtest runner result output.
    private static final String GTEST_RESULT_OUTPUT_PATH =
            "/data/local/tmp/cronet_test_results_output.json";
    // This contains the gtest logs that is printed to stdout.
    private static final String GTEST_OUTPUT_PATH = "/data/local/tmp/cronet_gtest_output.txt";
    private static final String CLEAR_CLANG_COVERAGE_FILES =
            "find /data/misc/trace -name '*.profraw' -delete";
    @Rule
    public DeviceJUnit4ClassRunner.TestLogData logs = new DeviceJUnit4ClassRunner.TestLogData();
    @Rule
    public TemporaryFolder folder = new TemporaryFolder();
    @Option(
            name = "dump-native-coverage",
            description = "Force APK under test to dump native test coverage upon exit"
    )
    private boolean mCoverage = false;

    @Option(
            name = "library-to-load",
            description = "Name of the .so file under test"
    )
    private String libraryToLoad;

    private String createRunAllTestsCommand() {
        InstrumentationCommandBuilder builder = new InstrumentationCommandBuilder(TEST_RUNNER)
                .addArgument(NATIVE_TEST_ACTIVITY_KEY, NATIVE_UNIT_TEST_ACTIVITY_KEY)
                .addArgument(RUN_IN_SUBTHREAD_KEY, "1")
                .addArgument(EXTRA_SHARD_NANO_TIMEOUT_KEY, String.valueOf(testsTimeout.toNanos()))
                .addArgument(LIBRARY_TO_LOAD_ACTIVITY_KEY, libraryToLoad)
                .addArgument(STDOUT_FILE_KEY, GTEST_OUTPUT_PATH)
                .addArgument(COMMAND_LINE_FLAGS_KEY, String.format("'--gtest_output=json:%s'",
                        ChromiumHostDrivenTest.GTEST_RESULT_OUTPUT_PATH));
        if (mCoverage) {
            builder.addArgument(DUMP_COVERAGE_KEY, "true");
        }
        return builder.build();
    }

    @Before
    public void setup() throws DeviceNotAvailableException {
        if (mCoverage) {
            getDevice().executeShellCommand(CLEAR_CLANG_COVERAGE_FILES);
            testsTimeout = Duration.ofMinutes(30);
        }
    }

    @Test
    public void testRunChromiumTests() throws Exception {
        if (Strings.isNullOrEmpty(libraryToLoad)) {
            throw new IllegalStateException("No library provided to be loaded.");
        }
        String cmd = createRunAllTestsCommand();
        CollectingOutputReceiver outputCollector = new CollectingOutputReceiver();
        getDevice().executeShellCommand(cmd, outputCollector, testsTimeout.toSeconds(),
                TimeUnit.SECONDS, 1);
        File logFile = folder.newFile();
        try (FileWriter fileWriter = new FileWriter(logFile)) {
            fileWriter.write(String.format("Library to be loaded: %s\n", libraryToLoad));
            fileWriter.write(String.format("Command used to run gtests: adb shell %s\n", cmd));
            if (mCoverage) {
                fileWriter.write("dump-native-coverage enabled!\n");
            }
            fileWriter.write(
                    String.format("Instrumentation Result: %s", outputCollector.getOutput()));
        }
        logs.addTestLog("cronet_extra_logs", LogDataType.TEXT,
                new FileInputStreamSource(logFile));
        // The files are included in the test report generated after executing the tests.
        File gtestTestResultsJson = getDevice().pullFile(GTEST_RESULT_OUTPUT_PATH);
        GTestsMetaData gTestsMetaData = GTestsMetaData.parseFile(gtestTestResultsJson);
        Assert.assertFalse(gTestsMetaData.hasAnyFailures());
        if (!gTestsMetaData.isOutputParsedCorrectly()) {
            Assert.fail(
                    "Failed to parse gtests result. Please check the logcat for more information.");
        } else if (gTestsMetaData.getTotalTests() == 0) {
            Assert.fail("No Test has been executed.");
        }
    }
}