# TinyWebPi

在这个仓库中，我将自定义一个web服务器，用在我的树莓派5B上，使我可以方便地访问我的设备

涉及如下知识点：

- 语言：C， html
- 数据结构：
- 编程技能：socket网络通信，线程池
- 协议：HTTP
- 工具：CMake

## 项目构建

项目使用CMake进行构建

```bash
mkdir build
cd build
cmake <flags> ..
make
./main              # 运行web服务器
```

其中`<flags>`有如下选择

```
-DDBG=ON    # 开启DBG打印到终端，默认为OFF
```

项目日志位于`logs/log.txt`，收集代码中使用`DBG`宏的信息，可用于分析

## 设计文档

C语言web服务器，核心是通过socket监听来自客户端的HTTP请求，服务器解析请求内容后，将需要响应的内容发送给客户端。结果就表现为，在浏览器输入对应地址后，显示出相应的页面

### socket网络编程

### HTTP/1.1协议

http/1.1协议定义在[RFC2616](https://rfc2cn.com/rfc2616.html)中

### HTML学习

HTML是用来描述网页的一种语言

html文档的后缀名为`.html`或`.htm`，没有区别

一个最简单的html文档包括

```html
<!DOCTYPE html>                 <!--声明为html5文档-->
<html>                          <!--页面起始-->
<head>                          <!--头部，不可见-->
<meta charset="utf-8">
<title>test</title>
</head>
<body>                          <!--可见页面内容起始-->
<h1>我的第一个标题</h1>
<p>我的第一个段落。</p>
</body>
</html>
```

html中包含一系列元素，可以控制展示内容的格式，也叫html标签

|html元素|效果|
|---|---|
|`<h1>`-`<h6>`|标题|
|`<p>`|段落|
|`<br>`|换行符|
|`<a>`的href属性|链接|
|`<b>`|粗体文本|
|`<i>`|斜体文本|

元素可以携带属性，提供附加信息。属性是`name="value"`的形式写在标签内

### 模块

### packet_parse模块

该模块负责监听网络接口收发的报文，进行解析