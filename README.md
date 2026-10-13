# RISC-V Custom-Instruction Accelerator for Min-Sum LDPC Decoding

A hardware/software co-design that accelerates **min-sum LDPC decoding** on an FPGA by extending the
open-source **NEORV32 RISC-V** soft core with a **custom instruction (CFU)** for the check-node
min-sum operation. Targeted at the **Terasic DE2-115** board (Intel **Cyclone IV E**, `EP4CE115F29C7`).

> Built during an industrial-training internship at the VeCAD Research Laboratory, Faculty of
> Electrical Engineering, Universiti Teknologi Malaysia (UTM).

## Highlights

- **~5.4× faster** check-node kernel (~140 → ~26 cycles/check) vs. the pure-software decoder.
- **128 logic elements (~0.1%)** CFU overhead; full CPU+CFU system fits in **4,082 LE (~4%)**.
- Meets timing at **59.04 MHz**, with **no loss of decoding accuracy**.
- Fixed-point study: a **5–6 bit** datapath matches a 32-bit floating-point reference.

## How it works

LDPC codes are decoded iteratively on a Tanner graph by passing log-likelihood ratios (LLRs)
between variable nodes and check nodes. The **check-node min-sum** step — the sign product and
the first/second minimum of the incoming magnitudes — dominates the runtime in software. This
project moves that step into a small combinational datapath exposed as a RISC-V **custom-0**
instruction via the NEORV32 CFU, while the message routing, variable-node updates and syndrome
check remain in C.

```
          +-------------------+        custom-0 instr        +------------------+
   C code |  NEORV32 RISC-V   |  ------------------------->  |  CFU: check-node |
  decoder |  (host processor) |  <-------------------------  |  min-sum datapath|
          +-------------------+        result (rd)           +------------------+
```

## Repository layout

| Path | Contents |
|---|---|
| `rtl/` | CFU check-node min-sum kernel + NEORV32 CFU wrapper |
| `constraints/` | SDC timing constraint for `clk_i` |
| `firmware/cfu_minsum/` | C firmware that calls the custom instruction and times it |
| `sim/reference/` | Pure-C min-sum decoder + fixed-point quantisation study (runs on a PC) |
| `sim/plots/` | Python scripts that produce the BER and convergence figures |
| `docs/` | Industrial-training report, paper, and figures |

## Prerequisites

- **Intel Quartus Prime Lite** (25.1 or similar) for synthesis.
- **NEORV32** RISC-V processor sources — https://github.com/stnolting/neorv32
- **RISC-V GCC** toolchain (`riscv32-unknown-elf-gcc`) for the firmware.
- A **Terasic DE2-115** board (Cyclone IV E `EP4CE115F29C7`).
- **GCC** and **Python 3** (with `numpy`, `matplotlib`) for the PC-side simulation and plots.

## Quick start

### 1. Software reference model + quantisation study (PC)
```bash
cd sim/reference
make
./ldpc_minsum            # prints BER and average iterations for float / 6-bit / 5-bit
cd ../plots
pip install -r requirements.txt
python plot_ber.py       # writes ber_high.png, ber_low.png, convergence.png
```

### 2. Hardware (DE2-115)
1. Add `rtl/ldpc_minsum.sv` and `rtl/neorv32_cpu_alu_cfu.vhd` to your NEORV32 Quartus project.
2. Enable the CFU in the processor top level (`RISCV_ISA_Xcfu => true`).
3. Add `constraints/neorv32_test_setup_approm.sdc` to the project (constrains `clk_i` to 50 MHz).
4. Compile in Quartus and program the `.sof` onto the board over the USB-Blaster.

### 3. Firmware
```bash
cd firmware/cfu_minsum
make clean_all install   # builds and generates neorv32_imem_image.vhd (boot from IMEM)
```
Re-compile the Quartus project so the updated IMEM image is included, then reprogram the board.
Because the setup has no serial adapter, correctness and the speedup are shown on the board LEDs:
all LEDs on = correct decode, then the blink pattern reports the measured cycle counts.

## Results

See `docs/figures/` for the BER waterfall curves, the convergence plot, the Quartus Flow Summary
(4,082 LE), and photos of the decoder running on the DE2-115.

| | Cycles / check | Speedup |
|---|---|---|
| Software min-sum (RISC-V) | ~140 | 1.0× |
| CFU custom instruction | ~26 | ~5.4× |

## Roadmap

- Extend the accelerator to larger / standardised LDPC codes (5G NR, Wi-Fi).
- Parallelise multiple check-node units for higher throughput.
- Explore hardware acceleration of **quantum LDPC (qLDPC)** decoding.

## Acknowledgements

Supervised by Dr. Jeevan at the VeCAD Research Laboratory, UTM. Built on the open-source
[NEORV32](https://github.com/stnolting/neorv32) RISC-V processor.

## License

MIT — see [LICENSE](LICENSE).
