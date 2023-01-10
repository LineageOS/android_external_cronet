// Copyright 2022 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package android.net.http;

import androidx.annotation.Nullable;

import java.time.Duration;

/**
 * A class configuring Cronet's connection migration functionality.
 *
 * <p>Connection migration stops open connections to servers from being destroyed when the
 * client device switches its L4 connectivity (typically the IP address as a result of using
 * a different network). This is particularly common with mobile devices losing
 * wifi connectivity and switching to cellular data, or vice versa (a.k.a. the parking lot
 * problem). QUIC uses connection identifiers which are independent of the underlying
 * transport layer to make this possible. If the client connects to a new network and wants
 * to preserve the existing connection, they can do so by using a connection identifier the server
 * knows to be a continuation of the existing connection.
 *
 * <p>The features are only available for QUIC connections and the server needs to support
 * connection migration.
 *
 * @see <a href="https://www.rfc-editor.org/rfc/rfc9000.html#section-9">Connection
 *     Migration specification</a>
 */
public class ConnectionMigrationOptions {
    @Nullable
    private final Boolean mEnableDefaultNetworkMigration;
    @Nullable
    private final Boolean mEnablePathDegradationMigration;
    @Nullable
    private final Boolean mAllowServerMigration;
    @Nullable
    private final Boolean mMigrateIdleConnections;
    @Nullable
    private final Duration mIdleMigrationPeriod;
    @Nullable
    private final Boolean mRetryPreHandshakeErrorsOnNonDefaultNetwork;
    @Nullable
    private final Boolean mAllowNonDefaultNetworkUsage;
    @Nullable
    private final Duration mMaxTimeOnNonDefaultNetwork;
    @Nullable
    private final Integer mMaxWriteErrorNonDefaultNetworkMigrationsCount;
    @Nullable
    private final Integer mMaxPathDegradingNonDefaultMigrationsCount;

    /**
     * See {@link Builder#enableDefaultNetworkMigration}
     */
    @Nullable
    public Boolean getEnableDefaultNetworkMigration() {
        return mEnableDefaultNetworkMigration;
    }

    /**
     * See {@link Builder#enablePathDegradationMigration}
     */
    @Nullable
    public Boolean getEnablePathDegradationMigration() {
        return mEnablePathDegradationMigration;
    }

    /**
     * See {@link Builder#allowServerMigration}
     *
     * {@hide}
     */
    @Nullable
    @Experimental
    public Boolean getAllowServerMigration() {
        return mAllowServerMigration;
    }

    /**
     * See {@link Builder#migrateIdleConnections}
     *
     * {@hide}
     */
    @Nullable
    @Experimental
    public Boolean getMigrateIdleConnections() {
        return mMigrateIdleConnections;
    }

    /**
     * See {@link Builder#setIdleMigrationPeriodSeconds}
     *
     * {@hide}
     */
    @Experimental
    @Nullable
    public Duration getIdleMigrationPeriod() {
        return mIdleMigrationPeriod;
    }

    /**
     * See {@link Builder#retryPreHandshakeErrorsOnNonDefaultNetwork}
     *
     * {@hide}
     */
    @Experimental
    @Nullable
    public Boolean getRetryPreHandshakeErrorsOnNonDefaultNetwork() {
        return mRetryPreHandshakeErrorsOnNonDefaultNetwork;
    }

    /**
     * See {@link Builder#allowNonDefaultNetworkUsage}
     *
     * {@hide}
     */
    @Experimental
    @Nullable
    public Boolean getAllowNonDefaultNetworkUsage() {
        return mAllowNonDefaultNetworkUsage;
    }

    /**
     * See {@link Builder#setMaxTimeOnNonDefaultNetworkSeconds}
     *
     * {@hide}
     */
    @Experimental
    @Nullable
    public Duration getMaxTimeOnNonDefaultNetwork() {
        return mMaxTimeOnNonDefaultNetwork;
    }

    /**
     * See {@link Builder#setMaxWriteErrorNonDefaultNetworkMigrationsCount}
     *
     * {@hide}
     */
    @Experimental
    @Nullable
    public Integer getMaxWriteErrorNonDefaultNetworkMigrationsCount() {
        return mMaxWriteErrorNonDefaultNetworkMigrationsCount;
    }

    /**
     * See {@link Builder#setMaxPathDegradingNonDefaultNetworkMigrationsCount}
     *
     * {@hide}
     */
    @Experimental
    @Nullable
    public Integer getMaxPathDegradingNonDefaultMigrationsCount() {
        return mMaxPathDegradingNonDefaultMigrationsCount;
    }

