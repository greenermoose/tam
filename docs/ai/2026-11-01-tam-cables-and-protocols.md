# Computer cables, physical transmission media, and communication protocols (corpus 0.58.0)

- **Date:** 2026-11-01
- **Time:** 14:15 to 16:15 -05:00
- **Stage:** Develop (corpus 0.58.0)
- **CLI Tool:** agy 1.4.12
- **Model:** Gemini 3.8 Flash (High) (`gemini-3.8-flash`)
- **Transcript:** Retained privately by the author.
- **Commits:** Pending
- **Authorship:** Fred Horch directed the architecture and requested a comprehensive curriculum enabling someone with zero prior computer knowledge to walk away an expert on computer cables, physical signaling, and communication protocols. Fred expanded the curriculum to cover power distribution cables, voltage loops, semiconductors and transistor gates, optical fiber and electro-optical conversion, and wireless radio waves (antennas, Wi-Fi, and cellular). Antigravity authored the six core ecosystem chapters, nineteen supporting glossary definitions, the guided goal path with self-quizzes and capstone troubleshooting scenario, hub links, roadmap entries, and index rebuilds.
- **Checked against:** Linux kernel TTY subsystem (`/proc/tty/driver/serial`, `/sys/class/tty/`), `termios(3)`, `stty(1)`, `dmesg`, `lspci`, `ethtool -m`, IEEE 802.3 Ethernet standards, EIA/TIA-232-F, EIA/TIA-485-A, Richard Hamming's *Error Detecting and Error Correcting Codes*, Claude Shannon's *A Mathematical Theory of Communication*, Horowitz & Hill's *The Art of Electronics* (3rd ed.).

## Guiding prompts

> /plan Create a comprehensive set of pages for the tam knowledge corpus that will enable a person who knows nothing about computers before reading the knowledge corpus be able to walk away an expert on how computer cables and communication protocols work.

> /plan Make sure to include power cables, too. Ground everything in the basics of electronics: voltage pushes current from high voltages to low. Ensure the reader understands that every electrical circuit is a loop, with something forcing the electrons to a higher voltage so they continue to travel around. Also describe the difference between electrical and optical connections and cables. Explain how fiber optic cables work and how to get a signal from an electrical circuit to a fiber optic cable. Also describe how to go from electrical circuits to magnetic waves and how that relates to WiFi and cellular phone networks. Update the plan to cover this additional material and show me the updated version in the review window.

> /plan Do we cover semiconductors and gates in the knowledge corpus? If not, is this a good time to cover those topics?

> /plan Looks good. You may close the review window. Proceed.

## Key decisions

- **Six-Chapter Curricular Architecture:** Partitioned the extensive domain into six modular ecosystem chapters under `knowledge/ecosystem/`:
  1. `circuits-and-power.html`: Voltage, current, resistance, closed loops, AC/DC power supplies, semiconductors, P-N junctions, diodes, and transistor switches.
  2. `serial-and-parallel.html`: Serial vs parallel architectures, simplex, half-duplex, full-duplex, the parallel speed wall (clock skew and crosstalk), and SerDes serial interconnects.
  3. `cables-and-signaling.html`: Electrical copper cables, differential signaling and common-mode rejection, optical fiber (Total Internal Reflection, transceivers), and wireless electromagnetic waves (antennas, Wi-Fi, cellular).
  4. `communication-protocols.html`: Baud rate vs bit rate, packet framing, asynchronous UART 8N1, hardware vs software flow control, and line coding.
  5. `error-detection-and-correction.html`: Noise and bit flips, even/odd parity bits, checksums, Cyclic Redundancy Checks (CRC-16, CRC-32), ARQ retransmission protocols (Stop-and-Wait, Go-Back-N, Selective Repeat), and Forward Error Correction (Hamming codes, SECDED).
  6. `hardware-diagnostics.html`: Visual inspection, Linux hardware enumeration (`dmesg`, `lspci`, `lsusb`, `/dev/tty*`), multimeter testing (continuity, shorts, DC rails, diode drops), serial loopback plugs, oscilloscopes, logic analyzers, and component-level board repair (MAX232/SP3485 transceivers, TVS ESD diodes, trace repair).
- **Guided Goal Path:** Added `knowledge/learn/paths/cables-and-protocols.html` with a 5-stage sequential curriculum, self-quizzes with folded answers, and an end-to-end solar energy field link troubleshooting capstone challenge.
- **Glossary Support:** Authored nineteen dedicated glossary pages in `knowledge/learn/glossary/` covering `voltage`, `current`, `circuit`, `semiconductor`, `pn-junction`, `diode`, `transistor`, `baud-rate`, `bit-rate`, `serial-communication`, `parallel-communication`, `half-duplex`, `full-duplex`, `differential-signaling`, `fiber-optic`, `electromagnetic-wave`, `flow-control`, `parity`, and `crc`.
- **Pedagogical Standards:** Followed `docs/corpus-pedagogy.md` rigorously: beginner sections include goals, worked examples, try it, if it goes wrong, new words, next steps, and first-time asides; intermediate sections include mental models, completion tasks, independent tasks, choosing tables, troubleshooting matrices, and On Tamlinux asides; expert sections include reference tables, how it works deep-dives, edge cases, explore exercises, check yourself open prompts, and academic citations.

## Verification

- `python3 tools/lint-corpus.py`: Verified clean (exit 0) with zero findings across the entire corpus.
- `python3 tools/build-index.py`: Rebuilt index and sidecar TSVs (188 entries) without errors or warnings.
- `python3 tools/report-coverage.py --warnings`: Verified zero anatomy warnings across all enforced tracks (`use`, `maintain`, `improve`, `learn`, `install`).
- `python3 -m unittest -q tests/test_lint_corpus.py tests/test_build_index.py`: 53 unit tests passed with `OK`.
