# Hadeed Platform — الخارطة المعمارية الكاملة من الصفر إلى البيع
## النسخ الأربع V0 / V1 / V2 / V3 — كل مرحلة، كل مشكلة، كل حلّ

**المهندس المسؤول:** Manus AI، خبير تصميم منتجات Tier-1  
**الإصدار:** v3.0 — الإصدار الموحّد النهائي  
**التاريخ:** 5 مايو 2026  
**الأساس:** كل الوثائق السابقة (12 ملف) + الأفكار الـ160 + المشاكل الـ23 + نظام الترقية المعياري + الميزات القاتلة الـ10.

---

## المقدّمة الإستراتيجية

هذا المستند هو **Master Plan** المرجعي للمشروع كاملاً. يُغطّي كل نسخة من النسخ الأربع عبر **9 مراحل تطوير** (من Concept إلى Aftersales)، مع تحديد المشاكل المتوقّعة في كل مرحلة وحلولها العملية. كل مرحلة تَرِث من سابقتها، وكل نسخة تَرِث من سابقتها معمارياً.

الفلسفة: **منصّة موحّدة، أربع تجهيزات، ترقية معيارية**. نفس الـ Mainboard الفيزيائية في كل النسخ، الفرق هو الـ DNP (Do Not Populate) في النسخ الأرخص و**Add-on Boards** في الأغلى.

---

## القسم الأول: المراحل التسع لتطوير أيّ نسخة

كل نسخة تمرّ بنفس التسلسل:

| # | المرحلة | المخرجات | المدّة المتوسّطة |
|---|---|---|---|
| **M1** | Concept & Specs | وثيقة PRD + ميزات + سعر مستهدف | 2 أسابيع |
| **M2** | Schematic Design | KiCad Schematic + Simulation | 4 أسابيع |
| **M3** | PCB Layout | Gerber Files + 3D STEP | 3 أسابيع |
| **M4** | Prototype Build (DVT1) | 5 وحدات للاختبار | 4 أسابيع |
| **M5** | Firmware & App | كود STM32 + ESP32 + Android | 12 أسبوعاً (متوازٍ مع M4) |
| **M6** | EVT/DVT/PVT Testing | 3 جولات اختبار + إصلاح | 8 أسابيع |
| **M7** | Certifications (CE/FCC/RoHS) | شهادات + EMC Test | 6 أسابيع |
| **M8** | Mass Production | 1000+ وحدة من JLCPCB أو مصنع صيني | 8 أسابيع |
| **M9** | Sales & Aftersales | تسليم، دعم، تحديثات OTA، Repair | مستمرّ |

**المدّة الإجمالية لإطلاق نسخة من الصفر: 6-9 أشهر**.

---

## القسم الثاني: V0 — Workshop Mule (تجريبي مختبر)

### الهدف الوحيد
إثبات أن سلسلة AFE (مضخّم AD8429 → AD797 → ADC) تعمل وتحقّق Noise Floor < 50 nV/√Hz قبل صرف دولار واحد على PCB.

### المعمارية المختصرة
```
ملف TX يدوي ── IRF740 ── PSU 12V
                  ↓
ملف RX يدوي ── AD8429 (Gain=100) ── AD797 (Gain=11) ── ADC داخلي STM32F411
                  ↓
              Black Pill F411 ── USB UART → PuTTY على PC
```

### القطع الأساسية (BOM المختصر)
| الصنف | العدد | السعر | المصدر |
|---|---|---|---|
| Black Pill STM32F411CEU6 | 1 | $4 | LCSC C2879521 |
| AD8429BRZ | 1 | $7 | LCSC C2890781 |
| AD797ARZ | 1 | $5 | LCSC C9016 |
| IRF740 | 1 | $0.6 | LCSC C152812 |
| TC4420 Gate Driver | 1 | $1.5 | LCSC C12876 |
| Linear PSU ±5V (B0505S) | 1 | $3 | LCSC C70094 |
| Perfboard 9×15cm | 1 | $1.2 | محلّي |
| 18650 + Holder | 2 | $4 | محلّي |
| ملف نحاس مطلي 0.5mm × 30m | 1 | $3 | محلّي |
| Resistors + Caps Kit | 1 | $2 | محلّي |
| **الإجمالي** | — | **~$31** | — |

### المراحل والمشاكل المتوقّعة

#### M1 — Concept (لا يحتاج)
V0 ليست منتجاً بل أداة. لا PRD مطلوب.

#### M2-M3 — لا توجد PCB
نتجاوز هذه المراحل ونذهب مباشرة لـ Perfboard.

#### M4 — Prototype Build
**المشاكل المتوقّعة:**
- **(P-V0-01) Noise Floor عالٍ جداً (>200 nV/√Hz)**  
  السبب الأشيع: GND مشوّش بسبب خلط AGND/DGND.  
  **الحلّ:** اعمل Star Ground واحد تحت AD797 بالضبط. كل الـ Returns تعود إليه عبر سلك مستقلّ. لا تشترك في track.
  
