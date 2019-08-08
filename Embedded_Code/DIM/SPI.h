#include <stdint.h>
#include <mcp2515.h>

void init_MCP2515_SPI();
unsigned char transmit_MCP2515_SPI(unsigned char value);
void MCP2515_reset();
void init_MCP2515_CANVariable(can_t * can);
void write_MCP2515(uint8_t addr, uint8_t data);
void write_many_registers_MCP2515(uint8_t addr, uint8_t len, uint8_t *data);
uint8_t read_MCP2515(uint8_t addr);
void read_many_registers_MCP2515(uint8_t addr, uint8_t length, uint8_t *data);
void write_id_MCP2515(uint8_t addr, BOOL ext, unsigned long id);
void read_id_MCP2515(uint8_t addr, unsigned long* id);
void MCP2515_init(void);
void bit_modify_MCP2515(uint8_t addr, uint8_t mask, uint8_t data);
void MCP2515_can_tx0(can_t *can);
void MCP2515_can_tx1(can_t *can);
void MCP2515_can_tx2(can_t *can);
void MCP2515_can_rx0(can_t *can);
void MCP2515_can_rx1(can_t *can);
void MCP2515_clear_rx0(void);
void MCP2515_clear_rx1(void);
void MCP2515_int_clear(void);
BOOL MCP2515_spi_test (void);

