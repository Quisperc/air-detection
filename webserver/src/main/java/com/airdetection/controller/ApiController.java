package com.airdetection.controller;

import com.airdetection.model.AirData;
import com.airdetection.service.DataService;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RestController;

import java.util.List;

@RestController
@RequestMapping("/api")
public class ApiController {

    @Autowired
    private DataService dataService;

    @GetMapping("/history")
    public List<AirData> getHistoryData() {
        return dataService.getHistoryData();
    }
    
    @GetMapping("/latest")
    public AirData getLatestData() {
        List<AirData> history = dataService.getHistoryData();
        if (history.isEmpty()) {
            return null;
        }
        return history.get(history.size() - 1);
    }
} 