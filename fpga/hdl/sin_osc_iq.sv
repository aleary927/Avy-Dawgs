/*
* Sine oscillaotor with in-phase and quadrature output. 
* A one-clock cycle next_sample pulse is needed to move to next 
* sample. This value is incremented on negedge so that next sample 
* will be available on second rising edge.
*/
module sin_osc_iq
#(
  DW,
  ABITS, 
  SCALE
)
(
  input clk, 
  input rst, 
  input next_sample, 
  output [DW-1:0] inphase_sample, 
  output [DW-1:0] quadrature_sample
);

  localparam QSIN_ABITS = ABITS - 2;

  // indexs
  reg [ABITS-1:0] i_idx; 
  reg [ABITS-1:0] q_idx;

  logic [1:0] i_quadrant;
  logic [1:0] q_quadrant;
  logic [QSIN_ABITS-1:0] i_idx_qsin;
  logic [QSIN_ABITS-1:0] q_idx_qsin;

  wire [DW-1:0] i_qsin_sample;
  wire [DW-1:0] q_qsin_sample;

  // negitive edge so that next sample will be availabe on second rising edge
  always_ff @(negedge clk) begin 
    if (rst) begin 
      i_idx <= 2**(ABITS-2);
      q_idx <= 'h0;
    end
    else if (next_sample) begin 
      i_idx <= i_idx + 1'h1;
      q_idx <= q_idx + 1'h1;
    end
  end

  assign i_quadrant = i_idx[ABITS-1:ABITS-2];
  assign q_quadrant = q_idx[ABITS-1:ABITS-2];
  assign i_idx_qsin = i_idx[QSIN_ABITS-1:0];
  assign q_idx_qsin = q_idx[QSIN_ABITS-1:0];

  /**************************** 
  * MODULES 
  * **************************/ 
  
  // SIN LUT 
  qsin_lut_2p 
  #(
    .DW(DW), 
    .ABITS(QSIN_ABITS), 
    .SCALE(SCALE)
  ) 
  qsin_lut 
  (
    .clk(clk), 
    .rst(rst), 
    .addr1(i_idx_qsin), 
    .addr2(q_idx_qsin), 
    .data1(i_qsin_sample), 
    .data2(q_qsin_sample)
  );

  // convert in-phase qsin to sin
  qsin_to_sin 
  #(
    .DW(DW)
  )
  qsin_to_sin_inphase 
  (
    .quadrant(i_quadrant), 
    .qsin_sample(i_qsin_sample), 
    .sin_sample(inphase_sample)
  );

  // convert quadrature qsin to sin
  qsin_to_sin 
  #(
    .DW(DW)
  )
  qsin_to_sin_quadrature
  (
    .quadrant(q_quadrant), 
    .qsin_sample(q_qsin_sample), 
    .sin_sample(quadrature_sample)
  );

endmodule
