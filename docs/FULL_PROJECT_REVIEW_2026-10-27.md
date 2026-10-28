# تقرير المراجعة الشاملة لمشروع Hadeed

**التاريخ:** 2026-10-27  
**الفرع:** `refactor/foundation-v1`  
**آخر commit مفحوص:** `5469c83 Add build scaffolding and project templates`  
**النطاق:** Firmware/C، Drivers/DSP، بروتوكول K210، Flutter، الأمن، البناء، الاختبارات، التوثيق، والملفات القديمة.

## 1. الخلاصة التنفيذية

المشروع حاليًا **نموذج هندسي متعدد المسارات، وليس منتجًا قابلًا للبناء أو التشغيل الكامل**.

الجزء الأكثر نضجًا هو بروتوكول Hadeed V1: ترميز C وPython وMicroPython متطابق من ناحية offsets وendian وCRC، والاختبارات الحالية تمر. لكن هذا النجاح محدود بنطاق البروتوكول؛ إذ إن CMake لا يبني Firmware، ولا توجد ملفات STM32CubeIDE/HAL/`main.h`/linker/startup، كما أن مسار استقبال STM32 لا يملك parser تراكميًا يعالج الإطارات المجزأة والضجيج.

أما Firmware، ففيه أخطاء ترجمة وتكامل مؤكدة، منها تواقيع غير متطابقة، headers مفقودة، رموز غير معرفة، مساران متنافسان لـ Core و`src` و`Hadeed_Core`، ومسار ADS1256 DMA غير آمن. تطبيق Flutter عبارة عن scaffold أولي: لا توجد منصات Android/iOS، ولا BLE فعلي، ولا assets runtime، و`main.dart` يعرض شاشة واحدة فقط.

**التقييم الحالي المقترح:**

- بروتوكول Host/K210: **نموذج أولي قابل للاختبار**.
- Firmware STM32: **غير قابل للبناء/التحقق حاليًا**.
- تطبيق Flutter: **هيكل أولي غير متكامل**.
- أمن BLE وOTA: **غير منفذ**.
- المنتج الميداني: **غير جاهز للاختبار أو التسويق كمنتج عامل**.

## 2. أدلة التحقق التي نُفذت

تم تشغيل الفحوصات التالية على النسخة الحالية:

```text
python3 -m pytest -q                 -> 6 passed
python3 -m compileall ...            -> passed
cc -std=c11 -Wall -Wextra -Werror \
   -pedantic ...                     -> passed
python3 tools/check_project_scaffolding.py -> project scaffolding: OK
git diff --check                     -> passed
```

لم يمكن تشغيل:

- `cmake` لأن الأداة غير مثبتة في بيئة المراجعة.
- `flutter analyze` و`dart format` لأن Flutter/Dart غير مثبتين.
- Firmware STM32 لأن HAL وCubeMX وملفات اللوحة غير موجودة.

نجاح اختبارات البروتوكول **لا يساوي** نجاح Firmware الكامل.

## 3. النتائج الحرجة P0

### P0-1 — Firmware لا يمر بمرحلة الترجمة حتى مع افتراض توفر HAL

الأدلة:

- `Core/Src/main.c:70` يستخدم `g_sys_health.current_state`، لكن `Hadeed_System_Health_t` في `Core/Inc/hadeed_math.h:121-129` لا يحتوي هذا العضو.
- `main.c:228-229` يستدعي `ADS1256_Check()` و`RM3100_Check()`، بينما المتاح في السائقين هو `ADS1256_Ping()` و`RM3100_Ping()`.
- `main.c:160` يستدعي `Hadeed_Analyze_SMF(&g_current_target)`، بينما تعريف الدالة في `Core/Src/hadeed_dsp.c:94` يتطلب أيضًا `uint32_t *raw_buffer`.
- `main.c:163-167` يستدعي `Hadeed_Fuse_Sensors(...)`، بينما التنفيذ الموجود يعرف `Hadeed_Fuse_Sensors_Logic(Hadeed_Target_Feature_t *)`، ويوجد مسار آخر في `src` يستخدم أنواعًا وثوابت غير موحدة.

