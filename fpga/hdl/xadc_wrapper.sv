
module xadc_wrapper 
(
  input clk,
  input rst,
  input vp, 
  input vn, 
  output [11:0] data,
  output drdy
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

assign dclk_in = clk;
assign reset_in = rst;
assign vp_in = vp; 
assign vn_in = vn;
assign den_in = eoc_out;        // trigger data read on end of conversion
assign daddr_in = {2'h00, channel_out};     // select channel for read
assign di_in = 'h0;     // not reading any data
assign dwe_in = 1'h0;   // not writing

assign data = do_out[15:4];  
assign drdy = drdy_out;   // pass along data ready


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
