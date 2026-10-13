# Build flow notes

## Hardware (Quartus Prime Lite)
1. Start from the NEORV32 `neorv32_test_setup` project for the DE2-115.
2. Add `rtl/ldpc_minsum.sv` and `rtl/neorv32_cpu_alu_cfu.vhd` to the project.
3. Enable the CFU: set `RISCV_ISA_Xcfu => true` in the processor top level.
4. Add `constraints/neorv32_test_setup_approm.sdc`.
5. Compile; check the Flow Summary (expect ~4,082 LE on EP4CE115F29C7) and the
   Timing Analyzer (expect Fmax ~59 MHz on clk_i).
6. Program the .sof over the USB-Blaster.

## Firmware
cd firmware/cfu_minsum && make clean_all install
Re-compile the Quartus project so the updated neorv32_imem_image.vhd is used,
then reprogram the board. Results are shown on the LEDs.

## Simulation (PC)
cd sim/reference && make && ./ldpc_minsum
cd sim/plots && pip install -r requirements.txt && python plot_ber.py