الأثر: لا يمكن اعتبار ملف `main.c` نقطة دخول قابلة للبناء. إصلاح ذلك يتطلب اختيار واجهة واحدة لكل DSP/Fusion/health وإضافة headers موحدة، وليس إضافة prototypes عشوائية.

### P0-2 — مسارات Core وsrc وHadeed_Core متنافسة

يوجد أكثر من تنفيذ لنفس المفاهيم:

- `Core/Inc/hadeed_math.h` و`Hadeed_Core/Inc/hadeed_math.h` بنوعين مختلفين.
- `Core/Src/hadeed_dsp.c` و`Hadeed_Core/Src/hadeed_dsp.c` و`DSP/`.
- `Core/Src/hadeed_fusion.c` و`src/logic/hadeed_fusion_engine.c`.
- `Core/Src/hadeed_logger.c` و`src/storage/hadeed_logger.c`.
- مسار BLE في `Core/Src/hadeed_comm.c` ومسار قديم في `src/comm/hadeed_ble_proto.c`.

الأثر: حتى بعد إصلاح أخطاء الترجمة، قد يتم ربط تنفيذ غير مقصود حسب include paths وترتيب build. يجب اختيار مسار إنتاج واحد ونقل البقية إلى `archive/legacy` أو منعها صراحة من البناء.

### P0-3 — مسار ADS1256 DMA غير صحيح زمنيًا ودلاليًا

الأدلة:

- `main.c:116-136` يعرّف `uint32_t raw_buffer[512]`.
- `Drivers/Custom/ads1256.c:133-146` يرسل DMA عدد `num_samples * 3` بايت إلى المخزن، أي إن كل عينة 24-bit لا تطابق عناصر `uint32_t`.
- يبدأ DMA ثم يقرأ `raw_buffer[5]` مباشرة دون انتظار callback أو semaphore.
- لا يوجد sign-extension واضح لمسار DMA، خلافًا لمسار القراءة المفردة.
- لا يوجد إنهاء واضح لـ CS بعد DMA ولا فحص كامل لقيمة إرجاع HAL.

الأثر: بيانات ADC قديمة أو غير مكتملة أو موجبة دائمًا، وقرارات DSP/AI غير موثوقة.

الإصلاح المطلوب:

1. استخدام `uint8_t raw_bytes[512 * 3]` أو مخزن `int32_t` بعد unpack صريح.
2. إضافة sign-extension 24-bit إلى `int32_t`.
3. إكمال العملية عبر callback/event/semaphore.
4. حماية Cache على STM32H7 عند استخدام DMA.
5. إدارة CS في callback النجاح والفشل.
6. منع بدء دورة جديدة قبل انتهاء السابقة.

### P0-4 — تطبيق Flutter ليس تطبيقًا فعليًا بعد

`Mobile_App/lib/main.dart` يعرض شاشة واحدة فقط برسالة أن الاتصال غير مهيأ. لا توجد:

- طبقة BLE scan/connect/characteristics.
- منصات `android/` و`ios/`.
- صلاحيات Bluetooth/camera/location.
- اختبارات Flutter.
- `pubspec.lock`.
- assets فعلية.

الأثر: `flutter run` لن ينتج تطبيقًا متكاملًا من هذا المجلد، وميزات AR والصوت والخزنة غير مربوطة بنقطة التشغيل.

### P0-5 — BLE والأمان الإنتاجي غير منفذين

`Mobile_App/lib/services/security_vault.dart:26-35` يرمي `UnsupportedError` للمصافحة وفك التشفير. كما أن `docs/BLE_Handshake_Spec.md` يقر بأن provisioning الآمن غير مكتمل.

لا يجوز تفعيل OTA أو إرسال بيانات حساسة قبل اعتماد وتنفيذ:

- BLE Secure Connections.
- Provisioning آمن ومفاتيح hardware-backed.
- AEAD مع nonce فريد.
- حماية replay عبر sequence/nonce/window.
- توقيع firmware وتحقق rollback.
- تدوير وإبطال المفاتيح.

## 4. النتائج العالية P1

### P1-1 — استقبال STM32 ليس incremental