- **(P-V0-02) AD8429 يتذبذب (Oscillation)**  
  السبب: قصور في decoupling أو تقارب Output من Input.  
  **الحلّ:** ضع 100nF X7R و10µF Tantalum بين V+ و GND مباشرة على pin، وأبعد Output track عن Input track ≥10mm.

- **(P-V0-03) IRF740 يحترق بعد دقائق**  
  السبب: TC4420 لا يدفع تيار كافٍ، أو Heatsink غير موجود.  
  **الحلّ:** ضع Heatsink TO-220، تأكّد أن TC4420 يحصل على 12V، تحقّق من Snubber Diode (UF4007) عبر TX Coil.

- **(P-V0-04) إشارة RX مشوّهة Square بدل Decay**  
  السبب: AFE في حالة saturation بسبب reflected pulse من TX.  
  **الحلّ:** ضع Diode Clamp (1N4148 ×2 مضادّة) قبل AD8429، وأضف dead-time ≥20µs بعد TX قبل القراءة.

#### M5 — Firmware (مبسّط)
كود C تجريبي ~200 سطر للـ STM32F411:
- Timer 6 يولّد TX pulse 200µs كل 5ms
- ADC يقرأ 100 عينة بعد كل pulse
- UART يُرسل القيم خام للـ PC
- Python على PC يحسب Noise Floor + يرسم منحنى Decay

**مشكلة محتملة:** ADC داخلي 12-bit غير كافٍ لرؤية إشارات صغيرة. الحلّ: استخدم Oversampling 64x → effective 15-bit.

#### M6 — Testing
معايير القبول:
- Noise Floor RMS < 50 nV/√Hz عند 1kHz
- TX Pulse مستقرّ 12V/5A في 200µs
- AFE Gain فعلي = 1100 ±10%
- اكتشاف عملة 25mm نحاسية على عمق ≥15cm في الهواء

إن نجح V0 → ابدأ V1 فوراً. إن فشل → عدّل وكرّر.

### مدّة V0: **3-4 أسابيع**. التكلفة الكلية: **$50** (قطع + أدوات استهلاكية).

---

## القسم الثالث: V1 Pioneer Plus — جهاز المنقّب المبتدئ

### الهدف التجاري
بيع 5000-7000 وحدة سنوياً بسعر $179. يحلّ مشكلة "أرخص جهاز PI حقيقي مع ميزات Confidence + Voice Coach عربي."

### المعمارية المعمّقة

```
┌─────────────────────────────────────────────────┐
│  Mainboard موحّد Hadeed-MB-001 (نفس PCB لكل V1)│
├─────────────────────────────────────────────────┤
│  STM32F411CEU6 @ 100MHz (المعالج الوحيد)        │
│  ├── ADC داخلي 12-bit + Oversampling 64x = 15-b │
│  ├── DAC 12-bit للـ DAC Reference                │
│  ├── 2× UART (Debug + Voice Module)              │
│  ├── 1× I2C (HDC2080 + Smart-Coil EEPROM)        │
│  ├── 1× SPI (OLED 1.3")                          │
│  └── 1× I2S (PCM5102A للـ Voice)                 │
│                                                  │
│  AFE Chain:                                      │
│  Coil RX → AD8429 (G=100) → AD797 (G=11)         │
│           → Anti-Alias 5kHz → STM32 ADC          │
│                                                  │
│  TX Driver:                                      │
│  STM32 GPIO → TC4420 → IRF740 → TX Coil          │
│                                                  │
│  Power Tree:                                     │
│  18650 ×2 (7.4V) → MP1584 Buck → +5V             │
│                  → AMS1117 → +3.3V               │
│                  → B0505S Isolated → ±5V_ANA     │
│                                                  │
│  UI:                                             │
│  OLED 1.3" SH1106 + 4 buttons + 12× WS2812 LED  │
│  Buzzer 12mm + Vibration motor 8mm               │
│                                                  │
│  Storage: µSD card slot 4GB                      │
│                                                  │
│  Add-on Connector (DNP في V1): M.2 Edge ×3      │
└─────────────────────────────────────────────────┘
```

### قائمة القطع الكاملة V1 (BOM v3)

