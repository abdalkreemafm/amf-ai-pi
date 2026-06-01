import 'package:flutter/material.dart';
import 'package:ar_flutter_plugin/ar_flutter_plugin.dart';
import 'package:ar_flutter_plugin/datatypes/node_types.dart';
import 'package:ar_flutter_plugin/models/ar_node.dart';
import 'package:vector_math/vector_math_64.dart' as vector;
import 'dart:math';

/**
 * ******************************************************************************
 * @file           : ar_spatial_map.dart
 * @brief          : AR-SLAM Heatmap Rendering Engine for Hadeed V7.0
 * @author         : Abdul Karim (AMF Tech) & Manus AI
 * ******************************************************************************
 * @attention
 * Visualizes electromagnetic anomalies over the physical ground.
 * Maps 24-bit signal strength to RGB color spectrum in 3D space.
 * ******************************************************************************
 */

class HadeedARVisualizer {
  ARSessionManager? arSessionManager;
  ARObjectManager? arObjectManager;
  
  // قائمة لتخزين النقاط المسجلة لمنع تكرار الرسم في نفس المكان
  List<vector.Vector3> _recordedPoints = [];
  double _minDistanceBetweenPoints = 0.05; // 5 سنتيمتر كحد أدنى بين النقاط

  /**
   * @brief تهيئة مشهد الواقع المعزز
   */
  void onARViewCreated(
      ARSessionManager arSessionManager,
      ARObjectManager arObjectManager,
      ARAnchorManager arAnchorManager,
      ARLocationManager arLocationManager) {
    this.arSessionManager = arSessionManager;
    this.arObjectManager = arObjectManager;

    this.arSessionManager!.onInitialize(
      showFeaturePoints: false,
      showPlanes: true, // إظهار سطح الأرض المكتشف
      customPlaneTexturePath: "assets/images/grid.png",
      showWorldOrigin: false,
    );
    this.arObjectManager!.onInitialize();
  }

  /**
   * @brief رسم بقعة حرارية بناءً على إشارة الجهاز
   * @param signal القوة الملتقطة (Normalized 0.0 to 1.0)
   * @param targetType نوع المعدن (من hadeed_logic.h)
   * @param estimatedDepth العمق بالسنتيمتر
   */
  Future<void> updateHeatmap(double signal, int targetType, double estimatedDepth) async {
    if (arSessionManager == null) return;

    // 1. الحصول على موقع الهاتف الحالي في الفضاء (يمثل موقع الكويل تقريباً)
    // نستخدم مصفوفة التحويل (View Matrix) للحصول على الإحداثيات (X, Y, Z)
    var currentPose = await arSessionManager!.getPose(); 
    if (currentPose == null) return;

    vector.Vector3 translation = currentPose.getTranslation();

    // 2. التحقق من المسافة (لا ترسم بقعة جديدة إلا إذا تحرك الكويل مسافة كافية)
    if (_isTooClose(translation)) return;

    // 3. تحديد اللون بناءً على نوع الهدف وقوة الإشارة
    Color pointColor = _getColorForTarget(targetType, signal);

    // 4. إنشاء نقطة 3D (نستخدم شكل كروي متوهج)
    final newNode = ARNode(
      type: NodeType.localGLTF2,
      uri: "assets/models/glow_sphere.gltf", // موديل كرة شفافة صغيرة
      scale: vector.Vector3(0.05, 0.05, 0.05), // حجم البقعة 5 سم
      position: translation,
      transformation: vector.Matrix4.identity()..translate(translation),
    );

    // 5. وضع النقطة تحت سطح الأرض الافتراضي بناءً على "العمق" المحسوب
    // ملاحظة: نقوم بإزاحة المحور Y للأسفل بمقدار العمق
    newNode.position.y -= (estimatedDepth / 100.0); 

    bool? didAdd = await arObjectManager!.addNode(newNode);
    if (didAdd == true) {
      _recordedPoints.add(translation);
    }
  }

  /**
   * @brief اختيار اللون بناءً على بصمة المعدن
   */
  Color _getColorForTarget(int type, double strength) {
    switch (type) {
      case 0x01: // TREASURE_GOLD
        return Colors.amber.withOpacity(strength);
      case 0x02: // NATURAL_GOLD
        return Colors.orangeAccent.withOpacity(strength);
      case 0x04: // CAVITY (فراغ)
        return Colors.blueAccent.withOpacity(strength);
      case 0x07: // HOT_ROCK
        return Colors.brown.withOpacity(strength * 0.5);
      default:
        return Colors.greenAccent.withOpacity(0.3); // أرضية طبيعية
    }
  }

  /**
   * @brief منع تكدس النقاط في مكان واحد لتقليل الضغط على المعالج
   */
  bool _isTooClose(vector.Vector3 newPoint) {
    for (var point in _recordedPoints) {
      if (point.distanceTo(newPoint) < _minDistanceBetweenPoints) return true;
    }
    return false;
  }

  /**
   * @brief مسح الخريطة للبدء من جديد
   */
  void clearMap() {
    arObjectManager?.removeallNodes();
    _recordedPoints.clear();
  }
}
