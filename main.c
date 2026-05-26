while (1) {
    // 1. قراءة البيانات الخام من ADS1256
    uint32_t raw_val = ADS1256_Read();
    
    // 2. تنظيف الإشارة فوراً (تستغرق ميكروثانية)
    float clean_val = Hadeed_Update_Kalman(&my_kf, (float)raw_val);
    
    // 3. إذا تجاوزت الإشارة العتبة، ابدأ التحليل
    if (clean_val > THRESHOLD) {
        Hadeed_Signal_Feature current_feat;
        current_feat.peak_amplitude = clean_val;
        current_feat.decay_slope = Hadeed_Calculate_Decay_Slope(buffer, 512);
        current_feat.mag_vector = RM3100_Get_Vector();
        
        // اتخاذ القرار
        Hadeed_Analyze_Target(&current_feat);
    }
    
    // 4. فحص سلامة النظام في الخلفية
    Hadeed_System_Check();
}