| الفئة | المرجع | القطعة | LCSC | الكمية | السعر |
|---|---|---|---|---|---|
| MCU | U1 | STM32F411CEU6 | C150107 | 1 | $4.20 |
| AFE | U2 | AD8429BRZ | C2890781 | 1 | $7.10 |
| AFE | U3 | AD797ARZ | C9016 | 1 | $5.30 |
| TX | Q1 | IRF740PBF | C152812 | 1 | $0.65 |
| TX | U4 | TC4420CPA | C12876 | 1 | $1.55 |
| Power | U5 | MP1584EN | C89358 | 1 | $0.42 |
| Power | U6 | AMS1117-3.3 | C6186 | 1 | $0.13 |
| Power | U7 | B0505S-1WR3 | C70094 | 1 | $2.95 |
| Audio | U8 | PCM5102A | C99715 | 1 | $1.85 |
| Sensor | U9 | HDC2080 (Salt mode) | C403093 | 1 | $1.20 |
| Sensor | U10 | BQ27441 (Fuel Gauge) | C167282 | 1 | $1.50 |
| Display | U11 | OLED SH1106 1.3" | DFR0648 | 1 | $4.80 |
| Indicator | LED1-12 | WS2812 5050 | C114586 | 12 | $0.80 |
| Audio | BZ1 | Magnetic Buzzer 12mm | C95298 | 1 | $0.30 |
| Haptic | M1 | Vibration motor 8mm | محلّي | 1 | $0.45 |
| Battery | BT1-2 | 18650 3500mAh | محلّي | 2 | $6.00 |
| Storage | J1 | µSD slot | C91145 | 1 | $0.55 |
| Smart-Coil | EE1 | 24LC02 EEPROM | C5294 | 1 | $0.30 |
| Coil | TX/RX | ملفات نحاس مُلفوفة محلّياً | محلّي | 1 set | $5.00 |
| Mechanics | — | ABS housing IP54 + Carbon Shaft | محلّي | 1 | $14.00 |
| PCB | — | 4-layer 100×60mm JLCPCB (لكنّ V1 يستخدم 2-layer لتوفير) | JLCPCB | 1 | $3.50 |
| Misc | — | Resistors + Caps + Connectors | محلّي | — | $4.00 |
| **الإجمالي** | | | | | **~$66.55** |

**+ تجميع وتوصيل وضمان:** ~$25 → التكلفة النهائية **$92** بهامش 49% على سعر $179.

### المراحل والمشاكل المتوقّعة

#### M1 — Concept (2 أسابيع)
**المشاكل:**
- **(P-V1-M1-01) إغراء إضافة ميزات أكثر من اللازم**  
  **الحلّ:** حدّد 8 ميزات قاتلة فقط: Confidence Meter، Voice Coach عربي، 5 أوضاع جاهزة، Discreet Mode، Treasure Diary (تطبيق)، LED Pinpoint Ring، Battery قابلة للاستبدال، Salt Compensation. أيّ ميزة إضافية تأخذ من ميزانية V2.

#### M2 — Schematic (4 أسابيع)
**المشاكل:**
- **(P-V1-M2-01) AFE Stability في PCB أصعب من Perfboard**  
  **الحلّ:** ارسم AGND و DGND كنُسج (Pours) منفصلة في L2/L3، اربطها بـ Star Point واحد تحت AD797 عبر Ferrite Bead 600Ω.

- **(P-V1-M2-02) MP1584 يُولّد ضجيج EMI عالي يُلوّث AFE**  
  **الحلّ:** ضع MP1584 على بُعد ≥20mm من AFE، أحطه بـ Guard Ring، وضع π-Filter (10µH + 2× 22µF) على خرجه قبل أن يُغذّي أيّ شيء آخر.

- **(P-V1-M2-03) USB Programming Connector يُسبّب ESD**  
  **الحلّ:** USLC6-4 TVS Array بـ $0.5 عبر D+/D-/VBUS.

#### M3 — PCB Layout (3 أسابيع)
**المشاكل:**
- **(P-V1-M3-01) 100×60mm ضيقة لتجنّب TX Coil noise على RX**  
  **الحلّ:** ضع TX driver في زاوية، AFE في الزاوية المقابلة قطرياً، بينهما Cu Pour متّصل بـ AGND.

- **(P-V1-M3-02) Edge Connectors للـ Add-on في V1 تأخذ مساحة دون استخدام**  
  **الحلّ:** ضع Footprint للـ M.2 Edge ولكن DNP (لا تركّب الموصل في V1، يُضاف عند الترقية بـ Hot-Air rework).

- **(P-V1-M3-03) WS2812 LEDs تستهلك تيار كبير عند 12 حبّة**  
  **الحلّ:** اجعل التيار limited عبر FET switch، شغّلهم فقط عند Pinpoint mode.

#### M4 — Prototype Build (DVT1) (4 أسابيع)
**المشاكل:**
- **(P-V1-M4-01) عينة JLCPCB تأتي بأخطاء Solder Mask فوق Test Points**  
  **الحلّ:** تحقّق ثلاث مرّات من ملف Solder Mask قبل الإرسال، استخدم opening مربّع بدل دائري.

- **(P-V1-M4-02) HDC2080 لا يستجيب على I2C**  
  **السبب الشائع:** Pull-ups مفقودة على SDA/SCL.  
  **الحلّ:** 4.7kΩ pull-ups إجبارية حتى لو كانت داخلية في MCU.

