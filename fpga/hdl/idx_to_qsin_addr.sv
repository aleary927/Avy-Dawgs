module idx_to_qsin_addr
#(
  QSIN_ABITS
) 
(
  input [1:0] quadrant, 
  input [QSIN_ABITS-1:0] idx,
  output logic [QSIN_ABITS-1:0] addr
  );

  localparam MAX_IDX = 2**QSIN_ABITS - 1;

  always_comb begin   
    case (quadrant)
      // increasing 
      2'h0, 2'h2: addr = idx;
      // decreasing 
      2'h1, 2'h3: addr = MAX_IDX - idx;
      default: addr = idx;
    endcase
  end
endmodule
