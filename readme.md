#### 该仓库用于学习课程 GAMES 102 几何建模与处理基础

课程主页：[GAMES102在线课程-刘利刚 (ustc.edu.cn)](http://staff.ustc.edu.cn/~lgliu/Courses/GAMES102_2020/default.html)

作业计划基于opengl和imgui完成，后面某些难以自己实现的功能可能会借用刘老师推荐的Utopia框架或Unity完成

之前我已经写好的opengl框架链接：[Lim-ss/opengl-study (github.com)](https://github.com/Lim-ss/opengl-study)

##### homework1

做了多项式插值、高斯函数插值、多项式线性回归、多项式岭回归

当点数量较多时，两种插值方法出现线不经过点的情况，怀疑是精度问题导致的。而且插值耗的CPU性能较大，因为要解大规模矩阵的逆。

使用了Eigen