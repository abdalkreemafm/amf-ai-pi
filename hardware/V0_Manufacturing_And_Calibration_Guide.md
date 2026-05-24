# خريطة تصنيع ومعايرة V0 — Workshop Mule
## دليل شامل لبناء النموذج التجريبي خطوة بخطوة، مع شرح كل قطعة وربطها وتصميمها الداخلي والخارجي

**النسخة:** V0 Engineering Bench Prototype  
**الكاتب:** Manus AI كخبير أجهزة كشف معادن من الفئة العالمية الأولى  
**الجمهور:** أنت — كمصمّم يبني أول نموذج للتحقق من المفاهيم  
**الزمن المتوقع:** 12–14 يوم عمل  
**التكلفة الإجمالية:** ~$25 (دون احتساب الأدوات المخبرية)

---

## 1. الفلسفة قبل أي شيء

> هذه النسخة لا تحاول أن تكون "جهاز كشف معادن جميل". هدفها الوحيد إثبات ثلاث حقائق:
> 1. سلسلة AFE تعمل وتعطي كسباً متوقعاً.
> 2. **Noise Floor < 50 nV/√Hz** على دخل قصير.
> 3. ملف TX يولّد نبضة Flyback نظيفة، ويُمكن قراءة منحنى الاضمحلال (Decay Curve) بعد Dead-Time صغير.

كل قرار لاحق في V1/V2/V3 يعتمد على نتائج هذه الثلاث القياسات. **لذلك لا تختصر الخطوات ولا تتساهل في تأريض النقطة النجمية.**

---

## 2. المخطط العام (System Block Diagram)

