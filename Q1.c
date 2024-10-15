#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "uart_tx.pio.h"  // PIO assembler file for UART

#define RS485_TX_PIN 0   // Définir la broche GPIO utilisée pour la transmission

// Fonction pour initialiser le PIO pour l'UART
void init_uart_pio(PIO pio, uint sm, uint offset, uint baud_rate, uint tx_pin) {
    // Configurer la machine à états PIO pour UART TX
    pio_gpio_init(pio, tx_pin);                         // Initialiser la GPIO pour le TX
    pio_sm_set_consecutive_pindirs(pio, sm, tx_pin, 1, true); // Définir la broche comme sortie

    // Configurer le programme PIO pour UART TX
    pio_sm_config c = uart_tx_program_get_default_config(offset);
    sm_config_set_sideset_pins(&c, tx_pin);  // Configurer la broche de sortie (TX)
    
    // Diviseur d'horloge pour générer le baud rate
    float clkdiv = (float)clock_get_hz(clk_sys) / (8 * baud_rate); 
    sm_config_set_clkdiv(&c, clkdiv);

    // Initialiser la machine à états avec cette configuration
    pio_sm_init(pio, sm, offset, &c);

    // Activer la machine à états
    pio_sm_set_enabled(pio, sm, true);
}

// Fonction pour envoyer un octet via UART (PIO)
void uart_pio_send_byte(PIO pio, uint sm, uint8_t data) {
    // Chaque envoi de byte est bloquant, le PIO gère la conversion UART
    pio_sm_put_blocking(pio, sm, data);
}

int main() {
    // Initialisation de la bibliothèque standard Pico
    stdio_init_all();

    // Initialisation du PIO
    PIO pio = pio0;                 // Utilisation du PIO 0
    uint sm = 0;                    // Machine à états 0
    uint offset = pio_add_program(pio, &uart_tx_program);  // Charger le programme PIO

    // Initialiser la machine PIO pour UART avec un débit de 115200 baud
    init_uart_pio(pio, sm, offset, 115200, RS485_TX_PIN);

    // Envoyer un message "Hello, World!" en boucle
    const char *message = "Hello, RS485 World!\n";
    while (true) {
        const char *ptr = message;
        while (*ptr) {
            uart_pio_send_byte(pio, sm, *ptr++);  // Envoyer chaque caractère
        }
        sleep_ms(1000);  // Attendre 1 seconde entre chaque envoi
    }

    return 0;
}
