1.complie uboot:
	make sure that you choose CONFIG_SFA18_RGMII_GMAC
	and don't run GDU reset.

2.complie linux:
	make sure that CONFIG_SFAX8_RGMII_GMAC=m so that you can
       	enable/disable GMAC by insmod/rmmod sgmac, and you have to
	define CONFIG_SFAX8_RGMII_GMAC 1 in sgmac.c because when
	CONFIG_SFAX8_RGMII_GMAC=m, macro CONFIG_SFAX8_RGMII_GMAC
	will not be complied.

3.Load and unload the driver:
	insmod lib/modules/3.18.29/sgmac.ko
	ifconfig eth0 192.168.1.10
	ifconfig eth0 down
	rmmod sgmac

4.test mode:
	we test 5 mode:
	1.idle mode(GMAC doesn't connect with PC by cable when phy
	is at auto negotiation mode)
	2.100M connect mode(GMAC connects with PC at 100M speed,
	and doesn't tx or rx)
	3.100M work mode(GMAC connects with PC at 100M speed,
	and run iperf with PC, tx speed 95Mbps and rx speed 88Mbps)
	4.1000M connect mode(GMAC connects with PC at 1000M speed,
	and doesn't tx or rx)
	5.1000M work mode(GMAC connects with PC at 1000M speed,
	and runs iperf with PC, tx speed is 430Mbps and rx speed
       	is 425Mbps, the speed of tx/rx is changing beacuse CPU is
       	at full load and the current is also chengling in a small range)
