// TODO(owner): replace this skeleton with your synthesised ldpc_minsum.sv
// from the DE2-115 project. This template shows the intended interface so the
// repo is self-describing; it is NOT the measured 128-LE design.
//
// Check-node min-sum kernel: given packed incoming message magnitudes/signs on
// rs1 (and optionally rs2), compute the sign product and the first/second
// minimum and return the outgoing check-node messages on `rd`.

module ldpc_minsum #(
    parameter int W = 6          // fixed-point datapath width (5-6 bit)
)(
    input  logic                 clk_i,
    input  logic                 rstn_i,
    input  logic                 start_i,   // from CFU: operands valid
    input  logic [31:0]          rs1_i,     // packed incoming messages
    input  logic [31:0]          rs2_i,     // (optional) control / second operand
    output logic [31:0]          rd_o,      // packed outgoing messages
    output logic                 done_o     // result valid
);
    // --- Example: unpack up to (32/W) signed-magnitude messages ---
    localparam int K = 32 / W;
    logic signed [W-1:0] msg [K];
    always_comb for (int i = 0; i < K; i++) msg[i] = rs1_i[i*W +: W];

    // --- first/second minimum of |msg| and overall sign product ---
    logic [W-1:0] mag  [K];
    logic         sgn  [K];
    always_comb for (int i = 0; i < K; i++) begin
        sgn[i] = msg[i][W-1];
        mag[i] = msg[i][W-1] ? (~msg[i] + 1'b1) : msg[i];
    end

    logic [W-1:0] min1, min2;
    logic         signprod;
    always_comb begin
        min1 = '1; min2 = '1; signprod = 1'b0;
        for (int i = 0; i < K; i++) begin
            signprod ^= sgn[i];
            if (mag[i] < min1) begin min2 = min1; min1 = mag[i]; end
            else if (mag[i] < min2) begin min2 = mag[i]; end
        end
    end

    // --- outgoing message for each edge: exclude own magnitude (min1/min2) ---
    logic signed [W-1:0] out [K];
    always_comb for (int i = 0; i < K; i++) begin
        logic [W-1:0] m = (mag[i] == min1) ? min2 : min1;      // exclude self
        logic         s = signprod ^ sgn[i];                   // exclude self
        out[i] = s ? (~m + 1'b1) : m;
    end

    // single-cycle result (adjust pipelining to meet timing on your device)
    always_ff @(posedge clk_i or negedge rstn_i) begin
        if (!rstn_i) begin rd_o <= '0; done_o <= 1'b0; end
        else begin
            for (int i = 0; i < K; i++) rd_o[i*W +: W] <= out[i];
            done_o <= start_i;
        end
    end
endmodule
