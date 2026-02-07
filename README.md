# 2026-OS

## Plan architektoniczny
Krótki, spójny plan projektu 2026-OS — ma być wystarczająco konkretny, by prowadzić implementację, ale bez przedwczesnej optymalizacji.

**Zasada pracy:** system rozwijamy zgodnie z poniższym planem. Każda zmiana architektury powinna mieć odzwierciedlenie w tym dokumencie.

### 0. Założenia nadrzędne
- Projekt hobbystyczny, ale pisany jak prawdziwy OS.
- Bez hacków „na chwilę” i bez magicznych frameworków.
- Modularny kod umożliwiający wymianę bootloadera, systemu plików i dodanie GUI bez przepisywania kernela.

### 1. Architektura sprzętowa
- CPU: x86_64 (long mode).
- Pełne oddzielenie user/kernel.
- SMP docelowo, ale start od single-core; architektura od początku SMP-aware (locki, per-CPU data).

### 2. Boot i start systemu
- Bootloader: GRUB (na start), kernel niezależny od GRUB-a.
- Jasny kontrakt przekazywania danych bootloader → kernel (bez logiki GRUB-specific).
- Kolejność inicjalizacji: CPU, pamięć, scheduler, VFS, init process.

### 3. Kernel (hybrydowy)
- Styl pomiędzy Windows NT a Linuxem.
- Core w kernelu, część usług jako system processes.
- Wymagane cechy: preemptive multitasking, prosty scheduler z możliwością rozbudowy, SMP-aware.
- Moduły: Memory Manager, Process/Thread Manager, Scheduler, IPC, Driver Interface, VFS.

### 4. Pamięć
- Paging 4-level.
- Oddzielna przestrzeń kernel/user.
- Kernel heap i user heap.
- Na start: bez NUMA i huge pages.

### 5. System plików
- Na teraz: istniejący prosty FS (ext-like/ISO/itp.).
- VFS obowiązkowy od dnia 1.
- Katalogi i pliki tekstowe, bez uprawnień na start.
- Mountowanie: tak.
- Docelowo własny FS jako osobny projekt.

### 6. Interfejs użytkownika
- Etap 1–2: CLI + TUI (krótkie komendy, logiczne zachowanie).
- Własny shell, skrypty, piping w przyszłości.
- Etap 3+: GUI jako osobny subsystem (framebuffer, okna, mysz), poza kernelem.

### 7. Programy i aplikacje
- Własny format binarny i loader w kernelu.
- Userland oddzielony.
- Języki: kernel (C/ASM, C++ z umiarem, opcjonalnie Rust), userland (C, C++, docelowo C#).

### 8. Programowanie pod 2026-OS
- API: syscalls ze stabilnym kontraktem.
- SDK: headers, docs.
- Build system: cross-compiling.

### 9. Bezpieczeństwo
- Użytkownicy i hasła: tak.
- Izolacja procesów: tak.
- Sandbox: nie na start.
- Security jako osobny etap, nie blokuje rozwoju.

### 10. Co wyróżnia 2026-OS
- Projektowany jak Windows NT, rozwijany jak Linux:
  - czysta architektura
  - modularność
  - brak legacy baggage
  - czytelny kod
  - dokumentacja jako część projektu

### 11. Roadmapa (wysoki poziom)
- Faza 0 – fundamenty
- Faza 1 – boot + kernel minimalny
- Faza 2 – procesy, pamięć, FS
- Faza 3 – CLI + TUI
- Faza 4 – userland + aplikacje
- Faza 5 – SMP + stabilizacja
- Faza 6 – GUI (opcjonalnie)

## Kernel (start)
W katalogu `kernel/` znajduje się minimalny kernel x86_64 uruchamiany przez GRUB (Multiboot2) z przejściem do long mode i prostym outputem do VGA. Build (wymaga cross-compiler `x86_64-elf-*`):

Struktura na start:
- `kernel/arch/x86_64/` — kod startowy i linker script
- `kernel/include/` — nagłówki kernela
- `kernel/main.c` — główne wejście kernela
- `kernel/init.c` — sekwencja inicjalizacji (MM, scheduler, procesy, IPC, VFS)
- `kernel/mm.c` — szkielet Memory Manager
- `kernel/scheduler.c` — szkielet schedulera
- `kernel/process.c` — szkielet procesów/wątków
- `kernel/ipc.c` — szkielet IPC
- `kernel/vfs.c` — prosty RAMFS/VFS (pliki w pamięci)
- `kernel/console.c` — prosta konsola tekstowa
- `kernel/keyboard.c` — podstawowy sterownik PS/2 (polling)
- `kernel/vga.c` — proste wyjście tekstowe VGA

```bash
cd kernel
make
```

Po uruchomieniu kernel oferuje minimalną konsolę z komendami `help`, `clear`, `about`, `ls`, `cat`, `echo`, `touch`, `rm`.

### Uruchamianie w QEMU
Wymaga `grub-mkrescue` oraz `xorriso`.

```bash
cd kernel
make iso
make run
```
