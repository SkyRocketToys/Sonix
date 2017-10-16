target=snx_scan

$(target): snx_scan.c get_frame_isp.c
	arm-linux-gcc -o snx_scan snx_scan.c get_frame_isp.c  -I../../middleware/include/ -L../../middleware/lib/ -lzbar 
clean:
	-rm -rf $(target)
distclean:clean
install:
