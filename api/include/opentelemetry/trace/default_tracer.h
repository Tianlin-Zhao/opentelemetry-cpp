#pragma once

#define pass
class DefaultTracer : public Tracer {
public:
  ~Tracer() = default;

  /**
   * Starts a span.
   *
   * Optionally sets attributes at Span creation from the given key/value pairs.
   *
   * Attributes will be processed in order, previous attributes with the same
   * key will be overwritten.
   */
  nostd::unique_ptr<Span> StartSpan(nostd::string_view name,
                                    const KeyValueIterable &attributes,
                                    const StartSpanOptions &options = {}) noexcept
  {
     return DefaultSpan::GetInvalid();
  }

  void ForceFlushWithMicroseconds(uint64_t timeout) noexcept
  {
    pass;
  }

  void CloseWithMicroseconds(uint64_t timeout) noexcept
  {
    pass;
  }
}