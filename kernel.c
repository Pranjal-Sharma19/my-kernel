/* kernel.c */
#include <stddef.h>
#include <stdint.h>

/* Read a byte from a hardware port */
uint8_t inb(uint16_t port) {
  uint8_t ret;
  asm volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
  return ret;
}

/* Write a byte to a hardware port */
void outb(uint16_t port, uint8_t val) {
  asm volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}

/* Hardware text mode color constants */
enum vga_color {
  VGA_COLOR_BLACK = 0,
  VGA_COLOR_GREEN = 2,
  VGA_COLOR_CYAN = 3,
  VGA_COLOR_WHITE = 15,
};

/* Video memory address for x86 (always this address) */
static uint16_t *const VGA_MEMORY = (uint16_t *)0xB8000;
static const size_t VGA_WIDTH = 80;
static const size_t VGA_HEIGHT = 25;
/* Helper to combine a character and a color into 16 bits */
static inline uint16_t vga_entry(unsigned char uc, uint8_t color) {
  return (uint16_t)uc | (uint16_t)color << 8;
}

/* Calculate buffer index based on x, y coordinates */
size_t terminal_row;
size_t terminal_column;
uint8_t terminal_color;
uint16_t *terminal_buffer;

/* Wait for a key press and return the scan code */
uint8_t keyboard_read_scancode() {
  // Loop forever until the "Output Buffer Status" bit (bit 0) is set
  // Port 0x64 tells us if data is waiting
  while ((inb(0x64) & 1) == 0) {
    // Do nothing, just wait...
  }

  // Data is ready! Read it from Port 0x60
  return inb(0x60);
}

void terminal_initialize(void) {
  terminal_row = 0;
  terminal_column = 0;
  terminal_color = VGA_COLOR_GREEN | VGA_COLOR_BLACK
                                         << 4; // Green text on Black
  terminal_buffer = VGA_MEMORY;

  /* Clear the screen with spaces */
  for (size_t y = 0; y < VGA_HEIGHT; y++) {
    for (size_t x = 0; x < VGA_WIDTH; x++) {
      const size_t index = y * VGA_WIDTH + x;
      terminal_buffer[index] = vga_entry(' ', terminal_color);
    }
  }
}
void terminal_putentryat(char c, uint8_t color, size_t x, size_t y) {
  const size_t index = y * VGA_WIDTH + x;
  terminal_buffer[index] = vga_entry(c, color);
}

void terminal_scroll(void) {
  // 1. Move every line up by one
  for (size_t y = 0; y < VGA_HEIGHT - 1; y++) {
    for (size_t x = 0; x < VGA_WIDTH; x++) {
      const size_t src_index = (y + 1) * VGA_WIDTH + x;
      const size_t dst_index = y * VGA_WIDTH + x;
      terminal_buffer[dst_index] = terminal_buffer[src_index];
    }
  }

  // 2. Clear the last line (fill with spaces)
  const size_t last_y = VGA_HEIGHT - 1;
  for (size_t x = 0; x < VGA_WIDTH; x++) {
    const size_t index = last_y * VGA_WIDTH + x;
    terminal_buffer[index] = vga_entry(' ', terminal_color);
  }
}
void terminal_putchar(char c) {
  // Handle Newline
  if (c == '\n') {
    terminal_column = 0;
    terminal_row++;
  }
  // Handle normal characters
  else {
    terminal_putentryat(c, terminal_color, terminal_column, terminal_row);
    terminal_column++;

    // Wrap to next line if we hit the edge of the screen
    if (terminal_column == VGA_WIDTH) {
      terminal_column = 0;
      terminal_row++;
    }
  }

  // Handle Scrolling if we go off the bottom
  if (terminal_row == VGA_HEIGHT) {
    terminal_scroll();
    terminal_row = VGA_HEIGHT - 1; // Stay on the last line
  }
}
void terminal_writestring(const char *data) {
  for (size_t i = 0; data[i] != '\0'; i++) {
    terminal_putchar(data[i]);
  }
}
char get_ascii_char(uint8_t scancode) {
  // Simple lookup table for the first row of keys (QWERTY...)
  // This is incomplete, but enough to prove it works!
  switch (scancode) {
  case 0x1E:
    return 'a';
  case 0x30:
    return 'b';
  case 0x2E:
    return 'c';
  case 0x20:
    return 'd';
  case 0x12:
    return 'e';
  case 0x21:
    return 'f';
  case 0x22:
    return 'g';
  case 0x23:
    return 'h';
  case 0x17:
    return 'i';
  case 0x24:
    return 'j';
  case 0x25:
    return 'k';
  case 0x26:
    return 'l';
  case 0x32:
    return 'm';
  case 0x31:
    return 'n';
  case 0x18:
    return 'o';
  case 0x19:
    return 'p';
  case 0x10:
    return 'q';
  case 0x13:
    return 'r';
  case 0x1F:
    return 's';
  case 0x14:
    return 't';
  case 0x16:
    return 'u';
  case 0x2F:
    return 'v';
  case 0x11:
    return 'w';
  case 0x2D:
    return 'x';
  case 0x15:
    return 'y';
  case 0x2C:
    return 'z';
  case 0x39:
    return ' '; // Spacebar
  case 0x1C:
    return '\n'; // Enter
  default:
    return 0; // Unknown key
  }
}
/* Global Command Buffer */
char input_buffer[256];
size_t buffer_index = 0;

