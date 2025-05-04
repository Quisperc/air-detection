package com.airdetection.service;

import com.airdetection.model.AirData;
import com.alibaba.fastjson.JSON;
import lombok.extern.slf4j.Slf4j;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.messaging.simp.SimpMessagingTemplate;
import org.springframework.stereotype.Service;

import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.ConcurrentLinkedQueue;

@Slf4j
@Service
public class DataService {

    private static final int MAX_HISTORY_SIZE = 100;
    
    // 使用线程安全的队列存储历史数据
    private final ConcurrentLinkedQueue<AirData> historyData = new ConcurrentLinkedQueue<>();
    
    @Autowired
    private SimpMessagingTemplate messagingTemplate;
    
    /**
     * 处理新收到的数据
     */
    public void processNewData(AirData data) {
        // 保存到历史数据
        addToHistory(data);
        
        // 通过WebSocket发送到前端
        sendToWebSocket(data);
        
        log.info("处理新数据：{}", data);
    }
    
    /**
     * 添加数据到历史记录，控制历史记录大小
     */
    private void addToHistory(AirData data) {
        historyData.add(data);
        // 如果超出最大容量，移除最旧的数据
        while (historyData.size() > MAX_HISTORY_SIZE) {
            historyData.poll();
        }
    }
    
    /**
     * 通过WebSocket发送数据到前端
     */
    private void sendToWebSocket(AirData data) {
        try {
            messagingTemplate.convertAndSend("/topic/air-data", JSON.toJSONString(data));
        } catch (Exception e) {
            log.error("发送数据到WebSocket失败: {}", e.getMessage(), e);
        }
    }
    
    /**
     * 获取历史数据
     */
    public List<AirData> getHistoryData() {
        return new ArrayList<>(historyData);
    }
} 