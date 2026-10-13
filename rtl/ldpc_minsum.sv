// ============================================================================
// ldpc_minsum - LDPC min-sum check-node kernel for the NEORV32 CFU
// rs1 : six 5-bit signed messages packed as { [4]=sign, [3:0]=magnitude }
//       message k -> rs1[k*5+4 : k*5],  k = 0..5   (bits 29..0)
// rd  : { [11]=overall sign, [10:8]=index of min1, [7:4]=min2, [3:0]=min1 }
// done pulses one cycle after start; rd is captured on start.
// ============================================================================
module ldpc_minsum (
  input  logic        clk,
  input  logic        rst_n,
  input  logic        start,
  input  logic [31:0] rs1,
  output logic [31:0] rd,
  output logic        done
);

  logic [3:0]  mag [0:5];
  logic        sgn [0:5];
  logic [3:0]  m1, m2;
  logic [2:0]  i1;
  logic        tot;
  logic [31:0] rd_comb;

  always_comb begin
    // unpack six 5-bit messages
    for (int k = 0; k < 6; k++) begin
      mag[k] = rs1[k*5 +: 4];
      sgn[k] = rs1[k*5 + 4];
    end
    // overall sign = XOR of all six signs
    tot = 1'b0;
    for (int k = 0; k < 6; k++) tot ^= sgn[k];
    // smallest (m1), second-smallest (m2) magnitude, index of smallest (i1)
    m1 = 4'hF; m2 = 4'hF; i1 = 3'd0;
    for (int k = 0; k < 6; k++) begin
      if (mag[k] < m1) begin
        m2 = m1; m1 = mag[k]; i1 = k[2:0];
      end else if (mag[k] < m2) begin
        m2 = mag[k];
      end
    end
    // pack result
    rd_comb       = 32'd0;
    rd_comb[3:0]  = m1;
    rd_comb[7:4]  = m2;
    rd_comb[10:8] = i1;
    rd_comb[11]   = tot;
  end

  always_ff @(posedge clk or negedge rst_n) begin
    if (!rst_n) begin
      rd   <= 32'd0;
      done <= 1'b0;
    end else begin
      done <= start;            // 1-cycle latency: done pulses after start
      if (start) rd <= rd_comb;  // capture result when triggered
    end
  end

endmodule