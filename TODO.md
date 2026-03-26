# TODO List
1. Include fw_jump.bin as "bios" in `dist/bin`. Have OpenSBISystem look for it automatically. Load it at 0x80000000, as per usual. Allow bios override with command like "--bios <filename>".

2. Update CLI to handle files differently. Specifically, for default systems,
`rv64i filename.bin --memory ...`
should simply run the file with default system, memory starting at 0, etc. For opensbi, however, `rv64i filename.bin --system opensbi --memory ...` filename.bin will be the kernel and it will automatically load the binaries from `dist/bin`. Unless overridden.