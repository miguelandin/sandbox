#pragma once
#include "ArchHandler.hpp"
#include "MemoryUtils.hpp"
#include <concepts>
#include <memory>
#include <string>

class Tracer {
private:
  std::unique_ptr<ArchHandler> m_archHandler;

public:
  Tracer();

  template <typename... Handlers>
    requires(std::invocable<Handlers, std::string> && ...)
  void run(Handlers... handlers);
};
