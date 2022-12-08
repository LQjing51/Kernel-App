

**1、1028  C6.2.89   ESB：Error Synchronization Barrier**

只有 FEAT_RAS 开启时才有定义，通过读 ID_AA64PFR0_EL1.RAS 知道M1机上开启该extension

```c
SynchronizeErrors();//未实现
AArch64.ESBOperation();//EL相关函数
if PSTATE.EL IN {EL0, EL1} && EL2Enabled() then AArch64.vESBOperation();//EL相关函数
TakeUnmaskedSErrorInterrupts();//未实现
```

**2、1032  C6.2.92   HINT**

涉及各类hint，其中

+ Hint_Yield：Hint_Yield 函数没有给出实现
+ Hint_DGH：Feat_DGH未开启，该op对应指令无定义
+ Hint_WFE/WFI：EL相关
+ Hint_SEV：SendEvent 函数未实现
+ Hint_SEVL：SendEventLocal EL无关
+ Hint_ESB：Feat_RAS开启，行为和ESB行为一样
+ Hint_PSB：Feat_SPE开启，函数未实现
+ Hint_TSB：Feat_TRF未开启，无定义
+ Hint_CSDB：函数未实现
+ Hint_BTI：set Btype_Next EL无关

**3、调用SetExclusiveMonitors()函数，导致可能EL相关：**

1070  C6.2.114   LDAXP 

1072  C6.2.115   LDAXR

1074  C6.2.116   LDAXRB

1075  C6.2.117   LDAXRH

1199  C6.2.174   LDXP

1201  C6.2.175   LDXR

1203  C6.2.176   LDXRB

1204  C6.2.177   LDXRH

**4、调用ExclusiveMonitorsPass()函数，导致可能EL相关：**

1368  C6.2.272  STLXP，Store-Release Exclusive Pair of registers

1371  C6.2.273   STLXR，Store-Release Exclusive Register

1374  C6.2.274   STLXRB

1376  C6.2.275   STLXRH

1438  C6.2.305   STXP

1441  C6.2.306   STXR

1443  C6.2.307   STXRB

1445  C6.2.308   STXRH

**5、WFE相关：**

1512  C6.2.348   WFE

**6、WFI相关：**

1514  C6.2.350   WFI