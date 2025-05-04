package com.airdetection.udp;

import com.airdetection.model.AirData;
import com.airdetection.service.DataService;
import lombok.extern.slf4j.Slf4j;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.stereotype.Component;

import javax.annotation.PostConstruct;
import javax.annotation.PreDestroy;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.nio.charset.StandardCharsets;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

@Slf4j
@Component
public class UDPServer {
    
    @Value("${udp.server.port:8080}")
    private int port;
    
    private DatagramSocket socket;
    private boolean running = false;
    private ExecutorService executorService;
    
    // 定义正则表达式模式匹配STM32发送的数据格式，支持负数和小数
    private static final Pattern DATA_PATTERN = Pattern.compile(
        "Humidity: (\\d+\\.\\d+)%, Temperature: (\\d+\\.\\d+) C, " +
        "Methane: (-?\\d+\\.\\d+) PPM, TVOC: (\\d+) PPB, CO2eq: (\\d+) PPM, " +
        "Dust\\(PM2\\.5\\): (\\d+\\.\\d+) ug/m\\^3"
    );
    
    @Autowired
    private DataService dataService;
    
    @PostConstruct
    public void start() {
        try {
            socket = new DatagramSocket(port);
            running = true;
            executorService = Executors.newSingleThreadExecutor();
            executorService.execute(this::receiveData);
            log.info("UDP服务器已启动，监听端口: {}", port);
        } catch (Exception e) {
            log.error("UDP服务器启动失败: {}", e.getMessage(), e);
        }
    }
    
    private void receiveData() {
        byte[] buffer = new byte[1024];
        DatagramPacket packet = new DatagramPacket(buffer, buffer.length);
        
        while (running) {
            try {
                socket.receive(packet);
                String data = new String(packet.getData(), 0, packet.getLength(), StandardCharsets.UTF_8);
                log.info("收到数据: {}", data);
                
                // 解析数据并通知服务
                AirData airData = parseData(data);
                if (airData != null) {
                    dataService.processNewData(airData);
                }
                
                // 重置packet长度，准备接收下一个数据包
                packet.setLength(buffer.length);
            } catch (Exception e) {
                if (running) {
                    log.error("接收数据出错: {}", e.getMessage(), e);
                }
            }
        }
    }
    
    // 解析接收到的数据字符串为AirData对象
    private AirData parseData(String data) {
        try {
            Matcher matcher = DATA_PATTERN.matcher(data);
            if (matcher.find()) {
                // 提取各个匹配组的数据
                double humidity = Double.parseDouble(matcher.group(1));
                double temperature = Double.parseDouble(matcher.group(2));
                double methane = Double.parseDouble(matcher.group(3));
                double tvoc = Double.parseDouble(matcher.group(4));
                double co2 = Double.parseDouble(matcher.group(5));
                double pm25 = Double.parseDouble(matcher.group(6));
                
                return AirData.builder()
                        .humidity(humidity)
                        .temperature(temperature)
                        .methane(methane)
                        .tvoc(tvoc)
                        .co2(co2)
                        .pm25(pm25)
                        .timestamp(System.currentTimeMillis())
                        .build();
            } else {
                log.warn("数据格式不匹配: {}", data);
            }
        } catch (Exception e) {
            log.error("解析数据出错: {}", e.getMessage(), e);
        }
        return null;
    }
    
    @PreDestroy
    public void stop() {
        running = false;
        if (socket != null && !socket.isClosed()) {
            socket.close();
        }
        if (executorService != null) {
            executorService.shutdown();
        }
        log.info("UDP服务器已关闭");
    }
} 