# Computer architecture, digital logic, instruction cycle, and assembly (corpus 0.55.0)

- **Date:** 2026-11-01
- **Time:** 09:18 to 09:58 -05:00
- **Stage:** Develop, corpus 0.55.0
- **CLI Tool:** Antigravity CLI 1.4.12 (from `agy --version`)
- **Model:** Gemini 3.8 Flash (High) (`gemini-3.8-flash-high`)
- **Transcript:** Retained privately by the author.
- **Commits:** `76983dc` adds the computer architecture, digital logic, instruction cycle, and assembly pages, ecosystem hub links, corpus version bump and roadmap update; this commit adds the record, provenance rows and refreshed indexes.
- **Authorship:** Fred evaluated the knowledge corpus for information about compiling, assembly language, microprocessor architectures, and how bits travel from storage and memory onto processor units to produce results. Following the assessment identifying gaps in digital logic, computer architecture, the microarchitectural instruction cycle, and assembly language programming, Fred instructed adding topics for these four foundational areas. The AI tool claimed paths with `tam-work`, verified hardware inspection and disassembly commands on the Arch Linux workstation, authored `knowledge/ecosystem/computer-architecture.html`, `knowledge/ecosystem/digital-logic.html`, `knowledge/ecosystem/instruction-cycle.html`, and `knowledge/ecosystem/assembly.html`, updated `knowledge/ecosystem/index.html`, bumped `knowledge/version` to 0.55.0, updated `ROADMAP.md`, regenerated all index sidecars with `python3 tools/build-index.py`, and recorded this provenance entry.
- **Checked against:** `docs/corpus-authoring.md`, `docs/corpus-pedagogy.md`, `knowledge/ecosystem/index.html`, `knowledge/improve/toolchains.html`, `knowledge/hardware/resources.html`, `knowledge/ecosystem/virtualization.html`, `knowledge/ecosystem/os/kernels.html`; live command output from `lscpu`, `lscpu -C`, `cat /sys/devices/system/cpu/cpu0/cache/index*/...`, `objdump -d -M intel build/tam`, `python3 tools/lint-corpus.py`, `python3 tools/build-index.py`, and `build/tam show --pipe`.

## Guiding prompts

> Evaluate the tam knowledge corpus for information about compiling and assembly language. Is there a complete discussion of microprocessor architectures and instruction sets? Would I reader understand how bits from memory and storage chips get loaded onto processor units and turned into results?

> Add topics for computer architecture, digital logic, the microarchitectural instruction cycle, and assembly language programming.

## Key decisions

- The guide is organized into four companion deep-dive pages under the `learn` track with parent `ecosystem/index.html`:
  - `ecosystem/computer-architecture.html` (topic: `computer-architecture`): covers Von Neumann and modified Harvard models, the CPU/cache/memory/storage hierarchy (registers, SRAM, DRAM, NAND flash), system buses (PCIe, DDR channels), Direct Memory Access (DMA), the Memory Management Unit (MMU) and TLB, and CISC vs RISC ISA trade-offs.
  - `ecosystem/digital-logic.html` (topic: `digital-logic`): covers binary voltages, CMOS transistor operation, fundamental logic gates (AND, OR, NOT, NAND, XOR) with truth tables, combinational logic (half and full adders, multiplexers, ALUs) vs sequential logic (D flip-flops, registers, clock synchronization), two's complement arithmetic, dynamic switching power ($P \propto C \cdot V^2 \cdot f$), and energy conservation through zero-polling.
  - `ecosystem/instruction-cycle.html` (topic: `instruction-cycle`): covers the fundamental Fetch-Decode-Execute loop, the 5-stage classic pipeline (IF, ID, EX, MEM, WB), pipeline hazards (structural, data RAW, control branch mispredictions), operand forwarding, the end-to-end journey of bits from NVMe storage over PCIe DMA into DRAM and through L3/L2/L1 caches into registers, and modern Out-of-Order superscalar microarchitecture (uop cache, register renaming, Reservation Stations, Reorder Buffer).
  - `ecosystem/assembly.html` (topic: `assembly`): covers machine code vs assembly mnemonics, x86-64 and ARM64 general-purpose registers and sub-register partitions, operand addressing modes, direct Linux system calls (`syscall`) without a C runtime, AT&T vs Intel syntax, the System V AMD64 ABI calling convention, stack frame layouts, the 128-byte Red Zone, and disassembly via `objdump -d -M intel`.
- Each page implements the complete three-tier pedagogical ladder (Beginner, Intermediate, Expert) with captured read-only or scratch command examples, faded completion tasks, decision tables, troubleshooting tables, Tamlinux design rationale, self-quizzes with folded answers, and cross-links.
- Corpus version moves to 0.55.0.

## Verification

- `python3 tools/lint-corpus.py`: passes with 0 findings.
- `python3 tools/build-index.py`: builds 146 entries with 6 index and metadata sidecars.
- `make corpus-and-test`: all 32 corpus-linter unit tests, 21 index unit tests, and 12 guide verification tests pass cleanly.
- `TAM_KNOWLEDGE_DIR=knowledge build/tam show --pipe` renders all four new pages cleanly without control sequences.
- `TAM_KNOWLEDGE_DIR=knowledge build/tam explain <topic>` resolves each topic directly to its topic guide.
