/*
* Top Level module.
*/
module top (
  input sysclk, 
  input extrst
); 

  localparam DW = 16;
  localparam SINLUT_ABITS = 10;

  logic rst; 
  wire clk;

  wire [DW-1:0] inphase_osc_sample;
  wire [DW-1:0] quadrature_osc_sample;


  wire [DW-1:0] xant_sample_in;
  wire [DW-1:0] xant_sample_out;
  wire [DW-1:0] yant_sample_in;
  wire [DW-1:0] yant_sample_out;

  /******************* 
  * Modules 
  * *****************/

  // pll

  // oscillator 
  sin_osc_iq 
  #(
    .DW(DW), 
    .ABITS(SINLUT_ABITS),
    .SCALE()
  )
    osc 
  (
    .clk(clk), 
    .rst(rst), 
    .next_sample(), 
    .inphase_sample(inphase_osc_sample), 
    .quadrature_sample(quadrature_osc_sample)
  );

  // x antenna signal path
  signal_path 
  #(
    .DW(DW)
  )
    xant_signal_path
  ( 
    .clk(clk), 
    .rst(rst), 
    .sample_clk(), 
    .sample_in(xant_sample_in), 
    .sample_inphase_osc(inphase_osc_sample), 
    .sample_quadrature_osc(quadrature_osc_sample), 
    .sample_out(xant_sample_out)
  );

  // y antenna signal path 
  signal_path 
  #(
    .DW(DW)
  )
    yant_signal_path
  ( 
    .clk(clk), 
    .rst(rst), 
    .sample_clk(), 
    .sample_in(yant_sample_in), 
    .sample_inphase_osc(inphase_osc_sample), 
    .sample_quadrature_osc(quadrature_osc_sample), 
    .sample_out(yant_sample_out)
  );

  // interface to ADC 

  // interface to MCU

endmodule