`Drivers/Custom/k210_link.c:38-49` يمرر كل حدث `ReceiveToIdle DMA` مباشرة إلى `Hadeed_Sync_DecodeVisionFrame`. لا يوجد buffer تراكمي أو parser يبحث عن SOF ويحتفظ ببقايا frame.

يفشل المسار عند:

- وصول frame على دفعتين.
- وجود noise قبل frame.
- وجود أكثر من frame في دفعة واحدة.
- CRC فاشل يتبعه frame صالح.

قارئ K210 في `AI_Vision_K210/hadeed_framer.py` أفضل، لكنه لا يعالج مسار H7. يجب إنشاء parser C stateful واختباره بحالات partial/noise/multiple/corrupt.

### P1-2 — تهيئة الأجهزة غير مكتملة

`Core/Src/main.c` يهيئ USART1 ولا يهيئ USART2، مع أن K210 يستخدم `huart2`. كما لا تظهر استدعاءات واضحة قبل إنشاء المهام لـ:

- `ADS1256_Init()`.
- `RM3100_Init()`.
- `H7_K210_Init()`.
- `Hadeed_SMF_Init()`.

الأثر: المهام قد تبدأ على peripherals غير مهيأة.

### P1-3 — تعارض SPI/I2C والتعافي

PulseTask وBrainTask وMonitorTask والتعافي يستخدمون peripherals متوازية دون mutex مستقل لكل SPI/I2C. التعافي يعيد تهيئة I2C/ADC بينما قد تكون معاملة DMA أو قراءة أخرى جارية.

الحل: ownership واضح لكل peripheral، mutex لكل bus، state machine للتعافي، وإيقاف/إلغاء DMA قبل re-init.

### P1-4 — headers أساسية مفقودة

الملفات التالية مستوردة لكنها غير موجودة في المستودع:

- `ads1256.h`.
- `rm3100.h`.
- `wien_bridge.h`.
- `hadeed_dsp.h`.
- `sd_card.h`.
- `main.h`.
- `stm32h7xx_hal.h` و`cmsis_os.h` من HAL الخارجي.

تمت إضافة `Drivers/Custom/k210_link.h`، لكنه لا يحل بقية فجوة STM32.

### P1-5 — فحوص HAL وtimeouts غير موحدة

عمليات SPI/I2C وDMA في السائقين تتجاهل قيم `HAL_StatusTypeDef` أو تنتهي بعد timeout دون إرجاع حالة واضحة. هذا يسمح باستخدام بيانات فاشلة كأنها صحيحة.

كل driver يجب أن يعيد status موحدًا مثل `OK/BUSY/TIMEOUT/CRC/IO_ERROR`، وتنتقل الحالة إلى degraded/safe state عند الفشل.

### P1-6 — مؤشرات ومخازن بلا تحقق

دوال DSP/decay/depth تفترض pointers غير NULL وأحجامًا ثابتة دون تمرير `count/capacity`. هذا قد يؤدي إلى HardFault عند خطأ DMA أو استدعاء مبكر.

### P1-7 — نموذج AI غير تنفيذي

`AI_Model/model_data.c` يحتوي placeholder صغيرًا، و`model_inference.h` يعلن `run_hadeed_inference` دون تنفيذ، بينما scheduler يستدعي `AI_Process_Inference` غير الموجود.

لا يوجد runtime أو model validated قابل للتشغيل. يجب عزل هذا المسار من Firmware أو توفير model/runtime/واجهة واحدة مع اختبار inference حقيقي.

## 5. النتائج المتوسطة P2

### P2-1 — K210 قد يقبل sync قديمًا أو خارج الترتيب

`AI_Vision_K210/h7_sync_slave.py` يفعّل `is_synced` لأي MSG_SYNC صحيح الشكل دون تحقق sequence/freshness، ويحسب offset بطرح عادي رغم احتمال wraparound في `ticks_ms`.

يلزم استخدام `ticks_diff/ticks_add`، التحقق من التسلسل، انتهاء صلاحية sync، واختبارات reorder/wraparound.

### P2-2 — نتائج K210 لا ترتبط بوضوح بـ sync صالح

