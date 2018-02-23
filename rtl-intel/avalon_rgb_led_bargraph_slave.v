module avalon_rgb_led_bargraph_slave
(
	// clocks and resets
	input		wire				clock,
	input		wire				resetn,
	
	// avalon bus
	input		wire				read,
	input		wire				write,
	input		wire				chipselect,
	input		wire	 [3:0]	address,
	input		wire	 [3:0]	byteenable,
	input		wire	[31:0]	writedata,
	output	reg   [31:0]	readdata,

	output   reg	         mtrx_wr,
	output   reg	 [8:0]	mtrx_wr_addr,
	output   reg	 [7:0]   mtrx_wr_data,
	output   reg            mtrx_buffer_select,
	input		wire           mtrx_buffer_current,
	output   reg	 [8:0]   mtrx_level,

	output	reg   [23:0]	timebase_reset_value,
	input   	wire           timebase_flag,
	output  	reg            timebase_flag_clear,
	
	output	reg	 [7:0]	leds
);

reg [31:0] slv_reg0;
reg [31:0] slv_reg1;
reg [31:0] slv_reg2;
reg [31:0] slv_reg3;
reg  [8:0] mtrx_addr;

always @ (posedge clock or negedge resetn)
begin
	if (!resetn)
	begin
		readdata <= 0;
		mtrx_addr <= 0;
		mtrx_wr <= 0;
		mtrx_wr_addr <= 0;
		mtrx_wr_data <= 0;
		mtrx_buffer_select <= 0;
		mtrx_level <= 9'h100;
		timebase_reset_value <= 1_666_666;
		timebase_flag_clear <= 0;
		slv_reg0 <= 32'hdeadbeef;
		slv_reg1 <= 32'hbeefcafe;
		slv_reg2 <= 32'habad1dea;
		slv_reg3 <= 32'hbad1dea5;
		mtrx_addr <= 0;
		leds <= 8'haa;
	end
	else
 	begin
	   mtrx_wr <= 0;

		timebase_flag_clear <= (chipselect && write && (byteenable == 4'b1111)) && (address == 4'h05) && writedata[0];

		if (chipselect && write && (byteenable == 4'b1111))
		begin
			case (address)
				4'h0: mtrx_addr <= writedata[8:0];
				4'h1: begin
					mtrx_addr <= mtrx_addr + 1;
					mtrx_wr <= 1;
					mtrx_wr_addr <= mtrx_addr;
					mtrx_wr_data <= writedata[7:0];
				end
				4'h2: mtrx_buffer_select <= writedata[0];
				4'h3: mtrx_level <= writedata[8:0];
				4'h4: timebase_reset_value <= writedata[23:0];
				4'h6: leds <= writedata[7:0];
			endcase
		end
		
		if (chipselect && read && (byteenable == 4'b1111))
		begin
			case (address)
	        4'h0   : readdata <= slv_reg0;
	        4'h1   : readdata <= slv_reg1;
	        4'h2   : readdata <= slv_reg2;
	        4'h3   : readdata <= slv_reg3;
	        4'h4   : readdata <= { 8'b0, timebase_reset_value };
	        4'h5   : readdata <= { 31'b0, timebase_flag };
			  default: readdata <= 0;
			endcase
		end
		
	end
end
	
endmodule
