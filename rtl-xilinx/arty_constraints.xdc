set_property -dict { PACKAGE_PIN G13  IOSTANDARD LVCMOS33 } [get_ports { row[1] }]; #Sch=JA1
set_property -dict { PACKAGE_PIN B11  IOSTANDARD LVCMOS33 } [get_ports { row[3] }]; #Sch=JA2
set_property -dict { PACKAGE_PIN A11  IOSTANDARD LVCMOS33 } [get_ports { col }];    #Sch=JA3
set_property -dict { PACKAGE_PIN D12  IOSTANDARD LVCMOS33 } [get_ports { latch }];  #Sch=JA4

set_property -dict { PACKAGE_PIN D13  IOSTANDARD LVCMOS33 } [get_ports { row[0] }]; #Sch=JA7
set_property -dict { PACKAGE_PIN B18  IOSTANDARD LVCMOS33 } [get_ports { row[2] }]; #Sch=JA8
set_property -dict { PACKAGE_PIN A18  IOSTANDARD LVCMOS33 } [get_ports { blank }];  #Sch=JA9
set_property -dict { PACKAGE_PIN K16  IOSTANDARD LVCMOS33 } [get_ports { sclk }];   #Sch=JA10
