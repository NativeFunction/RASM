# RASM

RASM is an assembler and disassembler targeted towards the RAGE scripting format.

Supported Targets
---------------------------------
* Grand Theft Auto 4
    * SCO format
* Red Dead Redemption
    * SCO format
    * XSC format
    * CSC format
* Grand Theft Auto 5
    * XSC format
    * CSC format
    * YSC format
* Red Dead Redemption 2
    * XSC format
    * CSC format
    * YSC format

```console
USAGE: rasm.exe <Required Platform Options> <Operation>

Required Platform Options:

        -target = {GTAIV | RDR | RDR_SCO | GTAV | RDR2}
        -platform = {XBOX | PSX | PC}
Operations:

        -dec = Decompile {Script Input Path} {Source Output Path} {Optional Decompile Flags}
        -com = Compile {Source Input Path} {Script Output Path} {Optional Compile Flags}
Decompile Flags:

        -v = Show Verbose Information
        -ow = Automatically Overwrite Source Output Path
        -dcs = Only Decompile Strings
        -dcn = Decompile Nops
        -nv = Specify Version for Natives {%d}
        -ov = Specify Version for Opcodes {%d}
Compile Flags:

        -ssn = Set Script Name {Script Name}
        -spb = Set Page Base {uint32 Pagebase}
        -in = Ignore Nops
        -norsc = Don't Write The RSC Header
        -ow = Automatically Overwrite Script Output Path
```
