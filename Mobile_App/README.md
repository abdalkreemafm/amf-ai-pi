# Hadeed Mobile App

هذا المجلد يحتوي نقطة تشغيل Flutter وطبقات الخدمات. تم توفير `pubspec.yaml`
والأصول الفارغة المطلوبة حتى يمكن إنشاء المشروع محليًا.

## تشغيل محلي

```bash
flutter pub get
flutter analyze
flutter test
flutter run
```

ملفات الصوت وملف `grid.png` و`glow_sphere.gltf` ليست أصولًا مولدة تلقائيًا؛ يجب
إضافتها من فريق التصميم أو استبدال مساراتها بأصول مرخصة قبل تفعيل شاشة AR
والمساعد الصوتي.
