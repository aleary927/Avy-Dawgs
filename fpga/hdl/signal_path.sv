/*
* Signal path for one antenna.
*/
module signal_path 
#(
  DW 
)
(
  input clk, 
  input rst,
  input sample_clk,
  input [DW-1:0] sample_in,
  input [DW-1:0] sample_inphase_osc, 
  input [DW-1:0] sample_quadrature_osc,
  output [DW-1:0] sample_out
);

  /********************* 
  * Modules 
  * *******************/

  // mixer 
  
  // lowpass filter

endmodule
