# 空气检测监控系统

这是一个基于 Spring Boot 的空气检测监控系统，用于接收 ESP8266 通过 UDP 发送的空气质量检测数据，并提供实时监控界面。

## 功能特点

- UDP 服务器接收 ESP8266 发送的传感器数据
- 实时数据展示（温度、湿度、甲烷浓度、TVOC、CO2 当量、PM2.5）
- 数据趋势图表展示
- WebSocket 实时更新前端数据
- 响应式界面设计，支持各种设备访问

## 系统架构

- 后端：Spring Boot 2.7.5
- 前端：Bootstrap 5 + Chart.js + WebSocket
- 通信：UDP（接收 ESP8266 数据）+ WebSocket（前后端实时通信）

## 快速开始

### 环境要求

- JDK 11 或更高版本
- Maven 3.6 或更高版本

### 构建与运行

1. 克隆项目到本地
2. 进入项目目录
3. 构建项目：`mvn clean package`
4. 运行项目：`java -jar target/air-monitor-0.0.1-SNAPSHOT.jar`
5. 浏览器访问：`http://localhost:9090`

### 配置说明

配置文件位于`src/main/resources/application.properties`，主要配置项包括：

- `server.port`: Web 服务器端口，默认 9090
- `udp.server.port`: UDP 服务器端口，默认 8080（需与 ESP8266 配置一致）

## 数据格式说明

STM32 通过 UART 发送数据，ESP8266 接收后通过 UDP 转发的数据格式为：

```
Humidity: 45.2%, Temperature: 25.3 C, Methane: 1.5 PPM, TVOC: 250 PPB, CO2eq: 450 PPM, Dust(PM2.5): 15.5 ug/m^3
```

## 注意事项

- 确保 ESP8266 的目标 IP 和端口与服务器 IP 和 UDP 端口一致
- 防火墙需开放 UDP 端口（默认 8080）和 Web 服务端口（默认 9090）
- 数据历史记录暂存在内存中，服务重启后历史数据会丢失

## 项目结构

```
WebServer/
├── src/
│   ├── main/
│   │   ├── java/
│   │   │   └── com/
│   │   │       └── airdetection/
│   │   │           ├── Application.java            # 应用启动类
│   │   │           ├── config/
│   │   │           │   └── WebSocketConfig.java    # WebSocket配置
│   │   │           ├── controller/
│   │   │           │   ├── ApiController.java      # REST API控制器
│   │   │           │   └── ViewController.java     # 视图控制器
│   │   │           ├── model/
│   │   │           │   └── AirData.java            # 数据模型
│   │   │           ├── service/
│   │   │           │   └── DataService.java        # 数据服务
│   │   │           └── udp/
│   │   │               └── UDPServer.java          # UDP服务器
│   │   └── resources/
│   │       ├── application.properties              # 应用配置
│   │       └── templates/                          # 前端模板
│   │           ├── index.html                      # 首页
│   │           └── dashboard.html                  # 监控面板
└── pom.xml                                         # Maven配置
```
