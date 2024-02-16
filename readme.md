#### 该仓库用于学习课程 GAMES 102 几何建模与处理基础

课程主页：[GAMES102在线课程-刘利刚 (ustc.edu.cn)](http://staff.ustc.edu.cn/~lgliu/Courses/GAMES102_2020/default.html)

作业计划基于opengl和imgui完成，后面某些难以自己实现的功能可能会借用刘老师推荐的Utopia框架或Unity完成

之前我已经写好的opengl框架链接：[Lim-ss/opengl-study (github.com)](https://github.com/Lim-ss/opengl-study)

##### homework1

做了多项式插值、高斯函数插值、多项式线性回归、多项式岭回归

当点数量较多时，两种插值方法出现线不经过点的情况，怀疑是精度问题导致的。而且插值耗的CPU性能较大，因为要解大规模矩阵的逆。

使用了Eigen

##### homework2

在Python文件夹中的fitting.py文件，使用tensorflow拟合散点，发现当学习率太大时用于让loss越来越大，然后造成nan结果。我目前用的是relu作激活函数而不是RBF。

##### homework3

做了三种参数化的方法，但发现效果不是很好，高次容易过拟合，而且不知道为什么弦长参数化效果反而是最差的，而加入岭回归的正则项之后曲线行为变得很奇怪，暂时没想明白。突然发现作业1和作业3的m_functionShouldUpdate都没起到作用，不过影响不大，bug就先不修了。