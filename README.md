# Detector_YOLOv3
The interface of YoloV3_detector with c++

## V1.0
### 命令行参数
.\Detector.exe --image=Test\000200.jpg
.\Detector.exe --video=video.mp4
.\Detector.exe --rtsp=rtsp://admin:chenhui561X@124.192.234.186:5544/h264/ch1/main/av_stream

### 深度学习配置文件
voc.names
yolov3-voc.cfg
yolov3-voc_11400.weights

### 使用说明
1.可以检测
   1）有头盔无反光衣（程序中为字符串10）
   2）无头盔有反光衣（程序中为字符串01）
   3）有头盔有反光衣（程序中为字符串11）
   共三类目标，其中前两类违规。
 “无头盔无反光衣”的情况，因为提供的视频材料没有该类目标，无法检测。
2.可以检测图片，视频或者rtsp视频流，每秒选择一帧进行检测，如果检测到违规，截图保存在项目根目录。
3.使用release模式生成的exe文件进行使用。

### 编译环境
VS2017+Opencv3.4.3

### 函数及变量说明
1.变量

```
     float confThreshold = 0.8; // 置信度阈值
​    long long DetectedIdx = 0; //违规图片索引
​    vector<string> classes;//目标类别 01 10 11
​    string pro_dir = ""; //配置文件目录，默认项目根目录
​    string DirOfDetectedFrame = "";//违规截图存储路径，默认项目根目录
​    float FPS ;//视频流帧率
​    int Interval；//两次检测之间的间隔帧数
```

   

2.函数

```
   //载入配置文件
​    Net LoadNetCfg();
​    //打开image/video/cam 
​    VideoCapture OpenInputFile(CommandLineParser parser);
​    // 去除置信度较低的目标
​    void postprocess(Mat& frame, const vector<Mat>& out, vector<int>&     classIds,vector<float>& confidences);
​    // 在图片中画框
​    void drawPred(int classId, float conf, int left, int top, int right, int bottom, Mat& frame);
​    // 获取神经网络最后一层
​    vector<String> getOutputsNames(const Net& net);
​    //处理帧
​    void ProcessFrame(Mat frame,Net net);
​    //是否有违规目标
​    bool ProcessClass(vector<int>& classIds, vector<float>& confidences);
```

## V2.0
### 使用说明
   可以检测
   1）有头盔无反光衣（程序中为字符串10）
   2）无头盔有反光衣（程序中为字符串01）
   3）有头盔有反光衣（程序中为字符串11）
   4）无头盔无反光衣（程序中为字符串00）
其中1 3 4违规。

### 深度学习配置文件
voc.names
yolov3-voc.cfg
yolov3-voc_9000.weights

### 函数及变量说明
   //是否有违规目标
    int ProcessClass(vector<int>& classIds, vector<float>& confidences);
   1）有头盔无反光衣 返回整型2，代表2级警告
   2）无头盔有反光衣 返回整型3，代表3级警告
   3）有头盔有反光衣 返回整型0，代表正常
   4）无头盔无反光衣 返回整型1，代表1级警告

## V3.0
   ### 主要改动

   1.增加Configuration.cfg配置文件
   2.增加数据库部分，将检测到的违规目标加入数据库

### 增加的配置文件

   C:\\Program Files\\Common Files\\System\\ADO\\msado15.dll

   SQLServer2017+SSMS2019

### 函数改动

 ```
 //是否有违规目标
​    int ProcessClass(vector<int>& classIds, int classid);
 ```

## V3.1

### 主要改动

1. 更改sqlserver连接字符串
2. 更改图片命名方式

###  函数改动

   1.增加 get_CurrentTime_s()

## V3.2

### 主要改动

1.修改线程结构

2.修改遍历帧方法（解决长时间播放，无响应问题）



## V3.3

1.修复一些bug

2.数据库名字可以自由指定



