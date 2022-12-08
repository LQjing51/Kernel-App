[TOC]

## 组内工作

攻击：漏洞挖掘（软件的安全漏洞分析：hyh，js引擎），漏洞利用（传怎样的输入给漏洞）

防御：数据保护（bac，利用硬件，保护指针防止篡改，密钥，分析linux的内核数据生命周期，不用的数据应该把其释放，读写权限关掉，有些数据只读）、代码保护（写同一个源程序，生成不同的二进制程序，防止别人进行源分析）、中关村实验室操作系统加固（代码持续随机化，模块隔离）

+ 代码混淆：
  二进制混淆，病毒可执行程序修改，识别不出来

  源代码混淆，每次编译出来都不一样，LLVM

+ 数据保护：
  + 变量的检查开关关掉，free，加完整性校验 ////
  + 内核态app，让进程使用特权的硬件，让进程自己使用页表，用户进程，JVM进行用户的GC，GC检查页表的访问情况，进行修改，用户运行在客户机的特权态，操作系统使用宿主机的特权态//M1 arm，pac /// LLVM+linux kernel，arm，不是x86，
  + BPF////
  + 体系结构安全，熔断幽灵，
  + 增强rust安全性////纯编译器
  + 代码随机，二进制代码变位置，操作系统解耦，返回地址的监控，

内核态app：

+ 保证功能性：arm异常（svc、IRQ、FIQ），arm异常处理，arm特权指令构成
+ 保证安全性：
  + 内存：app不能随意访问内核空间的数据和代码（os并不知道有用户运行在内核态，对os透明），通过隔离（isolation），arm 硬件特性：pac，mte
  + 特权指令：过滤filter安全门，通过安全门执行特权指令，有些指令（资源）可以，有些不行。
    + loader 扫描二进制文件，看有没有特权指令，（x86变长指令，修改指针，可以有原本没有的指令。arm固定长度指令，四个字节），之前组内做x86 内核态 app，利用虚拟化，虚拟化里面特权级，arm更加简单，直接跑在内核
    + elf：text段，rodata（程序用到的只读数据，理论上只可读，但是代码量比较少时，和text等放在一页中，就有了执行权限，pc跳到这里，就可以通过构造执行特权指令；编译器优化过程中，有的数据和代码段混合编在一起，就有了执行权限），plt。代码页数据的分离
+ 应用场景：JVM猜使用页面的情况，可以设计接口，直接告诉os，回收合并。app缓冲区溢出，使得攻击者可以读到key，将key存在内存特定区域，通过开关权限，syscall，改为直接通过安全接口。



四个SP，不用压栈，ELx可以用比等于小于的SP，实际上EL1还是用SP_EL1

内核态app移到os，还是使用SP_EL0，替换原本没用的Current exception level with SP_EL0的处理，跳到原本的用户异常处理

关闭内核空间，将临时向量表放到用户空间，先跳到临时向量表，然后开内核，跳到内核处理，再关内核，再eret，EL1-EL1。TLB命中时，不管后面内核size的检查，于是利用ASID机制，将内核空间和用户空间的ASID区分开来，阻止用户空间访问内核ASID的TLB项地址内容。需要访问内核时，临时向量区域会改为内核的ASID,注意不能不改，直观上只会造成多的miss，但是利用用户ASID加载页，返回之后用户态可以访问这些TLB项

用户空间通过异常到了临时向量表，会改SP，其它恶意挑战，SP不变，内核空间执行一段就检查一次

每当有可执行的页面出现：监控改页面的执行权限的系统调用，mmap，将其map到可执行的区域，execv执行完，即该进程的空间都分好，在调度之前，进行检查

硬件的DEP：WXn，若一个页有可执行可写权限，则屏蔽可执行



M1芯片，上不用mac os，改为linux（3月），双系统，加kenel moduler。不能调gdb，只能print，硬件串口调试，



MMU notifier：部分写PTE 的access flag未触发，；写保护页，权限由读变为读写，未触发

flush asid+1解决

KVM

LSB shared object/executable，代码无关



**毕设：**

静态二进制扫描，JIT编译器生成的代码扫描，快速。

二进制重写，根据筛查出来的指令，重改ELF。

segment section，每个函数之后emit constPool，数据和data编在一起



手册+决策树，先分为一样和不一样，再说可以根据环境的配置，将一些不一样的，变成了一样的，且没有需要模拟的，通过特权寄存器阻止了行为不一样，

代码加载的时候扫描，给定一个编码，快速识别，找到它是，二进制编码，看op是否在一个range里面，如果有，则不执行，

JIT 既有代码又有数据，怎么知道是代码，在os将其设置成可执行时，需要扫





背景：进程下放

线下扫描：替换成0。扫描代码段，（代码段中内嵌了数据，避免内嵌数据（二进制重写））

线上：verify 扫描可执行页中

load elf 和所有生成可执行页的操作前进行hook



spec中不会生成这些指令，报错的工作不可验证，自己加一个例子。



可执行权限数据（且满足特定编码）的消除——二进制方案

text section中的内嵌数据，rodatat和text编在一起，一些小section也会和text编在一起

内嵌数据的识别 NORAX

rodata拷贝出来，放在新的部分，然后重定位（PC+XX访问到这个数据，应该改数据引用指令的offset），只需要拷贝和重定位text同一页中后面的rodata，其它的被编为可执行的，将其权限修改。剩下的text改为只可执行。如果读原来的rodata段（将其改为0），会报page fault，处理它。静态，直接寻址，可以看出来，可能会有遗漏，采用动态重定位。



本来是只可读，和text编在一起后，会变得可执行。

磁盘上也有可能被攻击，load的时候：
./a.out，excve 创建进程，分配页表，设置映射等。load elf，

mmap没有映射，只有第一次访问该页的时候，触发page fault，才新建立映射。所有设置可执行页的时候要进行扫描。mmap写一个页，之后将其改为可执行。

do_fault_around，局部性原理，page fault一次，将其后的页的映射也给只上，扫描所有被置为可执行的页。

指令分类表格

kernel app背景



**扩展：**

指针保护/指针完全性

x86没有PAC机制，可以在内存中设置安全区（内存和安全区一比一的映射，源指针地址加一个offset就可以找到安全区的它，但是一比一很浪费，大部分地方是空洞，少数部分是指针，用hash算法压缩安全区，hash的输入是指针地址，通过hash后的值索引），备份敏感指针，保证其不会被修改，在解引用的时候，比较安全区中的内容和当前的指针值是否相同。

外面的指针产生，传播，使用，free，在安全区中都需要同步进行，

安全区的隔离，可以通过页表bit的设置，mpk

隔离为主，PAC为辅：利用PAC（输入context是指针地址，和key）替代hash，加快索引

PAC为主，隔离为辅：将context相同的指针放在安全区（实际上要放的是context相同，value不同的，context相同和value相同的，不需要保护）。这样子context的指针非常多，应该从指向的对象是否是一个来判断，看new的情况。代码指针不需要隔离，数据指针需要隔离，

函数指针，pac时context可以用function id，但是auth时只知道调用函数的类型，静态不可行

那些可以复用的指针，怎样设置好context，将相关的内容放在保护区中，可以动态计算，用的时候从保护区中获取数据进行计算。

## 编译器

Clang 和 gcc 都是c/c++的编译器， Clang 是编译器前端，采用底层虚拟机 LLVM 作为后端。相比GCC, Clang编译器速度快，内存占用小，诊断信息可读性强，兼容性好。Open64也是一个编译器

