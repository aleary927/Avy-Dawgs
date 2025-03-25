/*
* Converts a quadrant value and quarter sin LUT sample 
* to a reguler sin value.
*/
module qsin_to_sin 
#(
  DW
)
(
  input [1:0] quadrant,
  input [DW-1:0] qsin_sample, 
  output logic [DW-1:0] sin_sample
); 

  always_comb begin 
    case (quadrant) 
      // positive 
      2'h0, 2'h1: sin_sample = qsin_sample;
      // negative 
      2'h2, 2'h3: sin_sample = ~qsin_sample + 1'h1;
      default: sin_sample = rom_data;
    endcase
  end

endmodule
