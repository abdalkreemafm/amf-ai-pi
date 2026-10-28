# Hadeed Critical Fixes Implementation Plan

**Goal:** جعل واجهات Firmware متسقة وإزالة أخطر أخطاء البناء والـ DMA والاتصال دون اختلاق إعدادات STM32 غير المعروفة.

**Architecture:** اعتماد `Core/` و`protocol/` كمسار الإنتاج، وإبعاد `src/` و`Hadeed_Core/` عن البناء الافتراضي. طبقة ADC ستستخدم مخزن bytes وcompletion status، وطبقة K210 ستستخدم parser تراكميًا مستقلًا عن callbacks.

**Tech Stack:** C11، STM32 HAL/CMSIS-RTOS2 عند توفرهما، Python tests، Host protocol tests.

**Spec:** `docs/FULL_PROJECT_REVIEW_2026-10-27.md`.

## Global Constraints

- لا يتم اختلاق رقم STM32 أو pinout أو `.ioc` نهائي.
- لا يتم ادعاء نجاح Firmware build دون HAL ومشروع اللوحة الحقيقي.
- لا تستخدم structs مباشرة على السلك.
- كل DMA يجب أن يملك completion/error/timeout contract.
- المسار القديم لا يدخل البناء الافتراضي.

## Review Focus

- عدم تمرير buffer ADC قبل اكتمال DMA.
- توافق prototypes بين caller/implementation.
- رفض/تجميع إطارات K210 المجزأة والتالفة.
- عدم ربط raw BLE القديم مع V1.
- بقاء اختبارات Host الحالية ناجحة.

## Tasks

### Task 1: توحيد واجهات Core والـ health state

- إضافة headers موحدة لـ DSP وADS1256 وRM3100 وBLE/communication.
- إضافة `current_state` إلى `Hadeed_System_Health_t` أو تعديل callers إلى النوع المعتمد.
- تعديل `main.c` ليستعمل `Hadeed_Analyze_SMF(target, raw_buffer)` و`Hadeed_Fuse_Sensors_Logic(&target)`.
- استبدال `ADS1256_Check/RM3100_Check` بـ `Ping` عبر headers.
- استدعاء init functions الموجودة بعد تهيئة peripherals، مع إبقاء USART2/ HAL ضمن مشروع اللوحة.

### Task 2: عقد ADS1256 DMA

- تغيير burst API إلى مخزن bytes/status واضح.
- إضافة `ADS1256_Is_Burst_Complete`, `ADS1256_Get_Burst_Status` أو API مكافئ موثق.
- عدم قراءة العينة في PulseTask قبل completion.
- إضافة unpack/sign-extension helper قابل لاختبار Host.
- عدم تعديل pin/clock/MCU-specific code غير الموثق.

### Task 3: H7 incremental frame parser

- إضافة parser C مستقل إلى `protocol/` أو `Drivers/Custom/`.
- دعم chunks، noise، multiple frames، CRC failure، invalid length، buffer cap.
- توصيل callback به بدل تمرير DMA chunk مباشرة إلى decoder.
- إضافة اختبارات C/Host للحالات السابقة.

### Task 4: عزل legacy paths

- إضافة README/manifest يوضح أن `src/` و`Hadeed_Core/` legacy وغير داخلين في build.
- عدم حذفها قبل مراجعة المستخدم، لكن منع استعمالها كمرجع إنتاجي.
- تحديث التوثيق المتناقض.

### Task 5: Verification

- `pytest -q`.
- `compileall`.
- strict C protocol/parser tests.
- `git diff --check`.
- توثيق ما لم يُختبر بسبب غياب HAL/MCU.
