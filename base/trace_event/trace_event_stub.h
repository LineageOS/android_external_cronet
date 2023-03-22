
// Copyright 2020 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#ifndef BASE_TRACE_EVENT_TRACE_EVENT_STUB_H_
#define BASE_TRACE_EVENT_TRACE_EVENT_STUB_H_
#include <stddef.h>
#include <cstdint>
#include <memory>
#include <string>
#include "base/base_export.h"
#include "base/memory/weak_ptr.h"
#include "base/memory/ref_counted_memory.h"
#include "base/task/single_thread_task_runner.h"
#include "base/strings/string_piece.h"
#include "base/trace_event/common/trace_event_common.h"
#include "base/trace_event/memory_allocator_dump_guid.h"
#include "base/values.h"
#define TRACE_STR_COPY(str) str
#define TRACE_ID_WITH_SCOPE(scope, ...) 0
#define TRACE_ID_GLOBAL(id) 0
#define TRACE_ID_LOCAL(id) 0
namespace trace_event_internal {
const unsigned long long kNoId = 0;
template <typename... Args>
void Ignore(Args&&... args) {}
struct IgnoredValue {
  template <typename... Args>
  IgnoredValue(Args&&... args) {}
};
}  // namespace trace_event_internal
#define INTERNAL_TRACE_IGNORE(...) \
  (false ? trace_event_internal::Ignore(__VA_ARGS__) : (void)0)
#define INTERNAL_TRACE_EVENT_ADD(...) INTERNAL_TRACE_IGNORE(__VA_ARGS__)
#define INTERNAL_TRACE_EVENT_ADD_SCOPED(...) INTERNAL_TRACE_IGNORE(__VA_ARGS__)
#define INTERNAL_TRACE_EVENT_ADD_WITH_ID(...) INTERNAL_TRACE_IGNORE(__VA_ARGS__)
#define INTERNAL_TRACE_LOG_MESSAGE(...) INTERNAL_TRACE_IGNORE(__VA_ARGS__)
#define INTERNAL_TRACE_EVENT_ADD_SCOPED_WITH_FLOW(...) \
  INTERNAL_TRACE_IGNORE(__VA_ARGS__)
#define INTERNAL_TRACE_EVENT_ADD_WITH_ID_TID_AND_TIMESTAMP(...) \
  INTERNAL_TRACE_IGNORE(__VA_ARGS__)
#define INTERNAL_TRACE_EVENT_ADD_WITH_ID_TID_AND_TIMESTAMPS(...) \
  INTERNAL_TRACE_IGNORE(__VA_ARGS__)
// Defined in application_state_proto_android.h
#define TRACE_APPLICATION_STATE(...) INTERNAL_TRACE_IGNORE(__VA_ARGS__)
#define TRACE_HEAP_PROFILER_API_SCOPED_TASK_EXECUTION \
  trace_event_internal::IgnoredValue
#define TRACE_ID_MANGLE(val) (val)
#define INTERNAL_TRACE_EVENT_GET_CATEGORY_INFO(cat) INTERNAL_TRACE_IGNORE(cat);
#define INTERNAL_TRACE_EVENT_CATEGORY_GROUP_ENABLED_FOR_RECORDING_MODE() false
#define TRACE_EVENT_API_CURRENT_THREAD_ID 0
// Typed macros. For these, we have to erase the extra args entirely, as they
// may include a lambda that refers to protozero message types (which aren't
// available in the stub). This may trigger "unused variable" errors at the
// callsite, which have to be addressed at the callsite (e.g. via
// [[maybe_unused]]).
#define TRACE_EVENT_BEGIN(category, name, ...) \
  INTERNAL_TRACE_IGNORE(category, name)
#define TRACE_EVENT_END(category, ...) INTERNAL_TRACE_IGNORE(category)
#define TRACE_EVENT(category, name, ...) INTERNAL_TRACE_IGNORE(category, name)
#define TRACE_EVENT_INSTANT(category, name, ...) \
  INTERNAL_TRACE_IGNORE(category, name)
