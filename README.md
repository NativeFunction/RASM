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
        -decd = Decompile Directory {Script Folder Input Path} {Script Folder Output Path} {Optional Decompile Flags}
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

Opcodes
---------------------------------

Opcode | Description | Example
--- | --- | ---
SetStaticsCount | Loads a value from the code buffer, then sets it as the max statics available for the script (Custom Opcode) | ![setstaticscount](https://user-images.githubusercontent.com/27574733/184804292-084e1f45-8124-4ec2-a9ae-784d7642e2b5.PNG)
SetDefaultStatic | Loads the static index and a value from the code buffer, then defines the value at the index (Custom Opcode) | ![setdefaultstatic](https://user-images.githubusercontent.com/27574733/184803997-38cb19f9-5e95-46e4-8da0-c95555fd42ad.PNG)
SetStaticName | Loads the static index and a string from the code buffer, then defines the string as the index (Custom Opcode) | <img src="https://user-images.githubusercontent.com/27574733/180933620-17c5f55b-8925-4946-807a-437f531e527e.PNG" width="1500">
SetLocalName | Loads the local index and a string from the code buffer, then defines the string as the index (Custom Opcode) | ![setlocalname](https://user-images.githubusercontent.com/27574733/184804849-a83031bd-ef45-493e-82e5-790bb709fa52.PNG)
SetGlobalName | Loads the global index and a string from the code buffer, then defines the string as the index (Custom Opcode) | ![setglobalname](https://user-images.githubusercontent.com/27574733/184805069-f12e52d9-bfe2-4dec-b899-978965c1feed.PNG)
SetEnum | Loads the string and a value from the code buffer, then defines the string as the value (Custom Opcode) | ![setenum](https://user-images.githubusercontent.com/27574733/184805366-6735a5d1-b666-4a31-87a5-58421fb6de60.PNG)
Nop | No Operation / Padding | ![nop](https://user-images.githubusercontent.com/27574733/180931537-a81259e4-1525-470b-911b-f1a234cb491e.PNG)
Add | Adds the top two integers on the stack | ![add](https://user-images.githubusercontent.com/27574733/180931545-9f1e0861-a24e-495f-be3a-a7112202917e.PNG)
Sub | Subtracts the top two integers on the stack | ![sub](https://user-images.githubusercontent.com/27574733/184817591-ef259743-77a0-4dfd-8562-954686d02932.PNG)
Mult | Multiplies the top two integers on the stack | ![mult](https://user-images.githubusercontent.com/27574733/184817604-ea5d3985-cf54-4451-8435-1cb3838eed61.PNG)
Div | Divides the top two integers on the stack (Checks if the divisor is 0) | ![div](https://user-images.githubusercontent.com/27574733/184817612-369e75e7-6370-434d-9a0a-92e9665ad0ba.PNG)
Mod | Performs a modulus on the top two integers on the stack | ![mod](https://user-images.githubusercontent.com/27574733/184817622-cf63d7cf-0cfa-4dfd-aa0a-2a821bbac2bf.PNG)
Not | Performs a logical not on the top integer on the stack | ![not](https://user-images.githubusercontent.com/27574733/184817635-15387583-aeaa-48b7-8125-2a133c281aa2.PNG)
Neg | Performs a negate on the top integer on the stack | ![neg](https://user-images.githubusercontent.com/27574733/184817649-f02beb2e-ab5f-47da-913e-fe584dc2600e.PNG)
CmpEq | Checks if the top two integers on the stack are equal | ![cmpeq](https://user-images.githubusercontent.com/27574733/184817669-d42668f7-a139-4b5a-8a5e-f6806a6ec244.PNG)
CmpNe | Checks if the top two integers on the stack are not equal | ![cmpne](https://user-images.githubusercontent.com/27574733/184817682-865732af-0028-43d7-86ff-8cb20cff98eb.PNG)
CmpGt | Checks if the first pushed integer is greater than the second pushed integer | ![cmpgt](https://user-images.githubusercontent.com/27574733/184817692-0d3df107-722a-4cd2-856c-568dbd331419.PNG)
CmpGe | Checks if the first pushed integer is greater than or equal to the second pushed integer | ![cmpge](https://user-images.githubusercontent.com/27574733/184817698-d66995c5-82a0-433e-a512-e5210f1116d7.PNG)
CmpLt | Checks if the first pushed integer is less than the second pushed integer | ![cmplt](https://user-images.githubusercontent.com/27574733/184817706-28beac8c-c3fb-49e7-8b0e-77500bcd5bec.PNG)
CmpLe | Checks if the first pushed integer is less than or equal to the second pushed integer | ![cmple](https://user-images.githubusercontent.com/27574733/184817715-7952a55d-995c-4399-b85b-f33bf23ff7b5.PNG)
fAdd | Adds the top two floats on the stack | ![fadd](https://user-images.githubusercontent.com/27574733/184817732-dd1c45db-9974-487a-b3a5-e959d458b37c.PNG)
fSub | Subtracts the top two floats on the stack | ![fsub](https://user-images.githubusercontent.com/27574733/184817744-62a71447-7a56-4f35-9c20-0e5ebfbaec53.PNG)
fMult | Multiplies the top two floats on the stack | ![fmult](https://user-images.githubusercontent.com/27574733/184817756-2ae333bb-fcb9-4038-97a3-ce04391a6f90.PNG)
fDiv | Divides the top two floats on the stack (Checks if the divisor is 0) | ![fdiv](https://user-images.githubusercontent.com/27574733/184817774-70f5ba15-5572-4250-b82c-1d62b5ede507.PNG)
fMod | Performs a modulus on the top two floats on the stack | ![fmod](https://user-images.githubusercontent.com/27574733/184817779-b6c73e2c-e664-4e95-a566-16b8fe50b5d8.PNG)
fNeg | Performs a negate on the top float on the stack | ![fneg](https://user-images.githubusercontent.com/27574733/184817784-e6dbac96-ee6d-4868-baf7-367646a8dcd0.PNG)
fCmpEq | Checks if the top two floats on the stack are equal | ![fcmpeq](https://user-images.githubusercontent.com/27574733/184817796-71130e11-9080-40b8-8673-74d41c4f99d2.PNG)
fCmpNe | Checks if the top two floats on the stack are not equal | ![fcmpne](https://user-images.githubusercontent.com/27574733/184817808-ddb505b9-ab07-420d-83aa-81f77a6e9694.PNG)
fCmpGt | Checks if the first pushed float is greater than the second pushed float | ![fcmpgt](https://user-images.githubusercontent.com/27574733/184817821-f13e1178-b055-4916-8235-6bcf94d02095.PNG)
fCmpGe | Checks if the first pushed float is greater than or equal to the second pushed float | ![fcmpge](https://user-images.githubusercontent.com/27574733/184817833-50580efc-3438-42df-8a6b-43b8911e57d8.PNG)
fCmpLt | Checks if the first pushed float is less than the second pushed float | ![fcmplt](https://user-images.githubusercontent.com/27574733/184817848-502eac77-905d-4454-b800-8ee5a40f7485.PNG)
fCmpLe | Checks if the first pushed float is less than or equal to the second pushed float | ![fcmple](https://user-images.githubusercontent.com/27574733/184817857-917bd723-58eb-4e59-bc59-7a3f79b44114.PNG)
vAdd | Adds the top two vectors on the stack | ![vadd](https://user-images.githubusercontent.com/27574733/184817878-f457caaf-a24c-4aa6-a792-1c9d84fc5df3.PNG)
vSub | Subtracts the top two vectors on the stack | ![vsub](https://user-images.githubusercontent.com/27574733/184817887-d5c0bb1a-64af-4eb1-80af-b00217d7b24d.PNG)
vMult | Multiplies the top two vectors on the stack | ![vmult](https://user-images.githubusercontent.com/27574733/184817900-8af3c21b-c532-48a3-99a1-a5a6dc807c70.PNG)
vDiv | Divides the top two vectors on the stack | ![vdiv](https://user-images.githubusercontent.com/27574733/184817904-9d6da8ea-02f4-46ef-9009-563824533fc5.PNG)
vNeg | Performs a negate on the top vector on the stack | ![vneg](https://user-images.githubusercontent.com/27574733/184817925-bd6dcfd0-dff1-4773-aa7a-846e4f4da377.PNG)
And | Performs a bitwise and on the top two integers on the stack | ![and](https://user-images.githubusercontent.com/27574733/184817936-f1a0ab95-3fdf-4222-86fd-2da9d39dbe8a.PNG)
Or | Performs a bitwise or on the top two integers on the stack | ![or](https://user-images.githubusercontent.com/27574733/184817965-c9ce441c-2004-49a5-9491-b347ce5f7f95.PNG)
Xor | Performs a bitwise xor operation on the top two integers on the stack | ![xor](https://user-images.githubusercontent.com/27574733/184817978-43436820-9e4f-484f-89d6-6d675afd2b91.PNG)
ItoF | Converts the top integer on the stack to a float | ![itof](https://user-images.githubusercontent.com/27574733/184817990-4608bf5c-d535-4195-a2f2-5a69c41ebcb6.PNG)
FtoI | Converts the top float on the stack to an integer | ![ftoi](https://user-images.githubusercontent.com/27574733/184818003-292ee3cf-09ad-4bc7-a02d-eb74f1be9aeb.PNG)
FtoV | Loads a float from the stack, then returns a vector of that float | ![ftov](https://user-images.githubusercontent.com/27574733/184818016-7545d428-5d12-4507-a983-59ed0a5b67ab.PNG)
PushB2 | Pushes two unsigned 1 byte integers onto the stack | ![pushb2](https://user-images.githubusercontent.com/27574733/184818030-7cbd8add-386e-4d6b-882a-ba431ab8aa56.PNG)
PushB3 | Pushes three unsigned 1 byte integers onto the stack | ![pushb3](https://user-images.githubusercontent.com/27574733/184818040-6bdacc3e-27fb-412b-a9a9-df4a0a40c842.PNG)
Push | Pushes an integer onto the stack | ![push](https://user-images.githubusercontent.com/27574733/184818050-b69efb11-7854-4f3c-b6c3-a5082febaa6c.PNG)
PushF | Pushes a float into the stack | ![pushf](https://user-images.githubusercontent.com/27574733/184818061-b325f760-f926-4b9e-88f7-7bbee146d7e2.PNG)
Dup | Duplicates the top item on the stack | ![dup](https://user-images.githubusercontent.com/27574733/184818085-a260dba4-77ec-4ec5-bb55-00fad8c520f5.PNG)
Drop | Pops an item off the stack | ![drop](https://user-images.githubusercontent.com/27574733/184818101-faf142b2-dd4f-4ae7-8957-09ce396fbc2a.PNG)
CallNative | Loads param count and the return count from the code buffer, params if any off the stack, then calls the native function | ![callnative](https://user-images.githubusercontent.com/27574733/184820343-4864a2f2-8cc7-4f7d-b3df-a795e6c2ab9b.PNG)
Function | Loads the parameter count and the total var count from the code buffer | ![function](https://user-images.githubusercontent.com/27574733/184818128-a36f6936-3be3-4dd8-ab93-547cbb3e67cc.PNG)
Return | Loads the parameter count and the return variable count from the code buffer | ![return](https://user-images.githubusercontent.com/27574733/184818141-f76dd9d5-65c4-41f4-a455-2c86ea78e94a.PNG)
pGet | Loads a pointer from the stack, then returns the value of the pointer | ![pget](https://user-images.githubusercontent.com/27574733/184818154-7e434507-eed4-4ea6-bfe8-339ce9512cdd.PNG)
pSet | Loads a value and a pointer from the stack and stores the value at the pointer | ![pset](https://user-images.githubusercontent.com/27574733/184818170-ab1ff502-f977-4a57-9fde-79d085c3a063.PNG)
pPeekSet | Loads a pointer and a value from the stack, sets the value at the pointer, then keeps the value on the stack | ![ppeekset](https://user-images.githubusercontent.com/27574733/184818183-dd09f100-2032-42bb-ae09-ff819aa359fb.PNG)
ToStack | Loads the number of items and a pointer from the stack, then returns the items at the pointer |![tostack](https://user-images.githubusercontent.com/27574733/184821077-aafd5ab5-1296-4055-84f1-291651af741f.PNG)
FromStack | Loads the number of items and a pointer from the stack, then loads the items on the stack at the pointer | ![fromstack](https://user-images.githubusercontent.com/27574733/184821094-100d3d78-f25b-4be8-9f0e-b4d1863420f9.PNG)
GetArrayP | Loads the array index and the array pointer from the stack, loads the element size from the code buffer, then returns the pointer to the item | ![getarrayp](https://user-images.githubusercontent.com/27574733/184819718-8dac56c5-9d64-43f1-b109-32267f907003.PNG)
GetArray | Loads the array index and the array pointer from the stack and loads the element size from the code buffer, then returns the value | ![getarray](https://user-images.githubusercontent.com/27574733/184819733-4a49da87-45eb-4b72-85ac-fd5ad0da1b56.PNG)
SetArray | Loads the value, array index and array pointer from the stack, and loads the element size from the code buffer, then sets the value at the pointer | ![setarray](https://user-images.githubusercontent.com/27574733/184819753-0d1c9bd5-ea9a-45ef-9145-c683bcd10ab4.PNG)
GetLocalP | Loads the variable index of an internal function from the code buffer, then returns the pointer to the index | ![getlocalp](https://user-images.githubusercontent.com/27574733/184821192-6a36d569-a8af-44cc-b0fc-5876eda9a652.PNG)
GetLocal | Loads the variable index of an internal function from the code buffer, then returns the value | ![getlocal](https://user-images.githubusercontent.com/27574733/184821176-1514bac3-e2ec-4f9a-9e30-0d27da45beef.PNG)
SetLocal | Loads a value off the stack and the variable index of an internal function from the code buffer, then sets the value at the index |![setlocal](https://user-images.githubusercontent.com/27574733/184821229-8f95d335-168c-4598-a923-59312a1535eb.PNG) 
GetStaticP | Loads the index of a static from the code buffer, then returns the pointer to the index | ![getstaticp](https://user-images.githubusercontent.com/27574733/184818326-2f1aac60-fdb7-4bab-9159-b92c370faf81.PNG)
GetStatic | Loads the index of a static from the code buffer, then returns the value | ![getstatic](https://user-images.githubusercontent.com/27574733/184818344-27398ecd-7422-4374-bfdc-eefeee133056.PNG)
SetStatic | Loads a value off the stack and the index of a static from the code buffer, then sets the value at the index | ![setstatic](https://user-images.githubusercontent.com/27574733/184818359-bdcd94e6-9b09-4466-81af-0aecf8f54c99.PNG)
AddImm | Loads a value from the code buffer and adds it to the value on the stack | ![addimm](https://user-images.githubusercontent.com/27574733/184818386-c09792f2-c2f6-4a0f-acb1-02745b63fdef.PNG)
MultImm | Loads a value from the code buffer and multiplies it to the value on the stack | ![multimm](https://user-images.githubusercontent.com/27574733/184818419-78887413-5b84-4ade-a6ce-c2ebaaf6b3cd.PNG)
GetImmPs | Loads the pointer and the immediate index from stack, then returns the pointer at the index | ![getimmps](https://user-images.githubusercontent.com/27574733/184818453-343e84cb-2ddb-4480-9364-ffb738fc80e1.PNG)
GetImmP | Loads a pointer from stack and the immediate index from the code buffer, then returns the pointer at the index | ![getimmp](https://user-images.githubusercontent.com/27574733/184818434-a118dabc-3451-4f37-9200-e3a691805899.PNG)
GetImm | Loads a pointer from stack and the immediate index from the code buffer, then returns the value at the index | ![getimm](https://user-images.githubusercontent.com/27574733/184818531-59b03d22-b193-4c9d-9df8-b8318d6c1ce7.PNG)
SetImm | Loads a value and a pointer from the stack, and the immediate index from the code buffer, then sets the value at the index | ![setimm](https://user-images.githubusercontent.com/27574733/184818513-66cf9654-b300-47b3-913c-63e6984422dd.PNG)
GetGlobalP | Loads the index of a global from the code buffer, then returns the pointer to the index | ![getglobalp](https://user-images.githubusercontent.com/27574733/184818573-56bf242f-bf19-45bc-a739-c35d4a97d3c1.PNG)
GetGlobal | Loads the index of a global from the code buffer, then returns the value | ![getglobal](https://user-images.githubusercontent.com/27574733/184818587-0716f35d-f659-4c68-87e0-c45eabf56da9.PNG)
SetGlobal | Loads a value off the stack and the index of a global from the code buffer, then sets the value at the index | ![setglobal](https://user-images.githubusercontent.com/27574733/184818596-5a63d0e6-4f38-4024-a607-5eb43ffb532e.PNG)
Jump | Loads the jump position from the code buffer and performs an unconditional relative jump | ![jump](https://user-images.githubusercontent.com/27574733/184818606-3e34d5b9-0d0c-4680-a9bf-feaa2153cfc6.PNG)
JumpFalse | Loads the jump position from the code buffer and jumps to relative position if top of stack is 0 | ![jumpfalse](https://user-images.githubusercontent.com/27574733/184818615-f6e3dffa-ecf3-4ad5-a884-d3d897954ea2.PNG)
JumpTrue | Loads the jump position from the code buffer and jumps to relative position if top of stack is 1 | ![jumptrue](https://user-images.githubusercontent.com/27574733/184818631-3ea87079-611a-43fc-872b-fbcb02c6357d.PNG)
JumpNE | Loads the jump position from the code buffer and jumps to relative position if the top two items on the stack are not equal | ![jumpne](https://user-images.githubusercontent.com/27574733/184818653-91114e5e-e93e-4ceb-adcf-be25a394e5d3.PNG)
JumpEQ | Loads the jump position from the code buffer and jumps to relative position if the top two items on the stack are equal | ![jumpeq](https://user-images.githubusercontent.com/27574733/184818672-f91e67d1-4d61-4eff-a4bc-231096a0eda4.PNG)
JumpLE | Loads the jump position from the code buffer and jumps to relative position if the first item on the stack is less than or equal to the second | ![jumple](https://user-images.githubusercontent.com/27574733/184818688-dafbc301-85dc-4b0d-9f2a-488cd1751af0.PNG)
JumpLT | Loads the jump position from the code buffer and jumps to relative position if the first item on the stack is less than the second | ![jumplt](https://user-images.githubusercontent.com/27574733/184818703-ea983cd5-1618-4001-9270-6bed0c1dc1c9.PNG)
JumpGE | Loads the jump position from the code buffer and jumps to relative position if the first item on the stack is greater than or equal to the second | ![jumpge](https://user-images.githubusercontent.com/27574733/184818717-e8f46859-1ea5-4f92-80c8-8cc7259688ef.PNG)
JumpGT | Loads the jump position from the code buffer and jumps to relative position if the first item on the stack is greater than the second | ![jumpgt](https://user-images.githubusercontent.com/27574733/184818728-75c9e5c0-0e34-45f8-9884-bf6f086ec75d.PNG)
Call | Loads params if any off the stack, then calls to a function in the script | ![call](https://user-images.githubusercontent.com/27574733/184818744-d4c46811-2b07-4fa2-9c3c-b9b64fc38f55.PNG)
Switch | Loads the index off the stack, case numbers and jump positions from the code buffer, then jumps to the position at case equal to the index |![switch](https://user-images.githubusercontent.com/27574733/184818762-1bbc791d-72e0-4ddd-89e7-05315b77001e.PNG) 
PushString | Pushes a string pointer to the stack | ![pushstring](https://user-images.githubusercontent.com/27574733/184818775-8e05bd6d-e661-49b0-9788-516678c89cdb.PNG)
GetHash | Loads a value off the stack, then returns a Jenkins hash of the value | ![gethash](https://user-images.githubusercontent.com/27574733/184818795-fdfd230c-81f8-4016-8188-f88679815ff8.PNG)
StrCopy | Loads the source string pointer and the dynamic destination string pointer off the stack, and the length from the code buffer, then copies the source into the destination | ![strcopy](https://user-images.githubusercontent.com/27574733/184818813-0c0c21ce-6cb2-4298-aaf7-a91f60ad7574.PNG)
ItoS | Loads a value and a dynamic destination string pointer off the stack, the destination string length off the code buffer, then converts the value into string and places it at the pointer | ![itos](https://user-images.githubusercontent.com/27574733/184818833-d66a6489-41a5-4bfe-87e2-8ed4422cc23b.PNG)
StrAdd | Loads a source string pointer and the dynamic destination string pointer off the stack, and the length from the code buffer, then adds the source onto the destination | ![stradd](https://user-images.githubusercontent.com/27574733/184818859-861d119c-d284-48d0-b768-fdfe7c2818c6.PNG)
StrAddi | Loads an integer and the destination string pointer off the stack, and the length from the code buffer, then adds the string representation of the integer onto the destination | ![straddi](https://user-images.githubusercontent.com/27574733/184818870-d932d435-606e-48a2-9faa-ce92c36b8c24.PNG)
MemCopy | Loads the struct item count, struct size in bytes and the destination pointer the stack, then pops the struct off the stack and sets it into the destination | ![memcopy](https://user-images.githubusercontent.com/27574733/184818923-04e4e3ee-7ad8-4ed8-9048-d8a2733455cd.PNG)
Catch | Sets up a safe area that has the ability to catch errors (Deprecated) | 
Throw | Indicates an area that handles a script error relative to the catch opcode (Deprecated) | 
pCall | Loads a pointer from the stack and calls to the pointer as a function | ![pcall](https://user-images.githubusercontent.com/27574733/184818957-94b828cf-2052-4c56-8fd2-be22de013c2b.PNG)

