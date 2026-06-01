import 'dart:math';
import 'package:flutter/services.dart';
import 'package:soundpool/soundpool.dart';

/**
 * ******************************************************************************
 * @file           : voice_renderer.dart
 * @brief          : Zero-Latency Emotional Arabic Voice Engine
 * @author         : Abdul Karim (AMF Tech) & Manus AI
 * ******************************************************************************
 * @attention
 * This service maps Hex codes from STM32H7 to high-quality audio assets.
 * Implements "Dynamic Empathy" by rotating through phrase variations.
 * ******************************************************************************
 */

class HadeedVoiceRenderer {
  late Soundpool _pool;
  final Map<int, List<int>> _soundIdMap = {};
  bool _isInitialized = false;

  // تعريف الأكواد المتوافقة مع hadeed_logic.h
  static const int CMD_WELCOME      = 0xA0;
  static const int CMD_SOIL_READY   = 0xA1;
  static const int CMD_GOLD_HIGH    = 0xB0;
  static const int CMD_GOLD_DEEP    = 0xB1;
  static const int CMD_CAVITY_FOUND = 0xC0;
  static const int CMD_HOTROCK_SKIP = 0xD0;
  static const int CMD_SLOW_DOWN    = 0xE0;
  static const int CMD_COIL_HIGH    = 0xE1;
  static const int CMD_BATT_LOW     = 0xF0;

  // تهيئة المحرك وتحميل الملفات الصوتية في الذاكرة (RAM)
  Future<void> init() async {
    if (_isInitialized) return;

    _pool = Soundpool.fromOptions(
      options: SoundpoolOptions(
        streamType: StreamType.music,
        maxStreams: 5, // السماح بتداخل الأصوات (مثل رنة الذهب مع صوت المساعد)
      ),
    );

    // تحميل الجمل العاطفية (Variations) لكل كود
    await _loadVariation(CMD_WELCOME,      ['welcome_1.mp3']);
    await _loadVariation(CMD_SOIL_READY,   ['soil_ok_1.mp3']);
    await _loadVariation(CMD_GOLD_HIGH,    ['gold_high_1.mp3', 'gold_high_2.mp3']);
    await _loadVariation(CMD_GOLD_DEEP,    ['gold_deep_1.mp3', 'gold_deep_2.mp3']);
    await _loadVariation(CMD_CAVITY_FOUND, ['cavity_1.mp3', 'cavity_2.mp3']);
    await _loadVariation(CMD_HOTROCK_SKIP, ['hotrock_1.mp3', 'hotrock_2.mp3']);
    
    // جمل "التمهل" - الجمل التي طلبتموها لتكون رفيقة وغير مزعجة
    await _loadVariation(CMD_SLOW_DOWN, [
      'slow_down_empathetic_1.mp3', // "يا صديقي، تمهل قليلاً لكي لا يذهب عملنا سدى"
      'slow_down_empathetic_2.mp3', // "خطواتك سريعة، الأهداف العميقة تحتاج مسحاً هادئاً"
      'slow_down_empathetic_3.mp3', // "على مهلك، دعنا نأخذ وقتنا لنرى بوضوح"
    ]);

    await _loadVariation(CMD_COIL_HIGH, ['coil_low_1.mp3']);
    await _loadVariation(CMD_BATT_LOW,  ['battery_low_1.mp3']);

    _isInitialized = true;
    print("Hadeed Voice Engine: Ready with Zero-Latency");
  }

  // دالة داخلية لتحميل الملفات من Assets إلى الذاكرة المباشرة
  Future<void> _loadVariation(int cmdCode, List<String> assetNames) async {
    _soundIdMap[cmdCode] = [];
    for (String name in assetNames) {
      int id = await rootBundle.load("assets/audio/$name").then((ByteData soundData) {
        return _pool.load(soundData);
      });
      _soundIdMap[cmdCode]!.add(id);
    }
  }

  /**
   * @brief تشغيل الصوت بناءً على الكود المستلم من الجهاز
   * @param hexCode الكود القادم من البلوتوث (1 byte)
   * @param variationId رقم التنويع (اختياري، إذا أرسله الجهاز أو نختاره عشوائياً)
   */
  void playVoice(int hexCode, {int? variationId}) {
    if (!_isInitialized || !_soundIdMap.containsKey(hexCode)) return;

    List<int> ids = _soundIdMap[hexCode]!;
    int selectedId;

    if (variationId != null && variationId < ids.length) {
      selectedId = ids[variationId];
    } else {
      // اختيار عشوائي لكسر الرتابة إذا لم يحدد الجهاز نسخة معينة
      selectedId = ids[Random().nextInt(ids.length)];
    }

    _pool.play(selectedId);
  }

  /**
   * @brief تشغيل "رنة الذهب" فوق صوت المساعد (Layering)
   */
  void playBeep(double frequencyPower) {
    // هنا يمكن إضافة أصوات الـ VCO الكلاسيكية متداخلة مع المساعد
  }

  void dispose() {
    _pool.release();
    _isInitialized = false;
  }
}
