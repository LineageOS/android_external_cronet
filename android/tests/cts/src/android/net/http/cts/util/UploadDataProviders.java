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

package android.net.http.cts.util;

import android.net.http.UploadDataProvider;
import android.net.http.UploadDataSink;
import android.os.ConditionVariable;
import android.os.ParcelFileDescriptor;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.channels.FileChannel;
import java.nio.charset.StandardCharsets;

/**
 * Provides implementations of {@link UploadDataProvider} for common use cases. Similar to
 * {@code android.net.http.apihelpers.UploadDataProviders} which is not an exposed API.
 */
public final class UploadDataProviders {
    /**
     * Uploads the UTF-8 representation of {@code data}
     *
     * @param data String containing data to upload
     * @return A new UploadDataProvider for the given data
     */
    public static UploadDataProvider create(String data) {
        return new ByteBufferUploadProvider(ByteBuffer.wrap(data.getBytes(StandardCharsets.UTF_8)));
    }

    public static final class ByteBufferUploadProvider extends UploadDataProvider {
        private final ByteBuffer mUploadBuffer;

        // Signals when this provider has been closed
        private final ConditionVariable mClosed = new ConditionVariable();

        private ByteBufferUploadProvider(ByteBuffer uploadBuffer) {
            this.mUploadBuffer = uploadBuffer;
        }

        @Override
        public long getLength() {
            return mUploadBuffer.limit();
        }

        @Override
        public void read(UploadDataSink uploadDataSink, ByteBuffer byteBuffer) {
            if (!byteBuffer.hasRemaining()) {
                throw new IllegalStateException("Cronet passed a buffer with no bytes remaining");
            }
            if (byteBuffer.remaining() >= mUploadBuffer.remaining()) {
                byteBuffer.put(mUploadBuffer);
            } else {
                int oldLimit = mUploadBuffer.limit();
                mUploadBuffer.limit(mUploadBuffer.position() + byteBuffer.remaining());
                byteBuffer.put(mUploadBuffer);
                mUploadBuffer.limit(oldLimit);
            }
            uploadDataSink.onReadSucceeded(false);
        }

        @Override
        public void rewind(UploadDataSink uploadDataSink) {
            mUploadBuffer.position(0);
            uploadDataSink.onRewindSucceeded();
        }

        @Override
        public void close() throws IOException {
            mClosed.open();
        }

        public boolean blockForClose() { return mClosed.block(12_000); }
    }

    // Prevent instantiation
    private UploadDataProviders() {}
}