+   统编译器的工作原理，基本上都是三段式的，可以分为前端、优化器和后端。前端负责解析源代码，检查语法错误，并将其翻译为抽象的语法树；优化器对这一中间代码进行优化，试图使代码更高效；后端则负责将优化器优化后的中间代码转换为目标机器的代码，这一过程后端会最大化的利用目标机器的特殊指令，以提高代码的性能。 

<img src="C:\Users\24962\AppData\Roaming\Typora\typora-user-images\1662449605534.png" alt="1662449605534" style="zoom:67%;" />

SSA：静态单赋值，对某个变量的每次赋值用一个新变量名v1,v2……，每次引用采用最近的赋值的变量名。更新变量名后，每个变量只有一次静态赋值。有控制流的SSA，在if语句后，插入$\Phi$函数，$v3 = \Phi(v1,v2)$选择一个v作为后续使用。

RTT：Register Transfer Language，

交叉编译：

Build平台（X）：编译GCC的平台

Host平台（Y）：运行上述编译得到的GCC平台

Target平台（Z）：GCC生成的本地吗所属的平台

X=Y=Z：本地安装

X=Y!=Z：交叉编译

X!=Y!=Z：canadian 交叉编译

## 虚拟化

将一台计算机虚拟为多台逻辑计算机，实现资源的模拟，隔离和共享。整个虚拟机都保存在文件中，可以通过移动文件的方式迁移。

#### 虚拟化分类：

+ type1 virtualization，基于硬件的虚拟化：比如把x86平台的CPU，内存和外设作为资源，在同一个x86平台上可以虚拟多个x86平台，每个平台运行自己独立的操作系统。hypervisor架构，运行于x86裸机上。
+ type2 virtualization，基于操作系统的虚拟化：把操作系统及其提供的系统调用作为资源，例如Linux容器虚拟化，在同一个Linux操作系统上，虚拟出多个Linux操作系统。hosted架构，运行于host os上。例：lxc，docker

#### 虚拟化管理程序Hypervisor

以下均为type2虚拟化

（hypervisor上对应每一个虚拟机有一个**VMM，Virtual Machine Monitor**虚拟机监视器，VMM实现了虚拟机的硬件抽象并负责运行虚拟机系统）：

Hypervisor是一种运行在物理服务器和虚拟机客户端间的”元“操作系统，用于加载所有虚拟机客户端的操作系统，分配资源，并在不同虚拟机间施加防护，进行调度等。

 <img src="https://img-blog.csdn.net/20150418215204355?watermark/2/text/aHR0cDovL2Jsb2cuY3Nkbi5uZXQvZmx5Zm9yZnJlZWRvbTIwMDg=/font/5a6L5L2T/fontsize/400/fill/I0JBQkFCMA==/dissolve/70/gravity/Center" alt="img" style="zoom:80%;" /> 

特权指令：这些指令只有在最高特权级上能够正确运行，如果在非最高特权级上运行，可能会产生异常，然后陷入最高特权级进行执行/处理，也可能直接忽略。

敏感指令：操作特权资源的指令，包括修改虚拟机/物理机的运行状态；读写时钟、中断等寄存器；访问存储保护系统，地址重定位系统及所有的I/O指令等。

#### x86虚拟化

没有虚拟化时的特权架构为：

<img src="C:\Users\24962\AppData\Roaming\Typora\typora-user-images\1666963513024.png" alt="1666963513024" style="zoom:50%;" />

虚拟化的本质是：GuestOS的特权被解除（privilege deprivileging），从ring0降到ring1/3，特权指令在GuestOS中发生时，会产生trap，被VMM捕获，由VMM完成(trap-and-emulation)。虚拟化时敏感指令必须被VMM捕获完成。arm/x86架构均存在不是特权指令的敏感指令，且在ring0下和ring3下语义可能不同，即虚拟化漏洞。问题：如何在运行时陷入并翻译敏感指令和特权指令（即虚拟化这些指令），有以下三种解决方案：

+ 使用二进制翻译的全虚拟化：VMM对guest os中的二进制代码进行扫描，一旦发现guest os执行的代码中包含有运行在用户态上的特权指令二进制代码时，将这些二进制代码翻译成虚拟特权指令，或者翻译成运行在核心态中的特权指令，从而强制触发异常。异常陷入后，使用一系列作用于虚拟化硬件可达到效果的新指令序列替换那些不可虚拟化的指令。hypervisor将操作系统的指令翻译的结果缓存供之后使用。其余用户级指令无需修改直接运行在物理处理器上。模拟CPU让虚拟机使用，qemu，效率低。

  <img src="C:\Users\24962\AppData\Roaming\Typora\typora-user-images\1666960968392.png" alt="1666960968392" style="zoom: 50%;" />

+ 操作系统辅助的虚拟化/半虚拟化：修改guest os kernel，将不可虚拟化的指令对应的调用（例如TLB_flush()）替换为直接与虚拟化层交互的hypercalls。例：Xen

  <img src="C:\Users\24962\AppData\Roaming\Typora\typora-user-images\1666962210649.png" alt="1666962210649" style="zoom: 50%;" />

+ 硬件辅助的全虚拟化：Intel的VT-x和AMD的AMD-V，为CPU新增了一个执行模式root，用于运行VMM，特权和敏感调用自动陷入hypervisor，不需要二进制翻译或者半虚拟化，虚拟机的状态保存在虚拟机控制块中（VMCB,AMD-V；VMCS,VT-x）。只修改了CPU，还需要对网卡，存储使用虚拟化驱动程序。例：KVM， KVM通过在HostOS内核中加载**KVM Kernel Module**来将HostOS转换成为一个VMM。所以此时VMM可以看作是HostOS，反之亦然。 

<img src="C:\Users\24962\AppData\Roaming\Typora\typora-user-images\1666962382660.png" alt="1666962382660" style="zoom:50%;" />

内存虚拟化：VMM负责将VM的物理内存PA映射到硬件的机器内存MA，并使用影子表来加速映射过程：红线表示VMM使用硬件中的TLB来直接映射虚拟内存到机器内存，当VA到PA的映射改变时，VMM更新影子表。

<img src="C:\Users\24962\AppData\Roaming\Typora\typora-user-images\1666963201513.png" alt="1666963201513" style="zoom:67%;" />

设备和I/O虚拟化：

Libvirt 是一套支持linux主流虚拟机管理程序的虚拟软件，其旨在为包括KVM在内的各种Hypervisor提供一套可靠方便的编程接口。 当前主流linux平台上默认的虚拟化管理程序virt-manager、virsh等都是基于libvirt开发的。 

<img src="C:\Users\24962\AppData\Roaming\Typora\typora-user-images\1666950416679.png" alt="1666950416679" style="zoom:80%;" />

#### arm64虚拟化

硬件辅助虚拟化架构

VHE（virtualization host extensions）开启后，host os和hypervisor运行在同一级或者更高级。hypervisor运行在EL2，原本OS运行在EL1。支持VHE特性，将host os kernel移到EL2后，可以减少EL1和EL2直接模式切换的次数，否则KVM部分必须在EL2执行的代码（比如捕获guest os退出异常）在EL2执行，其它的KVM代码和host os都在EL1执行，这样hypervisor在进行资源管理和虚拟机调度时需要在EL1和EL2间进行切换。