`soil_classifier.py` يعيد استخدام آخر timestamp/sequence حتى لو انقطع sync، وقد يرسل نتائج بعد انتهاء صلاحيته. يجب تحديد contract: sequence للheartbeat أم sequence مستقل للنتيجة، ورفض النتائج stale.

### P2-3 — تضارب BLE codes وV1

`protocol/README.md` يعرّف رسائل V1 داخل frame. في المقابل `src/comm/hadeed_ble_proto.c` يرسل raw codes، و`voice_renderer.dart` ينتظر `0xA0..0xF0`، بينما المسارات الأخرى تستخدم `0x10..0x40`.

الأثر: تفسير خاطئ أو صامت للرسائل.

الحل: schema واحد، `MSG_VOICE_TRIGGER` برسالة V1 وpayload رسمي، وإزالة raw packet path.

### P2-4 — Flutter AR غير موصول واحتمال أخطاء API

`ar_spatial_map.dart` يستعمل `getPose()` و`removeallNodes()` مع اعتماد حساس للنسخة، لكن لا يوجد analyzer أو مشروع platform للتحقق. كما أن الخدمة غير مستعملة من `main.dart`.

### P2-5 — أصول الصوت وAR غير موجودة

الـ pubspec يعلن مجلدات assets، لكن الموجود `.gitkeep` فقط. الخدمة تطلب MP3، وAR يطلب `grid.png` و`glow_sphere.gltf`.

الحل: إضافة assets مرخصة مع attribution واختبار وجودها، أو تعطيل الميزات حتى توفرها.

### P2-6 — entitlement محلي قابل للتلاعب

`security_vault.dart` يخزن boolean في SharedPreferences باستخدام salt ثابت ظاهر في التطبيق. هذا ليس Vault ولا مصدر صلاحية موثوقًا.

الحل: entitlement موقّع قصير العمر، تحقق خادمي، Keychain/Keystore للمفاتيح، وحماية rollback.

### P2-7 — خصوصية البيانات غير محددة

الخدمة ترفع peak/slope/harmony/phase/type/date إلى endpoint ثابت. وصف README للبيانات بأنها anonymous وبدون GPS لا يكفي لإثبات عدم قابلية إعادة التعرف، كما أن وثائق أخرى تتحدث عن GPS وcloud/heatmap.

يلزم opt-in واضح، schema وتصنيف، سياسة retention/deletion، endpoint حسب environment، ووثيقة privacy حقيقية.

### P2-8 — تعارض وثائق العتاد والإصدارات

توجد تعارضات بين:

- 2-layer و4-layer PCB.
- STM32F411 في وثائق V0 مقابل MCU غير محدد في الإعداد الحالي.
- HC-05 Bluetooth Classic مقابل BLE 5.x.
- وثائق تخطط لميزات غير موجودة في الكود.

الحل: مصفوفة revisions تربط لكل نسخة MCU/radio/PCB/BOM/pinout/protocol، مع status/date لكل وثيقة.

### P2-9 — CI لا يفحص Flutter أو assets أو الأمن

CI الحالي يفحص Python وC protocol وCMake، لكنه لا يشغل:

- `flutter analyze`.
- `flutter test`.
- وجود assets.
- secret scanning.
- dependency/license audit.
- build Android/iOS.

## 6. النتائج المنخفضة P3

- `src/storage/hadeed_logger.c` يستخدم `sprintf` إلى buffer ثابت 128 بايت؛ خطر overflow.
- `H7_K210_ReceiveResponse` يعطل كل interrupts حول memcpy؛ قد يسبب jitter.
- `osThreadFlagsSet` من UART callback يحتاج API ISR-safe مناسبًا للـ RTOS، والإشارة `0x04` ليست مستهلكة بوضوح في BrainTask.
- `Hadeed_Unified_Decision_Engine` قد يحول confidence خارج `[0,100]` إلى `uint8_t` دون clamp.
- فحص الميلان يختبر القيم الموجبة فقط ولا يستخدم `fabsf` أو NaN checks.
- buffers الخاصة بـ SMF عالمية وغير محمية من إعادة الدخول.
- روابط CDN موقعة خاصة موجودة في وثائق hardware؛ ينبغي استبدالها بملفات أو روابط عامة مستقرة.
- ملفات `__pycache__` موجودة محليًا ويجب ألا تدخل commit؛ ينبغي تنظيفها أو تشديد `.gitignore`.

