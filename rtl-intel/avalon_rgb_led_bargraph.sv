module avalon_rgb_led_bargraph
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
	output	wire	[31:0]	readdata,
	
	// interface to bargraph display
	input		wire				dispclk,
	output   wire        	blank,
	output	wire	 [3:0]	row,
	output	wire				col,
	output	wire				sclk,
	output	wire				latch,
	
	// interface to user leds
	output	wire	 [7:0]	leds
);


// matrix registers and wires
wire				mtrx_wr;
wire   [8:0]	mtrx_wr_addr;
wire   [7:0]  	mtrx_wr_data;
wire           mtrx_buffer_select;
wire           mtrx_buffer_current;
wire   [8:0]   mtrx_level;

// timebase registers and wires
wire	[23:0]	timebase_reset_value;
reg         	timebase_flag;
wire        	timebase_flag_clear;
reg   [23:0]	timebase_counter;


// avalon register interface
avalon_rgb_led_bargraph_slave avalon_rgb_led_bargraph_slave
(
	.clock						(clock),
	.resetn						(resetn),
	
	.read							(read),
	.write						(write),
	.chipselect					(chipselect),
	.address						(address),
	.byteenable					(byteenable),
	.writedata					(writedata),
	.readdata					(readdata),

	.mtrx_wr						(mtrx_wr),
	.mtrx_wr_addr				(mtrx_wr_addr),
	.mtrx_wr_data				(mtrx_wr_data),
	.mtrx_buffer_select		(mtrx_buffer_select),
	.mtrx_buffer_current		(mtrx_buffer_current),
	.mtrx_level					(mtrx_level),

	.timebase_reset_value	(timebase_reset_value),
	.timebase_flag				(timebase_flag),
	.timebase_flag_clear		(timebase_flag_clear),
	
	.leds							(leds)
);


// rgb led bargraph driver
avalon_rgb_led_bargraph_driver avalon_rgb_led_bargraph_driver
(
	.clk							(dispclk),
	.rst_n						(resetn),

	.wr_clk						(clock),
	.wr							(mtrx_wr),
	.wr_addr						(mtrx_wr_addr),
	.wr_data						(mtrx_wr_data),

	.buffer_select				(mtrx_buffer_select),
	.buffer_current			(mtrx_buffer_current),

	.level						(mtrx_level),

	.blank                  (blank),
	.row							(row),
	.col							(col),
	.sclk                   (sclk),
	.latch                  (latch)
);


// CPU main loop tasks timebase timer

always @ (posedge clock or negedge resetn)
begin
    if (!resetn)
    begin
        timebase_counter <= 0;
        timebase_flag <= 0;
    end
    else
    begin
        if (timebase_counter == timebase_reset_value)
        begin
            timebase_counter <= 0;
            timebase_flag <= 1;
        end
        else
        begin
            timebase_counter <= timebase_counter + 1;
            if (timebase_flag_clear)
            begin
                timebase_flag <= 0;
            end
        end
    end
end

endmodule
