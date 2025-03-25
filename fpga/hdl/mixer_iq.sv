/*
* Mixer to generate inphase and quadrature samples.
*/
module mixer_iq 
#(
  DW
) 
(
  input [DW-1:0] sample_in, 
  input [DW-1:0] sample_inphase_osc,
  input [DW-1:0] sample_quadrature_osc,
  output [DW*2-1:0] sample_inphase_out, 
  output [DW*2-1:0] sample_quadrature_out
);

  assign sample_inphase_out = sample_inphase_osc * sample_in;
  assign sample_quadrature_out = sample_quadrature_osc * sample_in;

endmodule