- **(P-V1-M4-03) Battery 18650 holder ضعيف ميكانيكياً ويفصل عند الاهتزاز**  
  **الحلّ:** استخدم Keystone 1042 spring contacts بدل البلاستيك الصيني الرخيص.

#### M5 — Firmware & App (12 أسبوعاً)
**المشاكل:**
- **(P-V1-M5-01) Confidence Meter يحتاج baseline data**  
  **الحلّ:** اجمع 1000 sample من 50 موقع مختلف في المغرب، السعودية، مصر، الجزائر، اعملها dataset مرجعي مدمج في الـ Firmware.

- **(P-V1-M5-02) Voice Coach عربي يحتاج ملفات صوتية ضخمة**  
  **الحلّ:** استخدم Opus codec 16kHz, 32kbps. 100 عبارة ≈ 4MB → تدخل في µSD أو internal flash.

- **(P-V1-M5-03) BLE BLE pairing معقّد للمستخدم العادي**  
  **الحلّ:** NFC tag بـ $0.3 على ظهر الجهاز، اللمسة تُنشّط Quick Pair في تطبيق Hadeed.

#### M6 — Testing (8 أسابيع)
**اختبارات إجبارية:**
- Noise Floor field test في 5 مواقع مختلفة
- اكتشاف عملات معدنية 1g, 5g, 10g, 25g على أعماق 5-50cm
- Confidence Meter accuracy ≥85%
- Battery life real-world ≥18 ساعة
- IP54 spray test (15 دقيقة من كل اتجاه)
- Drop test 1m على الإسمنت

**المشاكل الشائعة:**
- **(P-V1-M6-01) Confidence Meter يكذب في التربة المالحة**  
  **الحلّ:** فعّل Salt Mode تلقائياً عبر HDC2080 + ADC reading لمحلول التربة.

#### M7 — Certifications (6 أسابيع)
- CE Mark (EMC + LVD): ~$2500 لجولة واحدة في مختبر معتمد
- FCC Part 15B (في حالة الإطلاق في الأمريكتين)
- RoHS (سهلة، إعلان self-declaration)

**مشكلة شائعة:** EMI emissions تتجاوز الحدّ في 30-100MHz بسبب MP1584. **الحلّ:** Common Mode Choke + Faraday Cage على الـ Buck.

#### M8 — Mass Production (8 أسابيع)
**المشاكل:**
- **(P-V1-M8-01) JLCPCB لديها MOQ 5 للقطع، بعض القطع غير متوفرة**  
  **الحلّ:** Sourcing ثاني من LCSC أو Mouser، احتياطي 20% من الـ BOM.

- **(P-V1-M8-02) معايرة كل وحدة تأخذ وقتاً طويلاً (15 دقيقة لكل وحدة)**  
  **الحلّ:** Test Jig آلية تعاير 10 وحدات بالتوازي. تكلفة Jig ~$2000، توفّر 60% من زمن الإنتاج.

#### M9 — Sales & Aftersales
- **OTA Updates** لكل سنة (Firmware يتطوّر، الجهاز يتحسّن)
- **Repair Center** في كل دولة عربية كبرى
- **Buyback Program** بعد سنتين (نشتري الجهاز القديم بـ 30% ونمنح خصم على V2)
- **Customer Education** عبر Hadeed Academy المجاني

### الجدول الزمني الكامل لـ V1
**6 أشهر** من بدء M1 إلى أوّل وحدة مباعة. **الميزانية الإجمالية للتطوير: $35,000** (PCB prototypes + Tools + Cert + Lab + 200 hrs engineering).

---

## القسم الرابع: V2 Hunter Pro — حصان عمل المنقّب الجاد

### الهدف التجاري
بيع 2000-3000 وحدة سنوياً بسعر $499. يحلّ مشكلة "أوّل جهاز هجين PI + Mag Gradiometer في الفئة المتوسطة."

### المعمارية المعمّقة