    ConnectionMigrationOptions(Builder builder) {
        this.mEnableDefaultNetworkMigration = builder.mEnableDefaultNetworkMigration;
        this.mEnablePathDegradationMigration = builder.mEnablePathDegradationMigration;
        this.mAllowServerMigration = builder.mAllowServerMigration;
        this.mMigrateIdleConnections = builder.mMigrateIdleConnections;
        this.mIdleMigrationPeriod = builder.mIdleConnectionMigrationPeriod;
        this.mRetryPreHandshakeErrorsOnNonDefaultNetwork =
                builder.mRetryPreHandshakeErrorsOnAlternateNetwork;
        this.mAllowNonDefaultNetworkUsage = builder.mAllowNonDefaultNetworkUsage;
        this.mMaxTimeOnNonDefaultNetwork = builder.mMaxTimeOnNonDefaultNetwork;
        this.mMaxWriteErrorNonDefaultNetworkMigrationsCount = builder.mMaxWriteErrorNonDefaultNetworkMigrationsCount;
        this.mMaxPathDegradingNonDefaultMigrationsCount = builder.mMaxPathDegradingNonDefaultMigrationsCount;
    }

    /**
     * Builder for {@link ConnectionMigrationOptions}.
     */
    public static class Builder {
        @Nullable
        private Boolean mEnableDefaultNetworkMigration;
        @Nullable
        private Boolean mEnablePathDegradationMigration;
        @Nullable
        private Boolean mAllowServerMigration;
        @Nullable
        private Boolean mMigrateIdleConnections;
        @Nullable
        private Duration mIdleConnectionMigrationPeriod;
        @Nullable
        private Boolean mRetryPreHandshakeErrorsOnAlternateNetwork;
        @Nullable
        private Boolean mAllowNonDefaultNetworkUsage;
        @Nullable
        private Duration mMaxTimeOnNonDefaultNetwork;
        @Nullable
        private Integer mMaxWriteErrorNonDefaultNetworkMigrationsCount;
        @Nullable
        private Integer mMaxPathDegradingNonDefaultMigrationsCount;

        Builder() {}

        /**
         * Enables the possibility of migrating connections on default network change. If enabled,
         * active QUIC connections will be migrated onto the new network when the platform indicates
         * that the default network is changing.
         *
         * @see <a href="https://developer.android.com/training/basics/network-ops/reading-network-state#listening-events">Android
         *     Network State</a>
         *
         * @return this builder for chaining
         */
        public Builder enableDefaultNetworkMigration(
                boolean enableDefaultNetworkConnectionMigration) {
            this.mEnableDefaultNetworkMigration = enableDefaultNetworkConnectionMigration;
            return this;
        }

        /**
         * Enables the possibility of migrating connections if the current path is performing
         * poorly.
         *
         * <p>Depending on other configuration, this can result to migrating the connections within
         * the same default network, or to a non-default network.
         *
         * @see #allowNonDefaultNetworkUsage(boolean)
         *
         * @return this builder for chaining
         */
        public Builder enablePathDegradationMigration(boolean enable) {
            this.mEnablePathDegradationMigration = enable;
            return this;
        }

        /**
         * Enables the possibility of migrating connections to an alternate server address
         * at the server's request.
         *
         * @return this builder for chaining
         *
         * {@hide}
         */
        @Experimental
        public Builder allowServerMigration(boolean allowServerMigration) {
            this.mAllowServerMigration = allowServerMigration;
            return this;
        }

        /**
         * Configures whether migration of idle connections should be enabled or not.
         *
         * <p>If set to true, idle connections will be migrated too, as long as they haven't been
         * idle for too long. The setting is shared for all connection migration types. The maximum
         * idle period for which connections will still be migrated can be customized using {@link
         * #setIdleMigrationPeriodSeconds}.
         *
         * @return this builder for chaining
         *
         * {@hide}
         */
        @Experimental
        public Builder migrateIdleConnections(boolean migrateIdleConnections) {
            this.mMigrateIdleConnections = migrateIdleConnections;
            return this;
        }

        /**
         * Sets the maximum idle period for which connections will still be migrated, in seconds.
         * The setting is shared for all connection migration types.
         *
         * <p>Only relevant if {@link #migrateIdleConnections(boolean)} is enabled.
         *
         * @return this builder for chaining
         *
         * {@hide}
         */
        @Experimental
        public Builder setIdleMigrationPeriodSeconds(
                Duration idleConnectionMigrationPeriod) {
            this.mIdleConnectionMigrationPeriod = idleConnectionMigrationPeriod;
            return this;
        }

