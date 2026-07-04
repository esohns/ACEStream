# ACEStream — Architecture Overview

## What is ACEStream?

ACEStream is a C++ wrapper library built on top of the [ACE Framework](https://www.dre.vanderbilt.edu/Doxygen/Stable/libace-doc/a07590.html) that simplifies stream-based data processing. It provides a modular, composable architecture for building data pipelines where modules process and transform data as it flows through the system. Modules can be connected in chains, with built-in support for network I/O, parsing, threading, and message queuing.

**Files in this commit:**
- `ARCHITECTURE.md` (this file)
- `ARCHITECTURE.mmd` (plain Mermaid source for rendering)

---

## Table of Contents

1. [Architecture Diagram](#architecture-diagram)
2. [Component Reference](#component-reference)
3. [Data Flow Example](#data-flow-example)
4. [Design Principles](#design-principles)
5. [Glossary](#glossary)
6. [How to Render Diagrams](#how-to-render-diagrams)

---

## Architecture Diagram

```mermaid
flowchart LR
  subgraph External[External Dependencies]
    ACE["ACE Framework<br/>(ace/Stream, ACE_Module, ACE_Task)"]
    OS["OS Sockets / File APIs"]
  end

  subgraph Library[ACEStream Library (src/)]
    direction TB
    API["Public API<br/>(e.g. Stream_IStream_T)"]
    Base["Stream_Base_T<br/>Core stream engine"]
    StreamModule["Stream_StreamModule_T<br/>Module wrapper"]
    ModuleBase["Stream_Module_Base_T<br/>Module foundation"]
    IModule["Stream_IModule_T<br/>Module interface"]
    NetListener["Stream_Module_Net_ListenerH_T<br/>Network source"]
    NetWriter["Stream_Module_Net_Source_Writer_T<br/>Network sink"]
    Parser["Stream_Module_ParserH_T<br/>Data parser"]
    Tasks["Stream_TaskBaseSynch_T<br/>Threading & scheduling"]
    Data["MessageQueue / Layout<br/>Data structures"]
    Utils["Utilities<br/>Logging, config, errors"]
  end

  subgraph Consumers[Applications & Tests]
    App["Applications / Examples"]
    Tests["Test Suite"]
  end

  ACE -->|provides| Tasks
  OS -->|underlies| ACE

  NetListener --> ModuleBase
  Parser --> ModuleBase
  NetWriter --> ModuleBase
  ModuleBase --> IModule
  IModule --> StreamModule
  StreamModule --> Base
  Base --> API

  Data --> Base
  Tasks --> Base
  Utils --> ModuleBase

  API --> App
  API --> Tests

  style External fill:#fff3cd,stroke:#333,stroke-width:2px
  style Library fill:#f3f4f6,stroke:#333,stroke-width:2px
  style Consumers fill:#e6ffed,stroke:#333,stroke-width:2px
```

**Legend:**
- 🟨 **Yellow (External)** — Dependencies outside the library (ACE Framework, OS APIs)
- ⚫ **Gray (Library)** — ACEStream's internal components
- 🟩 **Green (Consumers)** — Applications that use the library

---

## Component Reference

### Core Components

#### [`Stream_Base_T`](../src/stream_base.h) / [`stream_base.inl`](../src/stream_base.inl)
The **core engine** of the stream. Manages:
- Message flow through the pipeline
- Module lifecycle (initialization, start, stop)
- Task scheduling and thread management
- Synchronization and error handling

**Typical usage:** Instantiate as your main pipeline orchestrator.

#### [`Stream_StreamModule_T`](../src/stream_streammodule_base.h)
**Wrapper** that adapts Stream_Module_Base_T into a reusable component. Provides:
- Consistent interface for all module types
- Registration with the stream
- Lifecycle hooks (init, run, finalize)

**Typical usage:** Inherit from this when creating custom modules.

#### [`Stream_Module_Base_T`](../src/stream_module_base.h)
**Foundation class** for all modules. Defines:
- Module interface and virtual methods
- Configuration and state management
- Access to utilities (logging, error handling)

**Typical usage:** Not instantiated directly; extend Stream_StreamModule_T instead.

#### [`Stream_IModule_T`](../src/stream_imodule.h)
**Interface definition** for modules. Specifies:
- Callbacks (open, process, close)
- Data and control message handling
- Module state transitions

**Typical usage:** Reference for understanding the contract between stream and modules.

### I/O Modules

#### [`Stream_Module_Net_ListenerH_T`](../src/modules/net/stream_net_listener.h)
**Network source** — receives data from sockets. Handles:
- Socket binding and listening
- Inbound connections
- Data reception and frame parsing

**Typical usage:** Place at the head of a pipeline to read from the network.

#### [`Stream_Module_Net_Source_Writer_T`](../src/modules/net/stream_net_source.h)
**Network sink** — sends data to network endpoints. Handles:
- Connection management
- Outbound data transmission
- Connection pooling and reuse

**Typical usage:** Place at the tail of a pipeline to send processed data over the network.

### Processing Modules

#### [`Stream_Module_ParserH_T`](../src/modules/misc/stream_misc_parser.inl)
**Data transformation** — parses/transforms incoming data. Handles:
- Protocol parsing
- Message deserialization
- Data validation and extraction

**Typical usage:** Insert in the middle of a pipeline for protocol handling.

### Threading & Execution

#### [`Stream_TaskBaseSynch_T`](../src/stream_task_base.h)
**Synchronous task executor** — manages threading for modules. Provides:
- Thread pool management
- Message queue semantics (synchronous/asynchronous)
- Task scheduling and coordination

**Typical usage:** Created automatically by Stream_Base_T; customize thread pool behavior via configuration.

### Data Structures

#### [`stream_layout.h`](../src/stream_layout.h)
Defines:
- Message envelope and metadata
- Frame layout and serialization
- Common data type definitions

#### [`stream_messagequeue.h`](../src/stream_messagequeue.h)
Implements:
- Thread-safe message queuing
- Producer/consumer coordination
- Queue statistics and monitoring

### Utilities

#### [`src/`](../src/) — Logging, Configuration, Error Handling
- **Logging:** Structured logging with levels and filtering
- **Configuration:** INI/JSON-based module setup
- **Error handling:** Exception safe designs and error codes

---

## Data Flow Example

Here's how data moves through a typical ACEStream pipeline:

```
┌─────────────────────┐
│  Network Socket     │  (OS file descriptor)
└──────────┬──────────┘
           │ (TCP/UDP frames)
           ▼
┌─────────────────────────────────────┐
│ Stream_Module_Net_ListenerH_T       │  ◄── receives raw network data
│ (Network Source Module)             │      creates Message objects
└──────────┬──────────────────────────┘
           │ (stream::Message<T>)
           ▼
┌─────────────────────────────────────┐
│ Stream_Module_ParserH_T             │  ◄── deserializes/validates
│ (Parser Module)                     │      transforms to application types
└──────────┬──────────────────────────┘
           │ (parsed data)
           ▼
┌─────────────────────────────────────┐
│ Custom Processing Module            │  ◄── business logic
│ (inherit from Stream_StreamModule_T)│      filtering, enrichment, etc.
└──────────┬──────────────────────────┘
           │ (processed data)
           ▼
┌─────────────────────────────────────┐
│ Stream_Module_Net_Source_Writer_T   │  ◄── encodes & transmits
│ (Network Sink Module)               │
└──────────┬──────────────────────────┘
           │ (TCP/UDP frames)
           ▼
┌─────────────────────┐
│  Network Socket     │  (transmitted to peers)
└─────────────────────┘
```

**Threading:** Stream_TaskBaseSynch_T manages each module's execution, allowing:
- Modules to run in dedicated threads
- Asynchronous message passing between stages
- Graceful shutdown and resource cleanup

---

## Design Principles

### Template-Heavy Design
ACEStream uses C++ templates extensively to:
- **Avoid runtime polymorphism overhead** — templates allow compile-time specialization
- **Support type safety** — message types are known at compile time
- **Enable inlining** — hot paths are optimized by the compiler

**Trade-off:** Larger binary size, longer compile times, but faster runtime performance.

### Module Composition
- Modules are **independently testable** — each has clear input/output contracts
- Modules are **loosely coupled** — connected via message queues, not direct calls
- Modules are **reusable** — same module can appear multiple times in different pipelines

### Thread Safety
- **Message passing**, not shared memory — reduces contention
- **Queue-based coordination** — Stream_TaskBaseSynch_T handles synchronization
- **Minimize locks** — most operations are lock-free or use minimal locking

### Error Handling
- **Exceptions are avoided in hot paths** — prefer error codes
- **Graceful degradation** — modules can filter/drop bad data without crashing
- **Logging at all levels** — troubleshooting is built-in

---

## Glossary

| Term | Definition |
|------|-----------|
| **ACE** | Adaptive Communication Environment — a C++ framework for concurrent networked applications |
| **Module** | A reusable processing component (source, sink, or filter) in a stream pipeline |
| **Task** | An ACE abstraction for concurrent execution; often runs in its own thread |
| **Message** | A data packet flowing through the stream; carries both data and metadata |
| **Queue** | Thread-safe FIFO structure for asynchronous communication between modules |
| **Head Module** | A module at the start of a pipeline (source of data, usually a listener) |
| **Sink Module** | A module at the end of a pipeline (consumes data, often a writer) |
| **Pipeline** | A chain of modules where output of one feeds into input of the next |

---

## How to Render Diagrams

### Quick Preview
Open [`ARCHITECTURE.mmd`](./ARCHITECTURE.mmd) in the [Mermaid Live Editor](https://mermaid.live) for instant rendering.

### Local Rendering with mermaid-cli

1. Install mermaid-cli (requires Node.js):
   ```bash
   npm install -g @mermaid-js/mermaid-cli
   ```

2. Render to SVG:
   ```bash
   mmdc -i ARCHITECTURE.mmd -o ARCHITECTURE.svg
   ```

3. View the SVG in your browser or embed it in documentation.

### In GitHub
GitHub automatically renders Mermaid diagrams in markdown, so the diagram above displays natively.

---

## Next Steps

- **Read the [README](../README.md)** for build instructions and quick start
- **Explore [examples/](../examples/)** for sample pipelines
- **Check [test_i/](../test_i/)** for integration test examples
- **Review component headers** (e.g., `stream_base.h`) for detailed API documentation
