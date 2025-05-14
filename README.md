# WorkQueue

## Overview

WorkQueue is a C++ library that provides implementations for:
- Timer Thread (TickThread)
- Threaded Worker Queue (WorkQueue)
- Threaded Worker Queue Pool (WorkQueuePool)

This library offers efficient thread management solutions for various concurrent programming needs in C++ applications.

## Features

### Timer Thread (TickThread)
- Provides precise timing capabilities
- Can execute periodic tasks at specified intervals
- Low-overhead timing mechanism

### Threaded Worker Queue (WorkQueue)
- Thread-safe task queue implementation
- Guarantees FIFO (First-In-First-Out) execution of tasks
- Handles task submission and execution in a dedicated worker thread
- Supports synchronous and asynchronous task processing

### Worker Queue Pool (WorkQueuePool)
- Manages a pool of worker threads
- Efficiently distributes tasks across multiple worker threads
- Reduces thread creation/destruction overhead
- Optimizes concurrent execution on multi-core systems

## Use Cases

- Implementing event loops with timing capabilities
- Building multi-threaded applications with task queues
- Creating background processing systems
- Developing systems requiring periodic task execution
- Implementing producer-consumer patterns
- Building scalable server applications

## Requirements

- C++11 compatible compiler
- Support for standard threading libraries

## Installation

Clone the repository:

```bash
git clone https://github.com/cangursu/WorkQueue.git
```

Include the necessary headers in your project:

```cpp
#include "WorkQueue/TickThread.h"
#include "WorkQueue/WorkQueue.h"
#include "WorkQueue/WorkQueuePool.h"
```

## Basic Usage

### TickThread Example

```cpp
// Create a timer thread with a 100ms tick interval
TickThread timer(100);

// Add a task to be executed periodically
timer.AddTask([](){ 
    std::cout << "Timer tick!" << std::endl; 
}, 500); // Execute every 500ms

// Start the timer thread
timer.Start();

// ... later when done
timer.Stop();
```

### WorkQueue Example

```cpp
// Create a worker queue
WorkQueue worker;

// Submit a task to the queue
worker.Submit([](){ 
    std::cout << "Task executed!" << std::endl; 
});

// Submit a task and get a future for the result
auto future = worker.Submit<int>([](){
    // Do some work
    return 42;
});

// Get the result (blocks until task completes)
int result = future.get();
```

### WorkQueuePool Example

```cpp
// Create a worker queue pool with 4 threads
WorkQueuePool pool(4);

// Submit multiple tasks to the pool
for (int i = 0; i < 100; i++) {
    pool.Submit([i](){
        std::cout << "Task " << i << " executed!" << std::endl;
    });
}

// Wait for all tasks to complete
pool.WaitForCompletion();
```

## Advanced Features

- Exception handling in worker threads
- Task prioritization
- Task cancellation
- Thread-safe execution guarantees
- Resource management and cleanup

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

## License

This project is licensed under the MIT License - see the LICENSE file for details.

## Authors

- [cangursu](https://github.com/cangursu)
