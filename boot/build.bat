set PATH=C:\Users\naalg\AppData\Roaming\xPacks\@xpack-dev-tools\riscv-none-elf-gcc\15.2.0-1.1\.content\bin;%PATH%

riscv-none-elf-as boot.s -o boot.o

riscv-none-elf-ld boot.o -Ttext 0x80000000 -o boot.elf

riscv-none-elf-objcopy -O binary boot.elf boot.bin

& "C:\msys64\usr\bin\dtc.exe" -I dts -O dtb -o system.dtb system.dts

del boot.elf boot.o