## 7. ما هو جيد حاليًا

1. بروتوكول C/Python/MicroPython متطابق في layout وCRC ضمن الاختبارات الحالية.
2. وجود اختبارات round-trip وCRC وحالات noise/partial في K210.
3. إضافة CMake وCTest كنقطة بداية لاختبارات Host.
4. وجود توثيق صريح يقر بأن HAL وCubeMX والأمان الإنتاجي غير مكتملة.
5. إزالة XOR الوهمي بدل تقديمه كتشفير حقيقي.
6. استخدام `snprintf` في نسخة Core الجديدة من logger.
7. وجود فرع تطوير وPull Request بدل تعديل `main` مباشرة.

## 8. خطة الإصلاح المقترحة حسب الترتيب

### المرحلة 0 — تجميد الحقيقة

- تعديل README إلى ثلاث حالات فقط: Implemented / Prototype / Planned.
- حذف عبارات «مشفر»، «ميكروثانية»، «جاهز»، «أول/أفضل في العالم» ما لم يوجد اختبار أو قياس.
- اعتماد revision matrix للعتاد والبرنامج.
- اختيار `Core/` أو `src/` أو `Hadeed_Core/` كمسار إنتاج واحد.

### المرحلة 1 — جعل Firmware قابلًا للترجمة

- توفير رقم MCU واللوحة وملف `.ioc` الحقيقي.
- توليد `main.h`, HAL, startup, linker, CMSIS/FreeRTOS.
- إضافة headers لكل driver.
- توحيد prototypes وtypes وAPI names.
- إصلاح `current_state`, `*_Check/Ping`, `Hadeed_Analyze_SMF`, Fusion.
- جعل CMake Firmware target منفصلًا عن Host target.

### المرحلة 2 — إصلاح القياس والـ DMA

- تصميم ownership للـ SPI/I2C.
- إصلاح ADS1256 packed 24-bit DMA/sign extension/cache.
- callback/semaphore/timeout/CS lifecycle.
- منع القراءة قبل اكتمال DMA.
- إضافة اختبارات driver على Host أو HIL.

### المرحلة 3 — تثبيت الاتصال

- إضافة C incremental parser لـ H7.
- توحيد V1 لكل الرسائل، خصوصًا الصوت و3D/target.
- اختبارات cross-language وcorrupt/partial/multiple/reorder/wraparound.
- توحيد sequence/freshness contract بين heartbeat وvision result.

### المرحلة 4 — Flutter حقيقي

- إنشاء مشروع Flutter رسمي Android/iOS أو توضيح أن المجلد library فقط.
- إضافة BLE plugin موثوق، permissions، services، state management.
- إضافة assets أو feature flags.
- تشغيل analyze/test/build في CI.
- commit lockfile.

### المرحلة 5 — الأمان والبيانات

- threat model وprivacy policy.
- Secure Connections/provisioning/AEAD/replay protection.
- signed entitlement وsigned OTA مع rollback protection.
- secret/dependency/license scans.

### المرحلة 6 — تحقق ميداني

- protocol HIL.
- ADC/DMA timing وlogic analyzer.
- sensor calibration.
- ground-truth dataset للتصنيف.
- قياس false positives/false negatives/latency/battery/thermal.
- عدم إطلاق أي claim تسويقي قبل نتائج موثقة.

## 9. القرار النهائي

المشروع يستحق الاستمرار، لكن يجب التعامل معه الآن كـ **إعادة تنظيم هندسي وMVP تقني**، وليس كمنصة V7 مكتملة. لا أوصي بدمج الفرع في `main` كإصدار منتج قبل إصلاح P0/P1 وتوفير مشروع STM32 الحقيقي.

الخطوة الأكثر قيمة الآن ليست إضافة ميزات جديدة؛ بل:

1. حسم منصة العتاد والمسار البرمجي الإنتاجي.
2. جعل Firmware يترجم فعليًا.
3. إصلاح DMA والاتصال.
4. ربط Flutter بـ BLE حقيقي.
5. بناء بيانات ground truth واختبارات Hardware-in-the-loop.
