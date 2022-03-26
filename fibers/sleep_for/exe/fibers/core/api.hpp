#pragma once

#include <exe/coroutine/routine.hpp>

#include <exe/fibers/core/time.hpp>

#include <asio/io_context.hpp>

namespace exe::fibers {

using Routine = coroutine::Routine;

// I/O scheduler
using Scheduler = asio::io_context;

// API

// Starts a new fiber
void Go(Scheduler& scheduler, Routine routine);

// Starts a new fiber in the current scheduler
void Go(Routine routine);

namespace self {

void Yield();

void SleepFor(Millis delay);

}  // namespace self

}  // namespace exe::fibers