```
┌──────────────────────────────────────────────────────┐
│  Mainboard موحّد + Add-on Boards:                    │
│  AB-01 (Mag Gradiometer) + AB-02 (GPS+LTE) +         │
│  AB-08 (Premium TFT Display)                          │
├──────────────────────────────────────────────────────┤
│  STM32H743VIT6 @ 480MHz (مرقّى من F411)              │
│  ESP32-C3 mini للـ BLE 5.0 + WiFi (مساعد)            │
│                                                       │
│  AFE Chain موسّع:                                    │
│  Coil RX → AD8429 → AD797 → ADS1256 24-bit ADC       │
│  + PGA281 Auto-Gain (1×, 10×, 100×, 1000×)           │
│                                                       │
│  Mag Sensors (AB-01):                                │
│  RM3100 ×2 على ذراع 50cm (Gradiometer)               │
│  + MPU9250 IMU للتعويض الحركي                        │
│                                                       │
│  GPS (AB-02):                                        │
│  Quectel L86 GNSS (GPS+GLONASS+Beidou)               │
│                                                       │
│  TX Driver محسّن:                                    │
│  STM32 → TC4420 → IRF740 (×2 متوازي للقدرة)         │
│  + AQY210EH SSR للـ Active Damping                   │
│                                                       │
│  Power Tree احترافي:                                 │
│  LiFePO4 4S 4000mAh → TPS62130A Buck → +5V          │
│                     → TPS7A4700 LDO → +3.3V_ANA     │
│                     → B0505S → ±5V_ANA               │
│                     → BQ24650 MPPT (Solar ready)    │
│                                                       │
│  UI Premium (AB-08):                                 │
│  TFT 2.4" Color 320×240 + Touch resistive            │
│  + 8 capacitive buttons + 12× WS2812 + Buzzer       │
│  + Vibration motor (heavy duty)                     │
│                                                       │
│  Storage:                                            │
│  µSD 16GB + W25Q128 (Flash للـ Voice + Maps)        │
│                                                       │
│  3× Add-on Connectors نشطة                          │
└──────────────────────────────────────────────────────┘
```

### قائمة القطع الإضافية لـ V2 (فوق V1)

| الفئة | القطعة | LCSC | السعر |
|---|---|---|---|
| MCU | STM32H743VIT6 | C112388 | $11.50 |
| Companion | ESP32-C3 mini | C2913203 | $1.80 |
| ADC | ADS1256IDBT | C50867 | $14.20 |
| PGA | PGA281AIPW | C507598 | $9.50 |
| SSR | AQY210EH | C51068 | $2.30 |
| Sensors | RM3100 ×2 + breakout | DigiKey | $54.00 |
| IMU | MPU9250 | C113682 | $3.50 |
| GPS | Quectel L86 | محلّي | $9.00 |
| Power Mgmt | TPS62130A | C97527 | $1.85 |
| Power LDO | TPS7A4700RGW | C171024 | $4.20 |
| Battery Mgmt | BQ24650 | C36423 | $3.10 |
| Display | TFT 2.4" Color SPI | محلّي | $7.50 |
| Storage Flash | W25Q128JV | C97521 | $1.40 |
| Connectors | M.2 Edge ×3 | C146747 | $1.50 |
| Coils Set | DD 11" + 6" Sniper | محلّي | $35.00 |
| Mechanics | IP65 Composite + Carbon Shaft | محلّي | $28.00 |
| **زيادة BOM:** | | | **+$188.35** |

**التكلفة النهائية V2:** $66.55 (V1 base) + $188.35 = **$254.90**. بهامش 49% على سعر $499. مع تجميع ومعايرة وضمان: $310 → هامش فعلي 38%.

### المراحل والمشاكل المتوقّعة الإضافية

#### M1 — Concept
- **(P-V2-M1-01) إغراء إضافة كاميرا K210 إلى V2**  
  **الحلّ:** قاوم! K210 يبقى لـ V3 فقط. تأكيد القاعدة: V2 لا يستهدف ميزات الرؤية، يستهدف Mag + GPS + Gradiometer.

#### M2 — Schematic
- **(P-V2-M2-01) ADS1256 SPI clock يلوّث AFE**  
  **الحلّ:** SPI clock ≤ 1.92MHz، DRDY interrupt-driven بدل polling. اعزل ADS1256 SPI bus عن MCU SPI الباقي.

- **(P-V2-M2-02) PGA281 Footprint غير مدعوم في KiCad standard library**  
  **الحلّ:** ارسم Footprint يدوياً بناءً على datasheet (TSSOP-24 7.6×4.4mm)، تحقّق من 3D model.

- **(P-V2-M2-03) RM3100 على Mainboard يلتقط noise من Switching PSU**  
  **الحلّ:** ركّب RM3100 على daughterboard 15×15mm متّصل بـ FFC cable 30cm، يبتعد عن PSU.

#### M3 — PCB Layout
- **(P-V2-M3-01) 4 PCBs (Main + AB-01 + AB-02 + AB-08) تحتاج Mechanical alignment دقيق**  
  **الحلّ:** صمّم Tooling Holes 3.0mm في كل لوحة، Jig بلاستيك يجمعهم بدقّة 0.1mm.

- **(P-V2-M3-02) M.2 Edge connector يحتاج 8mm من حافة PCB**  
  **الحلّ:** خصّص 25mm من جانب Mainboard للـ Add-on slots (ضحّ بـ 25% من المساحة).

#### M5 — Firmware
- **(P-V2-M5-01) Plug-and-Play detection للـ Add-ons**  
  **الحلّ:** كل Add-on فيه EEPROM 24LC02 على عنوان ثابت 0x50/0x51/0x52. عند Boot، MCU يفحص العناوين الثلاثة ويُحمّل drivers مناسبة.

