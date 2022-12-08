[TOC]

M1芯片 10.3.0198 -p 1234 密码 2001.5.1Lqj

鲲鹏服务器host 10.208.130.12 -p 6543 密码 2001.5.1Lqj

鲲鹏虚拟机 ssh ubuntu@kappvm-xmy 密码123 

+ 要先开虚拟机：先virsh list —all，显示所有的虚拟机，然后“virsh start [虚拟机名称]” 把名字结尾是xmy的那个虚拟机启动
+ 关闭虚拟机：poweroff

funcCall.py：伪代码章节函数分类

alias.py：C6，7指令分类

fineSearch.py：C6 MSR/MRS涉及的寄存器分类

### 手册内容分布

​		**指令**：共1452条

+ C5.3-C5.6 System instruction

+ C6.2 base instruction

+ C7.2 SIMD instruction

  **寄存器**

+ C5.2 Special-purpose register，

+ D13.2-D13.8 System Register

  **函数**

+ J1.2.3 Pseudocode

### **C5 指令分类**：

C5共有指令126条指令

### **MRS/MSR读写系统寄存器指令分类:**

部分指令没有规整的分支，单拎出来，人工分类。
+ MRS:3594（1），3596（1），3598（1），3603（1），3604（1），3605（1），3606（2），3834（2），3854（2），3862（2）
+ MSR:3594（1），3596（1），3598（1），3606（2），3836（2），3854（2），3862（2），4011（3），4014（3），4017（3），4020（3），4023（3），4026（3），4029（3），4038（3），4141（3）
+ IsHighestEL（4011-4141）（认为系统最高的EL为EL2），halted(3836,3854，3862，与debug state相关，默认为false)

注意事项：分开考虑读写；注意有些寄存器有两种访存/存储方式，EL0,EL02，当作两个寄存器(有的后面的寄存器与别的章节的同名寄存器行为一致，不单独考虑)；注意换页；一页有多个MSR/MRS的情况；有些寄存器只有读没有写（D13.2.111），有些只有写没有读（比如D13.3.8），有读的总共299个，有写的总共233个

怎么做：先将EL0,EL1,EL2下所有干活指令找出来（不以if，elsif，else开头，可能为return，AArch64.SystemAccessTrap，undefined，其它。注意碰到分号就要结束，避免扫到页尾字符）。若EL0的干活指令都是undefined或trap，将其归为第一类；若EL0，1，2，相同，归为第二类；EL0，1，2存在不一致将其归为第三类

#### 别名寄存器

见2787页 D5.6 Virtualization Host Extensions。我们的系统开了FEAT_VHE：

支持虚拟化后，一般软件栈为：hypervisor运行在EL2，虚拟机运行在EL0/1。 通常，内核运行在EL1，但虚拟控制在EL2。这意味着大多数host OS运行在EL1，通过EL2的stub代码来访问虚拟化控制。这种安排效率低下，因为它可能涉及增加的上下文切换。 于是我们需要在EL2上运行host os。

<img src="C:\Users\24962\AppData\Roaming\Typora\typora-user-images\1666945795565.png" alt="1666945795565" style="zoom:67%;" />

HCR_EL2.{E2H,TGE}={1,1}时：

EL2下对于部分EL0/1 system register的访问实际上为访问相对应的EL2寄存器：

<img src="C:\Users\24962\AppData\Roaming\Typora\typora-user-images\1666942161276.png" alt="1666942161276" style="zoom:80%;" />

为了实际上还是访问EL0/1的寄存器，EL2下可以通过一些别名寄存器来表示仍要访问EL0/1寄存器：

<img src="C:\Users\24962\AppData\Roaming\Typora\typora-user-images\1666942278566.png" alt="1666942278566" style="zoom: 67%;" />

2793页 D5.7 Nested virtualization。Feat_NV开启时，一个EL2下的host hypervisor可以在EL1下跑一个guest hypervisor。Feat_NV2开启时，PE将system register的访问直接转变为内存的访问。根据ID_AA64MMFR2_EL1.NV可以知道其未开启？（HCR_EL2.{NV,NV1,AT}均为0）。在EL1下MRS/MSR部分寄存器会被改成load/store指令，直接访问内存某块区域。

即对于一个指令章节有多个寄存器读写的情况，需要额外考虑EL12/EL02的，不需要额外考虑EL1的。还发现有额外的EL2等，根据内容来排版，即xxEL2指令某些情况下返回的是EL1的值那该指令就会被放在EL1寄存器章节下，如417 ELR_EL1，内容在别的章节重复了，编码相同，不考虑。

C5.2寄存器 25个；D13.2 149个；D13.3 30个；D13.4 19个；D13.5 15个；D13.6 13个；D13.7 16个；D13.8 30个。总共寄存器 25+149+30+19+15+13+16+30=313个。

#### 第一类：Equal Inst

​	EL0,1,2分支内容完全相同，考虑页眉页脚的影响，先筛查EL0\==EL1\==EL2的，然后看其它的任意两个分支相等的，人为筛查。这里只包括EL0/1/2下有行为且相同的。

#### 第二类：Undefine Inst

