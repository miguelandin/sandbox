#include "Tracer.hpp"
#if defined(__x86_64__)
    #include "ArchHandler_x86_64.hpp"
#endif

Tracer::Tracer() {
#if defined(__x86_64__)
  m_archHandler = std::make_unique<ArchHandler_x86_64>();
#endif
}
