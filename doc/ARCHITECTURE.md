# ACEStream — Architecture Overview

This document provides a high-level architecture overview of the ACEStream wrapper library and maps the Mermaid diagram to real classes and headers found in the repository.

> Files added to the repository by this commit:
> - ARCHITECTURE.md (this file)
> - ARCHITECTURE.mmd (plain Mermaid source for easy rendering with CLI / online tools)

## Diagram

```mermaid
flowchart LR
  subgraph External[External Dependencies]
    ACE["ACE Framework\n(ace/Stream, ACE_Module, ACE_Task)"]
    OS["OS Sockets / File APIs"]
  end

  subgraph Library[ACEStream Library (src/)]
    direction TB
    API["Public API\n(e.g. Stream_IStream_T, stream_base.h)"]
    Base["Stream_Base_T\n(src/stream_base.h/.inl)"]
    StreamModule["Stream_StreamModule_T\n(src/stream_streammodule_base.h)"]
    ModuleBase["Stream_Module_Base_T\n(src/stream_module_base.h)"]
    IModule["Stream_IModule_T\n(src/stream_imodule.h)"]
    NetWriter["Stream_Module_Net_Source_Writer_T\n(src/modules/net/stream_net_source.h)"]
    NetListener["Stream_Module_Net_ListenerH_T\n(src/modules/net/stream_net_listener.h)"]
    Parser["Stream_Module_ParserH_T\n(src/modules/misc/stream_misc_parser.inl)"]
    Tasks["ACE_Thru_Task / TaskBase / Stream_TaskBaseSynch_T\n(src/*_task*.h)"]
    Data["Layout / MessageQueue / Data types\n(stream_layout.h, stream_messagequeue.h)"]
    Utils["Utilities\n(logging, config, errors)"]
  end

  subgraph Consumers[Applications & Tests]
    App["Applications / Examples\n(test_i/, examples)"]
    Tests["Test Suite\n(test_i/)"]
  end

  ACE -->|uses| NetWriter
  ACE -->|provides| Tasks
  OS -->|underlies| ACE

  NetWriter --> ModuleBase
  NetListener --> ModuleBase
  Parser --> ModuleBase
  ModuleBase --> StreamModule
  StreamModule --> Base
  Base --> API

  Data --> Base
  Tasks --> ModuleBase
  Utils --> ModuleBase

  API --> App
  API --> Tests
  Tests --> Base

  style External fill:#fff3cd,stroke:#333,stroke-width:1px
  style Library fill:#f3f4f6,stroke:#333,stroke-width:1px
  style Consumers fill:#e6ffed,stroke:#333,stroke-width:1px
```

## Component mapping (selected files / classes)

- Stream_Base_T — core stream implementation (src/stream_base.h, src/stream_base.inl)
- Stream_StreamModule_T — module wrapper for stream components (src/stream_streammodule_base.h)
- Stream_Module_Base_T — base class for modules (src/stream_module_base.h)
- Stream_IModule_T — module interface and callbacks (src/stream_imodule.h)
- Stream_Module_Net_Source_Writer_T — network source writer/connector (src/modules/net/stream_net_source.h)
- Stream_Module_Net_ListenerH_T — network listener / head module (src/modules/net/stream_net_listener.h)
- Stream_Module_ParserH_T — parser head module (src/modules/misc/stream_misc_parser.inl)
- Message queue / layout — stream_layout.h, stream_messagequeue.h
- Tasks — ACE_Thru_Task, Stream_TaskBaseSynch_T and related task classes in src/

## How to render

- For quick preview, open ARCHITECTURE.mmd in the Mermaid Live Editor: https://mermaid.live
- Locally with mermaid-cli:
  1. npm install -g @mermaid-js/mermaid-cli
  2. mmdc -i ARCHITECTURE.mmd -o ARCHITECTURE.svg

Notes about rendering in this environment:

- I committed both ARCHITECTURE.md (with the Mermaid block) and ARCHITECTURE.mmd (pure Mermaid source) to the repository so you can render the diagram yourself (or with CI) into SVG/PNG.
- I cannot reliably render Mermaid to a final SVG/PNG within this execution environment because there is no guaranteed mermaid rendering service available to call from here. If you want, I can:
  - Use an external rendering service (kroki, mermaid.live API) to generate an SVG and commit it, if you approve that I call such a service.
  - Or provide the exact CLI command(s) for you to run locally or in CI to produce ARCHITECTURE.svg from ARCHITECTURE.mmd.

---

Committed: ARCHITECTURE.md and ARCHITECTURE.mmd