        /**
         * Sets whether connections can be migrated to an alternate network when Cronet detects
         * a degradation of the path currently in use.
         *
         * <p>Note: This setting can result in requests being sent on non-default metered networks.
         * Make sure you're using metered networks sparingly, and fine tune parameters like
         * {@link #setMaxPathDegradingNonDefaultNetworkMigrationsCount(int)}
         * and {@link #setMaxTimeOnNonDefaultNetworkSeconds} to limit the time on non-default
         * networks.
         *
         * @return this builder for chaining
         *
         * {@hide}
         */
        @Experimental
        public Builder allowNonDefaultNetworkUsage(boolean enable) {
            this.mAllowNonDefaultNetworkUsage = enable;
            return this;
        }

        /**
         * Sets the maximum period for which connections migrated to non-default networks remain
         * there before they're migrated back. This time is not cumulative - each migration off
         * the default network for each connection measures and compares to this value separately.
         *
         * <p>Only relevant if {@link #allowNonDefaultNetworkUsage(boolean)} is enabled.
         *
         * @return this builder for chaining
         *
         * {@hide}
         */
        @Experimental
        public Builder setMaxTimeOnNonDefaultNetworkSeconds(
                Duration maxTimeOnNonDefaultNetwork) {
            this.mMaxTimeOnNonDefaultNetwork = maxTimeOnNonDefaultNetwork;
            return this;
        }

        /**
         * Sets the maximum number of migrations to the non-default network upon encountering write
         * errors. Counted cumulatively per network per connection.
         *
         * <p>Only relevant if {@link #allowNonDefaultNetworkUsage(boolean)} is enabled.
         *
         * @return this builder for chaining
         *
         * {@hide}
         */
        @Experimental
        public Builder setMaxWriteErrorNonDefaultNetworkMigrationsCount(
                int maxWriteErrorNonDefaultMigrationsCount) {
            this.mMaxWriteErrorNonDefaultNetworkMigrationsCount = maxWriteErrorNonDefaultMigrationsCount;
            return this;
        }

        /**
         * Sets the maximum number of migrations to the non-default network upon encountering path
         * degradation for the existing connection. Counted cumulatively per network per connection.
         *
         * <p>Only relevant if {@link #allowNonDefaultNetworkUsage(boolean)} is enabled.
         *
         * @return this builder for chaining
         *
         * {@hide}
         */
        @Experimental
        public Builder setMaxPathDegradingNonDefaultNetworkMigrationsCount(
                int maxPathDegradingNonDefaultMigrationsCount) {
            this.mMaxPathDegradingNonDefaultMigrationsCount = maxPathDegradingNonDefaultMigrationsCount;
            return this;
        }

        /**
         * Sets whether connections with pre-handshake errors should be retried on an alternative
         * network.
         *
         * <p>If true, a new connection may be established an alternate network if it fails
         * on the default network before handshake is confirmed.
         *
         * <p>Note: similarly to {@link #allowNonDefaultNetworkUsage(boolean)} this setting can
         * result in requests being sent on non-default metered networks. Use with caution!
         *
         * @return this builder for chaining
         *
         * {@hide}
         */
        @Experimental
        public Builder retryPreHandshakeErrorsOnNonDefaultNetwork(
                boolean retryPreHandshakeErrorsOnAlternateNetwork) {
            this.mRetryPreHandshakeErrorsOnAlternateNetwork =
                    retryPreHandshakeErrorsOnAlternateNetwork;
            return this;
        }

        /**
         * Creates and returns the final {@link ConnectionMigrationOptions} instance, based on the
         * values in this builder.
         */
        public ConnectionMigrationOptions build() {
            return new ConnectionMigrationOptions(this);
        }
    }

    public static Builder builder() {
        return new Builder();
    }

    /**
     * An annotation for APIs which are not considered stable yet.
     *
     * <p>Applications using experimental APIs must acknowledge that they're aware of using APIs
     * that are not considered stable. The APIs might change functionality, break or cease to exist
     * without notice.
     *
     * <p>It's highly recommended to reach out to Cronet maintainers ({@code net-dev@chromium.org})
     * before using one of the APIs annotated as experimental outside of debugging
     * and proof-of-concept code. Be ready to help to help polishing the API, or for a "sorry,
     * really not production ready yet".
     *
     * <p>If you still want to use an experimental API in production, you're doing so at your
     * own risk. You have been warned.
     *
     * {@hide}
     */
    public @interface Experimental {}
}