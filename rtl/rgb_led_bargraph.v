
`timescale 1 ns / 1 ps

	module rgb_led_bargraph #
	(
		// Users to add parameters here

		// User parameters ends
		// Do not modify the parameters beyond this line


		// Parameters of Axi Slave Bus Interface S_AXI
		parameter integer C_S_AXI_DATA_WIDTH	= 32,
		parameter integer C_S_AXI_ADDR_WIDTH	= 6
	)
	(
		// Users to add ports here
		output  wire            blank,
		output	wire	[3:0]	row,
		output	wire			col,
		output	wire			sclk,
		output	wire			latch,
		// User ports ends
		// Do not modify the ports beyond this line


		// Ports of Axi Slave Bus Interface S_AXI
		input wire  s_axi_aclk,
		input wire  s_axi_aresetn,
		input wire [C_S_AXI_ADDR_WIDTH-1 : 0] s_axi_awaddr,
		input wire [2 : 0] s_axi_awprot,
		input wire  s_axi_awvalid,
		output wire  s_axi_awready,
		input wire [C_S_AXI_DATA_WIDTH-1 : 0] s_axi_wdata,
		input wire [(C_S_AXI_DATA_WIDTH/8)-1 : 0] s_axi_wstrb,
		input wire  s_axi_wvalid,
		output wire  s_axi_wready,
		output wire [1 : 0] s_axi_bresp,
		output wire  s_axi_bvalid,
		input wire  s_axi_bready,
		input wire [C_S_AXI_ADDR_WIDTH-1 : 0] s_axi_araddr,
		input wire [2 : 0] s_axi_arprot,
		input wire  s_axi_arvalid,
		output wire  s_axi_arready,
		output wire [C_S_AXI_DATA_WIDTH-1 : 0] s_axi_rdata,
		output wire [1 : 0] s_axi_rresp,
		output wire  s_axi_rvalid,
		input wire  s_axi_rready
	);

    wire            mtrx_wr;
    wire    [8:0]   mtrx_wr_addr;
    wire    [7:0]   mtrx_wr_data;
    wire            mtrx_buffer_select;
    wire            mtrx_buffer_current;
    wire    [8:0]   mtrx_level;

	wire   [23:0]	timebase_reset_value;
	reg         	timebase_flag;
	wire        	timebase_flag_clear;

// Instantiation of Axi Bus Interface S_AXI
	rgb_led_bargraph_S_AXI # ( 
		.C_S_AXI_DATA_WIDTH(C_S_AXI_DATA_WIDTH),
		.C_S_AXI_ADDR_WIDTH(C_S_AXI_ADDR_WIDTH)
	) rgb_led_bargraph_S_AXI (
		.S_AXI_ACLK(s_axi_aclk),
		.S_AXI_ARESETN(s_axi_aresetn),
		.S_AXI_AWADDR(s_axi_awaddr),
		.S_AXI_AWPROT(s_axi_awprot),
		.S_AXI_AWVALID(s_axi_awvalid),
		.S_AXI_AWREADY(s_axi_awready),
		.S_AXI_WDATA(s_axi_wdata),
		.S_AXI_WSTRB(s_axi_wstrb),
		.S_AXI_WVALID(s_axi_wvalid),
		.S_AXI_WREADY(s_axi_wready),
		.S_AXI_BRESP(s_axi_bresp),
		.S_AXI_BVALID(s_axi_bvalid),
		.S_AXI_BREADY(s_axi_bready),
		.S_AXI_ARADDR(s_axi_araddr),
		.S_AXI_ARPROT(s_axi_arprot),
		.S_AXI_ARVALID(s_axi_arvalid),
		.S_AXI_ARREADY(s_axi_arready),
		.S_AXI_RDATA(s_axi_rdata),
		.S_AXI_RRESP(s_axi_rresp),
		.S_AXI_RVALID(s_axi_rvalid),
		.S_AXI_RREADY(s_axi_rready),

    	.mtrx_wr				(mtrx_wr),
    	.mtrx_wr_addr			(mtrx_wr_addr),
    	.mtrx_wr_data			(mtrx_wr_data),
    	.mtrx_buffer_select		(mtrx_buffer_select),
    	.mtrx_buffer_current	(mtrx_buffer_current),
    	.mtrx_level				(mtrx_level),

		.timebase_reset_value	(timebase_reset_value),
		.timebase_flag			(timebase_flag),
		.timebase_flag_clear	(timebase_flag_clear)
	);

	// Add user logic here
	rgb_led_bargraph_driver rgb_led_bargraph_driver
	(
    	.rst_n					(s_axi_aresetn),
    	.clk					(s_axi_aclk),

    	.wr_clk					(s_axi_aclk),
    	.wr						(mtrx_wr),
    	.wr_addr				(mtrx_wr_addr),
    	.wr_data				(mtrx_wr_data),

    	.buffer_select			(mtrx_buffer_select),
    	.buffer_current			(mtrx_buffer_current),

    	.level					(mtrx_level),

		.blank                  (blank),
		.row					(row),
		.col					(col),
		.sclk                   (sclk),
		.latch                  (latch)
	);

//----------------------------------------
// CPU main loop tasks timebase timer
//----------------------------------------

reg [23:0] timebase_counter;

always @ (posedge s_axi_aclk or negedge s_axi_aresetn)
begin
    if (!s_axi_aresetn)
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

	// User logic ends

	endmodule