​	EL0 undifined/（trap + undefined），不管跑在EL1还是EL2，都无条件改为报错指令；EL0下无定义可能是因为原本就是undefine，或者extension没有开，该指令undefine，或者根据配置情况，EL0下走trap分支。

#### 第三类：Conditional-Undefine Inst

##### 分类标准

​	EL0 有实际行为，且三个EL下的实际行为相同，根据系统配置情况对其进行处理

+ 有些配置下实际行为和EL1和EL2相同且有行为（即EL0==EL1\==EL2），此时不用对该指令进行处理（根据配置情况，直接走），归并到第一类
+ 有些配置下是trap/undifined，改为报错（根据配置情况，直接改），归并到第二类
+ 配置筛选：isHighestEL认为EL2是最高的，haveEL（EL3）认为false，EL2Enable认为true，HaveEL（EL2）认为True，其它的看寄存器配置，halt认为false。
+ 注意：寄存器（例如，CPTR_EL2）不同情况下，有不同的定义，假定部分情况都在前面的条件中判定了，例如426 C5.2.7，但extension没有判定；

##### 问题1：不同权级下，配置寄存器值可能不同

我们在EL2下读取某些寄存器的值来判断该指令它在不同权级下的执行情况，但是寄存器值在不同el下可能会变，甚至可能有不同实现？（切换权级的时候需要更改哪些寄存器值（除了PSTATE.EL）？解决：在EL0下读取这些配置寄存器的值，可能读不出来）

##### 问题2：某些种类寄存器因为Extension未开启，而undefine，解决：归入到第二类（undefine）

###### 1、PMU相关寄存器 结论：D13.4章的PMU相关寄存器均无定义（EL0/1/2下访问均失败）

PMUSERENR(Performance Monitors User Enable Register)

+ PMUSERENR_EL0 只有在有PMUv3的时候才可访问，访问 ID_AA64DFR0_EL1.PMUVer 值为f，指PMU由实现决定，PMUv3不支持。访问PMUSERENR_EL0，insmod时会segment fault。
  + 该寄存器的EN域为0时表示trap EL0下对某些PM寄存器的访问到EL1 或EL2（当EL2 enable 且 HCR_EL2.TGE=1）。
  + 该寄存器的CR域为0时表示trap EL0下对于cycle counter reads（PMCCNTR_EL0）到EL1或 EL2（为0时，还要求EN=0才可以trap，为1时，不管EN，允许其访问）。
  + 该寄存器的ER域为0表示trap EL0下对于event counter read的访问，限制 PMXEVCNTR_EL0，PMEVCNTR<n>_EL0的读，限制PMSELR_EL0的读写（和EN合作，为1时override EN，允许访问）
+ 所有条件中涉及这个寄存器的寄存器都只有在PMU开启时才有定义，EL0下访问时illegal instruction，EL2下访问insmod时报错 segment fault。

###### 2、AMU相关寄存器 结论：D13.5章的AMU相关寄存器均无定义

AMUSERENR(Activity Monitors User Enable Register)

+ AMUSERENR_EL0只有在有FEAT_AMUv1的时候才可访问，访问ID_AA64PFR0_EL1.AMU，值为0，AMU未开启。
+ 所有条件中涉及这个寄存器的寄存器都只有在AMU开启时才有定义，EL0下访问时illegal instruction，EL2下访问insmod时报错 segment fault。

AMCR_EL0(Activity Monitors Control Register)

+ AMCR_EL0只有在AMU开启时，才有定义。条件中涉及这个寄存器的寄存器也只有在AMU开启时才有定义。

###### 3、SPE相关寄存器 结论：D13.6章SPE相关寄存器均无定义

SPE(Statistical Profiling Extension)

##### 问题3：某些有定义寄存器的分支条件中存在undefine寄存器

###### 1、FGT相关寄存器 结论：认为其全0，行为符合未开启该扩展的行为

这些寄存器这些位表示EL0/1下访问某系统寄存器时，将其trap到EL2下。默认（3399页）HFGRTR_EL2为全0，不执行这些trap。

tips：在E2H TGE(trap general exception)均为1时，只有EL0和EL2，此时这些寄存器值影响不了EL0/2的行为。因为此时不需要细粒度的trap，EL0下所有应该trap到EL1的exception都被trap到了EL2。FGT存在的意义在于有EL1时，限制EL0/1下执行某些MSR/MRS/指令，并且让其trap到hypervisor，可以影响EL1的行为。

+ HFGRTR_EL2（Hypervisor Fine-Grained Read Trap Register，provide controls for traps of MRS/MRC(32位) reads of System register）：
  + 3138等MRS指令分支条件中需要访问HFGRTR_EL2寄存器，但是HFGRTR_EL2寄存器只有在实现了FEAT_FGT 时有定义，EL2下访问编译报错（编译器报错信息为：处理器不支持该系统寄存器名）。该MRS指令在鲲鹏服务器虚拟机（也没有实现FGT）中EL1下可以正常执行，认为走else分支，即HFGRTR_EL2.CTR_EL0为0。3143情况相同，EL1下正常执行，即HFGRTR_EL2.DCZID_EL0为0。3671，3741，3749情况相同。
