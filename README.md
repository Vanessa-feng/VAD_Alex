# VAD_Alex
Voice activity detection.

```
####################################################
#     voice activity detection via MFCC+FFNN
#
# Author: Vanessa Feng
# Blog: http://Vanessa-feng.github.com
# Email: vanessaxin-feng@gmail.com
####################################################
```

##Description
- python--神经网络方法, mfcc作为检测特征
- C++ 根据python版本改写, 提速15倍左右
- test C++调用Eigen和jsoncpp的测试

##Reference
- [对话管理系统:Alex](https://github.com/UFAL-DSG/alex), 其中`alex/tools/vad/train_vad_nn_theano.py`训练生成模型参数.
- [梅尔频率mfcc(c++)](https://github.com/weedwind/MFCC).
