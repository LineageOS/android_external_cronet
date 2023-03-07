// Copyright 2016 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
package android.net.http;

import java.util.concurrent.Executor;

/**
 * {@link UrlRequest} that exposes experimental features. To obtain an
 * instance of this class, cast a {@code UrlRequest} to this type. Every
 * instance of {@code UrlRequest} can be cast to an instance of this class,
 * as they are backed by the same implementation and hence perform identically.
 * Instances of this class are not meant for general use, but instead only
 * to access experimental features. Experimental features may be deprecated in the
 * future. Use at your own risk.
 *
 * {@hide since this class exposes experimental features that should be hidden}.
 */
public abstract class ExperimentalUrlRequest extends UrlRequest {
    /**
     * {@link UrlRequest#Builder} that exposes experimental features. To obtain an
     * instance of this class, cast a {@code UrlRequest.Builder} to this type. Every
     * instance of {@code UrlRequest.Builder} can be cast to an instance of this class,
     * as they are backed by the same implementation and hence perform identically.
     * Instances of this class are not meant for general use, but instead only
     * to access experimental features. Experimental features may be deprecated in the
     * future. Use at your own risk.
     */
    public abstract static class Builder extends UrlRequest.Builder {
        /**
         * Disables connection migration for the request if enabled for
         * the session.
         * @return the builder to facilitate chaining.
         */
        public Builder disableConnectionMigration() {
            return this;
        }

        /**
         * Associates the annotation object with this request. May add more than one.
         * Passed through to a {@link RequestFinishedInfo.Listener},
         * see {@link RequestFinishedInfo#getAnnotations}.
         *
         * @param annotation an object to pass on to the {@link RequestFinishedInfo.Listener} with a
         * {@link RequestFinishedInfo}.
         * @return the builder to facilitate chaining.
         */
        public Builder addRequestAnnotation(Object annotation) {
            return this;
        }

        /**
         * Sets a listener that gets invoked after {@link Callback#onCanceled onCanceled()},
         * {@link Callback#onFailed onFailed()} or {@link Callback#onSucceeded onSucceeded()}
         * return.
         *
         * <p>The listener is invoked  with the request finished info on an
         * {@link java.util.concurrent.Executor} provided by
         * {@link RequestFinishedInfo.Listener#getExecutor getExecutor()}.
         *
         * @param listener the listener for finished requests.
         * @return the builder to facilitate chaining.
         */
        public Builder setRequestFinishedListener(RequestFinishedInfo.Listener listener) {
            return this;
        }

        /**
         * Default request idempotency, only enable 0-RTT for safe HTTP methods. Passed to {@link
         * #setIdempotency}.
         */
        public static final int DEFAULT_IDEMPOTENCY = 0;

        /**
         * Request is idempotent. Passed to {@link #setIdempotency}.
         */
        public static final int IDEMPOTENT = 1;

        /**
         * Request is not idempotent. Passed to {@link #setIdempotency}.
         */
        public static final int NOT_IDEMPOTENT = 2;

        /**
         * Sets idempotency of the request which should be one of the {@link #DEFAULT_IDEMPOTENCY
         * IDEMPOTENT NOT_IDEMPOTENT} values. The default idempotency indicates that 0-RTT is only
         * enabled for safe HTTP methods (GET, HEAD, OPTIONS, and TRACE).
         *
         * @param idempotency idempotency of the request which should be one of the {@link
         *         #DEFAULT_IDEMPOTENCY IDEMPOTENT NOT_IDEMPOTENT} values.
         * @return the builder to facilitate chaining.
         */
        public Builder setIdempotency(int idempotency) {
            return this;
        }

        // To support method chaining, override superclass methods to return an
        // instance of this class instead of the parent.

        @Override
        public abstract Builder setHttpMethod(String method);

        @Override
        public abstract Builder addHeader(String header, String value);

        @Override
        public abstract Builder setDisableCache(boolean disableCache);

        @Override
        public abstract Builder setPriority(int priority);

        @Override
        public abstract Builder setUploadDataProvider(
                UploadDataProvider uploadDataProvider, Executor executor);

        @Override
        public abstract Builder setAllowDirectExecutor(boolean allowDirectExecutor);

        @Override
        public abstract ExperimentalUrlRequest build();
    }
}