<img src="C:\Users\24962\AppData\Roaming\Typora\typora-user-images\1667027338356.png" alt="1667027338356" style="zoom:67%;" />

HCR_EL1的E2H位表明是否VHE使能，TGE位表明当VHE使能时，EL0运行的应用时guest还是host的。

```c
E2H
EL2 Host. Enables a configuration where a Host Operating System is running in EL2, and the Host
Operating System's applications are running in EL0.
TGE
Trap General Exceptions, from EL0. All exceptions that would be routed to EL1 are routed to EL2.
```



<img src="C:\Users\24962\AppData\Roaming\Typora\typora-user-images\1667029006370.png" alt="1667029006370" style="zoom:50%;" />

  

**arm64 cpu 虚拟化：**新增EL2级，VMM运行在EL2，guest os运行在EL1，应用运行在EL0。EL2打开的模式下，EL1和EL0下的特权操作会陷入到EL2模式下（CPU自动检测），VMM模拟这些特权操作。ARM64为一部分寄存器提供虚拟寄存器，guest os读取虚拟寄存器，从而加快虚拟机的切换过程。增加HVC指令，让guest os必要时可以主动进入EL2执行，类似系统调用。

**arm64内存虚拟化：**开启虚拟机时，EL0/1的地址转换分为两个阶段：stage1：guest os将进程的虚地址转化为IPA；stage2：VMM把IPA转换为真正的物理地址PA。EL2的地址转发是直接由VA转变为PA。

在VHE引入之前，虚拟地址空间布局如下：EL2仅有一个区域，因为hypervisor不会host应用。

<img src="C:\Users\24962\AppData\Roaming\Typora\typora-user-images\1667031524929.png" alt="1667031524929" style="zoom: 67%;" />

当E2H为1时，即将host OS放到EL2上时，布局改为下图所示：TGE位控制os运行在EL1（guest os）还是EL2（host os）

<img src="C:\Users\24962\AppData\Roaming\Typora\typora-user-images\1667031659742.png" alt="1667031659742" style="zoom:67%;" />

此外还需要重定向寄存器访问：E2H为1时，内核访问EL1寄存器时将重定向到相应的EL2寄存器，但是因为hypervisor仍然需要访问真实的EL1寄存器，以实现上下文切换，引入一组新寄存器EL12,EL02，用于在EL2下访问真正的EL1寄存器。

**arm64下KVM虚拟化架构：**hypervisor（VMM）由KVM+host os共同担任

<img src="C:\Users\24962\AppData\Roaming\Typora\typora-user-images\1667027750313.png" alt="1667027750313" style="zoom:67%;" />

**kvm虚拟化系统的执行流：**
  <img src="C:\Users\24962\AppData\Roaming\Typora\typora-user-images\1667029047812.png" alt="1667029047812" style="zoom:67%;" />

