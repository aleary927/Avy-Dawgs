/*
* Clock divider. 
* 
* ** Only use even division factor of at least 2. **
*/
module clk_div 
#(
  DIV_FACTOR
) 
(
  input clk_ref,
  input rst,
  output reg clk_out
);

  localparam MAX_COUNT = DIV_FACTOR/2 - 1;

   reg [$clog2(MAX_COUNT) - 1:0] count;

  always_ff @(posedge clk_ref) begin 
    if (rst) begin 
      count <= 'h0;
    end 
    else if (count == MAX_COUNT) begin 
      count <= 'h0;
    end
    else begin
      count <= count + 1'h1;
    end
  end

  always_ff @(posedge clk_ref) begin 
    if (rst) begin 
      clk_out <= 1'h0;
    end
    else if (count == MAX_COUNT) begin 
      clk_out <= ~clk_out;
    end
  end

endmodule
