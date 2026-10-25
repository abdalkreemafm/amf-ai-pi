# Hadeed Architecture Baseline

هذه الصفحة هي نقطة البداية الهندسية بعد مراجعة المستودع.

## مصدر الحقيقة

- `protocol/`: الترميز الفعلي ومواصفة الإطارات. يمنع إرسال C structs مباشرة عبر UART/BLE.
- `Core/`: firmware المرشح للإنتاج، بعد اكتمال ملفات HAL ومشروع STM32CubeIDE.
- `AI_Vision_K210/`: كود K210، ويجب أن يستخدم نفس مواصفة `protocol/`.
- `Mobile_App/`: تطبيق Flutter، ويجب ألا يعرّف أكوادًا أو بروتوكولات مستقلة عن firmware.
- `archive/`: ملفات تاريخية لا تدخل في البناء أو الاختبار.

## قواعد المرحلة الأولى

1. لا توجد ادعاءات "ذهب مؤكد" قبل وجود بيانات ground truth ومقاييس موثقة.
2. لا يجوز استخدام `#pragma pack` كترميز سلكي؛ يتم الترميز byte-by-byte.
3. كل انتظار hardware أو DMA يجب أن يملك timeout ومسار خطأ.
4. كل إطار اتصال يجب أن يحتوي version وtype وlength وsequence وCRC.
5. كل تغيير يمر باختبار آلي أو يضاف معه اختبار يثبت السلوك.

## ترتيب العمل

1. جعل المشروع قابلًا للتحقق محليًا: protocol tests وPython checks وC host test.
2. اختيار معماري firmware واحدة وعزل `src/` التجريبي من مسار الإنتاج.
3. بناء مشروع STM32 حقيقي مع HAL/CMSIS dependencies موثقة.
4. ترحيل K210 وFlutter إلى protocol V1.
5. إضافة اختبارات DSP وsensor fusion وhardware-in-the-loop.
