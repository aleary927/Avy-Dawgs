/*
* Quarter sine lookup table with two ports.
*/
module qsin_lut_2p 
#(
  DW, 
  ABITS, 
  SCALE
)
(
  input clk, 
  input rst, 
  input [ABITS-1:0] addr1, 
  input [ABITS-1:0] addr2, 
  output reg [DW-1:0] data1, 
  output reg [DW-1:0] data2
); 
  localparam SIZE = 2**ABITS;
  
  reg [DW-1:0] rom [SIZE-1:0];

  // generate LUT
  integer i;
  initial begin 
    for (i = 0; i < SIZE; i++) begin 
      rom[i] = $signed(SCALE * $sin(2*3.1415926*i/(SIZE*4)));
    end
  end

  // port 1
  always_ff @(posedge clk) begin 
    if (rst) begin 
      data1 <= 'h0;
    end
    else begin 
      data1 <= rom[addr1];
    end
  end

  // port 2
  always_ff @(posedge clk) begin 
    if (rst) begin 
      data2 <= 'h0;
    end 
    else begin 
      data2 <= rom[addr2];
    end
  end

endmodule
