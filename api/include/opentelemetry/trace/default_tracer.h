#pragma once

#include "opentelemetry/trace/default_span.h"
#include "opentelemetry/nostd/string_view.h"
#include "opentelemetry/nostd/unique_ptr.h"
#include "opentelemetry/trace/span.h"
#include "opentelemetry/trace/tracer.h"
#include "opentelemetry/version.h"

#include <chrono>

OPENTELEMETRY_BEGIN_NAMESPACE
namespace trace
{
/**
 * Handles span creation and in-process context propagation.
 *
 * This class provides methods for manipulating the context, creating spans, and controlling spans'
 * lifecycles.
 */
class DefaultTracer : public Tracer
{
public:
  virtual ~DefaultTracer() = default;
  /**
   * Starts a span.
   *
   * Optionally sets attributes at Span creation from the given key/value pairs.
   *
   * Attributes will be processed in order, previous attributes with the same
   * key will be overwritten.
   */
  virtual nostd::unique_ptr<Span> StartSpan(nostd::string_view name,
                                            const KeyValueIterable &attributes,
                                            const StartSpanOptions &options = {}) noexcept {
    return nostd::unique_ptr<Span>(new DefaultSpan(this));
  }

  virtual void ForceFlushWithMicroseconds(uint64_t timeout) noexcept {}

  virtual void CloseWithMicroseconds(uint64_t timeout) noexcept {};
};
}  // namespace trace
OPENTELEMETRY_END_NAMESPACE
