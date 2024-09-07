/*
 * Copyright (C) 2024 The Android Open Source Project
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

package android.net.http.cts

import android.content.Context
import android.net.http.HttpEngine
import android.net.http.InlineExecutionProhibitedException
import android.net.http.UrlRequest
import android.net.http.cts.util.HttpCtsTestServer
import android.net.http.cts.util.TestUrlRequestCallback
import android.net.http.cts.util.UploadDataProviders
import android.os.Build
import androidx.test.core.app.ApplicationProvider
import com.android.testutils.DevSdkIgnoreRule
import com.android.testutils.DevSdkIgnoreRunner
import com.google.common.truth.Truth.assertThat
import java.util.concurrent.Executor
import java.util.concurrent.Executors
import kotlin.test.Test
import org.junit.After
import org.junit.Before
import org.junit.runner.RunWith

@RunWith(DevSdkIgnoreRunner::class)
@DevSdkIgnoreRule.IgnoreUpTo(Build.VERSION_CODES.R)
class InlineExecutionProhibitedExceptionTest {
    private val DIRECT_EXECUTOR = Executor { obj: Runnable -> obj.run() }

    private var mCallback: TestUrlRequestCallback? = null
    private var mTestServer: HttpCtsTestServer? = null
    private var mHttpEngine: HttpEngine? = null

    @Before
    fun setUp() {
        val context: Context = ApplicationProvider.getApplicationContext()
        val builder = HttpEngine.Builder(context)
        mHttpEngine = builder.build()
        mCallback = TestUrlRequestCallback()
        mTestServer = HttpCtsTestServer(context)
    }

    @After
    fun tearDown() {
            mHttpEngine?.shutdown()
            mTestServer?.shutdown()
    }

    @Test
    fun testInlineExecution_messageContainsInline() {
        val inlineException = InlineExecutionProhibitedException()

        assertThat(inlineException.message).contains("Inline")
    }

    @Test
    @Throws(InterruptedException::class)
    fun testInlineExecution_allowed_byUrlRequest() {
        val callback = TestUrlRequestCallback()
        callback.setAllowDirectExecutor(true)
        val builder: UrlRequest.Builder = mHttpEngine!!.newUrlRequestBuilder(
                mTestServer!!.echoBodyUrl,
            DIRECT_EXECUTOR,
            callback
        )
        val dataProvider = UploadDataProviders.create("test")
        builder.setUploadDataProvider(dataProvider, DIRECT_EXECUTOR)
        builder.addHeader("Content-Type", "text/plain;charset=UTF-8")
        builder.setDirectExecutorAllowed(true)
        builder.build().start()
        callback.blockForDone()
        if (callback.mOnErrorCalled) {
            throw AssertionError("Expected no exception", callback.mError)
        }
        assertThat(callback.mResponseInfo.httpStatusCode.toLong()).isEqualTo(200)
        assertThat(callback.mResponseAsString).isEqualTo("test")
    }

    @Test
    @Throws(Exception::class)
    fun testInlineExecution_disallowed_onUploadDataProvider_byUrlRequest() {
        val callback = TestUrlRequestCallback()
        // This applies just locally to the test callback, not to SUT
        callback.setAllowDirectExecutor(true)
        val builder: UrlRequest.Builder = mHttpEngine!!.newUrlRequestBuilder(
                mTestServer!!.echoBodyUrl,
            Executors.newSingleThreadExecutor(),
            callback
        )
        val dataProvider = UploadDataProviders.create("test")
        builder.setUploadDataProvider(dataProvider, DIRECT_EXECUTOR)
                .addHeader("Content-Type", "text/plain;charset=UTF-8")
                .build()
                .start()
        callback.blockForDone()
        assertThat(callback.mOnErrorCalled).isTrue()
        assertThat(
            callback.mError
        ).hasCauseThat().isInstanceOf(InlineExecutionProhibitedException::class.java)
    }

    @Test
    @Throws(Exception::class)
    fun testInlineExecution_disallowed_onUrlRequestBuilder_byUrlRequest() {
        val callback = TestUrlRequestCallback()
        // This applies just locally to the test callback, not to SUT
        callback.setAllowDirectExecutor(true)
        val builder: UrlRequest.Builder = mHttpEngine!!.newUrlRequestBuilder(
                mTestServer!!.echoBodyUrl,
            DIRECT_EXECUTOR,
            callback
        )
        val dataProvider = UploadDataProviders.create("test")
        builder.setUploadDataProvider(dataProvider, Executors.newSingleThreadExecutor())
                .addHeader("Content-Type", "text/plain;charset=UTF-8")
                .build()
                .start()
        callback.blockForDone()
        assertThat(callback.mOnErrorCalled).isTrue()
        assertThat(
            callback.mError
        ).hasCauseThat().isInstanceOf(InlineExecutionProhibitedException::class.java)
    }
}
