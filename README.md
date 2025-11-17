# ESP32-S2 脉冲生成器

## 功能说明

- **Pin 10**: 生成10Hz脉冲，1ms高电平
- **Pin 11**: 生成1Hz脉冲，1ms高电平
- **内置LED**: 1Hz闪烁（500ms亮，500ms灭）
- 10Hz和1Hz脉冲完全对齐（1Hz脉冲与每第10个10Hz脉冲对齐）

## 编译和上传

### 方法1: 使用PlatformIO CLI

如果已安装PlatformIO CLI，直接运行：
```bash
pio run
```

或
```bash
platformio run
```

上传到设备：
```bash
pio run --target upload
```

### 方法2: 使用批处理脚本

Windows系统可以直接运行：
```bash
build.bat
```

### 方法3: 使用VS Code

1. 安装PlatformIO IDE扩展
2. 打开项目文件夹
3. 点击底部状态栏的编译按钮或按快捷键

## 安装PlatformIO

如果未安装PlatformIO，可以选择以下方式：

1. **VS Code扩展**（推荐）:
   - 在VS Code中搜索并安装"PlatformIO IDE"扩展

2. **命令行安装**:
   ```bash
   pip install platformio
   ```

3. **独立安装**:
   - 访问 https://platformio.org/install/cli

## 硬件要求

- ESP32-S2开发板（本项目配置为Lolin S2 Mini）
- Pin 10和Pin 11用于脉冲输出
- 内置LED用于状态指示

## 技术细节

- 使用ESP32-S2硬件定时器（Timer 0）实现精确时序
- 定时器预分频器：80（时钟频率1MHz）
- 脉冲宽度：1ms
- 10Hz周期：100ms
- 1Hz周期：1000ms