/* Helper: String Compare (returns 0 if strings match) */
int strcmp(const char *s1, const char *s2) {
  while (*s1 && (*s1 == *s2)) {
    s1++;
    s2++;
  }
  return *(const unsigned char *)s1 - *(const unsigned char *)s2;
}

/* Helper: Clear the buffer for the next command */
void clear_buffer() {
  for (size_t i = 0; i < 256; i++) {
    input_buffer[i] = 0;
  }
  buffer_index = 0;
}
void reboot() {
  terminal_writestring("Rebooting...\n");

  // The "Magic" Command
  // 0x64 is the keyboard controller command port
  // 0xFE causes a CPU reset
  uint8_t good = 0x02;
  while (good & 0x02)
    good = inb(0x64);
  outb(0x64, 0xFE);

  // If that fails, halt the CPU so we don't crash weirdly
  asm volatile("hlt");
}

/* Helper: Execute Command */
void execute_command() {
  terminal_writestring("\n"); // Move to new line after user presses Enter

  if (strcmp(input_buffer, "clear") == 0) {
    terminal_initialize(); // Clears the screen
  } else if (strcmp(input_buffer, "help") == 0) {
    terminal_writestring("Available commands:\n");
    terminal_writestring(" - help: Show this menu\n");
    terminal_writestring(" - clear: Clear the screen\n");
    terminal_writestring(" - hello: Say hello\n");
    terminal_writestring(" - reboot: reboots the computer\n");
  } else if (strcmp(input_buffer, "hello") == 0) {
    terminal_writestring("Hello, This is a basic kernel.\n");
  } else if (strcmp(input_buffer, "reboot") == 0) {
    reboot();
  } else if (buffer_index > 0) {
    terminal_writestring("Unknown command: ");
    terminal_writestring(input_buffer);
    terminal_writestring("\n");
  }

  // Reset for next command
  clear_buffer();
  terminal_writestring("> ");
}

void kernel_main(void) {
  terminal_initialize();
  terminal_writestring("Welcome to MyOS Shell.\nType 'help' for commands.\n> ");

  clear_buffer(); // Ensure buffer is empty

  while (1) {
    uint8_t scancode = keyboard_read_scancode();

    // If key is pressed (not released)
    if (scancode < 0x80) {
      char c = get_ascii_char(scancode);

      // Handle ENTER key
      if (c == '\n') {
        execute_command();
      }
      // Handle BACKSPACE (Scan code 0x0E is typically Backspace)
      else if (scancode == 0x0E) {
        if (buffer_index > 0) {
          buffer_index--;
          input_buffer[buffer_index] = 0;
          // Visual hack: Move cursor back, print space, move back again
          terminal_column--;
          terminal_putentryat(' ', terminal_color, terminal_column,
                              terminal_row);
        }
      }
      // Handle Normal Letters
      else if (c != 0) {
        // Print to screen
        terminal_putchar(c);

        // Save to memory (Buffer)
        if (buffer_index < 255) {
          input_buffer[buffer_index] = c;
          buffer_index++;
        }
      }
    }
  }
}
