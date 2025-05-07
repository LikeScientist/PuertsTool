# PuertsTool

# 概述

Puerts插件的基础上实现了一些工具。
# 生成模版使用
## 生成规则（25.05.08）
UE5.4版本开发的插件，在蓝图窗口编译按钮旁会有一个生成模版按钮，鼠标悬浮按钮上会提示："单击生成(检查存在),双击强制覆盖"，Mixin的模版位于插件/Resources/TsMixinTemplate.ts，其中的%%包裹的字段会在插件方法FPuertsToolModule::ProcessTemplate中执行文本替换，从而实现模版根据蓝图来生成。
## 1、新建蓝图
### 2、点击Puerts生成ts定义，将新建蓝图导出
![在这里插入图片描述](https://i-blog.csdnimg.cn/direct/511f34df94a34b83a40b8d78e0eff2f4.png)
### 3、打开蓝图窗口，单击生成(检查存在),双击强制覆盖

单击按钮如果Mixin文件存在则不生成，双击按钮则生成覆盖原来的Mixin文件（如果已经在Mixin中写过代码慎用，因为会被重新生成模版）
![在这里插入图片描述](https://i-blog.csdnimg.cn/direct/79595fdae535427991a0ebb26ecb00c9.png)

点击后窗口右下角会有弹窗提示生成状态：

单击按钮初始生成模版：
![在这里插入图片描述](https://i-blog.csdnimg.cn/direct/7ac06302d9a440a0af81de3ed58e73fe.png)

单击按钮提示文件已经存在，不再创建:
![在这里插入图片描述](https://i-blog.csdnimg.cn/direct/f24cb87ff7714442b8f4a06352f72bbc.png)

双击按钮覆盖Mixin文件重新生成模版：
![在这里插入图片描述](https://i-blog.csdnimg.cn/direct/15679750d2a74661bf03431cd299dacc.png)

### 4、验证模版是否Mixin上
将新建的蓝图运行，我的模版是Actor，所以Actor蓝图直接丢在场景中，会直接运行beginpaly打印日志：
![在这里插入图片描述](https://i-blog.csdnimg.cn/direct/b51f9e294e8c4c7ebb46e892cd919d4f.png)

![在这里插入图片描述](https://i-blog.csdnimg.cn/direct/befff34fcf804946a87579b0ca362dc8.png)

![在这里插入图片描述](https://i-blog.csdnimg.cn/direct/b96e34bbafcf41ab90fc5d361758a74d.png)