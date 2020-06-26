#pragma once

#include <cstdint>
#include "opentelemetry/context/context.h"
#include "opentelemetry/nostd/string_view.h"
#include "opentelemetry/version.h"

OPENTELEMETRY_BEGIN_NAMESPACE
namespace trace
{
namespace propagation
{
// Set the span in the given context.
virtual static Context SetSpanInContext(Span span, Context &context) = 0;
// Retrieve the current span.
virtual static Span GetCurrentSpan(Context &context) = 0;

// The HTTPTextFormat class provides an interface that enables extracting and injecting
// context into headers of HTTP requests. HTTP frameworks and clients
// can integrate with HTTPTextFormat by providing the object containing the
//  headers, and a getter and setter function for the extraction and
// injection of values, respectively.
template <typename T>
class HTTPTextFormat {
    public:
        // Rules that manages how context will be extracted from carrier.
        using Getter = nostd::string_view(*)(T &carrier, nostd::string_view trace_type);

        // Rules that manages how context will be injected to carrier.
        using Setter = void(*)(T &carrier, nostd::string_view trace_type,nostd::string trace_description);

        // Returns the context that is stored in the HTTP header carrier with self defined rules.
        virtual Context extract(Getter get_from_carrier, const T &carrier, Context &context) = 0;

        // Sets the context for a HTTP header carrier with self defined rules.
        virtual void inject(Setter set_from_carrier, T &carrier, const Context &context) = 0;
};
}
}
OPENTELEMETRY_END_NAMESPACE