![Block Diagram](https://private-us-east-1.manuscdn.com/sessionFile/eFMjMi8kQvmf8Qo3QubYZg/sandbox/6hkneQwrSS7xGjCMg3tWkA-images_1777778066816_na1fn_L2hvbWUvdWJ1bnR1L3YwX2d1aWRlL2ltYWdlcy9EMV9ibG9ja19kaWFncmFt.png?Policy=eyJTdGF0ZW1lbnQiOlt7IlJlc291cmNlIjoiaHR0cHM6Ly9wcml2YXRlLXVzLWVhc3QtMS5tYW51c2Nkbi5jb20vc2Vzc2lvbkZpbGUvZUZNak1pOGtRdm1mOFFvM1F1YllaZy9zYW5kYm94LzZoa25lUXdyU1M3eEdqQ01nM3RXa0EtaW1hZ2VzXzE3Nzc3NzgwNjY4MTZfbmExZm5fTDJodmJXVXZkV0oxYm5SMUwzWXdYMmQxYVdSbEwybHRZV2RsY3k5RU1WOWliRzlqYTE5a2FXRm5jbUZ0LnBuZyIsIkNvbmRpdGlvbiI6eyJEYXRlTGVzc1RoYW4iOnsiQVdTOkVwb2NoVGltZSI6MTc5ODc2MTYwMH19fV19&Key-Pair-Id=K2HSFNDJXOU9YS&Signature=NdrUmzWE4AcK8PuCVGE85PiGiOnzxVKD~ajWEZ23TpGqD58KcCvKmzW1VUA1e-OMd8C~4slkALuL60PmBQ7~~TGtS3xC51xAScXz9UZAU6XRIav0oDkQw9CqtXQssSUV6-5oejt1dV47xmPvyFbrOpb5FhT75VUiNNmONx4JnCcsk4zvxjxPUkBPATKbKJV5cWJSXRA6dpYB2YaRukB32hm70btRHggIuzgENoCb~0agbVP0IqwjM8bFNxS5AXGd3qKj2-MgC9lGxFOyjIeegp5lvq9Ft-Xys0y41pVxx3GM940ugMeOq4jrzsYpRpTnHZJUpVqEolGOAFJ78EgNYA__)

**شرح المخطط:**
- **PSU مختبر**: يوفّر 12V لإرسال الـ TX، و+5V للـ Black Pill، و+5V معزولة لـ B0505S الذي يصنع ±5V للأنالوج.
- **Black Pill F411**: يولّد PWM 1kHz، عرض النبضة 300µs.
- **TC4420**: يقوّي إشارة PWM من 3.3V إلى 12V بتيار 6A لقيادة بوابة IRF740 بسرعة عالية.
- **IRF740 + ملف TX**: يضخ تيار ~3A في الملف ثم يقطع فجأة → Flyback 200V → التوهين السريع يرسل موجة كهرومغناطيسية تخترق الأرض.
- **ملف RX**: يلتقط الإشارة المرتدة (millivolts).
- **AD8429 → AD797**: يكبّر الإشارة ~11,000 مرة.
- **STM32 ADC**: يقرأ بعد Dead-Time (15µs) منحنى الاضمحلال ويرسله للحاسوب عبر USB CDC.

---

## 3. قائمة القطع الكاملة (Bill of Materials)

| # | القطعة | الوظيفة | الحزمة (Package) | الكمية | السعر | المتوفّر في |
|---|---|---|---|---|---|---|
| 1 | STM32F411 Black Pill | المعالج + ADC + UART | Module 53×21mm | 1 | $4 | علي اكسبرس / محلات إلكترونيات عدن |
| 2 | AD8429ARZ | مكبّر أجهزة منخفض الضوضاء | SOIC-8 | 1 | $5 | LCSC / DigiKey |
| 3 | AD797ANZ | مكبّر عملياتي فائق الدقة | DIP-8 (موصى به للـ V0) | 1 | $4 | LCSC |
| 4 | Resistor 6.81Ω 0.1% 1/4W | RG لكسب AD8429 | Through-Hole 0.25W | 1 | $0.5 | متجر محلي |
| 5 | Resistor 1kΩ + 10kΩ 1% | Gain stage 2 (AD797) | TH | 2 | $0.1 | محلي |
| 6 | Resistor 49.9Ω 1% | فلتر Anti-Alias | TH | 2 | $0.1 | محلي |
| 7 | Capacitor 1nF C0G + 100pF C0G | فلاتر الإشارة | TH | 4 | $0.2 | محلي |
| 8 | Capacitor 10µF + 100nF | تنعيم التغذية | TH/SMD | 8 | $0.4 | محلي |
| 9 | IRF740 | مفتاح TX | TO-220 | 1 | $1 | محلي |
| 10 | TC4420CPA | درايفر MOSFET | DIP-8 | 1 | $1 | LCSC |
| 11 | B0505S-1WR3 | عزل ±5V للأنالوج | SIP-4 | 1 | $1.5 | LCSC / محلي |
| 12 | Heat-shrink + Magnet wire 0.5mm | لف ملف TX | — | 1 | $2 | محلي |
| 13 | Magnet wire 0.2mm (200m) | لف ملف RX | — | 1 | $2 | محلي |
| 14 | Perfboard 7×9 cm | اللوحة | 2.54mm pitch | 1 | $1 | محلي |
| 15 | Female header 1×20 ×2 | لتركيب Black Pill | TH | 2 | $0.5 | محلي |
| 16 | Pin header 2.54mm | للموصلات | TH | 1 | $0.5 | محلي |
| 17 | TVS SMAJ12CA | حماية على ملف TX | DO-214 | 1 | $0.3 | LCSC |
| 18 | Adapter SOIC8→DIP8 | لتركيب AD8429 على Perfboard | — | 1 | $0.5 | علي اكسبرس |
| 19 | Twisted pair shielded cable 1m | لربط ملف RX | — | 1 | $1 | محلي |
| 20 | Solder, flux, lead-free 60/40 | اللحام | — | — | $1 | محلي |
| | **الإجمالي** | | | | **~$25** | |

**الأدوات المطلوبة (لا تُحتسب في التكلفة لأنها مخبرية):**
- مكواة لحام Hakko FX-888 أو ما يعادلها (≥40W).
- ملتيمتر Fluke 117 أو UNI-T UT139C.
- Oscilloscope ≥ 50MHz / 2 قناة.
- Bench PSU بمخارج 12V و 5V (ثنائي القناة موصى به).
- ST-Link V2 (USB Programmer للـ STM32).
- Tweezer + قاطع أسلاك + Wire stripper.

---

## 4. شرح كل قطعة بالصورة والتفاصيل

### 4.1 Black Pill STM32F411CEU6 — العقل
![Black Pill 3D](https://private-us-east-1.manuscdn.com/sessionFile/eFMjMi8kQvmf8Qo3QubYZg/sandbox/6hkneQwrSS7xGjCMg3tWkA-images_1777778066816_na1fn_L2hvbWUvdWJ1bnR1L3YwX2d1aWRlL2ltYWdlcy8wMl9ibGFja3BpbGxfM2Q.jpg?Policy=eyJTdGF0ZW1lbnQiOlt7IlJlc291cmNlIjoiaHR0cHM6Ly9wcml2YXRlLXVzLWVhc3QtMS5tYW51c2Nkbi5jb20vc2Vzc2lvbkZpbGUvZUZNak1pOGtRdm1mOFFvM1F1YllaZy9zYW5kYm94LzZoa25lUXdyU1M3eEdqQ01nM3RXa0EtaW1hZ2VzXzE3Nzc3NzgwNjY4MTZfbmExZm5fTDJodmJXVXZkV0oxYm5SMUwzWXdYMmQxYVdSbEwybHRZV2RsY3k4d01sOWliR0ZqYTNCcGJHeGZNMlEuanBnIiwiQ29uZGl0aW9uIjp7IkRhdGVMZXNzVGhhbiI6eyJBV1M6RXBvY2hUaW1lIjoxNzk4NzYxNjAwfX19XX0_&Key-Pair-Id=K2HSFNDJXOU9YS&Signature=t9so9BHYZjR7OUm07lSTHoU7OTlDZZy2RYeNIn1C2XAzsQ0JNsUOIz7IQAep9XME0ybh6DT9JAB2D8hhwJ33uwuYmSFAstS7bLSMJIjd04QtMeahvILZozRUgccRl~AjHShG7LzCRZIW6tgeKOEuhDM-OWh4dGzQlAFEOKGL2psQtBUb1WpWqh0Siwh9Cne4YrPeg8FiKzR6wXKpTlIf4NG~hz1FaT2ToQMxBDPHET~I7NRUIgLt1bRL671IUr86pzQsZtrJ5jmj8NpBgxVbZtaUb-YCjSh~i~S7Yrq-aHHutWRCxS7d1kvUyPEwNLpg9GXN2lhZTyk6jsHTmISUmA__)

**ما هي؟** لوحة تطوير صغيرة (53×21mm) تحوي معالج Cortex-M4 @100MHz، و512KB Flash، و128KB RAM. بها USB-C، ومنفذ SWD، وزر User+Reset، وكريستالة 25MHz.

**لماذا اخترناها لـ V0؟**
- رخيصة ($4)، تشحن من USB-C مباشرة.
- ADC داخلي 12-bit بسرعة 1MSPS كافٍ لإثبات المفهوم.
- لا تحتاج تصميم PCB.

**Pinout الكامل (الذي سنستخدمه):**

![Black Pill Pinout](https://private-us-east-1.manuscdn.com/sessionFile/eFMjMi8kQvmf8Qo3QubYZg/sandbox/6hkneQwrSS7xGjCMg3tWkA-images_1777778066816_na1fn_L2hvbWUvdWJ1bnR1L3YwX2d1aWRlL2ltYWdlcy8wMV9ibGFja3BpbGxfcGlub3V0.png?Policy=eyJTdGF0ZW1lbnQiOlt7IlJlc291cmNlIjoiaHR0cHM6Ly9wcml2YXRlLXVzLWVhc3QtMS5tYW51c2Nkbi5jb20vc2Vzc2lvbkZpbGUvZUZNak1pOGtRdm1mOFFvM1F1YllaZy9zYW5kYm94LzZoa25lUXdyU1M3eEdqQ01nM3RXa0EtaW1hZ2VzXzE3Nzc3NzgwNjY4MTZfbmExZm5fTDJodmJXVXZkV0oxYm5SMUwzWXdYMmQxYVdSbEwybHRZV2RsY3k4d01WOWliR0ZqYTNCcGJHeGZjR2x1YjNWMC5wbmciLCJDb25kaXRpb24iOnsiRGF0ZUxlc3NUaGFuIjp7IkFXUzpFcG9jaFRpbWUiOjE3OTg3NjE2MDB9fX1dfQ__&Key-Pair-Id=K2HSFNDJXOU9YS&Signature=cMP0F9OYrk0BknKuPe04qg8ycUwLT8GS9eqf44uQuXNVEaNsDygtX73M~j4oYmsxAqcCxIBXRl8aTlOy5gIubxeLw-0og6RXrxykVePSuku~9Txf079Y3UYjZCVrfynqA3GcJrQHWQsuAEGNrBNp~I4~Flzhsm3aLocRXHKZs~5OSpbqDVk-exluiwi8aC25RS2yZLe92MsiQvLDah8spLhDyej5WDY61sicqOxAedq7HsAb2I1bkDBt5euBD0i0BWXdZ6ndQsQSHPMAzeEXvFC79oc8~9zYnM7ezJsxsIyXryJUu9hWNdj~fscFokpkyyvZpNfcVAabAV9ojL9Txw__)

| Pin | الوظيفة في V0 | ملاحظة |
|---|---|---|
| PA8 | TX_PWM (output) | Timer1 CH1 → TC4420 |
| PA0 | ADC_IN (analog) | من خرج AD797 |
| PA9 / PA10 | UART1 TX/RX | للـ debug عبر USB-CDC |
| PA13 / PA14 | SWDIO / SWCLK | للبرمجة من ST-Link |
| 3V3 / GND | 3.3V regulated | يوفّرها AMS1117 على اللوحة |
| 5V | دخل/خرج 5V | نوصّل 5V بمحدد التيار |
| C13 | LED مدمج | للـ Heartbeat blink |

### 4.2 AD8429 — مضخّم الأجهزة (Front-end)
![AD8429 SOIC-8](https://private-us-east-1.manuscdn.com/sessionFile/eFMjMi8kQvmf8Qo3QubYZg/sandbox/6hkneQwrSS7xGjCMg3tWkA-images_1777778066816_na1fn_L2hvbWUvdWJ1bnR1L3YwX2d1aWRlL2ltYWdlcy8wM19BRDg0MjlfU09JQzg.jpg?Policy=eyJTdGF0ZW1lbnQiOlt7IlJlc291cmNlIjoiaHR0cHM6Ly9wcml2YXRlLXVzLWVhc3QtMS5tYW51c2Nkbi5jb20vc2Vzc2lvbkZpbGUvZUZNak1pOGtRdm1mOFFvM1F1YllaZy9zYW5kYm94LzZoa25lUXdyU1M3eEdqQ01nM3RXa0EtaW1hZ2VzXzE3Nzc3NzgwNjY4MTZfbmExZm5fTDJodmJXVXZkV0oxYm5SMUwzWXdYMmQxYVdSbEwybHRZV2RsY3k4d00xOUJSRGcwTWpsZlUwOUpRemcuanBnIiwiQ29uZGl0aW9uIjp7IkRhdGVMZXNzVGhhbiI6eyJBV1M6RXBvY2hUaW1lIjoxNzk4NzYxNjAwfX19XX0_&Key-Pair-Id=K2HSFNDJXOU9YS&Signature=qzhKN0~aRiXbqbyxMkizkSHAfE4utl5ynJfX7t7Uriuq7hRbW9599a4-rJ0JFsRcbtomQ-a32GPmb0DUcaFC4orYNMBzSpN8t413gtQcNlzQutuyMkPCVr5EDClveLYO15lcxWvXkSCJRvhkJ7gR1jXV~0Hr6NqgmlAFsX9qa7FDkQBFO-hc5CKcWrDJ3EP~6miojytD3OnB7RopeAvZ0jpkPEdDPh~dPeL0LBqAeASCT6eUdKS-sA~Fl8ArupxZJ1UKp3hngbadxE8jhaUl29hld4aLrXweI~iUzJwWjga3vENDMo5j9fvkmmiTVC-0-0tQqyhHEHqAhxDh-4omQA__)

**ما هو؟** Instrumentation Amplifier من Analog Devices، يعتبر **أهدأ مضخّم تجاري** (1 nV/√Hz @ 1kHz). يحوّل الإشارة التفاضلية الضعيفة (microvolts) من ملف RX إلى إشارة مفردة كبيرة.

**لماذا هو المفتاح؟** لأن إشارة معدن مدفون في الأرض تكون 1–10 µV. نحتاج كسب ≥1000× قبل أن نسلّمها لـ ADC.

**Pinout AD8429 (SOIC-8):**

```
        ┌─────U─────┐
   -IN ─┤1         8├─ +VS
   RG  ─┤2         7├─ Vout
   RG  ─┤3         6├─ REF
   +IN ─┤4         5├─ -VS
        └───────────┘
```

**كسب القناة:**
$$ G = 1 + \frac{6000}{R_G} $$

مع RG = 6.81Ω → G ≈ 882. (الكسب الفعلي ~880 لأن المعادلة الحقيقية أعقد قليلاً.)

**كيف نركّبه على Perfboard؟** عبر Adapter SOIC8→DIP8 (صورة في القسم 4.3 أدناه)، أو نشتري **AD8429 Module جاهز** بسعر $7 يحوي SMA + RG قابل للتعديل:

![AD8429 Module](https://private-us-east-1.manuscdn.com/sessionFile/eFMjMi8kQvmf8Qo3QubYZg/sandbox/6hkneQwrSS7xGjCMg3tWkA-images_1777778066816_na1fn_L2hvbWUvdWJ1bnR1L3YwX2d1aWRlL2ltYWdlcy8wNF9BRDg0MjlfbW9kdWxl.jpg?Policy=eyJTdGF0ZW1lbnQiOlt7IlJlc291cmNlIjoiaHR0cHM6Ly9wcml2YXRlLXVzLWVhc3QtMS5tYW51c2Nkbi5jb20vc2Vzc2lvbkZpbGUvZUZNak1pOGtRdm1mOFFvM1F1YllaZy9zYW5kYm94LzZoa25lUXdyU1M3eEdqQ01nM3RXa0EtaW1hZ2VzXzE3Nzc3NzgwNjY4MTZfbmExZm5fTDJodmJXVXZkV0oxYm5SMUwzWXdYMmQxYVdSbEwybHRZV2RsY3k4d05GOUJSRGcwTWpsZmJXOWtkV3hsLmpwZyIsIkNvbmRpdGlvbiI6eyJEYXRlTGVzc1RoYW4iOnsiQVdTOkVwb2NoVGltZSI6MTc5ODc2MTYwMH19fV19&Key-Pair-Id=K2HSFNDJXOU9YS&Signature=J-pZlXDWfkQu91sLijDlBnZ~~8u7dKKsw-EwMwTCTxzBHEhSR3SYJPvwHcic2TAOAvq4m4DicCRfrAw4EkGPfFz2iIQnNS1g3CyOJ7QBZm3Fe6XNTskcGnhkv48lepymGmvI~AiVsYz2OdtNd2Nw1PsimJ1CfFXNqa0R9sWW16X262wXwTXYeppx4ICq9hqX9SLfG1LKD6Ga4l9luwNDS0NO6Fj~YRzOvbpLzNzPg5QZGnbb4oEDrqagjzKEdmsnqtmMbiz8FCvmvN5zjLowm-DjzpUsUhoKexXHe1xLuFZbnN3PqLLXbF7tp8oxFAigeRLvRX9AoSut38KCotdFzw__)

**نوصي بالموديول الجاهز في V0** لتفادي صعوبة لحام SOIC-8 يدوياً.

### 4.3 AD797 — المضخّم الثاني (Gain Stage 2)
![AD797 DIP Adapter](https://private-us-east-1.manuscdn.com/sessionFile/eFMjMi8kQvmf8Qo3QubYZg/sandbox/6hkneQwrSS7xGjCMg3tWkA-images_1777778066816_na1fn_L2hvbWUvdWJ1bnR1L3YwX2d1aWRlL2ltYWdlcy8wNV9BRDc5N19ESVBfYWRhcHRlcg.jpg?Policy=eyJTdGF0ZW1lbnQiOlt7IlJlc291cmNlIjoiaHR0cHM6Ly9wcml2YXRlLXVzLWVhc3QtMS5tYW51c2Nkbi5jb20vc2Vzc2lvbkZpbGUvZUZNak1pOGtRdm1mOFFvM1F1YllaZy9zYW5kYm94LzZoa25lUXdyU1M3eEdqQ01nM3RXa0EtaW1hZ2VzXzE3Nzc3NzgwNjY4MTZfbmExZm5fTDJodmJXVXZkV0oxYm5SMUwzWXdYMmQxYVdSbEwybHRZV2RsY3k4d05WOUJSRGM1TjE5RVNWQmZZV1JoY0hSbGNnLmpwZyIsIkNvbmRpdGlvbiI6eyJEYXRlTGVzc1RoYW4iOnsiQVdTOkVwb2NoVGltZSI6MTc5ODc2MTYwMH19fV19&Key-Pair-Id=K2HSFNDJXOU9YS&Signature=LiGK5pLKtLGQg5rbABPBVb2xyoJNiEF-6ndPSVEC8HzPMRf3EjzepxzVR8ohtW6BZBUZf~NutjjocNORyzqEpohCR6vUam8zeQ6z9SpcwLP6-wWzxsJIuTSY3p-KgwDcnPQTY7vLaSOvV1FP7qSwuionzMoYdhIEgtK6JhgVbF0FC5wRw8gqUVDm7NrUzvV13pHoZG3EaCqxYfWndMop9dqCAAIwViaGZwsiYpVLwTXvnnNoZwhat5EgoncZNIVgMmABtIwxhdb~Jb4UcLFHS9rV3~MdAfcLnfE1ZsXfZpx-NZxNe1Ik5MQVSxR5CKCIv0KW70kUVIev6H7pL7oK8Q__)

**ما هو؟** Op-Amp فائق الدقة، ضوضاؤه 0.9 nV/√Hz. نستخدمه كمكبّر مرحلة ثانية بكسب ≈11×.

**Pinout (DIP-8):**

```
        ┌─────U─────┐
  OFFSET┤1         8├─ +VS
   -IN ─┤2         7├─ Vout
   +IN ─┤3         6├─ OFFSET
   -VS ─┤4         5├─ COMP
        └───────────┘
```

**Gain Configuration (Non-Inverting):**
- R7 = 1kΩ بين Vout و -IN.
- R8 = 100Ω من -IN إلى GND.
- $G = 1 + R7/R8 = 1 + 10 = 11$
- إجمالي الكسب من ملف RX إلى ADC = 880 × 11 ≈ **9,680**.

**لماذا DIP-8 وليس SOIC-8 في V0؟** لأن DIP يثبت في socket على Perfboard، يمكنك خلعه واستبداله إذا أحرقته.

### 4.4 IRF740 — مفتاح القدرة (TX Switch)
![IRF740 TO-220](https://private-us-east-1.manuscdn.com/sessionFile/eFMjMi8kQvmf8Qo3QubYZg/sandbox/6hkneQwrSS7xGjCMg3tWkA-images_1777778066816_na1fn_L2hvbWUvdWJ1bnR1L3YwX2d1aWRlL2ltYWdlcy8wNl9JUkY3NDBfVE8yMjA.jpg?Policy=eyJTdGF0ZW1lbnQiOlt7IlJlc291cmNlIjoiaHR0cHM6Ly9wcml2YXRlLXVzLWVhc3QtMS5tYW51c2Nkbi5jb20vc2Vzc2lvbkZpbGUvZUZNak1pOGtRdm1mOFFvM1F1YllaZy9zYW5kYm94LzZoa25lUXdyU1M3eEdqQ01nM3RXa0EtaW1hZ2VzXzE3Nzc3NzgwNjY4MTZfbmExZm5fTDJodmJXVXZkV0oxYm5SMUwzWXdYMmQxYVdSbEwybHRZV2RsY3k4d05sOUpVa1kzTkRCZlZFOHlNakEuanBnIiwiQ29uZGl0aW9uIjp7IkRhdGVMZXNzVGhhbiI6eyJBV1M6RXBvY2hUaW1lIjoxNzk4NzYxNjAwfX19XX0_&Key-Pair-Id=K2HSFNDJXOU9YS&Signature=ktlU-JcqjkBeiB~LBkLPv92dda~2TbntFci5VyoCT-GQSu5Y0KrBeMZQoDItsqD2kvvNTR~uNMiKKmokOa4B-K0zXjicuJdAfhe4g3NrroMLfLkSFL~ubH8XpHaw9OFhR4-1T3ZySK5GDw4QdyGqF9MIXY4mkTOmMKnXCF8sWWRjRGFopU78c7BcPqC3QHaVuXpcYJziC4nk3Fx7elqvIU-W9OVFmK~hujNwhHYKRSj6-AOj6QzWpMKmSwb5NCqABqY6mAxuP4Y1YzpDvuIj4D~KB0Jdrtv6F0rpU3nhluc9CW4vqNrolou9MI7~2YIQkM1o3ZiXQ5ThGseOKM1fzg__)

**ما هو؟** N-Channel MOSFET بقدرة 400V / 10A. مهمته **قطع التيار في ملف TX خلال 50ns** ليولّد Flyback نظيف.

**Pinout (TO-220 من اليسار لليمين والكتابة للأمام):**

```
   ┌──────┐
   │      │  TAB = Drain (المشترك مع الحوض الحراري)
   │   ●  │
   │ G  D │
   │ │  │ │
   └─G─D─S─
     1  2  3
```

| Pin | الوظيفة | يتصل بـ |
|---|---|---|
| 1 (G) | Gate | خرج TC4420 (Pin 7) |
| 2 (D) | Drain | طرف ملف TX السفلي |
| 3 (S) | Source | GND مباشرة |

**ملاحظة حرارية:** في V0 لا يحتاج Heatsink لأن نسبة التشغيل (Duty) صغيرة جداً (0.6% @ 20Hz). لكن لا تطلق نبضات أعرض من 500µs بدون مروحة.

### 4.5 TC4420 — درايفر MOSFET
![TC4420 DIP-8](https://private-us-east-1.manuscdn.com/sessionFile/eFMjMi8kQvmf8Qo3QubYZg/sandbox/6hkneQwrSS7xGjCMg3tWkA-images_1777778066816_na1fn_L2hvbWUvdWJ1bnR1L3YwX2d1aWRlL2ltYWdlcy8wN19UQzQ0MjBfRElQOA.jpg?Policy=eyJTdGF0ZW1lbnQiOlt7IlJlc291cmNlIjoiaHR0cHM6Ly9wcml2YXRlLXVzLWVhc3QtMS5tYW51c2Nkbi5jb20vc2Vzc2lvbkZpbGUvZUZNak1pOGtRdm1mOFFvM1F1YllaZy9zYW5kYm94LzZoa25lUXdyU1M3eEdqQ01nM3RXa0EtaW1hZ2VzXzE3Nzc3NzgwNjY4MTZfbmExZm5fTDJodmJXVXZkV0oxYm5SMUwzWXdYMmQxYVdSbEwybHRZV2RsY3k4d04xOVVRelEwTWpCZlJFbFFPQS5qcGciLCJDb25kaXRpb24iOnsiRGF0ZUxlc3NUaGFuIjp7IkFXUzpFcG9jaFRpbWUiOjE3OTg3NjE2MDB9fX1dfQ__&Key-Pair-Id=K2HSFNDJXOU9YS&Signature=bpOv7~xut193lh0grn3TEaqJamt3fD2A8SsjWjbo2tcpjyBoA3OnkK59X6nB~xz0-GNgKz1XcJUXL6H6rt8AZc6Ho9gWAN8KmQDhuLZzEyFAsIQZXiIPJL7BwIW6kwC~N1hpKmOlwEFIcCexZLuU73b6xZX3FLTKt0VT6N8bVtKhT~pdqHYuz~itgl9O0Nh~m-iFhn9ERYLuUoE21spWXL-WAA-1sQP6oV0ZXqDUaRwsOgNIhVq08lgtyCOH4jXMQSqXv9xQcC6-tZRctC6ME~1VBV-FqXdcSBm~Gb~m8xiMkzb1BHWB1lEdMCFf9Ng0NA3ggsBGgRIZac8xuZysAA__)

**ما هو؟** درايفر MOSFET بسرعة 6A، يقوّي 3.3V → 12V خلال 25ns.

**لماذا نحتاجه؟** لأن GPIO من STM32 يخرج 3.3V بتيار 20mA، وهذا لا يكفي لتشحين بوابة IRF740 (Cgs ≈ 1.4nF) بسرعة كافية. النتيجة بدون TC4420: نبضات بطيئة وحرارة في الموسفت.

**Pinout (DIP-8):**

```
        ┌─────U─────┐
    NC ─┤1         8├─ +VDD (12V)
   IN  ─┤2         7├─ OUT
    NC ─┤3         6├─ OUT (مكرّر للتيار)
   GND ─┤4         5├─ GND
        └───────────┘
```

| Pin | يتصل بـ | ملاحظة |
|---|---|---|
| 2 (IN) | PA8 من Black Pill | عبر مقاومة 100Ω لحماية GPIO |
| 7,6 (OUT) | بوابة IRF740 | عبر مقاومة 10Ω لتخميد الرنين |
| 8 (VDD) | +12V | مع مكثف 100nF + 10µF قريب جداً |
| 4,5 (GND) | الأرضي العام | مسار قصير جداً |

### 4.6 B0505S — العزل ±5V
![B0505S](https://private-us-east-1.manuscdn.com/sessionFile/eFMjMi8kQvmf8Qo3QubYZg/sandbox/6hkneQwrSS7xGjCMg3tWkA-images_1777778066816_na1fn_L2hvbWUvdWJ1bnR1L3YwX2d1aWRlL2ltYWdlcy8xNF9CMDUwNVNfaXNvbGF0ZWQ.jpg?Policy=eyJTdGF0ZW1lbnQiOlt7IlJlc291cmNlIjoiaHR0cHM6Ly9wcml2YXRlLXVzLWVhc3QtMS5tYW51c2Nkbi5jb20vc2Vzc2lvbkZpbGUvZUZNak1pOGtRdm1mOFFvM1F1YllaZy9zYW5kYm94LzZoa25lUXdyU1M3eEdqQ01nM3RXa0EtaW1hZ2VzXzE3Nzc3NzgwNjY4MTZfbmExZm5fTDJodmJXVXZkV0oxYm5SMUwzWXdYMmQxYVdSbEwybHRZV2RsY3k4eE5GOUNNRFV3TlZOZmFYTnZiR0YwWldRLmpwZyIsIkNvbmRpdGlvbiI6eyJEYXRlTGVzc1RoYW4iOnsiQVdTOkVwb2NoVGltZSI6MTc5ODc2MTYwMH19fV19&Key-Pair-Id=K2HSFNDJXOU9YS&Signature=dSpfy867ATydG-E0DPFKKYUQXVzXDiVSXKrh37B5NQG2JfFWnRpXocXwZt4OZOvHEjJ7lFK6E8U7IZlLNAzIPKcc-TKTaz-kPiRG2OlBn~vGu0NIMyk0Cj5Opl8ZtZJ7Cp0meiNclriUDwWBtIcNIL01Umatkj5D3fvKyULO9NBXHAaW6w23sW~9y7QMf9j68b0CvASji3ALQc9Z1NN7FglcsdrzfdDwk-D75qaojpS936Pa-2uFPf01zkUFBWsXK7U8rU6lDWPIhel-N9fQXICeD40M1Tc9UvxCp1mQt67GFXbhB3Jyanqo1F5k3Di~RYQL8LKDYsVtLYTADftKKQ__)

**ما هو؟** محوّل DC-DC معزول جلفانياً، يأخذ +5V رقمي ويعطي ±5V أنالوج معزول. هذه الخدعة هي **أهم سرّ** في تقليل الضوضاء.

**Pinout (SIP-4):**

```
   ┌──────┐
   │ B0505│
   │ S-1W │
   ├──────┤
   1 2 3 4
   │ │ │ │
  +VIN│ │ -VOUT
       GND +VOUT
```

| Pin | الوصف |
|---|---|
| 1 | +VIN (5V من الرقمي) |
| 2 | GND_DIG |
| 3 | +VOUT (+5V_ANA) |
| 4 | -VOUT أو GND_ANA حسب الموديل |

**ملاحظة مهمة:** بعض موديلات B0505S تخرج +5V/0V (غير معزول للـ Bipolar). للحصول على ±5V فعلية نستخدم اثنين من B0505S أو نختار **B0505XT-W2 (dual output)**. في V0 الأبسط، نستخدم B0505S واحد (+5V معزول) ونصنع -5V بـ ICL7660 (Charge Pump).

**ما هذا يحلّ؟** يكسر **Ground Loop** بين الجزء الرقمي (الذي يولّد ضوضاء عالية بسبب switching) والجزء الأنالوج (الحساس جداً). بدون هذا العزل، ستُلقي خمسة أيام بحثاً عن مصدر ضوضاء 30mV عشوائية.

### 4.7 ملف TX — ملف الإرسال
![DIY Mono Coil](https://private-us-east-1.manuscdn.com/sessionFile/eFMjMi8kQvmf8Qo3QubYZg/sandbox/6hkneQwrSS7xGjCMg3tWkA-images_1777778066816_na1fn_L2hvbWUvdWJ1bnR1L3YwX2d1aWRlL2ltYWdlcy8xMV9ESVlfbW9ub19jb2ls.jpg?Policy=eyJTdGF0ZW1lbnQiOlt7IlJlc291cmNlIjoiaHR0cHM6Ly9wcml2YXRlLXVzLWVhc3QtMS5tYW51c2Nkbi5jb20vc2Vzc2lvbkZpbGUvZUZNak1pOGtRdm1mOFFvM1F1YllaZy9zYW5kYm94LzZoa25lUXdyU1M3eEdqQ01nM3RXa0EtaW1hZ2VzXzE3Nzc3NzgwNjY4MTZfbmExZm5fTDJodmJXVXZkV0oxYm5SMUwzWXdYMmQxYVdSbEwybHRZV2RsY3k4eE1WOUVTVmxmYlc5dWIxOWpiMmxzLmpwZyIsIkNvbmRpdGlvbiI6eyJEYXRlTGVzc1RoYW4iOnsiQVdTOkVwb2NoVGltZSI6MTc5ODc2MTYwMH19fV19&Key-Pair-Id=K2HSFNDJXOU9YS&Signature=D7WNLNC0zSXdcvXEAPBlhYbIY~xUnMZPAUjlV37Q3hEovpf0osIeUspudnAhlt~7qdhUXbmNOYidKBpDOBPN3L5KtxhS0oDICggRDQddW2rX6Mt-NSJRvNXTznUsxUz-HXpl2t8OSb-lFBo-uqTTBw9GisKxO8IBLCEPDX3mUJF4NXCAcqV2AtctNH4iYXSvVLyScSUM4HsYxDZdwAYRBs87EQB6pQOx-tz52vg~KSPuYo281hg2xqT9lLk5oePkqF1hjevTnrNzX3pj1kb8fedX4E2u~UgI5KAV79hFpZTPeqMqaTLBTRiw7ZUvaWL-bWSRfmdkb8Ssdpa7CBl0SA__)

**المواصفات لـ V0 (نسخة مبسّطة):**
- **القطر:** 28cm (مونو).
- **عدد اللفات:** 25 لفة.
- **السلك:** سلك نحاس مطلي بالميناء (Magnet wire) بقطر 0.5mm.
- **القيمة:** ~300 µH ± 20%.
- **المقاومة DC:** ~0.4 Ω.

**كيف نلفّه (خطوة بخطوة):**
1. اقطع لوح خشب MDF بقطر 28cm.
2. ثبّت 25 مسماراً صغيراً على المحيط.
3. لفّ السلك حول المسامير 25 لفة، شدّه باعتدال (لا تشدّ كثيراً).
4. لفّ شريط لاصق Cotton fabric tape حول الملف لتثبيته.
5. أزل المسامير، ثم لفّ شريط ألومنيوم رقيق فوق الملف بنسبة تغطية **80%** (لا تكمل الدائرة! اترك فجوة 1cm).
6. لفّ شريط فوق الألومنيوم لعزله.
7. أوصل طرف الألومنيوم بسلك إلى **GND_ANA** (ليس DGND).
8. قِس بـ LC-meter — يجب أن تقرأ 280–320µH.

### 4.8 ملف RX — ملف الاستقبال
**المواصفات:**
- **القطر:** 25cm (يدخل داخل ملف TX).
- **عدد اللفات:** 120 لفة.
- **السلك:** Magnet wire 0.2mm.
- **القيمة:** ~1 mH ± 10%.
- **المقاومة DC:** ~25 Ω.

**ملاحظة:** اربط طرفي ملف RX إلى **Twisted pair shielded cable** بطول 50cm، الـ Shield يصل إلى GND_ANA من ناحية واحدة فقط (ناحية اللوحة) لتفادي Ground Loop.

### 4.9 Perfboard — اللوحة الأساس
![Perfboard Soldering](https://private-us-east-1.manuscdn.com/sessionFile/eFMjMi8kQvmf8Qo3QubYZg/sandbox/6hkneQwrSS7xGjCMg3tWkA-images_1777778066816_na1fn_L2hvbWUvdWJ1bnR1L3YwX2d1aWRlL2ltYWdlcy8wOV9wZXJmYm9hcmRfc29sZGVyX2Jhc2ljcw.jpg?Policy=eyJTdGF0ZW1lbnQiOlt7IlJlc291cmNlIjoiaHR0cHM6Ly9wcml2YXRlLXVzLWVhc3QtMS5tYW51c2Nkbi5jb20vc2Vzc2lvbkZpbGUvZUZNak1pOGtRdm1mOFFvM1F1YllaZy9zYW5kYm94LzZoa25lUXdyU1M3eEdqQ01nM3RXa0EtaW1hZ2VzXzE3Nzc3NzgwNjY4MTZfbmExZm5fTDJodmJXVXZkV0oxYm5SMUwzWXdYMmQxYVdSbEwybHRZV2RsY3k4d09WOXdaWEptWW05aGNtUmZjMjlzWkdWeVgySmhjMmxqY3cuanBnIiwiQ29uZGl0aW9uIjp7IkRhdGVMZXNzVGhhbiI6eyJBV1M6RXBvY2hUaW1lIjoxNzk4NzYxNjAwfX19XX0_&Key-Pair-Id=K2HSFNDJXOU9YS&Signature=PFH~O1ImjtR50ASMXC42djYt5zz2LKWK3nCwPjtACVxgfN2kwBuc7fjyrPHduJQXg7Ddi25xlU4x0jwBO21MCqoeB4eQpVB43MGqhBlyL3XTlTU-XqqqY3y5t6Wo18dLiXfTQxPvq-yeVO2fi0S~me~5nd~CLDYdoqHO15l7gyYHlOl~bTA7V395qaZ-TjGC5M2Psr8pN6keqO-KOGy4c8y4JJ-IYOwf9EdgVj1xQx1FaiOUVRSQEzAO4rxr860LJPVuG060tlNYburOlAgQmDosJKbFUvo4fxbulLVNfyUNunSbxir2AZ1KvgKa~YXoXAMqd90dS5a~cxU0zvAKMg__)

**النوع الموصى به:**
- **Single-sided** (لا تستخدم Double-sided في V0).
- **Pitch:** 2.54mm (القياسي).
- **مقاس:** 7×9 cm.

**خدعة Star Ground على Perfboard:** في الصف الأخير، خصّص نقطة واحدة (مسمار + قطعة نحاس مربعة) كـ Star Point. كل GND_ANA يصل إليها بسلك مباشر، وكل GND_DIG كذلك بسلك مستقل. لا تخلطهما إلا في هذه النقطة.

---

## 5. التصميم الداخلي — Layout الـ Perfboard

![Perfboard Layout](https://private-us-east-1.manuscdn.com/sessionFile/eFMjMi8kQvmf8Qo3QubYZg/sandbox/6hkneQwrSS7xGjCMg3tWkA-images_1777778066816_na1fn_L2hvbWUvdWJ1bnR1L3YwX2d1aWRlL2ltYWdlcy9EMl9wZXJmYm9hcmRfbGF5b3V0.png?Policy=eyJTdGF0ZW1lbnQiOlt7IlJlc291cmNlIjoiaHR0cHM6Ly9wcml2YXRlLXVzLWVhc3QtMS5tYW51c2Nkbi5jb20vc2Vzc2lvbkZpbGUvZUZNak1pOGtRdm1mOFFvM1F1YllaZy9zYW5kYm94LzZoa25lUXdyU1M3eEdqQ01nM3RXa0EtaW1hZ2VzXzE3Nzc3NzgwNjY4MTZfbmExZm5fTDJodmJXVXZkV0oxYm5SMUwzWXdYMmQxYVdSbEwybHRZV2RsY3k5RU1sOXdaWEptWW05aGNtUmZiR0Y1YjNWMC5wbmciLCJDb25kaXRpb24iOnsiRGF0ZUxlc3NUaGFuIjp7IkFXUzpFcG9jaFRpbWUiOjE3OTg3NjE2MDB9fX1dfQ__&Key-Pair-Id=K2HSFNDJXOU9YS&Signature=B~mxKZe0miS7xTx3CLcSz7eL3FENX2eO5UInIMT9dbNyaeKBSZN0k9Kn9hTMb5MZ46jko4wQ4tg4bqt8MMkrFcngYWBaRWQR~P70Y2lEBY6LrDzOh9lqPoi2P9NNErDN3L9gOcAyPytgtqdPLuFqfy80Pz9BtXweu7~~U4Z3t9374Qt0xnLqghpThjTEBHwdG4Z36xYW6pyMacUzUF~wxR6BUM1A3fHlzVN3YxYufdSuvKXgIxcRsuREzFaiGsKBnol~kHn0fDX384OQdp1~udtB~6-4i~Rjk9cPOCqND1b70ZQu9JJYodWubnM6mFzPTct0Ty9Xn6G5f4pFObw8Jg__)

### القاعدة الذهبية: "أنالوج يسار، رقمي يمين، ولا تعبر بينهما إلا بسلك واحد"

**التقسيم المكاني (الأبعاد بالتقريب):**

```
┌─────────────────────────────────────────────────┐
│   صف التغذية: GND BUS — +5V_DIG — ±5V_ANA       │  ← 1.5cm
├──────────────────────┬──────────────────────────┤
│ النصف الأيسر (3cm)   │ النصف الأيمن (4cm)        │
│ (أنالوج)             │ (رقمي)                    │
│                      │                            │
│  AD8429 موديول       │  Black Pill (في headers)  │
│  ─ يأخذ ±5V_ANA     │  ─ يأخذ +5V_DIG          │
│  ─ خرج → R7 ─ AD797 │                            │
│                      │  TC4420 (DIP-8)           │
│  AD797 (DIP socket)  │  ─ IN من PA8             │
│  ─ خرج → فلتر       │  ─ OUT → IRF740 Gate     │
│                      │                            │
│  فلتر Anti-Alias     │  IRF740 (TO-220 رأسي)    │
│  R=49.9 + C=1nF      │  ─ Drain → ملف TX        │
│  → ADC PA0           │  ─ Source → GND_DIG      │
│                      │                            │
├──────────────────────┴──────────────────────────┤
│   ★ Star Ground (تحت AD797 — هذه أهم نقطة)      │
├─────────────────────────────────────────────────┤
│   موصلات: TX coil ─ RX coil ─ ST-Link header    │
└─────────────────────────────────────────────────┘
```

### قاعدة Star Ground (أهم قاعدة في كل المشروع)

> **★ كل سلك أرضي في V0 يجب أن يُوصل بنقطة واحدة فقط على الـ Perfboard، تحت AD797 مباشرة.**

من هذه النقطة تنطلق ثلاثة أسلاك أرضية مستقلة لا تتقابل في أي مكان آخر:
1. **AGND** → نقطة الأرضي للـ AD8429 + AD797 + B0505S out.
2. **DGND** → نقطة الأرضي للـ Black Pill + TC4420 + IRF740 Source.
3. **CGND (Chassis)** → الـ Shield في كابل RX.

إن خلطت هذين الـ Ground في أي مكان آخر = **Ground Loop** = ضوضاء 50Hz بقدر 200mV. وداعاً للقياس.

### أمثل ترتيب اللحام (بالترتيب)

| اليوم | المهمة |
|---|---|
| 1 | لحام female headers لـ Black Pill + اختبار 3.3V |
| 2 | لحام TC4420 + IRF740 + اختبار PWM يصل لـ Gate بشكل سليم |
| 3 | لف ملف TX + اختبار قيمة الـ Inductance |
| 4 | لحام B0505S + قياس ±5V_ANA + اختبار العزل |
| 5 | لحام AD8429 module + RG + اختبار الكسب بإدخال إشارة معلومة |
| 6 | لحام AD797 socket + AD797 + اختبار كسب stage2 |
| 7 | لحام فلتر Anti-Alias وربط ADC |
| 8 | لف ملف RX + ربطه بـ Twisted-pair shielded |
| 9 | اختبار النظام الكامل بدون ملف TX (Noise Floor measurement) |
| 10 | تركيب ملف TX وأول قياس فعلي على معدن |

---

## 6. التصميم الخارجي — Mechanical Mock-up

### V0 ليس له علبة بمعنى الكلمة، لكن يحتاج **Bench Setup**:

```
                    ┌────────────────┐
                    │   Laptop +     │
                    │   SerialPlot   │
                    └────────┬───────┘
                             │ USB
                    ┌────────┴───────┐
                    │   ST-Link V2   │
                    └────────┬───────┘
                             │ SWD (4 أسلاك)
   ┌─────────┐    ┌──────────┴─────────┐
   │Bench PSU│────│   Perfboard V0     │
   │ ±5V/12V │    │  ┌─────┬─────┐    │
   └─────────┘    │  │Analog│Digit│    │
                  │  └─────┴─────┘    │
                  └────┬───────┬──────┘
                       │       │
                  RX coil    TX coil
                  (1mH)      (300µH)
                       │       │
                  ╔════╧═══════╧════╗
                  ║   منصّة خشبية    ║
                  ║   الاختبار       ║
                  ╚═════════════════╝
                          │
                  Test target (دينار + بطاقة + مفتاح حديد)
```

### تحضير منصّة الاختبار (Test Bench)

1. **اللوح الخشبي:** 60×40cm سماكة 2cm، خشبي خالص (لا MDF لأنه فيه غراء فيه أملاح).
2. **رفّ بلاستيكي للـ Perfboard:** ارفعه 5cm فوق الأرض لمنع التداخل.
3. **مسطرة قياس:** ملصقة على اللوح.
4. **أهداف اختبار قياسية:**
   - دينار ذهبي (8g) — اختبار الذهب الصغير.
   - عملة 100 ريال نحاس — اختبار الذهب الأكبر بديلاً.
   - مسمار حديد 3 إنش — اختبار التمييز.
   - بطاقة ذكية — اختبار Eddy currents في معدن رقيق.
5. **مسافات اختبار:** 0, 5, 10, 15, 20, 25, 30, 40, 50 cm.

> **ملاحظة:** ابعد عن أي حديد في الجدار (تسليح خرسانة)، أنابيب، حواسيب، شاشات. اختبر في غرفة بنوافذ شرقية إن أمكن (شمالية صناعياً تكون أهدأ).

---

## 7. توقيت دورة PI (Pulse Induction Cycle)

![PI Timing](https://private-us-east-1.manuscdn.com/sessionFile/eFMjMi8kQvmf8Qo3QubYZg/sandbox/6hkneQwrSS7xGjCMg3tWkA-images_1777778066816_na1fn_L2hvbWUvdWJ1bnR1L3YwX2d1aWRlL2ltYWdlcy9EM19waV90aW1pbmc.png?Policy=eyJTdGF0ZW1lbnQiOlt7IlJlc291cmNlIjoiaHR0cHM6Ly9wcml2YXRlLXVzLWVhc3QtMS5tYW51c2Nkbi5jb20vc2Vzc2lvbkZpbGUvZUZNak1pOGtRdm1mOFFvM1F1YllaZy9zYW5kYm94LzZoa25lUXdyU1M3eEdqQ01nM3RXa0EtaW1hZ2VzXzE3Nzc3NzgwNjY4MTZfbmExZm5fTDJodmJXVXZkV0oxYm5SMUwzWXdYMmQxYVdSbEwybHRZV2RsY3k5RU0xOXdhVjkwYVcxcGJtYy5wbmciLCJDb25kaXRpb24iOnsiRGF0ZUxlc3NUaGFuIjp7IkFXUzpFcG9jaFRpbWUiOjE3OTg3NjE2MDB9fX1dfQ__&Key-Pair-Id=K2HSFNDJXOU9YS&Signature=wHZh0ciAItUGyDFT-S20i5haGU4pk9m7PEVbFra8n-vlWe6V-1W3YzFwavzct6vCnPcAGj0b-sQJjqUcL5T1hGaPMXnRFSmkjmvceBT3-rxbnnsluyPk7MMS0w8I13B9n5e1dFswphLuPEabBZZVVqFThtVit6EiAGJJPvdrAte7lORNhuLCko3jJK~Vi8jxE~JHC7CwHm-p6fUJhrvmxz~TucN9blm78hHo37OKMZ-TW3hAorjzYsKgLSMB19WXVQRnuddxbgmflhYZsC~kxtOkPhFCOQPT4j8bmuxshkkfhEz3r9zBLgPA2NmnpE0PBwr0A3NAMPU~wJ-Z3BTf9g__)

### المعادلات العملية لـ V0

| المتغيّر | القيمة الأولية | المدى المسموح | كيف تُضبط |
|---|---|---|---|
| `TX_PERIOD` | 50ms (20Hz) | 25–100ms | TIM1 ARR |
| `TX_PULSE_WIDTH` | 300µs | 100–500µs | TIM1 CCR1 |
| `DEAD_TIME` | 15µs | 10–25µs | تأخير برمجي قبل ADC |
| `SAMPLE_COUNT` | 10 | 5–20 | عدد قراءات ADC |
| `SAMPLE_INTERVAL` | 10µs | 5–20µs | TIM2 |
| `GAIN_TOTAL` | 9,680× | 1,000–20,000 | RG + R7/R8 |

### كود C تجريبي للـ Black Pill (الجزء الحرج فقط)

```c
// V0 — STM32F411 — PI Engine (بسيط جداً)
#include "main.h"

// تحضير TIM1 لتوليد PWM 1kHz عرض 300µs
void TIM1_PWM_Init(void) {
    TIM1->PSC = 99;         // 100MHz/100 = 1MHz
    TIM1->ARR = 999;        // 1MHz/1000 = 1kHz
    TIM1->CCR1 = 300;       // عرض النبضة 300µs
    TIM1->CCMR1 = 0x0060;   // PWM mode 1
    TIM1->CCER = 0x0001;    // CH1 enable
    TIM1->BDTR = 0x8000;    // MOE enable (محصور لـ TIM1)
    TIM1->CR1 = 0x0001;     // Counter enable
}

// قراءة ADC بعد Dead-Time
uint16_t adc_buffer[10];

void Read_Decay_Curve(void) {
    // انتظر النبضة على PA8 تنخفض
    while ((TIM1->CR1 & 0x0001) && TIM1->CNT < 300);
    
    // Dead-Time 15µs
    Delay_us(15);
    
    // 10 قراءات ADC كل 10µs
    for (int i = 0; i < 10; i++) {
        ADC1->CR2 |= ADC_CR2_SWSTART;
        while (!(ADC1->SR & ADC_SR_EOC));
        adc_buffer[i] = ADC1->DR;
        Delay_us(10);
    }
}

// إرسال البيانات على UART
void Send_Buffer(void) {
    char msg[80];
    int len = sprintf(msg, "%u,%u,%u,%u,%u,%u,%u,%u,%u,%u\r\n",
        adc_buffer[0], adc_buffer[1], adc_buffer[2], adc_buffer[3], adc_buffer[4],
        adc_buffer[5], adc_buffer[6], adc_buffer[7], adc_buffer[8], adc_buffer[9]);
    HAL_UART_Transmit(&huart1, (uint8_t*)msg, len, 100);
}

int main(void) {
    HAL_Init();
    SystemClock_Config();
    GPIO_Init();
    UART1_Init();
    ADC1_Init();
    TIM1_PWM_Init();
    
    while (1) {
        Read_Decay_Curve();
        Send_Buffer();
        HAL_Delay(50);  // 20Hz refresh
    }
}
```

---

## 8. خريطة المعايرة (Calibration Roadmap)

### المرحلة 1: فحوصات الأرضي والتغذية (Day 1–2)

| الاختبار | الأداة | معيار النجاح | الإجراء عند الفشل |
|---|---|---|---|
| Black Pill 3.3V | ملتيمتر بين 3V3 و GND | 3.30V ± 50mV | افحص USB + AMS1117 |
| +5V_DIG | ملتيمتر | 5.00V ± 100mV | افحص B0505S input |
| +5V_ANA / -5V_ANA | ملتيمتر | ±5.0V ± 200mV | افحص B0505S output + الموديل |
| +12V (TX rail) | ملتيمتر | 12.0V ± 0.3V | افحص PSU |
| Star Ground continuity | ملتيمتر in continuity | 0Ω بين أي GND ونقطة الـ Star | أعد لحام نقطة الـ Star |
| GND isolation | ملتيمتر بين GND_ANA و GND_DIG **بعيداً عن نقطة Star** | يجب أن تقرأ مقاومة عالية أو لا شيء (لأن AGND/DGND يلتقيان فقط في Star) | إذا قرأت 0Ω فهناك Ground Loop |

### المرحلة 2: قياس Noise Floor (Day 3 — أهم قياس في كل المشروع)

**الهدف:** قياس الضوضاء الذاتية لـ AFE بدون أي إشارة.

**الخطوات:**
1. افصل ملف RX تماماً.
2. ضع مقاومة 50Ω 0.1% بين +IN و -IN لـ AD8429 (تحاكي impedance ملف RX).
3. وصّل scope على خرج AD797 مع AC coupling، 1mV/div، 10ms/div.
4. سجّل 10,000 عينة عبر UART إلى Python.
5. احسب σ (الانحراف المعياري) من العيّنات.
6. احسب Vnoise_in = σ / Gain_total = σ / 9680.
7. احسب Noise Density = Vnoise_in / √Bandwidth.
   - Bandwidth ≈ 100kHz (محدود بالفلتر).

**معيار القبول:**
- σ ≤ 0.5 mV → Vnoise_in ≤ 50 nV → Noise Density ≤ **0.16 nV/√Hz** (نظري).
- في الممارسة: نقبل ≤ 50 nV/√Hz في V0 (لأنه Perfboard).

**إن فشلت:**
- زاد عن 100 nV/√Hz: Star Ground خاطئ، أو RG ملتوي.
- زاد عن 200 nV/√Hz: B0505S تالف، أو PSU به ripple > 50mV.
- زاد عن 1 µV/√Hz: لحام بارد (Cold joint) في AD8429.

### المرحلة 3: اختبار TX (Day 4)

**الأداة:** Oscilloscope مع Probe على Drain لـ IRF740.

**الموجة المتوقّعة:**
```
       ┌──────┐
12V ───┘      └──╮
                  ╲ Flyback
                   ╲    /\___
                    ╲__/  ←  ~200V spike
                          ↓
                        TVS clamp
0V ──────────────────────────────
   ←─300µs─→←──Dead time──→
```

**معايير القبول:**
- ارتفاع Flyback يصل ≥ 150V قبل التوهين.
- عرض النبضة فعلياً 300µs ± 20µs.
- زمن الانطفاء (Fall time) ≤ 200ns.
- لا رنين (Ringing) أكثر من دورتين.

**إن لم يصل Flyback:** TVS قصّ الـ spike قبل أوانه (طبيعي إن استخدمت SMAJ12 — تكلم مع نفسك: هل أحتاج SMAJ100 لـ V0؟). أو ملف TX قيمته منخفضة جداً.

### المرحلة 4: اختبار AFE بإشارة معلومة (Day 5)

**الأداة:** Function Generator، إن لم يتوفر، استخدم PA9 من Black Pill كمولّد إشارة 1kHz.

**الإجراء:**
1. ولّد إشارة جيبية 1mV peak-to-peak على دخل AD8429.
2. توقّع: خرج AD797 = 1mV × 9680 = 9.68V.
3. **مشكلة:** AD797 مع ±5V لن يصل لـ 9.68V (سيتشبّع عند ~4.8V).
4. لذلك استخدم إشارة 0.4mV peak-to-peak → خرج متوقّع 3.87V.
5. قِس على scope.

**معيار القبول:** الكسب الفعلي = الكسب المحسوب ± 5%.

### المرحلة 5: اختبار النظام الكامل على معدن (Day 6–7)

1. ضع ملف TX أفقياً.
2. ضع ملف RX **داخله** (متحدا المركز).
3. اضبط RX ميكانيكياً حتى تكون الإشارة الراجعة على ADC = 0V (Null position).
4. ضع دينار ذهب في وسط الملفين على ارتفاع 5cm.
5. شغّل البرنامج.
6. على SerialPlot، يجب أن تشاهد منحنى اضمحلال ينخفض من ~1.5V إلى 0V خلال 100µs.

**معايير الأداء V0:**

| الهدف | العمق الأدنى المتوقّع |
|---|---|
| دينار ذهب 8g | ≥ 15 cm |
| صندوق سجائر ألومنيوم | ≥ 25 cm |
| لوح حديد 10×10 cm | ≥ 30 cm |

(لا تتوقع أداء أعلى لأن V0 مونوكويل و على Perfboard.)

---

## 9. أدوات ضرورية في المختبر

### 9.1 Power Supply ثنائي القناة
![Bench PSU](https://private-us-east-1.manuscdn.com/sessionFile/eFMjMi8kQvmf8Qo3QubYZg/sandbox/6hkneQwrSS7xGjCMg3tWkA-images_1777778066816_na1fn_L2hvbWUvdWJ1bnR1L3YwX2d1aWRlL2ltYWdlcy8xMF9iZW5jaF9QU1VfZHVhbA.jpg?Policy=eyJTdGF0ZW1lbnQiOlt7IlJlc291cmNlIjoiaHR0cHM6Ly9wcml2YXRlLXVzLWVhc3QtMS5tYW51c2Nkbi5jb20vc2Vzc2lvbkZpbGUvZUZNak1pOGtRdm1mOFFvM1F1YllaZy9zYW5kYm94LzZoa25lUXdyU1M3eEdqQ01nM3RXa0EtaW1hZ2VzXzE3Nzc3NzgwNjY4MTZfbmExZm5fTDJodmJXVXZkV0oxYm5SMUwzWXdYMmQxYVdSbEwybHRZV2RsY3k4eE1GOWlaVzVqYUY5UVUxVmZaSFZoYkEuanBnIiwiQ29uZGl0aW9uIjp7IkRhdGVMZXNzVGhhbiI6eyJBV1M6RXBvY2hUaW1lIjoxNzk4NzYxNjAwfX19XX0_&Key-Pair-Id=K2HSFNDJXOU9YS&Signature=P4FjlF7-p1jrJsQm8pqIqgyDe0qaHAdjUTAjRqTo01NIDdDDEcjku0EKWTDHkkqo1hpIl~Sr~tig4NvGK5BEbDHlAYDzg4JHlqWBNSSxljubkMtT5alEbgKelLebrZJ53mL~GQjXbpeQeYut9m3dPNWZ5FTQkfoiSy1xnKSxhbY~-DK2fJQ1xChrWUh3fPlLXcoDf5ObGFDxD5doeMMNX0tbleqwt3ZGLpKbZ3DgLBAqLqI8oAL3MeTW0nwa2sxVpFoF9zxmqsWp4PUo5rDSzTBPJuU9nShbLVwG3BXfcqv7ZG96uMkTgxbKtUgoriehKd6jfecoX-6v2L-o~AVadA__)

**موصى به:** UNI-T UTP3315TFL أو KORAD KA3305P. القناة الأولى 5V/3A، الثانية 12V/3A.

### 9.2 Oscilloscope
![Oscilloscope](https://private-us-east-1.manuscdn.com/sessionFile/eFMjMi8kQvmf8Qo3QubYZg/sandbox/6hkneQwrSS7xGjCMg3tWkA-images_1777778066816_na1fn_L2hvbWUvdWJ1bnR1L3YwX2d1aWRlL2ltYWdlcy8xM19vc2NpbGxvc2NvcGU.jpg?Policy=eyJTdGF0ZW1lbnQiOlt7IlJlc291cmNlIjoiaHR0cHM6Ly9wcml2YXRlLXVzLWVhc3QtMS5tYW51c2Nkbi5jb20vc2Vzc2lvbkZpbGUvZUZNak1pOGtRdm1mOFFvM1F1YllaZy9zYW5kYm94LzZoa25lUXdyU1M3eEdqQ01nM3RXa0EtaW1hZ2VzXzE3Nzc3NzgwNjY4MTZfbmExZm5fTDJodmJXVXZkV0oxYm5SMUwzWXdYMmQxYVdSbEwybHRZV2RsY3k4eE0xOXZjMk5wYkd4dmMyTnZjR1UuanBnIiwiQ29uZGl0aW9uIjp7IkRhdGVMZXNzVGhhbiI6eyJBV1M6RXBvY2hUaW1lIjoxNzk4NzYxNjAwfX19XX0_&Key-Pair-Id=K2HSFNDJXOU9YS&Signature=IKo2HWNL~h2dhiuvdKGPjGIMA9Zm2yXACaWyQArAmAjMuCxMWHch7sgdur65HxMTCRg~RoYUzFJw04NZpBVbOz~lI4a4sj~5FpyK--B3FnewNlPkQc6pZwTjZWSHSKqnQwwkVLmB-B2VirbWMkZxbe2ytNjJyL2ZPbbZ70NQ38yj6frRHerfRK~8fMFt~05uNg2xEd2WyzPgYPmtQMYV1JenUBbHPy9tBKn0clfC2jf-9EkiyNYK9YdPiLTSt2MOgpo~28GBe15E15IbcNVuxDQqNO2FTbfAiQuUKJ5y542~mcYzKhgzrSlrxr1FccQ4Lc5-IewoJpJOMO1o2k4KRg__)

**الحد الأدنى:** Hantek DSO5072P أو ما يعادله — 70MHz / 2 channels / 1GS/s.  
**موصى به:** Rigol DS1054Z مع تفعيل القنوات الأربع.

### 9.3 ST-Link V2 Programmer
![ST-Link V2](https://private-us-east-1.manuscdn.com/sessionFile/eFMjMi8kQvmf8Qo3QubYZg/sandbox/6hkneQwrSS7xGjCMg3tWkA-images_1777778066816_na1fn_L2hvbWUvdWJ1bnR1L3YwX2d1aWRlL2ltYWdlcy8xNV9TVExpbmtfVjI.jpg?Policy=eyJTdGF0ZW1lbnQiOlt7IlJlc291cmNlIjoiaHR0cHM6Ly9wcml2YXRlLXVzLWVhc3QtMS5tYW51c2Nkbi5jb20vc2Vzc2lvbkZpbGUvZUZNak1pOGtRdm1mOFFvM1F1YllaZy9zYW5kYm94LzZoa25lUXdyU1M3eEdqQ01nM3RXa0EtaW1hZ2VzXzE3Nzc3NzgwNjY4MTZfbmExZm5fTDJodmJXVXZkV0oxYm5SMUwzWXdYMmQxYVdSbEwybHRZV2RsY3k4eE5WOVRWRXhwYm10ZlZqSS5qcGciLCJDb25kaXRpb24iOnsiRGF0ZUxlc3NUaGFuIjp7IkFXUzpFcG9jaFRpbWUiOjE3OTg3NjE2MDB9fX1dfQ__&Key-Pair-Id=K2HSFNDJXOU9YS&Signature=UdMtZxSvyTOjst62OR-lioOXJDEe6ebnnfYKHdOJyXBGVTj~Tduh~F12qCgQgcjQQM2K3nzrMzYqB~uzOdvxDpAJE1pZ2UvwiLsX-PfvT4Ys6FiKmRd~BPS-MJdaJtLeTVnsXMy1iztrIXL3ZmAmpulDrW7DJnvA54jiElFm6cabXbHlwKJ2Hg81CTZjLXCsdq54QGG~XAWP8f73N6rKxLIMpiv2aQeQ023zqQq~KzSgvfQjohIxfO~Gye49lBzT3xvxtlUwfrKDqKl6tO0Wfr5d~i62yXHZI35JK8KFr7Ldz14qGbYTPfkCBH3bE3li9bPW3sZER16aIbfqemB9pw__)

**يحتاجه:** برمجة Black Pill عبر SWD (لا تستخدم DFU عبر USB لأنه يحتاج زر BOOT0 وغير مريح).

**التوصيل:**
| ST-Link | Black Pill |
|---|---|
| 3.3V | 3V3 |
| GND | GND |
| SWDIO | PA13 |
| SWCLK | PA14 |

---

## 10. سجل القياسات (Test Log Template)

في كل اختبار اكتب على ورقة منفصلة:

```
─────────────────────────────────────────
V0 Test Log — اليوم: ___ التاريخ: ____
─────────────────────────────────────────
المرحلة:        [ ] التغذية
                [ ] Noise Floor
                [ ] TX
                [ ] AFE Gain
                [ ] System

النتيجة:        Pass / Fail
القيمة المقاسة: __________
المتوقّع:       __________
الانحراف:       __________ %

ملاحظات:
___________________________
___________________________

التعديل المُتّخذ:
___________________________
─────────────────────────────────────────
```

---

## 11. ما الذي يجعل V0 ينجح أو يفشل؟

### العوامل الثلاثة التي تحسم الأمر:

1. **Star Ground صحيح** → بدونه، لا تكمل القراءة. الفرق بين star ground صحيح وخاطئ هو 60 dB من SNR (1000×).
2. **B0505S يعمل بسلامة** → عزل التغذية الأنالوج عن الرقمي. بدونه، ضوضاء الـ STM32 المنتقلة على VDD ستظهر مباشرة على خرج AD797.
3. **ملف TX/RX متعامدين أو متحدا المركز بدقة Null** → بدون Null، الإشارة الـ TX تُغرق ملف RX، يحدث تشبع AD8429، ولن ترى أي معدن.

### الأخطاء الشائعة التي يجب تجنّبها

| الخطأ | العاقبة | الحل |
|---|---|---|
| لحام Decoupling Cap بعيداً عن IC | ضوضاء switching تظهر على الخرج | لحام كل 100nF على بُعد ≤ 5mm من Pin VDD |
| نسيان Pull-down على Gate IRF740 | تشغيل عشوائي عند الإقلاع | أضف 10kΩ من Gate إلى GND |
| تأريض Shield الكابل من الناحيتين | Ground Loop عبر الكابل | أرضِه من ناحية اللوحة فقط |
| ملف TX قريب جداً من AFE | Magnetic coupling مباشر | افصلهما ≥ 30cm |
| استخدام Breadboard بدل Perfboard | Capacitance طفيلية + اتصالات سيئة | لا تستخدم Breadboard أبداً للـ AFE |
| عدم استخدام scope في المعايرة | تخمينات وتشخيص خاطئ | الـ scope ضروري، استعِره إن لم تملك |

---

## 12. الخطوات التالية بعد نجاح V0

عندما تتحقق المعايير الثلاث:
- ✅ Noise Floor < 50 nV/√Hz  
- ✅ Flyback نظيف ≥ 150V  
- ✅ كشف دينار على 15 cm

عندها فقط، انتقل إلى:
1. **رسم Schematic رسمي في KiCad** بناءً على ما تأكّد من V0.
2. **PCB 2-layer لـ V1** بنفس Layout (analog يسار، digital يمين).
3. **شراء قطع V1** ودخول مرحلة الإنتاج التجريبي.

> **تذكير أخير:** كل دقيقة تقضيها في V0 توفّر عليك يومين في V1 وأسبوعاً في V2. لا تتعجّل ولا تختصر.

---

## 13. المراجع المرئية في هذا الدليل

| الصورة | الموقع |
|---|---|
| Black Pill Pinout | `images/01_blackpill_pinout.png` |
| Black Pill 3D | `images/02_blackpill_3d.jpg` |
| AD8429 SOIC-8 | `images/03_AD8429_SOIC8.jpg` |
| AD8429 Module | `images/04_AD8429_module.jpg` |
| AD797 + Adapter | `images/05_AD797_DIP_adapter.jpg` |
| IRF740 TO-220 | `images/06_IRF740_TO220.jpg` |
| TC4420 DIP-8 | `images/07_TC4420_DIP8.jpg` |
| Perfboard back | `images/08_perfboard_back.jpg` |
| Perfboard soldering | `images/09_perfboard_solder_basics.jpg` |
| Bench PSU | `images/10_bench_PSU_dual.jpg` |
| DIY Mono Coil | `images/11_DIY_mono_coil.jpg` |
| Oscilloscope | `images/13_oscilloscope.jpg` |
| B0505S | `images/14_B0505S_isolated.jpg` |
| ST-Link V2 | `images/15_STLink_V2.jpg` |
| **D1: Block Diagram** | `images/D1_block_diagram.png` |
| **D2: Perfboard Layout** | `images/D2_perfboard_layout.png` |
| **D3: PI Timing** | `images/D3_pi_timing.png` |

---

**انتهى دليل V0. حظاً موفّقاً يا صديقي. ابدأ من اليوم الأول، ولا تترك خطوة بدون قياس وتسجيل.**
