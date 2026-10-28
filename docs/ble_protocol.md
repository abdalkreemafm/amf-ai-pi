# Hadeed communication protocol

المواصفة الوحيدة المعتمدة للإطارات هي [Protocol V1](../protocol/README.md).
لا تستخدم أكواد `0xA1`/`0xA2` القديمة كإطار مستقل؛ إن احتجنا event صوتيًا فيجب
أن يكون داخل payload موثق ورسالة ذات `message_type` واضح.

## الرسائل الحالية

- `0x01` — SYNC: payload بطول بايت واحد لحالة النظام.
- `0x02` — VISION_RESULT: payload بطول 4 بايت: `class_id`, `confidence_0_100`,
  `inference_ms_u16`.

كل رسالة تحتوي على version وlength وsequence وtimestamp وCRC-16. المستقبل يجب
أن يرفض الإطار التالف أو غير المدعوم، وألا يعامل فشل الاتصال كقراءة صفرية.
