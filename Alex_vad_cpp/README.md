# Voice Activity Detection 

---

## 语音激活检测
[源代码][1]：Alex语音对话管理系统(python)
`Alex/alex/comenents/vad`

## 编译运行
```sh
source ffnn
# 生成可执行文件 vad
./vad [音频文件]
# 输出每一帧是语音的概率
```

## 备注
改写Alex-vad检测部分代码。`.json`文件是训练得到的模型。

* 训练模型
    ```
    $ cd fx/fx/VAD/Alex-master/
    python alex/tools/vad/train_vad_nn_theano.py
    //得到的.json模型文件地址： ~/fx/VAD/Alex-master/alex/.json

    ```
    如果修改训练参数，需要在main.cpp中相应修改参数。

## contributors
```
/************************
 *    Vanessa Feng
 *    update 2017.4.14
 ************************/
```


  [1]: https://github.com/UFAL-DSG/alex
