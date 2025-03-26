module XADC_PmodDA2_top 
(
  input clk,
  input rst,
  input vn, 
  input vp, 
  output led,
  output [3:0] ja
); 

wire [15:0] di_in; 
wire [6:0] daddr_in; 
wire den_in; 
wire dwe_in; 
wire drdy_out; 
wire [15:0] do_out; 
wire dclk_in; 
wire reset_in; 
wire vp_in; 
wire vn_in; 
wire [4:0] channel_out;
wire eoc_out; 
wire alarm_out; 
wire eos_out;
wire busy_out;

wire da2_clk;
logic [11:0] da2_data;

assign dclk_in = clk;
assign reset_in = rst;
assign vp_in = vp; 
assign vn_in = vn;
assign den_in = eoc_out;
assign daddr_in = {2'h00, channel_out};
assign di_in = 'h0;
assign dwe_in = 1'h0;

assign da2_data = do_out[15:4];

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

xadc_wiz_0 xadc (
  .di_in(di_in),              // input wire [15 : 0] di_in
  .daddr_in(daddr_in),        // input wire [6 : 0] daddr_in
  .den_in(den_in),            // input wire den_in
  .dwe_in(dwe_in),            // input wire dwe_in
  .drdy_out(drdy_out),        // output wire drdy_out
  .do_out(do_out),            // output wire [15 : 0] do_out
  .dclk_in(dclk_in),          // input wire dclk_in
  .reset_in(reset_in),        // input wire reset_in
  .vp_in(vp_in),              // input wire vp_in
  .vn_in(vn_in),              // input wire vn_in
  .channel_out(channel_out),  // output wire [4 : 0] channel_out
  .eoc_out(eoc_out),          // output wire eoc_out
  .alarm_out(alarm_out),      // output wire alarm_out
  .eos_out(eos_out),          // output wire eos_out
  .busy_out(busy_out)        // output wire busy_out
);

endmodule
