/*
 * Copyright (C) 2019 The Android Open Source Project
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
import android.net.http.UploadDataProvider
import android.net.http.UrlRequest
import android.net.http.cts.util.HttpCtsTestServer
import android.net.http.cts.util.TestUrlRequestCallback
import android.net.http.cts.util.UploadDataProviders
import android.net.http.cts.util.assertOKStatusCode
import android.os.Build
import androidx.test.core.app.ApplicationProvider
import com.android.testutils.DevSdkIgnoreRule
import com.android.testutils.DevSdkIgnoreRunner
import com.google.common.base.Strings
import com.google.common.truth.Truth.assertThat
import java.net.URLEncoder
import org.junit.After
import org.junit.Before
import org.junit.Test
import org.junit.runner.RunWith

// Tests UploadDataProvider and UrlRequest actions relating to the it
@RunWith(DevSdkIgnoreRunner::class)
@DevSdkIgnoreRule.IgnoreUpTo(Build.VERSION_CODES.R)
class UploadDataProviderTest {
    private lateinit var mCallback: TestUrlRequestCallback
    private lateinit var mTestServer: HttpCtsTestServer
    private lateinit var mHttpEngine: HttpEngine
    private lateinit var mUploadDataProvider: UploadDataProvider

    @Before
    @Throws(Exception::class)
    fun setUp() {
        val context: Context = ApplicationProvider.getApplicationContext()
        val builder = HttpEngine.Builder(context)
        mHttpEngine = builder.build()
        mCallback = TestUrlRequestCallback()
        mTestServer = HttpCtsTestServer(context)
    }

    @After
    @Throws(Exception::class)
    fun tearDown() {
        assertThat(
            (mUploadDataProvider as UploadDataProviders.ByteBufferUploadProvider).blockForClose()
        ).isTrue()
        mHttpEngine.shutdown()
        mTestServer.shutdown()
    }

    private fun createUrlRequestBuilder(url: String): UrlRequest.Builder {
        return mHttpEngine.newUrlRequestBuilder(url, mCallback.executor, mCallback)
    }

    @Test
    fun testUploadDataProvider_close() {
        mUploadDataProvider = UploadDataProviders.create("test")
        mUploadDataProvider.close()
    }

    @Test
    fun testUploadDataProvider_urlrequest_post_echoRequestBody() {
        val testData = "test"
        val builder = createUrlRequestBuilder(mTestServer.echoBodyUrl)
        mUploadDataProvider = UploadDataProviders.create(testData)
        builder.setUploadDataProvider(mUploadDataProvider, mCallback.executor)
        builder.addHeader("Content-Type", "text/html")
        builder.build().start()
        mCallback.expectCallback(TestUrlRequestCallback.ResponseStep.ON_SUCCEEDED)
        assertOKStatusCode(mCallback.mResponseInfo)
        assertThat(mCallback.mResponseAsString).isEqualTo(testData)
    }

    @Test
    @Throws(Exception::class)
    fun testUploadDataProvider_urlrequest_post_withRedirect() {
        val body = Strings.repeat(
                "Hello, this is a really interesting body, so write this 100 times.", 100)
        val redirectUrlParameter = URLEncoder.encode(mTestServer.echoBodyUrl, "UTF-8")
        mUploadDataProvider = UploadDataProviders.create(body)
        createUrlRequestBuilder(String.format(
            "%s/alt_redirect?dest=%s&statusCode=307", mTestServer.baseUri, redirectUrlParameter))
            .setHttpMethod("POST")
            .addHeader("Content-Type", "text/plain")
            .setUploadDataProvider(mUploadDataProvider, mCallback.executor)
            .build()
            .start()
        mCallback.expectCallback(TestUrlRequestCallback.ResponseStep.ON_SUCCEEDED)
        assertOKStatusCode(mCallback.mResponseInfo)
        assertThat(mCallback.mResponseAsString).isEqualTo(body)
    }
}
