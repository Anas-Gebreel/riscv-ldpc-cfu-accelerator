# Constrain the user clock so Timing Analyzer reports Fmax on clk_i,
# not on the JTAG clock. 50 MHz system clock => 20 ns period.
create_clock -name clk_i -period 20.000 [get_ports {clk_i}]
derive_clock_uncertainty

# JTAG (leave as asynchronous / cut from the main analysis)
# create_clock -name tck -period 100.000 [get_ports {altera_reserved_tck}]
# set_clock_groups -asynchronous -group {clk_i} -group {tck}