+ 当位于应用层的qemu创建好虚拟机，准备为虚拟机执行代码时，调用ioctl进入内存层的KVM，然后KVM负责从host切换到guest(VMEntry)模式执行虚拟机代码。
+ 虚拟机正常执行代码，当执行到敏感指令或者要进行IO操作时，会自陷到EL2模式，进入VMM，这就是VMEXIT。
+ kvm(VMM）处理guest的退出原因，比如模拟敏感指令，如果是IO操作，得进入用户层qemu就行设备模拟。
+ qemu模拟完IO操作再此ioctl进入kvm,如此循环执行。
  

## ARM

CPU根据不同的指令集类型可以分为：

+ RISC：代表处理器（CPU）有Sun公司的SPARC系列、IBM的Power Architecture（Power PC）与ARM系列，mips
+ CISC：代表有AMD、Intel、VIA等x86架构的CPU，这样的CPU大量用于个人计算机，个人计算机常被称为x86计算机，linux内核的底层就是x86架构的cpu。

### 异常

ARM架构根据 exception level分为EL0（app）,EL1（kernel）,EL2（hypervisor/virtual machine monitor）,EL3（secure monitor，在trustzone切换过程中的mode）。一般情况下EL0是执行在unprivilege mode下的，其他三种都是执行在privilege mode下的，需要配置自己的TCR寄存器。

arm提出trustzone之后，定义了两种physical address space，在理论上，secure和non-secure的physical address应该是完全分开的两个地址空间，甚至存储器，但是在实现中，多通过bus attribute来进行控制。表示的是同一块地址，在进行secure和non-secure切换的时候，需要自己保存上下文。

要想改变执行状态，AArch64与AArch32之间的转化，只能在陷入异常或者从异常返回时才可以改变。从低的异常级别陷入到高的异常级别时，可以保持执行状态不变也可以从AArch32变为AArch64；反之类似。

<img src="C:\Users\24962\AppData\Roaming\Typora\typora-user-images\1667031323165.png" alt="1667031323165" style="zoom:80%;" />

+ 低level向高level切换通过exception的方式转换，有如下exceptions，expection entry时会写好ELR寄存器，用于异常的返回

+ 高level向低level则通过ERET指令；

异常分为同步异常和异步异常两种：

+ 同步异常：由刚刚执行的指令或者与之相关的操作引起，异常同步于执行流。例如：执行了无效的指令，内存访问地址未对其，MMU权限检查出错。异常生成指令：SVC,HVC,SMC用于帮助系统实现接口调用，允许权限较低的代码像权限较高的代码请求服务。

1. SVC for transition to EL1 (system calls)
2. HVC for transition to EL2 (hypervisor：超级监督者 calls)
3. SMC for transition to EL3 (secure monitor call)

+ 异步异常：外部产生的，ARMv8需要它在一个限定的时间内产生，异步异常可以被短暂屏蔽。比如物理中断（SError系统错误,IRQ,FIQ），虚拟中断（由执行在EL2的软件产生，vSError,vIRQ,vFIQ）。IRQ和FIQ均用于外设中断，用于安全和非安全中断。SError指由内存系统生成的响应去反应错误的内存访问，例如通过了MMU权限检查但在内存总线上遇到错误的内存访问，或对RAM的奇偶校验码或纠错码ECC检查出错。

异常处理：

先跳到向量表一个位置，该位置包含通用代码，负责将当前程序状态压栈退栈等，再跳转到进一步处理中断的代码，之后再回来。ELR （exception link register）：保存exception返回地址到ELR_ELx中，x是异常跳转到的EL的层级；SPSR （saved processor state register）： 执行exception前保存当前的PSTATE到SPSR_x中，执行exception完返回时，SPSR会被copied到PSTATE。

异常跳转到的EL的层级由更高的EL来决定：且SCR_EL3(Secure Configuration Register)可以屏蔽HCR_EL2(Hypervisor Configuration Register)

<img src="C:\Users\24962\AppData\Roaming\Typora\typora-user-images\1665469471067.png" alt="1665469471067" style="zoom:80%;" />

不同的level下，有不同向量表地址，向量表基址存在VBAR_ELx中，向量表是包含指令的普通内存区域：根据exception type（synchronous，IRQ，FIQ or systerm Error）或者exception origin（same or lower exception level）和register width来区分不同的异常的offset，syndrome寄存器提供了exception 信息。

<img src="C:\Users\24962\AppData\Roaming\Typora\typora-user-images\1665469778164.png" alt="1665469778164" style="zoom:80%;" />

### MMU

 RMv8理论上最高可以提供提供了2^64个虚拟地址，但是超过16 Exabyte （2^4 * 2\^60）意义并不大，所以选择跟x86一样，可以使用最大支持2\^48虚拟地址的寻址范围就足够； **使用TTBR0存储user-space的page table地址，TTBR1存kernel-space页表地址**；根据TCR_EL1.T0/T1SIZE 来表示高bit的位数。VA的最高有效位（MSB）为0时，MMU使用TTBR0的转换表来翻译，VA最高位为1时，MMU使用TTBR1的转换表来翻译。**EL2和EL3有TTBR0，但是没有TTBR1，只能使用下面的虚拟地址范围。**

![1663601024761](C:\Users\24962\AppData\Roaming\Typora\typora-user-images\1663601024761.png)



![1663589363718](C:\Users\24962\AppData\Roaming\Typora\typora-user-images\1663589363718.png)

translation:

![1663589600576](C:\Users\24962\AppData\Roaming\Typora\typora-user-images\1663589600576.png)



### 栈溢出攻击与防御

栈溢出攻击：局部变量放在栈上，通过buffer的溢出，可以修改栈上其它的内容，然后再执行新写进去的内容，或者跳转到新写进去的地址执行。

通过在页表上加上执行权限的设置，可以防止某些页被执行：系统控制器**SCTLR_ELx**中可以设置**UXN**（User Execute-nerver，EL0）和**PXN**（Privileged Execute-nerver，EL1），XN（Execute Never）；SCTLR_ELx中还提供了控制位，使得**所有可写地址都不可执行**，这样，堆栈空间内容是不可执行的。

<img src="C:\Users\24962\AppData\Roaming\Typora\typora-user-images\1663422202380.png" alt="1663422202380" style="zoom:67%;" />

#### ROP，JOP

基于此防御机制，Return-oriented programming 和 Jump-oriented programming的改进，返回地址或者跳转的地址来源于相应的寄存器，攻击者可以通过修改栈中的内容，进而修改寄存器的内容（在A函数调用函数B时，会将返回地址压栈，B函数返回时，先从栈中找到之前存的地址，然后将其放到返回地址寄存器LR中，然后再return，由此在函数B的buffer溢出时，就可以修改对应的返回值）。跳转/返回到新的地址后，执行gadget，再跳转……，形成gadget串，就可以实现执行一个有效的由现有代码片段组成的新程序（之所以要现有的代码，是因为可写内存是不可执行的，不能够执行新的写的代码，找gadget流程（c库函数中有丰富的可用的gadget）可以自动化并且多次使用，地址空间随机化ASLR可以防止自动和多次攻击）。开始的几个 gadget 应该实现给rax，rdx等寄存器赋想要的值的功能，都准备好了之后，才可以继续跳转到想要的地方。CFI 控制流完整性，按照本意的控制流执行，不能走别的路，通过静态和动态检查，知道某个指针到底指向谁，跳过去的时候，检查是否是原来指向的东西。直接跳转改不了，主要针对间接跳转，可以篡改。

+ MTE：不可以越界/free后写；
+ PAN：跳转地址前后不能变，符合某pattern；
+ BTI：间接跳转后一定要到BTI指令；
+ PAN：kenel不可以访问（读/执行/写）user space的代码

#### MTE

**MTE（Memory Tagging Extension），阻止数组越界访问和释放之后再访问。**空间局部安全性：内存对象的访问超出了该内存对象所允许的最大有效范围，数组越界。时间局部安全性：当一个内存对象被释放之后，再次访问：

<img src="C:\Users\24962\AppData\Roaming\Typora\typora-user-images\1663427470600.png" alt="1663427470600" style="zoom:80%;" />

**在分配内存的时候，就给地址的高位（4bits）打tag，指针访问时需要比较指针中的tag（key）与实际内存的tag（lock）是否匹配。**MTE使用Armv8-A体系结构的最高字节忽略（TBI）功能。启用TBI后，将虚拟地址的用作地址转换的输入时，将忽略该地址的高位字节。这允许高位字节存储元数据。图中一个tag能够管理的内存颗粒大小是16字节。

<img src="C:\Users\24962\AppData\Roaming\Typora\typora-user-images\1663429103724.png" alt="1663429103724" style="zoom:67%;" />



#### PAC

PAC：指针以64位存储，但是前12位都是000（kernel space地址）或者FFF（user space地址），利用这些**高位可以存储签名（pointer authentication code），存储返回地址时（不一定是存储时，后续觉得要用的时候，给它加上pac，真正用的时候auth），对返回地址进行签名，存在栈中（内存），若返回时的寄存器中的返回地址没有通过PAC认证，则报异常。（用于indirect branch，load）**签名用指令**PAC**,验证用指令**AUT**。pac = mac(pointer, key, modifier)，不同的pointer可以选用不同的key（总共5个，两个用于instruction pointers，两个用于data pointers，一个通用），modifier可以是一些context信息，例如此次function call的Stack pointer(SP)信息。每个running application可以使用不同的key，相同application每次运行被分配不同的key。只有context和指针值都相同时，auth才可以通过

放在高位：除了因为高位不用，还因为方便指针的拷贝/传播

指令指针：context存SP，指针（返回地址）的位置，a调用b，然后c，SP相同，漏洞

数据指针：context指数据结构的类型，不存指针的位置，因为指针传播过程中，地址会变，那么每次传播都得先验证，再生成pac。若int*变为char\*，类型变化了，还是需要重新pac。问题在于：相同类型的指针，int a, int b, ptra  = &a, ptrb = &b，ptra，ptrb都加上了pac，本来希望得到b的地址，若将ptra的值赋值给ptrb之后，对ptrb的解引用，不会fail，得到的a的地址，访问a的内容。即相同context的指针可以替换，漏洞。

<img src="C:\Users\24962\AppData\Roaming\Typora\typora-user-images\1663423389981.png" alt="1663423389981" style="zoom:67%;" />

<img src="C:\Users\24962\AppData\Roaming\Typora\typora-user-images\1663429491065.png" alt="1663429491065" style="zoom: 67%;" />

#### BTI

**BTIs（Branch target instruction）也称为landing pads，使得间接分支(BR和BLR）只能跳转到着陆台（BTI指令）**，着陆台有参数（比如BTI c，BTI j），指明了它可以接受哪些类型的间接跳转指令。原本利用的是库中已有的代码作为跳转目标，但是现在限制必须跳转到BTI指令，那么就只能使用库中有BTI指令的gadget，并且这里BTI还做了细化，必须要特定参数的BTI，找gadget难度增大。

<img src="C:\Users\24962\AppData\Roaming\Typora\typora-user-images\1663423684150.png" alt="1663423684150" style="zoom:67%;" />



Intel 的 SHSTK 和 IBT，编译时在函数开头插入ENDBR指令，每次间接跳转过来之后，要碰到ENDBR才可以继续走，不能限制跳到别的函数开头，但是可以限制跳到不是函数的地方（函数内部）。







#### PAN

**指不让特权态访问非特权态的内存，即不让EL1访问EL0的内存**。背景：用户只能看到和执行自己内存空间内的内容，但是内核往往可以看到和执行所有空间的内容，根据PXN位，可以设置不可执行，但针对某些情况，不仅仅要求不能执行user space的代码，还不能read。例如： 虽然没有办法直接执行userspace的代码，但是可以load userspace中的结构体，进而导致任意代码执行（和rop，jop有何不同：rop和jop是跳到自己可见空间的其它代码部分，此处是内核虽然被限制不能直接执行用户代码，但是它可以加载用户的代码到内核空间，再执行）

<img src="C:\Users\24962\AppData\Roaming\Typora\typora-user-images\1663585936257.png" alt="1663585936257" style="zoom:80%;" />

arm的页表属性：表示用户态和内核的权限RWX，则需要6个bits，为了节省空间，ARM使用4个bits。BIT[54] UXN：为1表示用户态没有执行权限；BIT[53] PXN：为1表示内核态没有执行权限；AP[2] = 1表示内核态是readonly；AP[2] = 0表示内核态是RW；AP[1] = 0表示用户态没有任何权限；AP[1] = 1表示用户态跟内核态权限一样，即看AP[2]。由上述可以知道，只要用户态有权限，内核态的权限就和用户态的权限一样。于是，又在**pstate**中使用一个bit来存PAN，PAN=1时，若是在EL1中，则不允许访问EL0的内存。

![1663585631569](C:\Users\24962\AppData\Roaming\Typora\typora-user-images\1663585631569.png)

开启pan之后，若想要访问用户态的数据，需要copy_from_user,copy_to_user。arm中还定义了指令**LDTR和STTR**，在EL1执行数据的加载和写入时，使用EL0的权限，在其他的except level中，LDTR和STTR指令与普通的LDR和STR相同。

### ARM指令   

PE：指的是 Process Element， 就是逻辑核心(logic core)，一个逻辑核心上可以跑一个线程。这个概念引出是由于现在有很多双线程的处理器(double-thread core)，可以一个核心运行两个完全不同的任务/线程, 一个当两个用，所以不能单单当成一个核了，就说一个核有两个PE。1个PE可以跑1个线程(thread)。PE指的是硬件，线程是跑在PE上的软件。比如说因特尔的i7-4790K，就是4核8线程处理器，每个核有两个线程，也就是有8个PE。

RISC 架构特点：

![1663145909794](C:\Users\24962\AppData\Roaming\Typora\typora-user-images\1663145909794.png)

ARM AArch64执行模式：

![1663145849138](C:\Users\24962\AppData\Roaming\Typora\typora-user-images\1663145849138.png)

ARMv8执行模式：AArch64（对应A64指令集）、AArch32

#### 寄存器：

unbaked register: 31个通用的64位寄存器（X0-X30用于整数运算或者寻址，如果只访问低32位用W0-W30，访问相应32位寄存器，V0-V31用于浮点运算），注意没有W31或X31寄存器，31寄存器一般是栈指针寄存器或零寄存器（视指令而定），如果是栈指针，则指SP，如果是零寄存器，则指WZR（32位）或XZR（64位）

banked register（指一个寄存器不同模式下会对应不同的物理地址）：SPx、ELRx、SPSRx、PC、CPSR，其中，SPSRx是32位寄存器

<img src="C:\Users\24962\AppData\Roaming\Typora\typora-user-images\1663599801125.png" alt="1663599801125" style="zoom:67%;" />

Arm架构中寄存器分为两大类：

##### 系统寄存器

System Register: provide system control or status reporting

+ 13.2 general system control registers: 149
+ 13.3 debug register:30
+ 13.4 performance monitors register:19
+ 13.5 activity monitors registers:15
+ 13.6 statistical profiling extension registers:13
+ 13.7 RAS registers:16
+ 13.8 Generic Timer register:30

##### 通用寄存器，SIMD和浮点寄存器，特殊寄存器

Registers for instruction processing and handling exceptions

+ general-purpose 寄存器：X0-X30/W0-W30 31个。X30也是LR寄存器，用于存储子程序的返回地址。没有X31/W31，一些指令会将31号寄存器编译为zero寄存器，或SP。D1.6
+ SIMD and floating-point寄存器：V0-V31（128-bit）(还可以以下列方式访问这些寄存器：D0-D31（64-bit）,S0-S31（32-bit）,H0-H31（16-bit）,B0-B31（8-bit））。D1.6
+ 特殊寄存器：ELR （exception link register）：保存exception返回地址；SPSR （saved processor state register）： 执行exception前保存当前的processor state, 执行exception完返回时，SPSR会被copied到PSTATE。C5.2

<img src="C:\Users\24962\AppData\Roaming\Typora\typora-user-images\1664174789363.png" alt="1664174789363" style="zoom:80%;" />





<img src="C:\Users\24962\AppData\Roaming\Typora\typora-user-images\1664170029466.png" alt="1664170029466" style="zoom:80%;" />



##### 抽象的PSTATE寄存器

D1.7。分为以下域：

+ condition flags: N,Z,C,V
+ execution state controls: SS,IL,nRW(current execution state,0是AArch64),EL,SP
+ exception mask bits: D,A,I,F
+ access control bits: PAN,UAO,TCO,BTYPE
+ time control bits: DIT
+ speculation control bits: SSBS

可以用以下寄存器访问对应的域，其它的域是不可读写的：

<img src="C:\Users\24962\AppData\Roaming\Typora\typora-user-images\1664179899340.png" alt="1664179899340" style="zoom:80%;" />

用MSR(register)可以将通用寄存器的值存到系统寄存器(包括特殊寄存器)，但MSR(immediate)只能操纵PSTATE的各个域。

## Kernel Module

dmesg 显示信息

dmesg -c 显示信息后清除信息

### Ftrance hook

ftrace hook (functionTracer ) 是linux内核的一个跟踪框架。

**原理：**使用ftrace需要目标Linux操作系统在编译时使用-pg选项和-mfentry选项（将对mcount的调用放到函数的第一条指令处）。此时会在每个非内联内核函数的入口插桩一个对\_mcount函数(或\__fentry__函数，若gcc>=4.6且为x86架构)的调用。mcount函数本身只是一个简单的返回指令，并没有什么实际意义，动态ftrace框架（开启CONFIG_DYNAMIC_FTRACE选项）会在启动时将所有对mcount的调用位置都填充为nop指令（通过scripts/recordmcount.pl脚本实现，所有函数都有个call指令，会严重影响性能），这样一来就在这些内核函数的开头产生了足以容纳一个call指令的空白区。

静态ftrace：frompc为当前函数的起始地址，selfpc为上级函数的起始地址，MCOUNT_INSN_SIZE为call mcount指令长度。

 ![1669172579132](C:\Users\24962\AppData\Roaming\Typora\typora-user-images\1669172579132.png)

动态ftrace： 在编译的时候调用recordmcount.pl搜集所有_mcount()函数的调用点，并且所有的调用点地址保存到section _\_mcount_loc， 在初始化init_ftrace时，遍历section __mcount_loc的调用点地址，默认给所有“bl _mcount”替换成“nop” 

![1669172813084](C:\Users\24962\AppData\Roaming\Typora\typora-user-images\1669172813084.png)

在需要hook时：

+ 被hook函数func()的入口出的nop被替换成了call frace\_caller()/frace\_regs\_caller()，称为一级hook点
+ frace\_caller()/frace\_regs\_caller()函数内的nop被替换为了 call ftrace_ops_no_ops()/ftrace_ops_list_ops() ，称为二级hook点。

+  在ftrace_ops_no_ops()/ftrace_ops_list_ops()函数中会逐个调用执行frace_ops_list链表中函数。 我们自定义的函数需要注册到这个链表中，称为三级链表调用点。

 <img src="https://img-blog.csdnimg.cn/20200718112001145.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3B3bDk5OQ==,size_16,color_FFFFFF,t_70" alt="img" style="zoom:67%;" /> 

ftrace对外接口函数：

+ ftrace\_set_filter_ip()：对需要hook函数的完成上述一、二级步骤，也就是完成了对自定义函数的调用执行
+ register\_ftrace\_function()：将我们自定义的函数加入到frace\_ops\_list中

**实例：**假如我们需要hook掉内核的cmdline\_proc\_show()函数

**1、定义结构体 frace_hook**

```c
struct ftrace_hook {
        const char *name;//被hook函数的函数名 execve()
        void *function;//指向我们新定义的实现监控和修改目的函数 myExecve()
        void *original;//指向该被hook函数地址的指针

        unsigned long address;//被hook函数的地址
        struct ftrace_ops ops;
}
```

**2、frace\_hook结构体的初始化**，需要将结构体的前三项设上：

```C
//新建一个结构体时original值为0（指向void类型的指针大小为8字节，即original占8字节，值为0，此时*original相当于取内存0地址的值，会报segment fault），初始化时，将其赋值为一个数，相当于将该地址由0改为某数，*original就是取内存该数对应地址的值。将其赋值为&a，即将其地址由0改为a的地址，*original的时候得到的是a。
#define HOOK(_name, _function, _original) \
        { \
            .name = (_name), \
            .function = (_function), \
            .original = (_original), \
        }

static struct ftrace_hook hooked_functions[] = {
        HOOK("cmdline_proc_show", fh_cmdline_proc_show, &real_cmdline_proc_show),
};

```

**3、install frace hook**，fh\_install\_hook(struct ftrace\_hook *hook)：

+ 查找被hook函数地址，并备份。resolve\_hook\_address(hook)
  + hook->address = kallsyms_lookup_name(hook->name)
  + *(hook->original) = hook->address，即将 real_cmdline_proc_show 的内容改为被hook函数的地址，后续直接调用 real_cmdline_proc_show 即可调用原函数

+ 初始化ops结构
  + hook->ops.func = fh_ftrace_thunk
  + hook->ops.flag =   FTRACE_OPS_FL_SAVE_REGS | FTRACE_OPS_FL_IPMODIFY; 
+ 使能被hook函数的第一、二级hook点
  + ftrace\_set\_filter_ip(&hook->ops,hook->address,0,0)，倒数第二个0表示使能hook点，若为1表示关闭hook点。
+ 把ops注册到第三级hook点的ftrace\_ops\_list链表中
  + register\_ftrace\_function(&hook->ops)

**4、三级hook调用函数fh\_frace\_thunk()**

使用该函数作为跳板，跳到真正想要执行的函数fh\_cmdline\_proc\_show()。 上述的技巧需要使能`CONFIG_DYNAMIC_FTRACE_WITH_REGS`。

为什么不能直接在三级hook处执行我们的函数：在三级hook处，我们只知道原函数运行的时机，但是拿不到原函数运行的数据。解决：定义一个和原函数参数一致的函数，插入到原函数的原有调用点，即hook层层返回后，用该函数代替原函数执行（函数的参数都在栈上）：

fh_frace_thunk()函数：notrace属性指no instrument function属性，不允许对该函数插入mcount， 可用于标记Linux内核跟踪中禁止使用ftrace的函数 。

```C
static void notrace fh_ftrace_thunk (unsigned long ip, unsigned long parent_ip, struct ftrace_ops *ops, struct pt_regs *regs)
{
    struct ftrace_hook *hook = container_of(ops, struct ftrace_hook, ops);

    /* Skip the function calls from the current module. */
	/* (1) 防止递归 */
    /* (2) 通过修改`ftrace_caller()/ftrace_regs_caller()函数的返回函数来实现hook,原本执行完ftrace hook后返回原函数cmdline_proc_show(),将其替换成新函数fh_cmdline_proc_show()*/
    if (!within_module(parent_ip, THIS_MODULE))
            regs->ip = (unsigned long) hook->function;//regs->ip存的第一级hook函数的返回地址
}
```

fh\_cmdline\_proc_show()：该函数中可以`pre hook`、`调用原函数`、`post hook`。 

```C
/* 定义和原函数参数一致的fh_cmdline_proc_show()函数 */
static int fh_cmdline_proc_show(struct seq_file *m, void *v)
{
    int ret;  
	/* (1) pre hook 点 */
	seq_printf(m, "%s\n", "this has been ftrace hooked");
	/* (2) 调用原函数 */
    ret = real_cmdline_proc_show(m, v);
	/* (3) post hook点 */
    pr_debug("cmdline_proc_show() returns: %ld\n", ret);
	return ret;
}
```

**4.1、config_dynamic_ftrace_with_regs特性** 

若有该特性，则在ftrace_regs_caller()中会把寄存器情况保存到pt_regs中，并逐级传递给fh_ftrace_thunk，用于修改regs->ip，即ftrace_regs_caller的返回地址来插入hook。

```c
ftrace_regs_caller() {
	save pt_regs	
	ftrace_ops_list_func(regs)
	restore pt_regs
}
```

```C
static void ftrace_ops_list_func(unsigned long ip, unsigned long parent_ip,
				 struct ftrace_ops *op, struct pt_regs *regs)
{
	__ftrace_ops_list_func(ip, parent_ip, NULL, regs);//若不支持该特性，则后面两个参数都是NULL
}
```



**5、流程图**，防递归指，在fh_cmdline_proc_show中调用read_cmdline_proc_show时，里面不会再hook。通过判断调用三级hook函数的父亲ip来自kernel还是来自本kernel实现。

 ![img](https://img-blog.csdnimg.cn/20200718112026711.jpg?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3B3bDk5OQ==,size_16,color_FFFFFF,t_70) 



**-pg选项：**

内核编译时采用O2优化，O2优化使能-fipa-ra优化，即若函数a调用了函数b，未开优化时，A需要对caller-save register进行save和restore，B需要对callee-saved register进行save和restore。当开启优化时，a不会保存那些b没有写的寄存器。这样子存在问题：例如我们在函数a执行时修改了函数b的代码，让其代码第一条是一个jump指令，跳转到函数bnew。a跳到b之后，b马上跳到bnew，但是因为编译时，a所看到的她调用的函数b是没有跳转的，且没有使用某寄存器，那该寄存器就不会被保存。bnew中可能修改了该寄存器，返回A之后就会出现结果错误。开启-pg选项之后，会禁用-fipa-ra优化，且如果某函数A调用的函数B在另外一个文件中，那么在C调用A时会保存所有寄存器，因为编译器不知道被调用函数B的情况（调用mcount()函数满足了该情况，每个函数中都插入了mcount函数调用）。

a修改b第一条指令的代码如下：mprotect用于修改一段指定内存区域的保护属性。`` int mprotect(const void *start, size_t len, int prot); ``，start必须按页对齐，len大小是页大小的整数倍（15代表2^15次方）。b代码段第一个字节是0xe9，代表jump指令的opcode。后8字节存偏移地址： 目标地址-当前地址-5 = 偏移地址 。

```c
int main(void)
{
        int x;
        scanf("%d", &x);

        if (mprotect((void*)(((unsigned long)&b) & (~0xFFFF)), 15, 
                             PROT_WRITE | PROT_EXEC | PROT_READ)) {
                perror("mprotect");
                return 1;
        }

        /* 利用 jump 指令将函数 b 替换为 newb 函数 */
        ((char*)b)[0] = 0xe9;
        *(long*)((unsigned long)b + 1) = (unsigned long)&newb
                                         - (unsigned long)&b - 5;
        
        printf("%d", a(x));
        return 0;
}
```





### 编程语言与 linux 内核：

#pragma GCC diagnostic ignored 告诉编译器消除warning（将ignored 改为warning即为开启警告，改为error即为将警告升级为error），“-Wint-conversion”将整数未经强制类型转换就赋值给指针的waring。

struct mm_struct：每个进程/线程队都是task_struct（process descriptor进程描述符）的一个实例，记录该进程所有的context。task_struct中有一个mm_struct（内存描述符memory descriptor），该结构抽象地描述了进程地址空间的信息。

 <img src="https://images0.cnblogs.com/blog/516769/201304/13214813-ece9b1c2abbd4ea8b1de1dd266849b73.png" alt="img" style="zoom:67%;" /> 

struct file 每个打开的文件在内核空间都对应一个struct file，每个进程的PCB中存储了指向文件描述符表的指针，通过文件描述符fd索引该表，得到指向对应文件struct file的指针。

struct file\* get_mm_exe_file(struct mm_struct *mm) 索引到mm对应的可执行文件。

kallsymbols_lookup_name()：用于获取某未exported函数的指针位置，linux内核在5.7.0版本后，该kallsysmbols_lookup_name()函数也不export了（即不可以直接调用，不可以通过函数名索引到该函数），可以通过kprobe获得该函数的起始地址，然后调用它。

+ 调用register_kprobe获得 kallsyms_lookup_name函数的地址kallsyms_lookup_name_func (该变量是一个函数指针，初始化为 unsigned long (*kallsyms_lookup_name_fun)(const char *name) = NULL; ）
+ 调用kallsyms_lookup_name_func获取其它未export的函数（即我们想要知道地址的函数）的地址（直接kallsyms_xx_func("函数名")，函数指针与其它指针不一样，不需要加*）

current：define current get_current()，get_current()返回struct task_sturct*,指向当前进程task_struct的指针。

kmalloc(size_t size, int flags)：flags表示分配内存的类型。kmalloc/vmalloc用于在内核模块中动态开辟内存（利用kfree/vfree释放空间），malloc用于用户空间申请内存：

+  kmalloc申请的是较小的物理地址和虚拟地址都连续的空间。kmalloc和get_free_page最终调用实现是相同的，只不过在调用最终函数时所传的flag不同，且不对获得空间清零。 

+  kzalloc 先是用 kmalloc() 申请空间 , 然后用 memset() 清零来初始化 ,所有申请的元素都被初始化为 0。
+  vmalloc用于申请较大的内存空间，虚拟内存是连续，但是在物理上它们不要求连续。
+  malloc 用于用户空间申请内存，且不对获得空间清零 

file_path()：返回指向path字符串（即该可执行文件的源代码路径，在我的程序中是/home/lqj/Kernel-App/hook-example/tests/hello.spec，对于别的进程来说，可能是/usr/bin/cat等，每个进程都有自己的可执行文件，相应地在系统中有路径）的指针。

```c++
char *file_path(struct file *filp, char *buf, int buflen)
{
	return d_path(&filp->f_path, buf, buflen);
}
d_path: Convert a dentry into an ASCII path name. If the entry has been deleted the string " (deleted)" is appended
```

 current指针指向当前进程的task_struct

char \*strrchr(const char \*str, int c)搜索str所指向字符串中最后一次出现c的位置，未找到则返回空指针。

int strncmp(const char \*str1, const char \*str2, size_t n) 把 str1 和 str2 进行比较，最多比较前 n 个字节。 返回指小于/大于/等于0，则str1</>/=str2

### ELF

一个ELF文件主要由ELF头，程序头表（program header table），节头表（section header table）构成

elf提供两种视图：链接视图（以节section为单位）和执行视图（以段segment为单位）， 当多个可重定向文件最终要整合成一个可执行的文件的时候（链接过程linker），链接器把目标文件中相同的 section 整合成一个segment，在程序运行的时候，方便加载器loader的加载。 

 <img src="https://pic2.zhimg.com/v2-2c33f6155faccf24839b897ee9db10a1_r.jpg" alt="img" style="zoom: 80%;" /> 



<img src="C:\Users\24962\AppData\Roaming\Typora\typora-user-images\1669464286503.png" alt="1669464286503" style="zoom:80%;" />

**ELF 头**：位于ELF的开始，通过``readelf -h``可以读ELF头部信息

```C
typedef struct elf64_hdr {
	 unsigned char	e_ident[EI_NIDENT];	/* Magic number and other info */
	  Elf64_Half	e_type;			/* Object file type */
	  Elf64_Half	e_machine;		/* Architecture */
	  Elf64_Word	e_version;		/* Object file version */
	  Elf64_Addr	e_entry;		/* Entry point virtual address _start函数的地址 */
	  Elf64_Off	e_phoff;		/* Program header table file offset */
	  Elf64_Off	e_shoff;		/* Section header table file offset */
	  Elf64_Word	e_flags;		/* Processor-specific flags */
	  Elf64_Half	e_ehsize;		/* ELF header size in bytes */
	  Elf64_Half	e_phentsize;	/* Program header table entry size */
	  Elf64_Half	e_phnum;		/* Program header table entry count */
	  Elf64_Half	e_shentsize;	/* Section header table entry size */
	  Elf64_Half	e_shnum;		/* Section header table entry count */
	  Elf64_Half	e_shstrndx;		/* Section header string table index */
} Elf64_Ehdr;

```

**程序头表**：执行视图。列举了所有有效的段(segments)和他们的属性。 加载器根据程序头表将文件中的节加载到虚拟内存段中。每一项都是一个``elf64_phdr``结构，``readelf -l`` 命令可以查看一个链接后的elf可执行文件的段信息，以及每个segment对应哪些section的映射关系，即程序头相关信息。program header 中的 p_pvaddr 表示该segment对应的物理地址， 在带有 MMU 的平台上，这个字段不用考虑，输出等于虚拟地址，物理地址的概念只有在uboot或者裸机代码中存在。

**节头表**：链接视图。 一个ELF文件中到底有哪些具体的 sections，由包含在这个ELF文件中的 section head table(SHT)决定。表中每个``elf64_shdr``描述了这节的信息，比如每个节的节名、节的长度、在文件中的偏移、读写权限及节的其它属性。 ``readelf -S xx.o`` 可以读取节信息。

所有节区名都存在.shstrtab字符串表中，``elf64_shdr``中的``sh_name``存的是名字在字符串表中的偏移，段没有段名，只有p_type。

Linkable file格式：

 <img src="https://img-blog.csdnimg.cn/20210515101639285.png" alt="img" style="zoom:67%;" /> 

Executable file 格式：

 <img src="https://img-blog.csdnimg.cn/20210515184446614.png" alt="img" style="zoom:67%;" /> 

内存映像（memory snapshot）：

 <img src="https://img-blog.csdnimg.cn/20210515215709945.png" alt="img" style="zoom: 25%;" /> 

通常elf中有以下节：

+ .text 代码段，可以通过objdump -d反汇编，查看elf代码段的内容
+ .data 初始化了的数据，占用程序空间
+ .bss 未初始化的全局变量和静态局部变量，程序开始时，将这些数据初始化为0，不占用可执行文件空间，仅仅是占位符。
+ .init 进程初始化代码，程序在调用main函数前调用这些代码
+ .rodata 只读数据
+ .comment 版本控制信息
+ .eh_frame / .eh_frame_hdr 生成描述如何unwind堆栈的表
+ .debug 用于符号调试的信息
+ .dynsym 动态链接符号表
+ .shstrtab sh字符串表，存放section名。用elf头的e_shstrndx*e_shentsize可以得到shstrtab header在section header中的位置，由此可以得到shstrtab的位置。
+ .strtab 字符串表，在ELF文件中，会用到很多字符串，比如节名，变量名等。ELF将所有的字符串集中放到一个表里，每一个字符串以’\0’分隔，然后使用字符串在表中的偏移来引用字符串。这样在ELF中引用字符串只需要给出一个数组下标即可 
+ .symtab 符号表， 在链接的过程中需要把多个不同的目标文件合并在一起，不同的目标文件相互之间会引用变量和函数。在链接过程中，我们将函数和变量统称为符号，函数名和变量名就是符号名，它们的符号值就是他们的地址。``readelf -s `` 可以看.symtab的内容
+ .got 全局偏移表
+ .plt 过程链接表
  + .relname 重定位信息，包含了链接器在处理各目标文件时进行重定位的信息，每个需要重定位的段都有自己的重定位表，例如.text节区的重定位区名字为.rel.text。``readelf -r``可以看可重定位信息。

objdump: -d 反汇编目标文件中需要执行指令的section

nm：-a列出所有符号，-g只显示外部符号，-u只显示没有定义的符号





### execve()

 在linux中创建一个新进程，一般是先用fork()从父进程复制一个新的进程空间，然后调用execve()加载新的exe文件，创建新的代码段、数据段、bss、heap、stack、mmap区域。 在这个过程中，就已经切换到新的进程了，退出内核的时候，这个新进程才开始执行。

+ Execve：
  + 根据文件名找到a.out，用它来取代当前进程的内容
  + 以ELF格式装载a.out，读取文件头，如果是静态链接程序，直接执行_start
  + 如果是动态链接程序
    + 读取.interp段，对应动态链接器的名称(ld-linx.so.2)
    + 将控制权交给ld-linux.so.2（在用户空间执行ld-linux.so.2，也是一个elf文件，也需要装载等操作，执行_start），检查并装载共享库，对外部引用进行重定位
    + 退出动态链接器，执行_start，进行初始化（流程为 _start -> _\_libc_start_main -> __libc_csu_init -> _init -> main -> _fini. ）



elf有四种格式ET_REL,ET_EXEC,ET_DYN,ET_CORE，一般为ET_EXEC，因为需要支持随机地址映射ASLR（ 通过随机放置进程关键数据区域的地址空间来防止攻击者能可靠地跳转到内存的特定位置来利用函数 ），可执行文件需要编译成位置无关码，格式为ET_DYN（对应共享目标文件Shared Object File）。

ASLR开启之前进程的用户空间布局：

 <img src="https://img-blog.csdnimg.cn/20201026144113472.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3B3bDk5OQ==,size_16,color_FFFFFF,t_70#pic_center" alt="在这里插入图片描述" style="zoom:80%;" /> 



ASLR开启之后，用户空间布局：text段，heap段的基地址有偏移，mmap段和stack的top有随机偏移

 <img src="https://img-blog.csdnimg.cn/20201026144152591.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3B3bDk5OQ==,size_16,color_FFFFFF,t_70#pic_center" alt="在这里插入图片描述" style="zoom: 80%;" /> 

load_elf时映射：

内核使用包括连续页的VMA（virtual memory area）来识别进程。在每个VMA中可能映射了一个或多个section，每个VMA代表一个ELF文件的segment。 

进程空间的映射，有些是和文件关联的。例如：code、data，以及mmap映射的动态库的code、data。还有些是存储临时数据的，是匿名映射。例如：bss、heap、stack，以及mmap映射动态库的bss。bss较小时，映射时和data放在同一个vma中。load_elf 时 elf一般分为两个 PT_LOAD segment来加载：

+ 第一个segment包含了.text,.init.rodata等段，加载成一个只读可执行的vma
+ 第二个segment包含了 .init_array .fini_array .dynamic .got .data .bss等，除去bss，加载成两个vma，data的vma可读写，剩下的vma只读，bss根据其大小要么和data在一起，要么是另外一个独立的vma。

下面是一个简单程序的readelf -l a.out的分段情况：可以知道第一个LOAD段除了.text, .init. .rodata外还有别的一些小段，.fini, .plt, .rela等。

![1669553724888](C:\Users\24962\AppData\Roaming\Typora\typora-user-images\1669553724888.png)

对于 LOAD 的 segment，因为是直接加载到内存中，所以其必须满足 (virtaddr-offset) % pagesize == 0，这里的 pagesize 也就是 align 的值：2M。

type为PT_LOAD的段： 给出了一个可加载的段,段的大小由 p_filesz 和 p_memsz 描述。文件中的字节被映射到内存段开始处。如果 p_memsz 大于 p_filesz,“剩余”的字节要清零。p_filesz 不能大于 p_memsz。 

<img src="C:\Users\24962\AppData\Roaming\Typora\typora-user-images\1669286594847.png" alt="1669286594847" style="zoom:80%;" />

stack前面的内容也由execve构造好：

 <img src="https://img-blog.csdnimg.cn/20201027193443775.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3B3bDk5OQ==,size_16,color_FFFFFF,t_70#pic_center" alt="在这里插入图片描述" style="zoom: 67%;" /> 



内核中用struct linux_binprm来保存每个可执行文件的信息（可执行文件的路径，运行的参数，环境变量等）

每种可以被注册的可执行文件格式都用一个struct linux_binfmt存储该格式可执行文件的处理函数等，最常见的可执行文件格式就是elf。它对应的linux_binfmt为elf_format

```C
struct linux_binfmt {
    struct list_head lh;
    struct module *module;
    int (*load_binary)(struct linux_binprm *);//通过读存放在可执行文件中的信息为当前进程建立一个新的执行环境
    int (*load_shlib)(struct file *);//用于动态的把一个共享库捆绑到一个已经在运行的进程, 这是由uselib()系统调用激活的
    int (*core_dump)(struct coredump_params *cprm);//在名为core的文件中, 存放当前进程的执行上下文. 这个文件通常是在进程接收到一个缺省操作为”dump”的信号时被创建的, 其格式取决于被执行程序的可执行类型
    unsigned long min_coredump;     /* minimal dump size */
 };

```

```C
static struct linux_binfmt elf_format = {
	.module		= THIS_MODULE,
	.load_binary	= load_elf_binary,
	.load_shlib	= load_elf_library,
	.core_dump	= elf_core_dump,
	.min_coredump	= ELF_EXEC_PAGESIZE,
};
```

调用流程为：do_execveat_common() → bprm_execve() → exec_binprm->search_binary_handler() → fmt->load_elf_binary(struct linux_binprm *bprm) （先填充好binprm，再找到对应的format/handler，用其中的函数处理二进制文件）

load_elf_binary()负责解析elf文件格式，加载PT_LOAD segment到内存，并进行地址空间映射。