#define PERFETTO_INTERNAL_ADD_EMPTY_EVENT() INTERNAL_TRACE_IGNORE()
namespace protozero {
namespace proto_utils {
namespace internal {
template <typename T>
using FieldMetadataHelper = T (*)(void);
}
}
}
namespace base {
namespace trace_event {
class BASE_EXPORT ConvertableToTraceFormat {
 public:
  ConvertableToTraceFormat() = default;
  ConvertableToTraceFormat(const ConvertableToTraceFormat&) = delete;
  ConvertableToTraceFormat& operator=(const ConvertableToTraceFormat&) = delete;
  virtual ~ConvertableToTraceFormat();
  // Append the class info to the provided |out| string. The appended
  // data must be a valid JSON object. Strings must be properly quoted, and
  // escaped. There is no processing applied to the content after it is
  // appended.
  virtual void AppendAsTraceFormat(std::string* out) const = 0;
};
class BASE_EXPORT TracedValue : public ConvertableToTraceFormat {
 public:
  explicit TracedValue(size_t capacity = 0) {}
  void EndDictionary() {}
  void EndArray() {}
  void SetInteger(const char* name, int value) {}
  void SetDouble(const char* name, double value) {}
  void SetBoolean(const char* name, bool value) {}
  void SetString(const char* name, base::StringPiece value) {}
  void SetValue(const char* name, TracedValue* value) {}
  void BeginDictionary(const char* name) {}
  void BeginArray(const char* name) {}
  void SetIntegerWithCopiedName(base::StringPiece name, int value) {}
  void SetDoubleWithCopiedName(base::StringPiece name, double value) {}
  void SetBooleanWithCopiedName(base::StringPiece name, bool value) {}
  void SetStringWithCopiedName(base::StringPiece name,
                               base::StringPiece value) {}
  void SetValueWithCopiedName(base::StringPiece name, TracedValue* value) {}
  void BeginDictionaryWithCopiedName(base::StringPiece name) {}
  void BeginArrayWithCopiedName(base::StringPiece name) {}
  void AppendInteger(int) {}
  void AppendDouble(double) {}
  void AppendBoolean(bool) {}
  void AppendString(base::StringPiece) {}
  void BeginArray() {}
  void BeginDictionary() {}
  void AppendAsTraceFormat(std::string* out) const override;
};
class BASE_EXPORT TracedValueJSON : public TracedValue {
 public:
  explicit TracedValueJSON(size_t capacity = 0) : TracedValue(capacity) {}
  std::unique_ptr<base::Value> ToBaseValue() const { return nullptr; }
  std::string ToJSON() const { return ""; }
  std::string ToFormattedJSON() const { return ""; }
};
struct MemoryDumpArgs;
class MemoryAllocatorDumpGuid;
class BASE_EXPORT MemoryAllocatorDump {
    public:
        enum Flags {
            DEFAULT = 0,
            // A dump marked weak will be discarded by TraceViewer.
            WEAK = 1 << 0,
        };
        struct BASE_EXPORT Entry {
            enum EntryType {
              kUint64,
              kString,
            };
            // By design name, units and value_string are  always coming from
            // indefinitely lived const char* strings, the only reason we copy
            // them into a std::string is to handle Mojo (de)serialization.
            // TODO(hjd): Investigate optimization (e.g. using StringPiece).
            Entry();  // Only for deserialization.
            Entry(std::string name, std::string units, uint64_t value);
            Entry(std::string name, std::string units, std::string value);
            Entry(Entry&& other) noexcept;
            Entry(const Entry&) = delete;
            Entry& operator=(const Entry&) = delete;
            Entry& operator=(Entry&& other);
            bool operator==(const Entry& rhs) const;
            std::string name;
            std::string units;
            EntryType entry_type;
            uint64_t value_uint64;
            std::string value_string;
          };
        void AddScalar(const char* name, const char* units, uint64_t value) {}
        const MemoryAllocatorDumpGuid& guid() const {
           static MemoryAllocatorDumpGuid obj;
           return obj;
        }
        static const char kNameSize[];
        static const char kUnitsBytes[];
        static const char kUnitsObjects[];
};
class ProcessMemoryDump {
    public:
    MemoryAllocatorDump* CreateAllocatorDump(const std::string& absolute_name) {
        static MemoryAllocatorDump obj;
        return &obj;
    }
    MemoryAllocatorDump* CreateAllocatorDump(const std::string& absolute_name,
                                               const MemoryAllocatorDumpGuid& guid) {
    static MemoryAllocatorDump obj;
    return &obj;
   }
    void AddSuballocation(const MemoryAllocatorDumpGuid& source,
                            const std::string& target_node_name) {}
};
class BASE_EXPORT MemoryDumpProvider {
 public:
  struct Options {
      Options() : dumps_on_single_thread_task_runner(false) {}
      // |dumps_on_single_thread_task_runner| is true if the dump provider runs on
      // a SingleThreadTaskRunner, which is usually the case. It is faster to run
      // all providers that run on the same thread together without thread hops.
      bool dumps_on_single_thread_task_runner;
  };
  MemoryDumpProvider(const MemoryDumpProvider&) = delete;
  MemoryDumpProvider& operator=(const MemoryDumpProvider&) = delete;
  virtual ~MemoryDumpProvider();
  virtual bool OnMemoryDump(const MemoryDumpArgs& args,
                            ProcessMemoryDump* pmd) = 0;
 protected:
  MemoryDumpProvider() = default;
};
class BASE_EXPORT MemoryDumpManager {
 public:
  static constexpr const char* const kTraceCategory =
      TRACE_DISABLED_BY_DEFAULT("memory-infra");
  static MemoryDumpManager* GetInstance() {
    static MemoryDumpManager obj;
    return &obj;
  }
   const char* system_allocator_pool_name() {
    static const char* obj;
    return obj;
   }
   void RegisterDumpProvider(MemoryDumpProvider* mdp,
                              const char* name,
                              scoped_refptr<SingleThreadTaskRunner> task_runner) {}
   void RegisterDumpProvider(MemoryDumpProvider* mdp,
                              const char* name,
                              scoped_refptr<SingleThreadTaskRunner> task_runner,
                              MemoryDumpProvider::Options options) {}
   void UnregisterAndDeleteDumpProviderSoon(std::unique_ptr<MemoryDumpProvider> mdp) {};
};
enum TraceRecordMode {
  // Record until the trace buffer is full.
  RECORD_UNTIL_FULL,
  // Record until the user ends the trace. The trace buffer is a fixed size
  // and we use it as a ring buffer during recording.
  RECORD_CONTINUOUSLY,
  // Record until the trace buffer is full, but with a huge buffer size.
  RECORD_AS_MUCH_AS_POSSIBLE,
  // Echo to console. Events are discarded.
  ECHO_TO_CONSOLE,
};
class BASE_EXPORT TraceConfig {
    public:
        TraceConfig(StringPiece category_filter_string,
                      StringPiece trace_options_string) {}
        TraceConfig(StringPiece category_filter_string, TraceRecordMode record_mode) {}
};
class BASE_EXPORT TraceBufferChunk {
 public:
  void Reset(uint32_t new_seq);
  bool IsFull() const { return next_free_ == kTraceBufferChunkSize; }
  uint32_t seq() const { return seq_; }
  size_t capacity() const { return kTraceBufferChunkSize; }
  size_t size() const { return next_free_; }
  // These values must be kept consistent with the numbers of bits of
  // chunk_index and event_index fields in TraceEventHandle
  // (in trace_event_impl.h).
  static const size_t kMaxChunkIndex = (1u << 26) - 1;
  static const size_t kTraceBufferChunkSize = 64;
 private:
  size_t next_free_;
  uint32_t seq_;
};
class BASE_EXPORT TraceBuffer {
 public:
  // For iteration. Each TraceBuffer can only be iterated once.
  const TraceBufferChunk* NextChunk() {
    static TraceBufferChunk obj;
    return &obj;
  }
  static TraceBuffer* CreateTraceBufferRingBuffer(size_t max_chunks) {
    static TraceBuffer obj;
    return &obj;
  }
  static TraceBuffer* CreateTraceBufferVectorOfSize(size_t max_chunks) {
    static TraceBuffer obj;
    return &obj;
  }
};
class BASE_EXPORT TraceLog {
 public:
  class BASE_EXPORT AsyncEnabledStateObserver {
   public:
    virtual ~AsyncEnabledStateObserver() = default;
    virtual void OnTraceLogEnabled() = 0;
    virtual void OnTraceLogDisabled() = 0;
  };
  enum Mode : uint8_t {
      // Enables normal tracing (recording trace events in the trace buffer).
      // This is the only tracing mode supported now.
      // TODO(khokhlov): Clean up all uses of tracing mode and remove this enum
      // completely.
      RECORDING_MODE = 1 << 0,
  };
  static TraceLog* GetInstance() {
    static TraceLog obj;
    return &obj;
  }
  void SetTraceBufferForTesting(std::unique_ptr<TraceBuffer> trace_buffer) {}
  bool BufferIsFull() {
    return true;
  }
  bool IsEnabled() {
    return false;
  }
  void SetEnabled(const TraceConfig& trace_config, uint8_t modes_to_enable) {}
  using OutputCallback =
        base::RepeatingCallback<void(const scoped_refptr<base::RefCountedString>&,
                                     bool has_more_events)>;
  void Flush(const OutputCallback& cb, bool use_worker_thread = false) {}
  void SetDisabled() {}
  void AddAsyncEnabledStateObserver(WeakPtr<AsyncEnabledStateObserver>) {}
  void RemoveAsyncEnabledStateObserver(AsyncEnabledStateObserver*) {}
};
class BASE_EXPORT TraceResultBuffer {
 public:
    using OutputCallback = base::RepeatingCallback<void(const std::string&)>;
    struct BASE_EXPORT SimpleOutput {
        OutputCallback GetCallback() {
            static OutputCallback obj;
            return obj;
        }
        void Append(const std::string& json_string);
        // Do what you want with the json_output_ string after calling
        // TraceResultBuffer::Finish.
        std::string json_output;
    };
    void SetOutputCallback(OutputCallback json_chunk_callback) {}
    void Start() {}
    void AddFragment(const std::string& trace_fragment) {}
    void Finish() {}
};
}  // namespace trace_event
}  // namespace base
// Stub implementation for
// perfetto::StaticString/ThreadTrack/TracedValue/TracedDictionary/TracedArray.
namespace perfetto {
template <typename T>
std::string TracedValueToString(T&& value) {
  return "";
}
class TracedArray;
class TracedDictionary;
class EventContext;
class StaticString {
 public:
  template <typename T>
  StaticString(T) {}
};
class DynamicString {
 public:
  template <typename T>
  explicit DynamicString(T) {}
};
class TracedValue {
 public:
  void WriteInt64(int64_t) && {}
  void WriteUInt64(uint64_t) && {}
  void WriteDouble(double) && {}
  void WriteBoolean(bool) && {}
  void WriteString(const char*) && {}
  void WriteString(const char*, size_t) && {}
  void WriteString(const std::string&) && {}
  void WritePointer(const void*) && {}
  TracedDictionary WriteDictionary() &&;
  TracedArray WriteArray() &&;
};
class TracedDictionary {
 public:
  TracedValue AddItem(StaticString) { return TracedValue(); }
  TracedValue AddItem(DynamicString) { return TracedValue(); }
  template <typename T>
  void Add(StaticString, T&&) {}
  template <typename T>
  void Add(DynamicString, T&&) {}
  TracedDictionary AddDictionary(StaticString);
  TracedDictionary AddDictionary(DynamicString);
  TracedArray AddArray(StaticString);
  TracedArray AddArray(DynamicString);
};
class TracedArray {
 public:
  TracedValue AppendItem() { return TracedValue(); }
  template <typename T>
  void Append(T&&) {}
  TracedDictionary AppendDictionary();
  TracedArray AppendArray();
};
template <class T>
void WriteIntoTracedValue(TracedValue, T&&) {}

namespace protos::pbzero {
namespace SequenceManagerTask {

enum class QueueName {
  UNKNOWN_TQ = 0,
  DEFAULT_TQ = 1,
  TASK_ENVIRONMENT_DEFAULT_TQ = 2,
  TEST2_TQ = 3,
  TEST_TQ = 4,
};
inline const char* QueueName_Name(QueueName value) {
  switch (value) {
    case QueueName::UNKNOWN_TQ:
      return "UNKNOWN_TQ";
    case QueueName::DEFAULT_TQ:
      return "DEFAULT_TQ";
    case QueueName::TASK_ENVIRONMENT_DEFAULT_TQ:
      return "TASK_ENVIRONMENT_DEFAULT_TQ";
    case QueueName::TEST2_TQ:
      return "TEST2_TQ";
    case QueueName::TEST_TQ:
      return "TEST_TQ";
  }
}
}  // namespace protos::pbzero::SequenceManagerTask
template <typename MessageType>
class TracedProto {
 public:
  // implicit
  TracedProto(TracedValue&& value)
      : TracedProto() {}
  ~TracedProto() = default;
  TracedProto(const TracedProto&) = delete;
  TracedProto& operator=(const TracedProto&) = delete;
  TracedProto& operator=(TracedProto&&) = delete;
  TracedProto(TracedProto&&) = default;
  MessageType* operator->() const { return message_; }
  MessageType* message() { return message_; }
  // Write additional untyped values into the same context, which is useful
  // when a given C++ class has a typed representation, but also either has
  // members which can only be written into an untyped context (e.g. they are
  // autogenerated) or it's desirable to have a way to quickly extend the
  // trace representation of this class (e.g. for debugging).
  //
  // The usage of the returned TracedDictionary should not be interleaved with
  // writing into |message| as this results in an inefficient proto layout. To
  // enforce this, AddDebugAnnotations should be called on TracedProto&&, i.e.
  // std::move(message).AddDebugAnnotations().
  //
  // This requires a 'repeated DebugAnnotations debug_annotations' field in
  // MessageType.
  template <typename Check = void>
  TracedDictionary AddDebugAnnotations() && {
    static TracedDictionary obj;
    return obj;
  }
  // Start writing a single entry corresponding to the given |field| and return
  // TracedProto should be used to populate this further.
  // This method requires |field|'s type to be a nested message, but both
  // repeated and non-repeated complex fields are supported.
  template <typename FieldMetadata>
  TracedProto<typename FieldMetadata::cpp_field_type> WriteNestedMessage(
      protozero::proto_utils::internal::FieldMetadataHelper<FieldMetadata>) {
        static TracedProto obj;
        return obj;
  }
  // Write a given |value| into proto  as a new |field| of the current message.
  // This method supports both nested messages and primitive types (i.e. int or
  // string), but requires the |field| to be non-repeateable (i.e. optional).
  // For repeatable fields, AppendValue or AppendFrom should be used.
  template <typename FieldMetadata, typename ValueType>
  void Set(protozero::proto_utils::internal::FieldMetadataHelper<FieldMetadata>,
           ValueType&& value) {}
  // Write a given |value| a single entry into the repeated |field| of the
  // current message. If the field is not repeated, Set() should be used
  // instead.
  template <typename FieldMetadata, typename ValueType>
  void AppendValue(
      protozero::proto_utils::internal::FieldMetadataHelper<FieldMetadata>,
      ValueType&& value) {}
  // Write a given |value| as a set of entries into the repeated |field| of the
  // current message. If the field is not repeated, Set() should be used
  // instead.
  template <typename FieldMetadata, typename ValueType>
  void AppendFrom(
      protozero::proto_utils::internal::FieldMetadataHelper<FieldMetadata>,
      ValueType&& value) {}
  // Write a nested message into a field according to the provided metadata.
  // TODO(altimin): Replace the current usages in Chrome with the functions
  // above and make these methods private.
  template <typename FieldMetadata>
  TracedProto<typename FieldMetadata::cpp_field_type> WriteNestedMessage() {
        static TracedProto obj;
        return obj;
  }
 private:
  friend class EventContext;
  friend class TracedValue;
  // Allow TracedProto<Foo> to create TracedProto<Bar>.
  template <typename T>
  friend class TracedProto;
  // Wraps a raw protozero message using the same context as the current object.
  template <typename ChildMessageType>
  // Context might be null here when writing typed message which is
  // nested into untyped legacy trace event macro argument.
  // TODO(altimin): Turn this into EventContext& when this case is eliminated
  // and expose it in public API.
  EventContext* context() const { return context_; }
  TracedProto(MessageType* message, EventContext* context)
      : message_(message), context_(context) {}
  MessageType* const message_;
  EventContext* context_;
};

}  // namespace SequenceManagerTask

namespace ChromeProcessDescriptor {

enum ProcessType {};

}  // namespace ChromeProcessDescriptor

}  // namespace protos::pbzero
}  // namespace perfetto
#endif  // BASE_TRACE_EVENT_TRACE_EVENT_STUB_H_