+ HFGWTR_EL2（Hypervisor Fine-Grained Write Trap Register）：
+ HDFGRTR_EL2（Hypervisor Debug Fine-Grained Read Trap Register，MRS/MRC reads of debug, trace, PMU, and Statistical Profiling System registers.）：
+ HDFGWTR_EL2 （Hypervisor Debug Fine-Grained Write Trap Register）：
+ HFGITR_EL2 （Hypervisor Fine-Grained Instruction Trap Register）：
+ HAFGRTR_EL2 （Hypervisor Activity Monitors Fine-Grained Read Trap Register）

#### 第四类：BehaviorDiff Inst

EL0,1,2实际行为存在不一样，需要模拟。差别在于不同权级下可以返回不同权级的寄存器，例如：EL0和EL2下可以返回EL2/EL0，EL1下可以是返回NVM或者EL0。或者有extension，返回值不同，4214。

根据系统配置情况，对其进行细粒度划分，部分指令可以归为第1，2类：认为Feat_Sel2没有开启，

##### MRS：

1. 4202 D13.8.16   CNTP_CTL_EL0, Counter-timer Physical Timer Control register：归为Undefine Inst

   ```c
   EL0: 满足EL2Enabled() && HCR_EL2.<E2H,TGE> == '11' && CNTHCTL_EL2.EL0PTEN == '0' , 走trap分支
   EL1: CNTHCTL_EL2.EL1PTEN == '0'不满足, return CNTP_CTL_EL0
   EL2: SCR_EL3 Undefine，return CNTP_CTL_EL0
   ```

   CNTHCTL_EL2寄存器用于``Controls the generation of an event stream from the physical counter, and access from EL1 to the physical counter and the EL1 physical timer``，EL0PTEN：当HCR_EL2.TGE为1的时候，该域为0，host app，EL0下对于相关CNTP寄存器的访问会被trap到EL2；HCR_EL2.TGE为0的时候，该域为1，guets app，不trap指令。EL1PTEN：当TGE为0的时候，该域为0，trap EL0和EL1对于相关CNTP寄存器的访问到EL2，TGE为1的时候，该域为1，不trap。
   SCR_EL3.NS指no secure state

   归到第四类时因为EL1下有返回NVMem的情况，FEAT_NV，extension未开启，不会走该分支

2. 4206：与上相同

3.  4210：与上相同

4.  4214 D13.8.19   CNTPCTSS_EL0, Counter-timer Self-Synchronized Physical Count register：看EL0PCTEN（0）和EL1PCTEN（0），FEAT_VCE未开启，EL0,EL1均trap，EL2 return

5.  4216 与上结果相同

6.  4227 D13.8.25   CNTV_CTL_EL0, Counter-timer Virtual Timer Control register：看EL0VTEN（0）和EL1TVT（0）：EL0 trap，EL1/2下均返回CNTV_CTL_EL0

7.  4231：与上相同

8.  4235：与上相同

##### MSR:

1. 4202，4206，4210，4227，4231，4235六条指令与上情况相同，均为EL0 trap，EL1/2write。

### **C6，7指令筛查：**

+ C6共354条指令，C7共404条指令，共758条指令

+ 先找出所有的指令别名情况：指令的Operation（“Operation for all encodings”）：The description of xxxx gives the operational pseudocode for this instruction，记录该假指令与真指令的对应关系。指令A的某个域可变，在某些情况下，可以通过它的别名指令充当这个指令，例如C7.2.371 UMOV 的域 imm5可变，imm5 = 'xxx00'时，该指令等价于MOV（to general）指令。