- **(P-V2-M5-02) Target Imaging يحتاج FFT 1024-point في real-time**  
  **الحلّ:** استخدم CMSIS-DSP `arm_rfft_fast_f32` على M7 core، يُنفّذ في 0.4ms على H743.

- **(P-V2-M5-03) GPS L86 يحتاج 30 ثانية لـ first fix**  
  **الحلّ:** فعّل AGPS عبر BLE من الهاتف → first fix في 3 ثوان.

#### M6 — Testing
- اختبار Mag Gradiometer في موقع خالٍ من الحديد (داخل صالة فارغة)
- اختبار GPS في 3 بيئات: مدينة، ريف، صحراء
- اختبار 5G interference rejection

### الجدول الزمني الكامل لـ V2
**8 أشهر**. **الميزانية: $75,000**.

---

## القسم الخامس: V3 Sovereign Elite — العين تحت الأرض

### الهدف التجاري
بيع 800-1500 وحدة سنوياً بسعر $1099. يحلّ مشكلة "ينافس Garrett ATX و Minelab CTX 3030 بنصف السعر، ويتفوّق عليهما بميزات لا توجد فيهما."

### المعمارية المعمّقة

```
┌─────────────────────────────────────────────────────────┐
│  Mainboard موحّد + كل Add-on Boards (9 لوحات)         │
├─────────────────────────────────────────────────────────┤
│  STM32H7S7L8H6 @ 600MHz + Cortex-M4 @ 240MHz          │
│  + ESP32-C3 mini للـ BLE                              │
│  + ESP32-S3 للـ WiFi 6 + Voice "Hey Hadeed"           │
│                                                          │
│  AFE 4-Channel متوازي (AB-05):                        │
│  4× (AD8429 + AD797) + 4× PGA281                       │
│  + ADS1256 ×2 (8 قنوات إجمالاً)                       │
│  + AD7768-4 (ADC اختياري للأبحاث)                     │
│                                                          │
│  Mag Tetrahedron (AB-06):                              │
│  RM3100 ×4 في تكوين Tetrahedron                       │
│  + MPU9250 IMU                                          │
│  + LIS2DH12 Backup Accel                                │
│                                                          │
│  GPS Premium (AB-02 Pro):                              │
│  u-blox NEO-M9N Multi-GNSS                             │
│  + Antenna Active 25dB                                 │
│                                                          │
│  AI Vision (AB-04):                                    │
│  K210 + OV5640 Camera 5MP                              │
│  + 3× LED 1W flash للحفر الليلي                       │
│                                                          │
│  Connectivity (AB-07):                                 │
│  SIM7080G LTE-M + GPS + 4G fallback                   │
│  + Solar charge port (DC 9-15V)                        │
│  + Wireless headphone aptX                             │
│                                                          │
│  TX Driver احترافي:                                    │
│  IRF740 ×4 متوازي + Snubber                            │
│  + Active Damping AQY210EH ×2                          │
│  + Soil EC AC (Wien Bridge OPA2134 + INA128)          │
│                                                          │
│  Power Tree متطوّر:                                    │
│  LiFePO4 8000mAh + Solar MPPT BQ24650                 │
│  + LTC3588 Energy Harvesting from TX flyback          │
│                                                          │
│  UI Premium:                                            │
│  TFT 3.5" Touch Capacitive 480×320                     │
│  + AMOLED option للـ Pro version                       │
│  + 12× WS2812 RGB Pinpoint Ring                        │
│  + Heavy-duty Vibration motor                          │
│  + Speaker 1W mono                                     │
│                                                          │
│  Storage:                                               │
│  eMMC 4GB on-board + µSD 64GB + W25Q128 boot          │
│                                                          │
│  Anti-Theft:                                            │
│  Fingerprint sensor R503 + GPS+LTE Tracking           │
│                                                          │
│  External:                                              │
│  4 Coils في الصندوق + Drone Companion port (optional) │
└─────────────────────────────────────────────────────────┘
```

### قائمة القطع الإضافية لـ V3 (فوق V2)

