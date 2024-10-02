#include <hardware/timer.h>
#include <lwip/err.h>
#include <lwip/ip4_addr.h>
#include <lwip/ip_addr.h>
#include <lwip/pbuf.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <time.h>
// #include <openssl/aes.h>

#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "lwip/udp.h"
#include "tiny-HMAC-c/src/hmac.h"
#include "config.h"

char ssid[] = CONFIG_WIFI_SSID;
char pass[] = CONFIG_WIFI_PASS;

// Turn the led on or off
void set_status_led(bool led_on) {
#if defined(PICO_DEFAULT_LED_PIN)
    // Just set the GPIO on or off
    gpio_put(PICO_DEFAULT_LED_PIN, led_on);
#elif defined(CYW43_WL_GPIO_LED_PIN)
    // Ask the wifi "driver" to set the GPIO on or off
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, led_on);
#endif
}

// Wake-on-lan target (broadcast on port 9)
#define WOL_ADDR "255.255.255.255"
#define WOL_PORT 9

const uint8_t mac_key[] = {
    SECRET_KEY
};

// Blink the LED to signal internal error to the user
void err_fn() {
    while (true) {
        set_status_led(true);
        sleep_ms(200);
        set_status_led(false);
        sleep_ms(100);
        set_status_led(true);
        sleep_ms(200);
        set_status_led(false);
        sleep_ms(800);
    }
}

// Debug helper function
void hexdump(const void *data, size_t size) {
    unsigned char *p = (unsigned char*)data;
    unsigned char c;
    size_t i;

    for (i = 0; i < size; i++) {
        if (i % 16 == 0) {
            printf("%08x: ", i);
        }
        printf("%02x ", p[i]);

        if ((i+1) % 16 == 0) {
            printf("  ");
            for (size_t j = i - 15; j <= i; j++) {
                c = p[j];
                printf("%c", (c >= 32 && c <= 126) ? c : '.');
            }
            printf("\n");
        }
    }

    // Print the remaining bytes if not a multiple of 16
    if (i % 16 != 0) {
        for (size_t j = i % 16; j < 16; j++) {
            printf("   ");
        }
        printf("  ");
        for (size_t j = i - (i % 16); j < i; j++) {
            c = p[j];
            printf("%c", (c >= 32 && c <= 126) ? c : '.');
        }
        printf("\n");
    }
}
// Error check (assert), don't bother cleaning up
void check_err(bool ok, char *msg, void *dump_target) {
    if (ok) return;
    printf("%s\n", msg);
    if (dump_target != NULL) hexdump(dump_target, 256);
    err_fn();
}

struct udp_pcb *pcb;
struct pbuf *msg_buf;

ip_addr_t target_addr;

#define WOL_LEN (6 + 16*6)

// Sends wake-on-LAN magic packet
void send_WOL() {
    char *packet = (char *)msg_buf->payload;

    size_t i = 0;
    // 6 bytes of FF
    for (; i < 6; i++) {
        packet[i] = 0xFF;
    }
    // 6 byte of MAC address, repeated 6 times
    for (; i < WOL_LEN; i += 6) {
        memcpy(packet + i, WOL_TARGET_MAC, 6);
    }
    err_t err = udp_sendto(pcb, msg_buf, &target_addr, WOL_PORT);
    hexdump(packet, WOL_LEN);
    printf("%p\n", msg_buf);
    if (err != ERR_OK) {
        printf("Failed to send UDP packet! error=%d\n", err);
    } else {
        printf("Sent packet\n");
    }
}

const char req_char[] = "request";

void recv_callback(void *arg, struct udp_pcb *pcb, struct pbuf *p,
        const ip_addr_t *addr, u16_t port) {
    printf("Recieved data from %s:%d\n", ip4addr_ntoa(addr), port);
    hexdump(p->payload, 32);
    uint8_t hmac_result[SHA1HashSize];
    // TODO: change req_char
    hmac_sha1((uint8_t*)mac_key, sizeof(mac_key), (uint8_t*)req_char, sizeof(req_char) - 1, hmac_result);
    printf("key\n");
    hexdump(mac_key, 32);
    printf("HMAC\n");
    hexdump(hmac_result, 32);

    // uint32_t challenge_num = time_us_32();

    // version number, request type, hmac
    char *data = p->payload;
    if (data[0] != 0x01 || data[1] != 0x01) {
        uint32_t challenge_num = time_us_32();
        struct pbuf *response = pbuf_alloc(PBUF_TRANSPORT, 4, PBUF_RAM);
        if (response == NULL) {
            printf("Failed to allocate!\n");
            err_fn();
        }
        memset(response->payload, 0xff, 4);
        hexdump(response->payload, 32);
        err_t err = udp_sendto(pcb, response, addr, port);
        if (err != ERR_OK) {
            printf("Failed to send UDP packet! error=%d\n", err);
        } else {
            printf("Sent challenge\n");
        }
        pbuf_free(response);
    }
    if (memcmp(data + 2, hmac_result, HMAC_SHA1_HASH_SIZE) == 0) {
        struct pbuf *response = pbuf_alloc(PBUF_TRANSPORT, 4, PBUF_RAM);
        if (response == NULL) {
            printf("Failed to allocate!\n");
            err_fn();
        }
        memset(response->payload, 0x01, 4);
        hexdump(response->payload, 32);
        err_t err = udp_sendto(pcb, response, addr, port);
        if (err != ERR_OK) {
            printf("Failed to send UDP packet! error=%d\n", err);
        } else {
            printf("Sent challenge\n");
        }
        send_WOL();
        pbuf_free(response);
    } else {
    }
    pbuf_free(p);
}

int main() {
    stdio_init_all();

    if (cyw43_arch_init_with_country(CYW43_COUNTRY_USA)) {
        printf("failed to initialise\n");
        err_fn();
        return 1;
    }
    printf("initialised\n");

    cyw43_arch_enable_sta_mode();

    if (cyw43_arch_wifi_connect_timeout_ms(ssid, pass, CYW43_AUTH_WPA2_AES_PSK, 10000)) {
        printf("failed to connect\n");
        err_fn();
        return 1;
    }
    printf("connected\n");

    pcb = udp_new();

    err_t err;

    ipaddr_aton(WOL_ADDR, &target_addr);
    err = udp_bind(pcb, IP4_ADDR_ANY, RECV_PORT);
    check_err(err == ERR_OK, "Failed to bind!", NULL);
    udp_recv(pcb, recv_callback, NULL);

    msg_buf = pbuf_alloc(PBUF_TRANSPORT, WOL_LEN, PBUF_RAM);
    check_err(msg_buf != NULL, "Failed to allocate!", NULL);
    if (msg_buf == NULL) {
        printf("Failed to allocate!\n");
        err_fn();
    }

    while (true) {
        set_status_led(false);
        printf("sleeping %d\n", time_us_32());
        sleep_ms(2950);
        // send_WOL();
        set_status_led(true);
        sleep_ms(50);

#if PICO_CYW43_ARCH_POLL
        printf("polling\n");
        // if you are using pico_cyw43_arch_poll, then you must poll periodically from your
        // main loop (not from a timer) to check for Wi-Fi driver or lwIP work that needs to be done.
        cyw43_arch_poll();
        // you can poll as often as you like, however if you have nothing else to do you can
        // choose to sleep until either a specified time, or cyw43_arch_poll() has work to do:
        cyw43_arch_wait_for_work_until(make_timeout_time_ms(1000));
#else
#endif
    }
    pbuf_free(msg_buf);
}