+ 将所有真指令分类：
    + 对于指令operation/decode中有EL相关的，直接分类。
      
      + operation（“Operation for all encodings”，例1016 DSB）中读写系统寄存器
        + 注意除去那些已经筛查出来在EL0,1,2下行为完全一样的，
        + 分开读写，写：寄存后接“ = ”，否则为读
        + PSTATE各个域的访问情况，与寄存器不同，比如PSTATE.<A,I,F>；PSTATE还有域IL,nRW,BTYPE，认为它们读写也均与EL相关
        + DBGBCR<n>_EL1这种寄存器在指令中会以DBGBCR_EL1[n/xx]形式访问，改为DBGBCR_EL1[
      
    + 看指令的指令operation/decode中调用的函数是否被标记（EL相关）。
      + 先找所有原本就有EL相关的函数，将其标记上，看剩下的函数是否（多级）调用这些被标记的函数。共931（402（24+57+165+77+79）+529（69+4+409+5+42））个函数.
        
        ```c++
        tips：
        	注意章节之间的切换时的context终止位置，注意不要读入目录，注意匹配title的时候还有/和-。
            找函数时，注意前面不能是字母，后面匹配[]或()(有些只能匹配[]，有些只能匹配()，没有处理，有些函数是type结构，后面没有()/[]，不会匹配)；不匹配注释中的内容；不匹配".",因为AArch64.AddrTop和AddrTop不同，但是存在例外，比如AArch64.BranchTargetException和BranchTargetException相同，人工处理；
        ```
        
        + 删去V，VL，isSVEEnable函数，UsingAArch32函数（认为读nRW域和EL无关）
        
      + 再看指令调用上述函数的情况 568项：
      
        + CheckFPAdvSIMDEnabled64（312项），AArch64.CheckFPAdvSIMDEnabled（31项），CheckFPEnabled64（53项）：最终调用AArch64.CheckFPEnabled()，EL2下一定trap；EL0,EL2的isInHost是true，EL1是false，EL2下走trap；EL1下正常返回。实操指令验证？
        + SP 232项（EL2下内核app使用的是SP_EL0，认为SP返回值不变）
        + Mem 185项 （因为BigEndian）
        + CheckSPAlignment 152项（认为check都成功，sp！=Align(sp，16)不满足？一定不满足吗，若可能满足需要看SCTLR相关寄存器）
        + MemAtomic 27项（因为BigEndian）
        + BigEndian 14项（需要看SCTLR相关寄存器）
        + AArch64.SetExclusiveMonitors 8项
        + AArch64.ExclusiveMonitorsPass 8项
        + AArch64.MemTag 8项
        + BranchTo 7项
        + AArch64 Abort 5项
        + MemAtomicCompareAndSwap 4项（因为BigEndian）
      
    + 对于剩下的仍然认为和EL相关的真指令，人工筛查。[见后文](#C6.C7 EL相关指令)
    
+ 对别名指令（假指令）进行分类，共92条指令是别的指令的别名。

+ 未实现的函数相关：总共有64条NonImplement的函数，删去其中RoundUp，RoundDown和physMemRead之后，相关的指令总共有594条（C6 187条，C7 404条）。指令调用的NonImple函数中最多的是V（383条），SetTagCheckedInstruction（165条），BracnTo（13条）

    + 调用的函数有：SetTagCheckedInstruc（159条），V（383条）
    + V不再考虑，通过查看 ID_AA64PFR0_EL1 的SVE域，认为FEAT_SVE未开启，采用默认的128，后面的ConstraintUnpredictableBool，无需关注，和EL无关。
    + SetTagCheckedInstruction 未实现。有的调用它的条件是HaveMTE2Ext()，通过查看 ID_AA64PFR1_EL1 的MTE域，认为MTE未开启，条件不满足；有的没条件，（1323，1351，1447），保留。

**更新：**originEL相关，只看是否有PSTATE.EL字样。对于内容中出现的el，target_el等字样，一定是通过某个函数调用（在调用的函数中会出现PSTATE.EL，没有出现的情况，可能是HaveEL等，涉及的是系统配置，不是当前EL状态），或者传入的参数得到的（在原本的指令处，会将PSTATE.EL当作参数）。

C5 sysytem instruction 126条指令 

+ 106条 Undefine Inst
+ 13条 Conditional Undefine Inst
+ 7条 Conditional Equal Inst

MRS

+ 238条 Undefine Inst
+ 74条 Conditional Undefine Inst
+ 3条 Equal Inst
+ 5条 Conditional Equal Inst

MSR

+ 181条 Undefine Inst
+ 58条 Conditional Undefine Inst
+ 3条 Equal Inst
+ 3条 Conditional Equal Inst

C6 base instruction & C7  Advanced SIMD and Floating-point Instruction

+ Equal Inst 172条

+ EL相关的 585+1条

  + operation callELfunction的有585条

    operation 有 PSTATE.EL 的有8条（有一条不在callELfunction中）: 原来的note中都考虑了

    decode 部分没有callELfunction的

    decode 部分 PSTATE.EL 的有12条（都在callELfunction中）

  + 认为下列函数EL无关

    + SP EL无关：在EL2下和EL0下均返回SP_EL0，无条件EL无关
    + V，Vpart无关：读写 SIMD&FP register，SVE未开启。
    + BranchTo 无关：
    + CheckFPAdvSIMDEnabled64，CheckFPAdvSIMDEnabled，CheckFPEnabled64，CheckFPEnabled无关
    + CheckSPAlignment 无关
    + BigEndian，Mem，MemAtomic，MemAtomicCompareAndSwap 无关
    + AArch64.SetExclusiveMonitors，AArch64.ExclusiveMonitorsPass无关
    + AArch64.MemTag 无关
    + AArch64 Abort 无关
    + 剩下的指令筛查之后，四条EL0下 undefine，若干条待定





#### C6.C7 EL相关指令

4条 EL0下undefine（或部分条件下undefine，如MSR）

先暂时将未实现的指令当作EL无关的，之后另外考虑。

**887 ADDG：add with tag，将立即数加到源寄存器中地址的tag上，将结果放在目的寄存器中**

EL相关原因：访问了``GCR_EL1.Exclude``,调用了EL相关函数:``AArch64.AllocationTagAccessIsEnabled``,``AArch64.AddressWithAllocationTag``。

解决：FEAT_MTE开启时才有定义：MTE（Memory Tagging Extension）通过给内存地址设tag的方式实时检查内存访问错误。该指令无定义。

**913  C6.2.20   AUTDA, AUTDZA，Authenticate Data address using key A**

只调用AuthDA，认为EL无关

**914  C6.2.21   AUTDB, AUTDZB**

只调用AuthDB，认为EL无关

**915  C6.2.22   AUTIA, AUTIA1716, AUTIASP, AUTIAZ, AUTIZA，Authenticate Instruction address, using key A**

只调用AuthIA，认为EL无关

**917  C6.2.23   AUTIB, AUTIB1716, AUTIBSP, AUTIBZ, AUTIZB**

只调用AuthIB，认为EL无关

**920  C6.2.25   B.cond，直接有条件（cond）跳转到PC+imm的一个label处**

调用函数ConditionHolds（只访问PSTATE.NZCV，EL无关）和BranchTo

**921  C6.2.26   B，直接无条件跳转到PC+imm的一个label处**

调用BranchTo

**934  C6.2.33   BL，branch with link to a PC-relative offset，直接跳转到PC+imm处，同时将X30写为PC+4**

调用BranchTo

**935 BLR：Branch with Link to Register，间接跳转到register中的存的地址处，同时将X30写为PC+4**

EL相关原因：origin EL相关因为注释内容（PSTATE.BTYPE，与BTI相关），此外还调用了``BranchTo``,类别为BranchType_INDCALL(Indirect Branch with link, link指将PC+4写到寄存器中，指令中存的是寄存器地址或者内存地址，读出来相应的值再跳过去)。

解决：BranchTo EL无关

``S1TranslationRegime``：EL0：return EL2；EL1/2：return EL1/2; 

``HavePACExt``:等于HasArchVersion(Armv8p3)，组内机器上是v8p4，认为返回true

``EffectiveTBI``:TBI指Top Byte Ingore，虚地址的高8位用作标记。需要看寄存器和VA[55]，结果为1或者0。TCR_EL1.TBI1/TBI0和TCR_EL2.TBI1/TBI0，EL1和EL2的寄存器相同，和EL无关，va[55]为1则为返回0，为0则返回1。

``AddrTop``: return the MSB number(最高有效位) of virtual address in the stage 1 translation regime for el.若EffectiveTBI结果为1，则返回55，否则返回63

``BranchAddr``:按照AddrTop返回的最高有效位对va进行截取并补齐（符号位扩展，实际上都是补0，因为AddrTop返回55时，va[55]为0）

``BranchTo``:根据BranchAddr的结果跳转过去

**936 C6.2.35   BLRAA, BLRAAZ, BLRAB, BLRABZ：Branch with Link to Register,with point authentication，有PAC，用key检验将要跳转过去的地址**

EL相关原因：注释部分有PSTATE.BTYPE，还调用了target = ``AuthIA``或``AuthIB``，``BranchTo``

解决：PAC会先调用``ADDPACIA/B``将地址高位加上签名，然后需要使用这个地址的时候（寄存器内容作为branch地址），调用``AuthIA/B``对其进行验证，验证成功，将pointer authenticantion code field bits（若开启了TBI，则从次高字节开始）全部替换为va[55]（extension），并返回新的地址。若验证失败，将field的第二，三高位设置为一个unique的error码（表示不是extension），并返回，在后续地址翻译的时候报错。``AuthPACIA/B``和``AuthIA/B``中均会先对``SCTLR_EL1/2.EnIA/B``做判断（表示使用key A/B），enable均为0，不进行后续操作，直接返回源target，相当于未开启PAC，不设置pac域。

机器上暂时关闭了pac，认为pac EL无关

**938 C6.2.36   BR：Branch to Register**

EL相关原因：涉及BTYPENext赋值，BrachTo

**939  C6.2.37   BRAA, BRAAZ, BRAB, BRABZ：Branch to Register，with pointer authentication**

解决：BType，branchTo，AuthIA/B

**941  C6.2.38   BRK：Breakpoint instruction**

调用AArch64.SoftwareBreakpoint(imm16);

TGE 为1时EL0/EL1/EL2最后都调用 AArch64.TakeException(EL2, exception, preferred_exception_return, vect_offset) 参数内容都相同

**942  C6.2.39   BTI：Branch Target Identification，用于阻止那些不是intended target of a branch 的指令的执行，阻止它们跳转到guarded memory pages**

EL相关原因：op的别的case有PSTATE.EL字样

解决：FEAT_BTI未实现，v8p5才有BTI extension，不会有该条指令。BTI类似于intel 的 IBT。

间接跳转BR,BLR的指令末尾，会根据指令类型，跳转前的内存区域，间接跳转用的寄存器的类型，把PSTATE.BTYPE设置为一个非零值（其它指令，会将其设置为00）。当下一条指令处于guarded memory region 并且 PSTATE.BTYPE ！=0b00时，若这条指令不是兼容的BTI或者某些条件下的PACIASP,PACIBSP，Breakpoint Instruction exception，Halt Instruction debug event，则一定会报Branch Target exception错误。

即在每个BB块前要插桩插入BTI指令，BTI的target参数由跳转来的jump指令类型决定。

若这条BTI指令target type和当前的BType相兼容，则可以继续执行，否则报Branch Target Exception。BTI的操作数op2\<2:1\>表明了期望的间接跳转的type，有(omiited),c,j,jc四种，用于兼容性判断。SystemHintop会被设置为SystemHintOp_BTI，在此op下，将BtypeNext设为00。

**954  C6.2.44   CBNZ**

BranchTo

**955  C6.2.45   CBZ**

BranchTo

**1009  C6.2.76   DCPS1 Debug change PE state to EL1** 

External Debug相关，！Halted（），则undefine，平常执行时都是undefine的，在debug state情况下，调用DCPSInstruction。EL0 下 undefine

<img src="C:\Users\24962\AppData\Roaming\Typora\typora-user-images\1667915436790.png" alt="1667915436790" style="zoom:80%;" />

**1010  C6.2.77   DCPS2** 

External Debug相关，！Halted（），则undefine （认为一直不会处于debug state，是否合理，hlt指令EL0下不可执行，）

**1011  C6.2.78   DCPS3** 

External Debug相关，！Halted（），则undefine

**1015  C6.2.81   DRPS：Debug restore process state** 

decode部分说明 ！Halted（），则undefine，且EL0下无定义

**1016  C6.2.82   DSB：Data Synchronization Barrier**

EL相关原因：涉及FEAT_XS的有PSTATE.EL字样，还调用了``DataSynchronizationBarrier``未实现函数或``SpeculativeStoreBypassBarrierToVA()``，``SpeculativeStoreBypassBarrierToPA()``

FEAT_XS，memory引入一个XS属性，指明a access could take a long time to complete，由此给DSB和TLB指令引入nXS变量，用于处理有XS属性的内存访问。v8p7开始支持，未实现。

解决：Feat_XS未实现

**1026  C6.2.87   ERET** ++

decode部分说明 EL0下无定义

**1027  C6.2.88   ERETAA, ERETAB**++

decode部分说明 EL0下无定义

**1028  C6.2.89   ESB：Error Synchronization Barrier** ？？

只有 FEAT_RAS 开启时才有定义，平台开启该extension。RAS （Reliability, Availability, and Serviceability） RAS 框架允许平台为同步外部中止 (SEA)、异步外部中止（也记为SError）、fault handing 和 error recovery interrupts 等错误导致的异常路由到EL3下进行处理 。涉及RAS寄存器，例如 4092 DISR_EL1, Records that an SError interrupt has been consumed by an ESB instruction。

operation：

+ 无实现函数SynchronizeErrors，同步各个错误

+ ESBOperation：

  ``route_to_el2  = 1``，``target = EL2``，``当处于EL0/EL2时，mask_active为1``，``mask_set = PSTATE.A == 1``，``intdis = 0/1?`` ， ``masked = ?``

  PSTATE.A 异步异常mask：A：SError interrupt mask bit 

**1032  C6.2.92   HINT **？？

有包括 Hint_Yield()，Hint_DGH()，Hint_WFE/I，SendEvent，ESB，PSB在内的各种Hint。

Hint_Yield：未实现 Provides a hint that the task performed by a thread is of low importance so that it could yield to improve overall performance.

Hint_DGH： 未开启

Hint_WFE/WFI：

+ EL0下trap到EL1
+ EL1下走最后的waitForEvent/Interrupt
+ EL2下走最后的waitForEvent/Interrupt

Hint_SEV/SEVL：SendEvent未实现，SendEventLocal EL无关 

Hint_ESB：Feat_RAS，和ESB行为一样

Hint_PSB：Feat_SPE，ID_AA64DFR0_EL1.PMSVer

Hint_TSB：Feat_TRF，ID_AA64DFR0_EL1.TraceFilt

Hint_CSDB：未实现

Hint_BTI：set Btype Next EL无关

**1034  C6.2.93   HLT Halt Instruction** ++

EL0 下undefine 不可执行

产生一个 Halt Instruction debug event，导致进入 debug state

看能不能halt，可以的话调用Halt(DebugHalt_HaltInstruction);

**1035  C6.2.94   HVC **

描述：Hypervisor Call causes an exception to EL2. Software executing at EL1 can use this instruction to call the hypervisor to request a service

因为 HCR_EL2.HCD ==0(HVC instruction disable)，该指令为均为undefine

**1037  C6.2.96   IRG**

描述：Insert Random Tag, MTE未开启，未定义

**1040  C6.2.98   LD64B， Single-copy Atomic 64-byte**

FEAT_LS64开启才有定义，v8p7才有，该指令无定义

**1070  C6.2.114   LDAXP** ？？

调用AArch64.SetExclusiveMonitors（address，dbytes）调用 AArch64.CheckAlignment和AArch64.TranslateAddress EL相关，待检查

可能调用AArch64.Abort：其中调用的各类Abort/Exception EL0/1/2下均AArch64.TakeException(EL2, exception, preferred_exception_return, vect_offset) 参数内容都相同。

**1072  C6.2.115   LDAXR？**

调用AArch64.SetExclusiveMonitors（address，dbytes）

**1074  C6.2.116   LDAXRB**？

调用AArch64.SetExclusiveMonitors（address，1）

**1075  C6.2.117   LDAXRH**？

调用AArch64.SetExclusiveMonitors（address，2）

**1090  C6.2.124   LDG**

load an allocation tag from a memory address，MTE未开启，无定义

**1091  C6.2.125   LDGM**

load tag multiple，MTE未开启，无定义

**1113  C6.2.135   LDRAA, LDRAB，load register with pointer authentication**

LDR：根据base register value 和 offset register value 算出地址，从这个地址load数

AuthDA，AuthDB PAC相关操作，认为和EL无关

**1199  C6.2.174   LDXP** ？

调用 AArch64.Abort(EL无关) ，AArch64.SetExclusiveMonitors

**1201  C6.2.175   LDXR** ？

 AArch64.SetExclusiveMonitors

**1203  C6.2.176   LDXRB** ？

 AArch64.SetExclusiveMonitors(

**1204  C6.2.177   LDXRH** ？

 AArch64.SetExclusiveMonitors

**1237  C6.2.195   MSR (immediate)**  ++ 

MSR(immediate) 将imm数写到PSTATE中各个域，会在encoding部分检查权限和extension，没有extension的时候，不管EL，全部undifine；若权限下不可以写，trap 或者 undifine

EL0下，若op1不是011，则undefine，且当op1是011，且op2是110/111时，也undefine

EL1下，若op1是100/101/110/111，则undefine，Feat_SEL2没有开(Secure EL2，看 ID_AA64PFR0_EL1.SEL2发现没有开 )

EL2下，若op1是110/111，则undefine

MSR(register) 将general-purpose 的寄存器内容写到system register

**1261  C6.2.208   PACDA, PACDZA，Pointer Authentication Code for Data address**

调用AddPACDA

**1262  C6.2.209   PACDB, PACDZB**

调用AddPACDB

**1263  C6.2.210   PACGA，using generic key**

调用AddPACGA

**1264  C6.2.211   PACIA, PACIA1716, PACIASP, PACIAZ, PACIZA，for Instruction address**

调用AddPACIA

**1267  C6.2.212   PACIB, PACIB1716, PACIBSP, PACIBZ, PACIZB**

调用AddPACIB

**1282  C6.2.220   RET**

BranchTo 和 BtypeNext

**1283  C6.2.221   RETAA, RETAB**

BranchTo，BtypeNext，AuthIA/B

**1316  C6.2.241   SMC，secure Monitor Call causes an exception to EL3**

EL0下undefine，EL1/2也下undefine？没有EL3，认为它在EL0/1/2下都被被拦截

**1323  C6.2.247   ST2G**

MTE未实现，无定义

**1325  C6.2.248   ST64B**

FEAT_LS64开启才有定义，v8p7才有，该指令无定义

**1326  C6.2.249   ST64BV**

FEAT_LS64开启才有定义，v8p7才有，该指令无定义

**1328  C6.2.250   ST64BV0**

FEAT_LS64开启才有定义，v8p7才有，该指令无定义

**1348  C6.2.260   STG，store allocation tag**

MTE未开

**1350  C6.2.261   STGM，Store Tag Multiple**

MTE未开启，未定义

**1351  C6.2.262   STGP，Store Allocation Tag and Pair of registers**

MTE未开启，未定义

**1368  C6.2.272   STLXP，Store-Release Exclusive Pair of registers**？？

描述：stores two 32-bit words or two 64-bit doublewords to a memory location
if the PE has exclusive access to the memory address, from two registers, and returns a status value of 0 if the store was successful, or of 1 if no store was performed.

调用AArch64.ExclusiveMonitorsPass

**1371  C6.2.273   STLXR，Store-Release Exclusive Register**

调用AArch64.ExclusiveMonitorsPass

**1374  C6.2.274   STLXRB**

调用AArch64.ExclusiveMonitorsPass

**1376  C6.2.275   STLXRH**

调用AArch64.ExclusiveMonitorsPass

**1438  C6.2.305   STXP**

调用AArch64.ExclusiveMonitorsPass

**1441  C6.2.306   STXR**

调用AArch64.ExclusiveMonitorsPass

**1443  C6.2.307   STXRB**

调用AArch64.ExclusiveMonitorsPass

**1445  C6.2.308   STXRH**

调用AArch64.ExclusiveMonitorsPass

**1447  C6.2.309   STZ2G**

MTE未实现，无定义

**1449  C6.2.310   STZG**

MTE未实现，无定义

**1451  C6.2.311   STZGM**

store tag and zero ，MTE未开启，未定义

**1459  C6.2.315   SUBG**

subtract with tag，MTE未开启，未定义

**1470  C6.2.321   SVC，Supervisor Call causes an exception to be taken to EL1**  不需要处理

调用 AArch64.CheckForSVCTrap(imm16) 先看HaveFGTExt （Fine Grained Trap），未实现，直接返回，调用AArch64.CallSupervisor(imm16); 行为不一致：EL0/EL2下都调用AArch64.TakeException(EL2, exception, preferred_exception_return, vect_offset)，EL1下调用的参数是EL1。是否还考虑EL1？

**1485  C6.2.330   TBNZ**

BranchTo

**1486  C6.2.331   TBZ**

BranchTo

**1512  C6.2.348   WFE** ？？

Hint_WFE 0/2下行为不一致

**1513  C6.2.349   WFET**

Feat_WFxT未实现，无定义

**1514  C6.2.350   WFI** ？？

Hint_WFI 0/2下行为不一致

**1515  C6.2.351   WFIT**

Feat_WFxT未实现，无定义

**1517  C6.2.353   XPACD, XPACI, XPACLRI**

Strip Pointer Authentication Code. This instruction removes the pointer authentication code from an address

调用 Strip，认为和EL无关。

### **指令编码**

![1668491782930](C:\Users\24962\AppData\Roaming\Typora\typora-user-images\1668491782930.png)

#### **C7 指令:**  

**1、 datat processing--scalar floating-point and SIMD所有**

**2、loads and stores下的SIMD部分：下图op1=1**

<img src="C:\Users\24962\AppData\Roaming\Typora\typora-user-images\1668749798625.png" alt="1668749798625" style="zoom:80%;" />

#### **C6指令：**

##### 1、Data processing--immidiate 所有

##### 2、Branches，Exception Generating and System instructions的大部分：除了System instruction/register move部分

**（1）从conditional branch(imm)到exception generation:**

**（2）从System instructions with register argument到PSTATE：**

均满足 op0 == 0b00 （C5.1.3）：CRn区分种类

![1668511133597](C:\Users\24962\AppData\Roaming\Typora\typora-user-images\1668511133597.png)

+ hint：YIELD,WFE,PACIA,BTI等

+ Barriers and CLREX

+ MSR（imm)，access the PSTATE fields

**（3）从 Unconditional branch（register）到Test and branch（imm）：**

##### 3、loads and stores除了SIMD的部分：下图op1=0

<img src="C:\Users\24962\AppData\Roaming\Typora\typora-user-images\1668749731000.png" alt="1668749731000" style="zoom:80%;" />

##### 4、data processing--register所有

#### **C5指令：**

##### 1、Branches，Exception Generating and System instructions 中的System instructions

+ C5.1.4 op0==0b01：cache maintenance, TLB maintenance, and address translation instructions，prediction restriction instructions

  ![1668507675095](C:\Users\24962\AppData\Roaming\Typora\typora-user-images\1668507675095.png)

  满足该格式的还有C6的指令 1482 sys，Operation为	

  ```c
  AArch64.SysInstr(1, sys_op1, sys_crn, sys_crm, sys_op2, X[t]);
  ```

  实际上就是类似于MSR/MRS，C6的sys指令给定编码之后就是C5的指令。

+ op0==0b01：且L位为1，此时为1484 SYSL指令（system instruction with result），Operation也是执行C5的指令，按照C5的情况对其进行trap。

  ````c
  X[t] = AArch64.SysInstrWithResult(1, sys_op1, sys_crn, sys_crm, sys_op2);
  ````

1010-1011-1100-1101-1110

0100-0101

#### MSR/MRS指令：

##### 1、Branches，Exception Generating and System instructions 中的 System register move（MSR（register）/MRS）、

L=1表示读，L=0表示写

+ D12.2 op0==0b10，用于访问debug system register（D13.3中部分）

![1668509181749](C:\Users\24962\AppData\Roaming\Typora\typora-user-images\1668509181749.png)

+ D12.3 op0==0b11 MSR/MRS: 用于访问所有的Non-debug register（13.2，13.4，13.5，13.6，13.7，13.8），special-purpose register和部分D13.3debug寄存器。CRn区分种类，对于sepcial-register CRn=4

![1668504565684](C:\Users\24962\AppData\Roaming\Typora\typora-user-images\1668504565684.png)



![1670340506117](C:\Users\24962\AppData\Roaming\Typora\typora-user-images\1670340506117.png)

#### 编码分类总结：

**system Inst：1101010100x01**：1101010100

**MRS/MSR：    1101010100x1**

**Base Inst：不满足上述编码的情况**

| C7     | 27-25：111，27-25：110                             |
| ------ | -------------------------------------------------- |
| **C6** | **28-25：100x**                                    |
|        | **28-25：101x，且 !=1101010100x1,!=1101010100x01** |
|        | **27-25：100**                                     |
|        | **27-25：101**                                     |

#### 哈希算法：

基于range的筛查方法，若不区分两类指令，即使最优，对于每一条指令也至少需要比较6次：system inst（上range界+下range界），MRS/MSR（上+下），base inst（上+下）。

基于哈希的筛查方法，第一次比较可以得出落在第一类还是还是第二类，然后进行hash计算（位运算），判断是否在阴影部分，若在还需要进行hash varify（一次比较，没有hash冲突），若不在，则不需要varify。总共比较两次，hash计算一次。

针对第一类的21条待筛查指令，hash算法为：-（inst[21:12] ^~ inst[11:5]），21条指令的哈希结果均不同，hash表大小大约4K，但是仅访问其中不到100B的内容。