| الفئة | القطعة | LCSC | السعر |
|---|---|---|---|
| MCU Premium | STM32H7S7L8H6 | C5404474 | $19.50 |
| WiFi 6 | ESP32-S3-WROOM | C2913201 | $3.20 |
| AFE ×3 إضافي | 3× (AD8429 + AD797 + PGA281) | — | $66.30 |
| ADC ثاني | ADS1256IDBT | C50867 | $14.20 |
| ADC أبحاث | AD7768-4BSTZ (optional) | DigiKey | $42.00 |
| Mag إضافي | RM3100 ×2 إضافي | DigiKey | $54.00 |
| GPS Premium | u-blox NEO-M9N | DigiKey | $36.00 |
| Antenna | Active GPS 25dB | محلّي | $4.50 |
| AI Vision | K210 (Sipeed M1) + OV5640 | محلّي | $22.00 |
| LED | 6× LED 1W high power + driver | محلّي | $4.00 |
| LTE | SIM7080G + SIM holder | محلّي | $15.00 |
| Solar | Solar panel 5W foldable + cable | محلّي | $14.00 |
| Wireless Audio | aptX BT module | محلّي | $5.00 |
| Soil EC AC | OPA2134 + INA128 + 4× SSR | LCSC | $9.50 |
| Energy Harvesting | LTC3588 + components | LCSC | $4.50 |
| Display | TFT 3.5" Capacitive Touch | محلّي | $18.00 |
| Storage | eMMC 4GB + µSD 64GB | محلّي | $12.00 |
| Anti-Theft | R503 Fingerprint sensor | محلّي | $7.50 |
| Speaker | 1W mono speaker + amp | محلّي | $3.00 |
| Coils Premium | 4 ملفات (DD 13" Smart, 17" Deep, 5" Sniper, 15" Folding) | محلّي | $135.00 |
| Mechanics | IP68 Fiberglass + Premium Carbon | محلّي | $58.00 |
| Wristband | Haptic Feedback BLE Wristband | محلّي | $18.00 |
| **زيادة BOM:** | | | **+$565.20** |

**التكلفة النهائية V3:** $254.90 (V2 base) + $565.20 = **$820.10**. بهامش ضعيف 25% على سعر $1099. الواقع: يجب رفع السعر إلى **$1299** أو خفض BOM. أوصي ببيع V3 Pro كـ Bundle بـ $1399 مع 4 ملفات + Wristband + Solar Panel + Hadeed Academy Pro سنة كاملة.

### المراحل والمشاكل المتوقّعة الإضافية

#### M1
- **(P-V3-M1-01) خطر السقوط في Feature Creep**  
  **الحلّ:** قاعدة "30% Rule" — V3 يجب أن يحتوي 30% فقط من ميزات V2 إضافة. أيّ ميزة تتجاوز هذه النسبة تذهب لـ V4 المستقبلية.

#### M2
- **(P-V3-M2-01) STM32H7S BGA 196-pin يحتاج PCB 6-layer**  
  **الحلّ:** خصّص $20 إضافية لكل PCB لـ 6-layer. JLCPCB JLC04161H-7628.

- **(P-V3-M2-02) 4 RM3100 في Tetrahedron يحتاجون calibration متبادل دقيق**  
  **الحلّ:** Software calibration باستخدام Tikhonov regularization، يأخذ يوم كامل لكل وحدة.

#### M3
- **(P-V3-M3-01) كثرة الـ Add-ons تخلق مشكلة EMI shielding**  
  **الحلّ:** كل Add-on في faraday box منفصل بـ Wurth 365130 series، تكلفة $1.5 لكل box.

#### M5
- **(P-V3-M5-01) AI 16-class model حجمه >2MB، لا يدخل في STM32 internal flash**  
  **الحلّ:** خزّنه في eMMC، حمّله إلى RAM عند Boot (DTCM 128KB كافي للـ inference).

- **(P-V3-M5-02) "Hey Hadeed" wake word يحتاج 100% accuracy**  
  **الحلّ:** TFLite micro model مدرّب على 10,000 sample بـ 5 لهجات عربية مختلفة. False Acceptance Rate < 1/24h.

- **(P-V3-M5-03) 3D Volumetric Imaging يحتاج معالجة ضخمة**  
  **الحلّ:** افعلها على الهاتف (تطبيق) بدلاً من الجهاز. Hadeed يرسل raw data، الهاتف يفعل reconstruction.

#### M6
- اختبار Tetrahedron Mag في 3 مواقع تاريخية معروفة (يجب اكتشاف كنوز معروفة)
- اختبار AI accuracy مع 200 عيّنة معدنية مختلفة
- IP68 underwater test (1m / 30 min)
- Drop test 2m
- LTE-M coverage test في 5 دول

#### M7
- شهادات إضافية: PTCRB للـ LTE-M، SAR (إن كان قريب من جسم المستخدم)

#### M8
- Mass production V3 يجب أن يكون في مصنع متخصّص (ليس JLCPCB)
- Allwiner أو Foxconn small-batch line بـ MOQ 500 وحدة

### الجدول الزمني الكامل لـ V3
**12-14 شهراً**. **الميزانية: $180,000**.

---

## القسم السادس: نظام الترقية المعياري بالتفصيل

### المعمارية الكهربائية للـ Add-on Connector

