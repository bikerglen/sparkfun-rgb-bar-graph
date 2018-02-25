module top
(
	input		wire			clk12_in,
	input		wire			rst_n,
	output   wire        blank,
	output	wire	[3:0]	row,
	output	wire			col,
	output	wire			sclk,
	output	wire			latch,
	output	wire	[7:0] user_leds
);

// registers and wires
wire clk100, clk50;

// generate 100MHz and 50MHz clocks
altpll_12_50 altpll_12_50
(
	.inclk0					(clk12_in),
	.c0						(clk100),
	.c1						(clk50)
);

// nios ii embedded system
// nios ii processor clocked at 100MHz
// bar graph driver clocked at 50MHz
embedded_system u0 (
	.bargraph_blank (blank), 		// bargraph.blank
	.bargraph_row   (row),   		//         .row
	.bargraph_col   (col),   		//         .col
	.bargraph_sclk  (sclk),  		//         .sclk
	.bargraph_latch (latch), 		//         .latch
	.bargraph_leds  (user_leds),	//         .leds
	.bargraph_clk   (clk50),		//         .clk
	.clk_clk        (clk100),    	//      clk.clk
	.reset_reset_n  (rst_n)   		//    reset.reset_n
);

endmodule
