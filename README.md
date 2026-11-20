# CMD Restriction Bypass via Programming Language Runtimes

> **Disclaimer:** This project is intended purely for **security awareness and educational research**. All techniques documented here are meant to help IT teams, blue teamers, and security engineers understand the limitations of common endpoint restrictions — so they can defend against them properly. Do not use any of this on systems you do not own or have explicit written permission to test.

---

## Table of Contents

- [Project Overview](#project-overview)
- [Level 1 — Direct Runtime Execution](#level-1--direct-runtime-execution)
  - [Why This Bypasses the Restriction](#why-this-bypasses-the-restriction)
  - [How It Works — Technical Summary](#how-it-works--technical-summary)
  - [Code Examples](#code-examples)
  - [Risk Level](#risk-level)
  - [Relevant Terminology](#relevant-terminology)
  - [Mitigation Recommendations](#mitigation-recommendations)

---

## Project Overview

A common but critically underestimated security gap exists in many enterprise environments:

> IT teams block `cmd.exe` — but forget that **any installed programming language runtime is an equivalent path to the same shell.**

This project documents, level by level, the techniques through which OS-level command execution can be achieved even when the shell is "locked down." Each level increases in sophistication — from direct standard-library calls, to indirect execution, to Living-off-the-Land and beyond.

The goal is not exploitation — it is **awareness**: if your defense only blocks `cmd.exe`, your defense is incomplete.

---

## Level 1 — Direct Runtime Execution

### Overview

When an IT policy restricts `cmd.exe` (via Group Policy, AppLocker, renaming the binary, or removing shortcuts), it typically targets only the `cmd.exe` executable itself.

It does **not** stop a general-purpose programming language — already installed and trusted on the same machine — from invoking the OS's process creation API and reaching the shell directly.

This is Level 1 because:
- No obfuscation is required.
- No privilege escalation is needed.
- No evasion of AV/EDR is involved.
- It relies entirely on **standard library functions** that every developer knows.

---

### Why This Bypasses the Restriction

A typical `cmd.exe` lockdown does one or more of the following:

| Restriction Method | What It Blocks |
|---|---|
| GPO: "Prevent access to the command prompt" | Interactive `cmd.exe` sessions launched by the user |
| AppLocker path rule on `cmd.exe` | Direct execution of `C:\Windows\System32\cmd.exe` |
| Renaming or removing `cmd.exe` | The binary itself |
| Removing Start Menu / Run shortcut | User-initiated launch shortcuts |

**What none of these block:**

A child process spawned **programmatically** by another executable that is itself allowed to run — such as `python.exe`, a compiled C/C++ binary, or a Rust-built `.exe`.

If the language runtime is trusted and unrestricted, it becomes a **transparent proxy to the shell**, regardless of what GPO says about `cmd.exe`.

---

### How It Works — Technical Summary

All three examples below ultimately do the same thing: they ask the **Windows kernel** (not the Group Policy layer) to create a new process. Group Policy restrictions live in a layer above the kernel and are enforced at the shell/UI level — not at the `CreateProcess` API level.

| Language | Function Used | Underlying Windows API |
|---|---|---|
| C / C++ | `system()` | `CreateProcessW` via CRT, spawns `cmd.exe /c` |
| Python | `os.system()` | Same CRT wrapper, same result |
| Rust | `std::process::Command` | Direct `CreateProcessW` call, optionally through `cmd.exe` |

The common thread: **the restriction is on the entry point, not the API.**

---

### Code Examples

#### C / C++

```c
#include <stdlib.h>

int main() {
    system("whoami");
    return 0;
}
```

> `system()` in the C runtime (CRT) on Windows internally calls `cmd.exe /c <command>`.
> Even if `cmd.exe` is blocked interactively, the process creation succeeds because the caller (`yourprogram.exe`) is trusted.

---

#### Python

```python
import os

os.system("whoami")
```

> `os.system()` is a thin wrapper around the same CRT `system()` call.
> `python.exe` is typically whitelisted in enterprise environments — making this a one-liner bypass.

---

#### Rust

```rust
use std::process::Command;

fn main() {
    let _ = Command::new("cmd")
        .args(["/C", "whoami"])
        .status();
}
```

> Rust's `std::process::Command` calls `CreateProcessW` directly.
> Unlike C's `system()`, this does not necessarily require `cmd.exe` as an intermediary — it can spawn any binary directly, making it slightly more flexible and harder to detect via simple parent-process rules.

---

### Risk Level

| Attribute | Assessment |
|---|---|
| **Severity** | Low–Medium |
| **Skill Required** | Beginner |
| **Privileges Needed** | Standard user (no admin required) |
| **Detectability** | High — easily caught by EDR via parent-child process trees |
| **Obfuscation Used** | None |
| **Real-World Relevance** | High — most environments with cmd.exe locked are still vulnerable to this |

This level is classified **Low–Medium** not because the technique is unimportant, but because:
1. It requires the ability to run arbitrary code already (Python installed, or ability to run a compiled binary).
2. It produces loud, easily-detected telemetry in any environment with EDR.
3. It is well-documented and covered by most standard detection rules.

Its **real-world relevance**, however, is **High** — because many organizations stop at blocking `cmd.exe` and consider it sufficient.

---

### Relevant Terminology

| Term | Definition |
|---|---|
| **Living off the Land (LOTL)** | Using legitimate, pre-installed system tools to perform malicious or restricted actions — without bringing in external malware |
| **Process Spawning** | One process (parent) programmatically creating another (child) via OS-level APIs |
| **Child Process** | A new process created by an existing process; inherits certain permissions and environment variables from its parent |
| **Application Whitelisting** | A security model that only allows explicitly approved executables to run (e.g., AppLocker, WDAC) |
| **Shelling out** | Informal term for a program invoking an OS shell to execute a command string |
| **EDR (Endpoint Detection and Response)** | Security tooling that monitors endpoint behavior — including process creation chains — in real time |
| **GPO (Group Policy Object)** | Windows mechanism for enforcing configuration and restrictions across machines in a domain |
| **CreateProcessW** | The core Windows API function used to launch any new process — the layer at which these techniques operate |
| **CRT (C Runtime)** | The C standard library implementation on Windows; `system()` is part of it and wraps `CreateProcessW` |
| **Parent-Child Process Relationship** | The traceable link between a process that spawns another; a primary signal used in EDR detections |

---

### Mitigation Recommendations

Blocking `cmd.exe` alone is a **single-point, easily-bypassed control.** A proper defense requires layered restrictions:

1. **Implement full Application Control (not just cmd.exe blocking)**
   - Use **Windows Defender Application Control (WDAC)** or **AppLocker** to whitelist *only* explicitly approved executables.
   - This should cover interpreters (`python.exe`, `node.exe`, `powershell.exe`) and compilers, not just `cmd.exe`.

2. **Remove unnecessary language runtimes from end-user machines**
   - If users don't need Python, it shouldn't be installed. Each installed interpreter is an additional attack surface.

3. **Monitor parent-child process relationships via EDR**
   - Alert on anomalous chains such as:
     - `python.exe → cmd.exe`
     - `excel.exe → cmd.exe`
     - `winword.exe → powershell.exe`
   - These are well-known Indicators of Attack (IoA) and are covered by most modern EDR platforms.

4. **Apply Script Execution Policies**
   - PowerShell: enforce `Constrained Language Mode` and signed-script requirements.
   - Python: consider application-level sandboxing or removing write permissions to `site-packages`.

5. **Principle of Least Privilege**
   - Standard users should not be able to install runtimes, compilers, or interpreters.
   - Developer machines should be treated as a separate risk category with enhanced monitoring.

6. **Log and Alert on `CreateProcess` calls from unexpected parents**
   - Native Windows Event ID `4688` (Process Creation) with command-line logging enabled is a free starting point.
   - Sysmon (Event ID 1) provides richer telemetry including hashes and full command-line arguments.

---

*More levels coming soon — covering indirect execution, DLL-based approaches, LOLBins, and beyond.*
