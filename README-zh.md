# flipperzero-YearProgress

[ENGLISH](README.md)

显示当前时间在今年中的进度条和百分比

![](screen/image0.png)

本项目基于 Flipper Zero 平台开发，用于计算今年初到当前秒数与今年总计秒数比值，并以进度条和百分比展示的 App。

这个 App 的主旨是提醒使用者珍惜时间。

## 构建

安装 [uFBT](https://github.com/flipperdevices/flipperzero-ufbt) 构建工具到系统中。

```bash
pip install ufbt
```

将项目源码克隆到本地。

```bash
git clone https://github.com/SocialSisterYi/flipperzero-YearProgress
```

使用 uFBT 构建应用 （如第一次使用需等待自动下载工具链）。

```bash
ufbt
```

## 安装

将你的 Flipper 通过 USB 数据线连接电脑，然后使用下方命令进行构建+上传。

```bash
ufbt launch
```

也可以在`./dist/yaer_progress.fap`寻找编译结果文件，或从 [Release](https://github.com/SocialSisterYi/flipperzero-YearProgress/releases) 中下载这个`.fap`文件。

接着你可以使用 [qFlipper](https://flipperzero.one/downloads) 上传这个文件到 `/ext/apps/tools/`。