كل Add-on Slot على Mainboard هو **M.2 NGFF Style** بـ 75 pin فعلي مستخدم:
- 8 pin Power (+3.3V, +5V, ±5V_ANA, GND ×3)
- 4 pin SPI (MISO, MOSI, SCK, CS)
- 4 pin I2C (×2 buses: SDA1/SCL1, SDA2/SCL2)
- 8 pin GPIO عام (للـ IRQ، Reset، Add-on-specific)
- 2 pin UART (TX, RX)
- 4 pin USB (D+, D-, VBUS, GND) للـ USB add-ons
- 3 pin Identification (EEPROM SDA, SCL, WP)
- باقي pins: حشوة GND للـ shielding

### Plug-and-Play Boot Sequence

```
1. Boot → MCU power ON
2. MCU يقرأ EEPROM في كل add-on slot عبر I2C
3. كل add-on يُعرّف نفسه بـ: type, version, capabilities
4. MCU يُحمّل drivers مناسبة من eMMC/Flash
5. MCU يُهيّئ BLE service مع الميزات الجديدة
6. التطبيق يكتشف الميزات تلقائياً ويُظهرها للمستخدم
```

### مثال: ترقية V1 إلى V2 خطوة بخطوة

1. المستخدم يشتري حزمة الترقية AB-01+AB-02+AB-08 من متجر Hadeed بـ **$205**.
2. الحزمة تأتي مع: 3 Add-on Boards + 3 M.2 Edge connectors (للحام إلى Mainboard) + housing extension + توجيهات.
3. المستخدم يأخذ الجهاز إلى Hadeed Service Center محلّي (في عمّان، الرياض، القاهرة، الدار البيضاء، إلخ).
4. الفنّي يلحم الـ Edge connectors في 30 دقيقة، يُركّب الـ Boards، يُحدّث Firmware عبر USB.
5. يُسلّم الجهاز للمستخدم خلال ساعة. الميزات الجديدة تعمل فوراً.

**كلفة الخدمة:** $20 (مدمجة في سعر الحزمة).

### الفائدة التسويقية

- المستخدم لا يفقد جهازه القديم → ولاء عالي
- المستخدم يدفع تدريجياً → ضغط أقل على ميزانيته
- Hadeed تكسب 3 مرّات: V1 + Add-ons + Service
- بناء قاعدة عملاء طويلة الأمد

---

## القسم السابع: ملخّص المخاطر والحلول

### المخاطر الأعلى أولوية

| المخاطرة | الاحتمال | الأثر | الحلّ |
|---|---|---|---|
| فشل Confidence Meter في حقل حقيقي | متوسط | عالٍ | اختبار Beta مع 50 مستخدم قبل الإطلاق |
| نضوب قطعة AD8429 من السوق | منخفض | عالٍ | Second source: INA128 (خسارة 15% أداء) |
| مشاكل EMI تفشل CE | عالٍ | عالٍ جداً | Pre-compliance scan قبل المختبر الرسمي |
| Plug-and-Play لا يعمل بشكل موثوق | متوسط | عالٍ | اختبار 50 ترقية على prototypes قبل الإطلاق |
| Battery LiFePO4 supply chain | منخفض | متوسط | عقد طويل الأمد مع 2 موزّعين صينيّين |
| Voice Coach العربي بلهجة غير محبّبة | متوسط | منخفض | تسجيلات بـ 5 لهجات، خيار للمستخدم |
| السرقة الفكرية من المنافسين الصينيّين | عالٍ | متوسط | تسجيل Patents جوهرية + سرعة في الإطلاق |
| تعطّل الـ Cloud Service | منخفض | متوسط | Multi-region AWS + Offline-First design |

---

## القسم الثامن: الخارطة الزمنية الإجمالية للمشروع

| الشهر | الحدث | النسخة |
|---|---|---|
| 1-2 | V0 Workshop Mule على Perfboard | V0 |
| 3-4 | بدء V1 schematic + PCB | V1 |
| 5-6 | V1 prototype + firmware MVP | V1 |
| 7 | V1 Beta مع 50 مستخدم | V1 |
| 8-9 | V1 mass production + launch | V1 |
| 10-12 | بدء V2 (متوازٍ مع مبيعات V1) | V2 |
| 13-15 | V2 prototype + firmware | V2 |
| 16-17 | V2 mass production + launch | V2 |
| 18-20 | بدء V3 + R&D AI models | V3 |
| 21-24 | V3 prototype + 4 add-ons | V3 |
| 25-28 | V3 testing + certifications | V3 |
| 29-30 | V3 launch + Drone Companion R&D | V3 + V4 |

**عمر المشروع المتوقّع: 30 شهر للوصول إلى Steady State بثلاث نسخ.**

---

## الخلاصة

هذا المستند هو **Master Architecture** للمشروع الكامل. كل قرار تقني، كل قطعة، كل مشكلة متوقّعة، وكل حلّ موجود هنا. اتّبع هذه الخارطة بحذافيرها وستحصل على أفضل خط منتجات لكشف المعادن في السوق العربي والعالمي بنصف سعر المنافسين.

القرار الآن لك يا صديقي. ابدأ بـ V0 خلال أسبوع.
