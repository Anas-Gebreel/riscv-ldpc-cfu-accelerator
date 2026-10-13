/* TODO(owner): replace with your real NEORV32 firmware (cfu_minsum/main.c).
 * This template shows how the custom instruction is invoked and timed, and how
 * results are shown on the board LEDs (no serial adapter needed).
 */
#include <neorv32.h>

/* custom-0 R-type instruction: funct7=0000000, funct3=000. Adjust to match
 * the decode in your CFU wrapper. */
#define CN_MINSUM(rs1, rs2) \
    neorv32_cpu_cfu_r_type(0 /*funct7*/, 0 /*funct3*/, (rs1), (rs2))
/* If that helper is unavailable in your NEORV32 version, use the intrinsic:
 * RISCV_INSTR_R_TYPE(RISCV_OPCODE_CUSTOM0, 0b000, 0b0000000, rs1, rs2)      */

static inline uint32_t cycles(void) { return (uint32_t)neorv32_cpu_csr_read(CSR_MCYCLE); }

int main(void) {
    neorv32_rte_setup();
    neorv32_gpio_port_set(0);

    /* ---- example packed inputs (replace with real decoder messages) ---- */
    uint32_t packed_msgs = 0x05030A01u;

    /* ---- software check-node (reference) ---- */
    uint32_t t0 = cycles();
    volatile uint32_t sw = 0; /* ... software min-sum over packed_msgs ... */
    uint32_t t_sw = cycles() - t0;

    /* ---- hardware custom instruction ---- */
    t0 = cycles();
    volatile uint32_t hw = CN_MINSUM(packed_msgs, 0);
    uint32_t t_hw = cycles() - t0;

    /* show correctness + speedup on the LEDs */
    if (hw == sw) neorv32_gpio_port_set(0xFF);          /* all on = correct */
    uint32_t speedup = (t_hw ? t_sw / t_hw : 0);
    neorv32_gpio_port_set(speedup & 0xFF);              /* e.g. ~5 */

    while (1) { }
    return 0;
}
