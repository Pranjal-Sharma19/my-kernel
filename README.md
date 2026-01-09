A minimalistic, monolithic 32-bit kernel written in C and Assembly for the x86 architecture.

This project was built from scratch to explore low-level systems engineering concepts, including:

* **Booting:** Transitioning from Real Mode to Protected Mode.
* **Drivers:** VGA Text Mode (Memory Mapped I/O) and PS/2 Keyboard Polling.
* **Kernel Space:** Manual memory management and string manipulation without the C Standard Library.
* **Shell:** A custom command-line interface.

## Tech Stack

* **Language:** C (Freestanding), NASM Assembly
* **Architecture:** x86 (IA-32)
* **Build Tools:** GCC, LD, Make
* **Emulation:** QEMU

## How to Run

1. **Prerequisites** (Arch Linux):

   ```bash
   sudo pacman -S nasm qemu-system-x86 base-devel
