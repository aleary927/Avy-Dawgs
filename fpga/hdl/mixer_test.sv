module mixer_test #(
  DW
) (
  input clk, 
  input rst, 
  input [DW-1:0] lo_sample, 
  input [DW-1:0] data_in,
  input dval_in, 

  output [DW-1:0] data_out,
  output reg drdy_out
); 

/*
* 1. remove dc offset 
* 2. mix with local oscillator 
* 3. apply dc offset
*/

logic [DW-1:0] din_nodc;  // dc removed 

reg [DW*2-1:0] mixed;     // has been mixed

always_ff @(posedge clk) begin 
  if (rst) begin 
    mixed <= 'h0;
  end
  else if (dval_in) begin 
    mixed <= $signed(din_nodc) * $signed(lo_sample);
  end
end

always_ff @(posedge clk) begin 
  if (rst) begin 
    drdy_out <= 1'h0;
  end 
  else if (dval_in) begin 
    drdy_out <= 1'h1; 
  end 
  else begin 
    drdy_out <= 1'h0; 
  end
end

assign din_nodc = $signed(data_in) - 2**(DW-1);
assign data_out = mixed[DW*2-1:DW] + 2**11;

endmodule

