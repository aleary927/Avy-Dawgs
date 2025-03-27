/*
* Top-level module for testing XADC with DA2.
*/
module XADC_PmodDA2_top 
(
  input clk,
  input rst,
  input vn, 
  input vp, 
  output led,
  output [3:0] ja
); 

wire da2_clk;
wire [11:0] da2_data;
wire [11:0] xadc_data;
wire xadc_rdy;

wire [11:0] inphase_sample;

wire clk_nsample;
wire clk_nsample_rising;

mixer_test #(
  .DW(12)
) 
mixer (
  .clk(clk), 
  .rst(rst), 
  .lo_sample(inphase_sample), 
  .data_in(xadc_data), 
  .dval_in(xadc_rdy), 
  .data_out(da2_data), 
  .drdy_out()
);

clk_div #(
  .DIV_FACTOR(2)
)
clkdiv_nsample
(
  .clk_ref(clk), 
  .rst(rst), 
  .clk_out(clk_nsample)
);

edge_detect clk_200k_edge
(
  .clk(clk), 
  .rst(rst), 
  .clk_test(clk_nsample), 
  .edge_rising(clk_nsample_rising), 
  .edge_falling()
); 

sin_osc_iq #(
  .DW(12), 
  .ABITS(10), 
  .SCALE(2**11)
  )
  sin_osc (
    .clk(clk), 
    .rst(rst), 
    // .next_sample(clk_nsample_rising), 
    .next_sample(1'h1),
    .inphase_sample(inphase_sample), 
    .quadrature_sample()
  );


DA2RefComp refComp1 (
  .CLK(da2_clk), 
  .RST(rst), 
  .D1(ja[1]), 
  .D2(ja[2]), 
  .CLK_OUT(ja[3]), 
  .nSYNC(ja[0]), 
  .DATA1(da2_data), 
  .DATA2(da2_data), 
  .START(clk_div3), 
  .DONE(led)

);

clk_div #(
    .DIV_FACTOR(2)
)clk_div2er(
    .clk_ref(clk), 
    .rst(rst), 
    .clk_out(da2_clk)
);


clk_div #(
    .DIV_FACTOR(20)
)clk_div3er(
    .clk_ref(clk), 
    .rst(rst), 
    .clk_out(clk_div3)
);

xadc_wrapper xadc 
(
  .clk(clk), 
  .rst(rst), 
  .vp(vp), 
  .vn(vn), 
  .data(xadc_data), 
  .drdy(xadc_rdy)
);

endmodule
