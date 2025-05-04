package com.airdetection.model;

import lombok.AllArgsConstructor;
import lombok.Builder;
import lombok.Data;
import lombok.NoArgsConstructor;

@Data
@Builder
@NoArgsConstructor
@AllArgsConstructor
public class AirData {
    private double temperature;    // 温度 (℃)
    private double humidity;       // 湿度 (%)
    private double methane;        // 甲烷浓度 (PPM)
    private double tvoc;           // 总挥发性有机化合物 (PPB)
    private double co2;            // 二氧化碳当量浓度 (PPM)
    private double pm25;           // PM2.5浓度 (μg/m³)
    private long timestamp;        // 时间戳
} 