# 导入所需模块
import tensorflow as tf
from matplotlib import pyplot as plt
import numpy as np
import pandas as pd
import time

x_data = np.array([-10.00, -9.09, -8.18, -7.27, -6.36, -5.45, -4.55, -3.64, -2.73, -1.82, -0.91,
      0.00, 0.91, 1.82, 2.73, 3.64, 4.55, 5.45, 6.36, 7.27, 8.18, 9.09, 10.00])

y_data = np.array([1.56, 1.23, 0.92, 0.59, 0.23, -0.17, -0.62, -1.12, -1.64, -2.25, -2.94,
      -3.76, -4.76, -5.99, -7.53, -9.42, -11.73, -14.54, -17.91, -21.93, -26.68, -32.27, -38.81])

# x_data = np.array(
#     [-9.59183673, -9.18367347, -8.7755102 , -8.36734694, -7.95918367,
#      -7.55102041, -7.14285714, -6.73469388, -6.32653061, -5.91836735,
#      -5.51020408, -5.10204082, -4.69387755, -4.28571429, -3.87755102,
#      -3.46938776, -3.06122449, -2.65306122, -2.24489796, -1.83673469,
#      -1.42857143, -1.02040816, -0.6122449 , -0.20408163,  0.20408163,
#       0.6122449 ,  1.02040816,  1.42857143,  1.83673469,  2.24489796,
#       2.65306122,  3.06122449,  3.46938776,  3.87755102,  4.28571429,
#       4.69387755,  5.10204082,  5.51020408,  5.91836735,  6.32653061,
#       6.73469388,  7.14285714,  7.55102041,  7.95918367,  8.36734694,
#       8.7755102 ,  9.18367347,  9.59183673, 10.95918367        
#       ])

# y_data = np.array(
#     [ -16.05207517,  -16.74164979,  -14.47479193,  -14.65919936,
#       -13.00822684,  -11.97459415,  -13.52259426,   -9.02020768,
#        -9.84562442,   -8.1135365 ,   -6.34610723,   -7.2564414 ,
#        -4.28674593,   -3.47975684,   -4.69808282,   -3.70473462,
#        -1.61052155,   -1.73217569,   -0.22819614,    1.52082568,
#         0.18321125,    3.20666368,    4.33035318,    3.81776867,
#         4.88810918,    6.57577816,    6.32604238,    9.88446883,
#         9.60880897,    8.93696851,   11.48628978,   11.72669849,
#        13.27565359,   13.40635017,   14.63445268,   14.37150581,
#        16.75044899,   16.16929244,   18.46001986,   18.21647477,
#        19.80073827,   20.29319555,   20.23217981,   21.62366213,
#        22.60727917,   24.4401903 ,   24.10493578,   24.79747619,
#        26.14124449   
#        ])

# x_data = np.array([-9.59183673, -7.14285714, -1.83673469, 6.32653061, 10.95918367])
# y_data = np.array([-16.05207517, -13.52259426, 1.52082568, 18.21647477, 26.14124449])

x_train = x_data.reshape(-1, 1)
y_train = y_data.reshape(-1, 1)

# 转换x的数据类型，否则后面矩阵相乘时会因数据类型问题报错
x_train = tf.cast(x_train, tf.float32)
y_train = tf.cast(y_train, tf.float32)

# from_tensor_slices函数切分传入的张量的第一个维度，生成相应的数据集，使输入特征和标签值一一对应
train_db = tf.data.Dataset.from_tensor_slices((x_train, y_train)).batch(32)

# 生成神经网络的参数，输入层为1个神经元，隐藏层为10个神经元，1层隐藏层，输出层为1个神经元
# 用tf.Variable()保证参数可训练
w1 = tf.Variable(tf.random.normal([1, 10]), dtype=tf.float32)
b1 = tf.Variable(tf.constant(0.01, shape=[10])) 

w2 = tf.Variable(tf.random.normal([10, 1]), dtype=tf.float32)
b2 = tf.Variable(tf.constant(0.01, shape=[1]))

lr = 0.002  # 学习率为
epoch = 1500  # 循环轮数


# 训练部分
for epoch in range(epoch):
    for step, (x_train, y_train) in enumerate(train_db):
        with tf.GradientTape() as tape:  # 记录梯度信息
            
            h1 = tf.matmul(x_train, w1) + b1  # 记录神经网络乘加运算
            h1 = tf.nn.relu(h1)
            y = tf.matmul(h1, w2) + b2

            # 采用均方误差损失函数mse = mean(sum(y-out)^2)
            loss_mse = tf.reduce_mean(tf.square(y_train - y))

            # 添加l2正则化
            loss_regularization = []
            # tf.nn.l2_loss(w)=sum(w ** 2) / 2
            loss_regularization.append(tf.nn.l2_loss(w1))
            loss_regularization.append(tf.nn.l2_loss(w2))
            
            loss_regularization = tf.reduce_sum(loss_regularization)
            loss = loss_mse + 0.03 * loss_regularization  # REGULARIZER = 0.03
            #loss = loss_mse # 不使用正则项
                
        # 计算loss对各个参数的梯度
        variables = [w1, b1, w2, b2]
        grads = tape.gradient(loss, variables)

        # 实现梯度更新
        # w1 = w1 - lr * w1_grad
        w1.assign_sub(lr * grads[0])
        b1.assign_sub(lr * grads[1])
        w2.assign_sub(lr * grads[2])
        b2.assign_sub(lr * grads[3])

    # 每200个epoch，打印loss信息
    if epoch % 20 == 0:
        print('epoch:', epoch, 'loss:', float(loss))

# 预测部分
print("*******predict*******")

# 原始 NumPy 数组
x_grid = np.arange(-10, 10, 0.2)

# 将 NumPy 数组中的 标量 转换为一个个一行一列的 TensorFlow 张量
x_tensor_grid = [tf.convert_to_tensor(it) for it in x_grid]
x_tensor_grid = [tf.cast(it, tf.float32) for it in x_tensor_grid]
x_tensor_grid = [tf.reshape(it, (-1, 1)) for it in x_tensor_grid]

# 将网格坐标点喂入神经网络，进行预测，probs为输出
y_grid = []
for x in x_tensor_grid:
    # 使用训练好的参数进行预测
    h1 = tf.matmul(x, w1) + b1
    h1 = tf.nn.relu(h1)
    y = tf.matmul(h1, w2) + b2  # y为预测结果
    y_grid.append(y.numpy()[0][0])

plt.plot(x_data, y_data, color='green', marker='.', linestyle='')
plt.plot(x_grid, y_grid, color='red', marker='', linestyle='-')
plt.show()