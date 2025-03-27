module edge_detect
(
  input clk, 
  input rst, 
  input clk_test, 
  output edge_rising, 
  output edge_falling
);

  reg q1, q2;

  always_ff @(posedge clk) begin 
    if (rst) begin 
      q1 <= 1'h0;
      q2 <= 1'h0;
    end
    else begin 
      q1 <= clk_test;
      q2 <= q1;
    end
  end

  assign edge_rising = ~q2 & q1;
  assign edge_falling = q2 & ~q1;

endmodule
