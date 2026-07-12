#include <stdint.h>
#include <stddef.h>

// Universal Hardware Port Assignments
#define PORT_COM1_SERIAL_DATA       0x3F8
#define PORT_KEYBOARD_DATA          0x60
#define PORT_KEYBOARD_STATUS        0x64

// Keyboard Bitmasks & Target Scancodes
#define KEYBOARD_STATUS_DATA_READY  (1 << 0)
#define SCANCODE_KEY_A              0x1E

// Custom UI/UX Color Definitions (Standard 32-bit Hex formats)
#define COLOR_INDIGO_PURPLE         0x4B0082

// Limine Boot Protocol Configuration Constants
#define LIMINE_BASE_REVISION_ID_1   0xfc7f8de1151ccc16
#define LIMINE_BASE_REVISION_ID_2   0x1d01e12745994ef0
#define LIMINE_FB_REQUEST_ID_1      0xc759926c619a85a4
#define LIMINE_FB_REQUEST_ID_2      0x15ee4da0775d0e77

// Limine Data Layout Overlays
struct limine_framebuffer {
    void *address;
    uint64_t width;
    uint64_t height;
    uint64_t pitch;
    uint16_t bpp;
};

struct limine_framebuffer_response {
    uint64_t revision;
    uint64_t framebuffer_count;
    struct limine_framebuffer **framebuffers;
};

struct limine_framebuffer_request {
    uint64_t id;
    uint64_t revision;
    struct limine_framebuffer_response *response;
};

// Hooking into Limine dynamically using our configured ID definitions
__attribute__((used)) static volatile uint64_t limine_base_revision = {
    LIMINE_BASE_REVISION_ID_1, 
    LIMINE_BASE_REVISION_ID_2
};

__attribute__((used)) static volatile struct limine_framebuffer_request framebuffer_request = {
    .id = {LIMINE_FB_REQUEST_ID_1, LIMINE_FB_REQUEST_ID_2},
    .revision = 0
};

// Raw Hardware Wrappers & Abstractions
void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

uint8_t wait_and_get_keyboard_scancode(void) {
    while ((inb(PORT_KEYBOARD_STATUS) & KEYBOARD_STATUS_DATA_READY) == 0) {
        // Safe operational spinlock waiting loop
    }
    return inb(PORT_KEYBOARD_DATA);
}

void print_string_to_serial(const char* str) {
    while (*str) {
        outb(PORT_COM1_SERIAL_DATA, *str++);
    }
}

void clear_screen_to_color(uint32_t color) {
    if (framebuffer_request.response != NULL && framebuffer_request.response->framebuffer_count > 0) {
        struct limine_framebuffer *fb = framebuffer_request.response->framebuffers;
        uint32_t *video_memory = (uint32_t*)fb->address;
        
        uint64_t total_pixels = (fb->pitch / (fb->bpp / 8)) * fb->height;
        for (uint64_t i = 0; i < total_pixels; i++) {
            video_memory[i] = color;
        }
    }
}

// Operating System Root Logic Gateway
void _start(void) {
    print_string_to_serial("SYSTEM INIT: Dynamic C/ASM Kernel loaded cleanly.\n");

    clear_screen_to_color(COLOR_INDIGO_PURPLE);

    print_string_to_serial("SYSTEM STATUS: Standing by for hardware key interaction...\n");

    while (1) {
        uint8_t code = wait_and_get_keyboard_scancode();
        if (code == SCANCODE_KEY_A) { 
            print_string_to_serial("KEY DETECTED: Target Key captured!\n");
            break; 
        }
    }

    print_string_to_serial("SYSTEM SHUTDOWN: Locking processor core safely down.\n");
    while (1) {
        __asm__ volatile ("cli\n\thelt");
    }
}
