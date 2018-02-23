//=============================================================================================
// SparkFun RGB LED Bargraph Driver
// Copyright 2014-2017 by Glen Akins.
// All rights reserved.
// 
// Set editor width to 96 and tab stop to 4.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//=============================================================================================

module rgb_led_bargraph_driver
(
	input	wire			rst_n,
	input	wire			clk,

	input	wire			wr_clk,
	input	wire			wr,
	input	wire	[8:0]	wr_addr,			// 2 buffers x 16 rows x 16 columns
	input	wire	[7:0]	wr_data,			// 8 bits per pixel

	input	wire			buffer_select,		// display buffer to use
	output	wire			buffer_current,		// currently active display buffer
	
	input	wire	[8:0]	level,				// global dimming 0 to 256

	output	reg				blank,				// display blanking
	output	reg		[3:0]	row,				// row address output
	output	reg				col,				// column data output
	output	reg				sclk,				// column data clock
	output	reg				latch				// column data latch
);


//---------------------------------------------------------------------------------------------
// state machine states
//

localparam WAIT = 0,
		   BLANK = 1,
		   LATCH = 2,
		   UNBLANK = 3,
		   READ = 4,
		   SHIFT1 = 5,
		   SHIFT2 = 6;
		   
localparam BASE_DELAY = 54;


//---------------------------------------------------------------------------------------------
// registers and wires
//

reg   [2:0] state;
reg  [15:0] timer;
reg  [15:0] blanktimer;
reg   [3:0] delay;
reg         rd_buffer;		// buffer 0 to 1
reg   [3:0] rd_row;			// row 0 to 15
reg   [3:0] rd_col;			// col 0 to 15
reg   [2:0] rd_bit;			// bit 0 to 7
wire  [8:0] rd_addr;		// { buffer, row, col }
wire  [7:0] rd_data;		// pixel level
wire        rd_data_bit;	// pixel bit


//---------------------------------------------------------------------------------------------
// display memory
//

// turn current buffer, row, column, and bit number into a memory address
assign buffer_current = rd_buffer;
assign rd_addr = { rd_buffer, rd_row[3:0], rd_col[3:0] };

// memory
rgb_led_bargraph_dpram512x8 rgb_led_bargraph_dpram512x8
(
	.clka					(wr_clk),
	.wea					(wr),
	.addra					(wr_addr[8:0]),
	.dina					(wr_data),
	.clkb					(clk),
	.addrb					(rd_addr),
	.doutb					(rd_data)
);

// turn read data into individual pixel bits
assign rd_data_bit = rd_data[rd_bit];


//---------------------------------------------------------------------------------------------
// clocked logic
//

always @ (posedge clk)
begin
	if (!rst_n)
	begin
		blank <= 1;
		row <= 0;
		col <= 0;
		sclk <= 0;
		latch <= 0;

		state <= READ;

		timer <= 0;
		blanktimer <= 0;
		delay <= 0;

		rd_buffer <= 0;
		rd_row <= 0;
		rd_col <= 0;
		rd_bit <= 0;
	end
	else
	begin
		// implemnt timer for binary coded modulation
		// bit plane 0 is displayed for ~192 clock cycles
		// each succesfive bit plane is displayed for 2x the clocks of the previous bit plane
		if (timer == 0)
		begin
			// for 10MHz clock, use 192-1,  384-1,  768-1, 1536-1 below
			// for 25MHz clock, use 480-1,  960-1, 1920-1, 3840-1 below
			// for 50MHz clock, use 960-1, 1920-1, 3840-1, 7680-1 below
			case (rd_bit)
				0: timer <=   1*BASE_DELAY-1;
				1: timer <=   2*BASE_DELAY-1;
				2: timer <=   4*BASE_DELAY-1;
				3: timer <=   8*BASE_DELAY-1;
				4: timer <=  16*BASE_DELAY-1;
				5: timer <=  32*BASE_DELAY-1;
				6: timer <=  64*BASE_DELAY-1;
				7: timer <= 128*BASE_DELAY-1;
			endcase
			case (rd_bit)
				0: blanktimer <= ((  1*BASE_DELAY-1) * level) >> 8;
				1: blanktimer <= ((  2*BASE_DELAY-1) * level) >> 8;
				2: blanktimer <= ((  4*BASE_DELAY-1) * level) >> 8;
				3: blanktimer <= ((  8*BASE_DELAY-1) * level) >> 8;
				4: blanktimer <= (( 16*BASE_DELAY-1) * level) >> 8;
				5: blanktimer <= (( 32*BASE_DELAY-1) * level) >> 8;
				6: blanktimer <= (( 64*BASE_DELAY-1) * level) >> 8;
				7: blanktimer <= ((128*BASE_DELAY-1) * level) >> 8;
			endcase
		end
		else
		begin
			timer <= timer - 1;
			blanktimer <= blanktimer - 1;
		end

		// move blanking control outside state machine to implement global dimming
		if ((blanktimer == 0) || (timer == 0))
		begin
			blank <= 1;
		end
		else if ((state == LATCH) && (delay == 0))
		begin
			blank <= 0;
		end
		
		// state machine
		case (state)

			// wait for timer to expire then blank the display
			WAIT: begin
				sclk <= 0;
				if (timer == 0)
				begin
					// blank <= 1;
					delay <= 8;
					state <= BLANK;
				end
			end

			// wait a while then latch in data previosly shifted into display
			BLANK: begin
				if (delay == 0)
				begin
					latch <= 1;
					delay <= 8;
					state <= LATCH;
					row <= rd_row;
				end
				else
				begin
					delay <= delay - 1;
				end
			end

			// wait a while then unblank the display to display the latched data
			LATCH: begin
				if (delay == 0)
				begin
					// blank <= 0;
					latch <= 0;
					state <= UNBLANK;
				end
				else
				begin
					delay <= delay - 1;
				end
			end

			// find the next bit, row, column, and buffer to display
			// this is converted to a read address using combinatorial logic above
			UNBLANK: begin
				if (rd_bit == 7)
				begin
					rd_bit <= 0;
					if (rd_row == 15)
					begin
						rd_row <= 0;
						rd_buffer <= buffer_select;
					end
					else
					begin
						rd_row <= rd_row + 1;
					end
				end
				else
				begin
					rd_bit <= rd_bit + 1;
				end
				rd_col <= 0;
				state <= READ;
			end
			
			// the read, shift1, and shift2 states could be reduced to two states
			// if I knew which edge of sclk latched the data into the shift registers
			// this is good enough for one panel but for more than about four panels
			// it'd be worth reducing to two clocks instead of three clocks.

			// wait for read data to be output from RAM
			READ: begin
				state <= SHIFT1;
				sclk <= 0;
			end

			// drive the column data out the outputs
			SHIFT1: begin
				sclk <= 0;
				col <= rd_data_bit;
				state <= SHIFT2;
				rd_col <= rd_col + 1;
			end

			// clock the data into the RAM, move to next column, repeat 16x
			SHIFT2: begin
				sclk <= 1;
				if (rd_col == 0)
				begin
					state <= WAIT;
				end
				else
				begin
					state <= SHIFT1;
				end
			end

		endcase
	end
end

endmodule
