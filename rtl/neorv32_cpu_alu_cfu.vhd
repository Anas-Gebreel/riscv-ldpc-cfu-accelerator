-- ================================================================================ --
-- NEORV32 CPU - ALU Custom (RISC-V Instructions) Functions Unit (CFU)              --
-- -------------------------------------------------------------------------------- --
-- Modified for LDPC Min-Sum Hardware Accelerator                                   --
-- ================================================================================ --

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

library neorv32;
use neorv32.neorv32_package.all;

entity neorv32_cpu_alu_cfu is
  port (
    -- global control --
    clk_i    : in  std_ulogic; -- global clock, rising edge
    rstn_i   : in  std_ulogic; -- global reset, low-active, async
    -- request --
    start_i  : in  std_ulogic; -- start trigger, single-shot
    inst_i   : in  std_ulogic_vector(31 downto 0); -- full instruction word
    rs1_i    : in  std_ulogic_vector(31 downto 0); -- register source operand 1
    rs2_i    : in  std_ulogic_vector(31 downto 0); -- register source operand 2
    -- response --
    result_o : out std_ulogic_vector(31 downto 0); -- operation result
    valid_o  : out std_ulogic                      -- operation done; result valid
  );
end entity;

architecture neorv32_cpu_alu_cfu_rtl of neorv32_cpu_alu_cfu is

  -- -------------------------------------------------------------------------------------------
  -- Custom LDPC Min-Sum Hardware Declaration
  -- -------------------------------------------------------------------------------------------
  component ldpc_minsum is
    port (
      clk   : in  std_ulogic;
      rst_n : in  std_ulogic;
      start : in  std_ulogic;
      rs1   : in  std_ulogic_vector(31 downto 0);
      rd    : out std_ulogic_vector(31 downto 0);
      done  : out std_ulogic
    );
  end component;

begin

  -- -------------------------------------------------------------------------------------------
  -- Instantiate Custom LDPC Min-Sum Hardware
  -- -------------------------------------------------------------------------------------------
  -- The NEORV32 ALU will assert start_i when a custom instruction is executed,
  -- and it will stall the processor pipeline until valid_o (done) goes high.
  ldpc_minsum_inst: ldpc_minsum
  port map (
    clk   => clk_i,
    rst_n => rstn_i,
    start => start_i,   
    rs1   => rs1_i,     
    rd    => result_o,  
    done  => valid_o    
  );

end architecture;