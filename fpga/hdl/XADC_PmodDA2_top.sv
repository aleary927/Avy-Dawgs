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


assign da2_data = xadc_data;

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
