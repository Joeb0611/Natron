# Crash dumps → tickets (Natron+)

CMake does not enable Breakpad (`NATRON_USE_BREAKPAD`). On Linux, Natron+ still installs SIGSEGV/SIGABRT handlers that print a backtrace and write a dump so a crash can become a ticket.

## Where dumps land

1. **App cache crashes directory:** `<cache>/crashes/crash-YYYYMMDDThhmmss.txt`
   (on this Linux host: `~/.cache/Natron+/Natron+/crashes/`)
2. **Beside the binary:** `<bindir>/last-crash.txt` (overwritten each crash)

The advertised binary is the CMake build (`App/Natron`, `Renderer/NatronRenderer`). Keep unstripped binaries (RelWithDebInfo, or `ENABLE_EXPORTS` on those targets) next to the dump so `addr2line` / `llvm-symbolizer` can turn frames into functions.

## Turning a dump into a ticket

1. Attach `crash-*.txt` (or `last-crash.txt`).
2. Record the tree SHA (`git rev-parse HEAD`) and the binary path.
3. If frames are only addresses, from the same build:
   `addr2line -e build/App/Natron -f <address>`
4. File the ticket against Natron+ with the stack, SHA, and a one-line repro. Do not file it as an official Natron Project crash.

Breakpad GUI upload remains qmake-only until a CMake Breakpad client is revived.
