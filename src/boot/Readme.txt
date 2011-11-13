2011-10-18 晚上
新建，编写引导扇区，实现引导功能。
使用
	bximage
指令根据提示可创建一个软盘或硬盘映像。
使用
	dd if=boot.bin of=a.img bs=512 count=1 conv=notrunc
可将一个文件写到引导扇区。
看完如何从FAT12文件系统的根目录中找出一个文件名，P111 。
